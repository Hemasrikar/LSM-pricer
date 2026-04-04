#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "lsm_pricer.hpp"
#include "underlying_sde.hpp"
#include "option_payoff.hpp"
#include "basis_functions.hpp"
#include "finite_difference.hpp"
#include <memory>
#include <cmath>

using namespace lsm::core;
using namespace lsm::engine;

//helper function to build a basis set

static BasisSet makeTestBasis() {
    BasisSet basis;
    basis.makeMonomialSet(2);
    return basis;
}

//Test 1: check if priceWithData() returns correctly shaped paths
TEST_CASE("LSMPricer priceWithData returns correctly shaped path data", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 100;
    config.useAntithetic = false;
    config.numExerciseDates = 10;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    GeometricBrownianMotion process(0.06, 0.2);
    Put_payoff payoff(40.0);
    auto basis = makeTestBasis();

    LSMPricer pricer(process, payoff, basis, config);

    const double S0 = 40.0;
    auto resultPair = pricer.priceWithData(S0);

    const SimulationResult& result = resultPair.first;
    const PathData& data = resultPair.second;

    REQUIRE(data.numPaths == 100);
    REQUIRE(data.numTimeSteps == 10);

    REQUIRE(data.paths.size() == 100);
    REQUIRE(data.cashFlows.size() == 100);

    for (int i = 0; i < data.numPaths; ++i) {
        REQUIRE(data.paths[i].size() == 11);
        REQUIRE(data.cashFlows[i].size() == 11);
        REQUIRE(data.paths[i][0] == Approx(S0));
    }

    REQUIRE(result.numPaths == 100);
    REQUIRE(result.numExerciseDates == 10);
}

//TEST 2: Check whether LSMPricer enforces even number of pahs when using antithetic 
//variation reduction

TEST_CASE("LSMPricer enforces even number of paths when using antithetic variates", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 101;              // odd number
    config.useAntithetic = true;
    config.numExerciseDates = 10;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    GeometricBrownianMotion process(0.06, 0.2);
    Put_payoff payoff(40.0);
    auto basis = makeTestBasis();

    LSMPricer pricer(process, payoff, basis, config);

    const double S0 = 40.0;
    auto resultPair = pricer.priceWithData(S0);
    const PathData& data = resultPair.second;

    // Should drop 1 path to make it even
    REQUIRE(data.numPaths == 100);
    REQUIRE(data.paths.size() == 100);
    REQUIRE(data.cashFlows.size() == 100);
}

//TEST 3: Check whether LSM returns finite and consistent results

TEST_CASE("LSMPricer returns finite and consistent results", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 2000;          // large to ensure stability
    config.useAntithetic = true;
    config.numExerciseDates = 20;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    GeometricBrownianMotion process(0.06, 0.2);
    Put_payoff payoff(40.0);
    auto basis = makeTestBasis();

    LSMPricer pricer(process, payoff, basis, config);

    const double S0 = 40.0;
    SimulationResult result = pricer.price(S0);

    // --- Finite values ---
    REQUIRE(std::isfinite(result.optionValue));
    REQUIRE(std::isfinite(result.standardError));
    REQUIRE(std::isfinite(result.europeanValue));
    REQUIRE(std::isfinite(result.earlyExercisePremium));

    // --- Basic sanity ---
    REQUIRE(result.optionValue >= 0.0);
    REQUIRE(result.standardError >= 0.0);

    // --- Internal consistency ---
    REQUIRE(result.earlyExercisePremium ==
            Approx(result.optionValue - result.europeanValue));

    // --- Metadata consistency ---
    REQUIRE(result.numPaths > 0);
    REQUIRE(result.numExerciseDates == config.numExerciseDates);
}

// ------------------------------------------------------------
// Financial intuition tests
// ------------------------------------------------------------

//TEST 4: American put has to be at least European value
TEST_CASE("American put value is at least European value", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 5000;
    config.useAntithetic = true;
    config.numExerciseDates = 50;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    GeometricBrownianMotion process(0.06, 0.2);
    Put_payoff payoff(40.0);
    auto basis = makeTestBasis();

    LSMPricer pricer(process, payoff, basis, config);

    const double S0 = 40.0;
    SimulationResult result = pricer.price(S0);

    REQUIRE(result.optionValue >= Approx(result.europeanValue).margin(1e-10));
}

//TEST 5: Put value decreases as spot increases, i.e., S1 < S2 -> P(S1) >= P(S2)
TEST_CASE("Put option value decreases as spot increases", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 5000;
    config.useAntithetic = true;
    config.numExerciseDates = 50;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    GeometricBrownianMotion process(0.06, 0.2);
    Put_payoff payoff(40.0);
    auto basis = makeTestBasis();

    LSMPricer pricer1(process, payoff, basis, config);
    LSMPricer pricer2(process, payoff, basis, config);
    LSMPricer pricer3(process, payoff, basis, config);

    double P_low  = pricer1.price(30.0).optionValue;
    double P_mid  = pricer2.price(40.0).optionValue;
    double P_high = pricer3.price(50.0).optionValue;

    REQUIRE(P_low >= P_mid);
    REQUIRE(P_mid >= P_high);
}

//TEST 6: LSM must be approximately Finite difference benchmark
//use margin of 0.5 since LSM is noisy since its Monte Carlo estimate
//whereas FD is deterministic (almost exact benchmark)

TEST_CASE("LSM price is close to finite difference benchmark", "[lsm_pricer]") {
    // --- Common parameters ---
    const double S0 = 40.0;
    const double K  = 40.0;
    const double r  = 0.06;
    const double sigma = 0.2;
    const double T  = 1.0;

    // --- LSM config ---
    LSMConfig config;
    config.numPaths = 10000;          // higher for stability
    config.useAntithetic = true;
    config.numExerciseDates = 50;
    config.maturity = T;
    config.riskFreeRate = r;
    config.rngSeed = 24;

    // --- LSM pricer ---
    GeometricBrownianMotion lsm_process(r, sigma);
    Put_payoff lsm_payoff(K);
    auto lsm_basis = makeTestBasis();
    LSMPricer lsm_pricer(lsm_process, lsm_payoff, lsm_basis, config);

    double lsm_price = lsm_pricer.price(S0).optionValue;

    // --- FD benchmark ---
    GeometricBrownianMotion gbm(r, sigma);
    Put_payoff put(K);
    lsm::fd::FDPricer fd_pricer(gbm, put);

    double fd_price = fd_pricer.price(S0, T);

    // --- Comparison ---
    REQUIRE(lsm_price == Approx(fd_price).margin(0.5));
}

//TEST 7: Put increases as strike increases
TEST_CASE("Put option value increases as strike increases", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 5000;
    config.useAntithetic = true;
    config.numExerciseDates = 50;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    GeometricBrownianMotion process(0.06, 0.2);
    Put_payoff payoff35(35.0), payoff40(40.0), payoff45(45.0);
    auto basis = makeTestBasis();

    LSMPricer pricer1(process, payoff35, basis, config);
    LSMPricer pricer2(process, payoff40, basis, config);
    LSMPricer pricer3(process, payoff45, basis, config);

    const double S0 = 40.0;

    double P_lowK  = pricer1.price(S0).optionValue;
    double P_midK  = pricer2.price(S0).optionValue;
    double P_highK = pricer3.price(S0).optionValue;

    REQUIRE(P_lowK <= P_midK);
    REQUIRE(P_midK <= P_highK);
}

//TEST 8: Put value increases as volatility increases
TEST_CASE("Put option value increases as volatility increases", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 5000;
    config.useAntithetic = true;
    config.numExerciseDates = 50;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    GeometricBrownianMotion proc15(0.06, 0.15), proc25(0.06, 0.25), proc35(0.06, 0.35);
    Put_payoff payoff(40.0);
    auto basis = makeTestBasis();

    LSMPricer pricer1(proc15, payoff, basis, config);
    LSMPricer pricer2(proc25, payoff, basis, config);
    LSMPricer pricer3(proc35, payoff, basis, config);

    const double S0 = 40.0;

    double P_lowVol  = pricer1.price(S0).optionValue;
    double P_midVol  = pricer2.price(S0).optionValue;
    double P_highVol = pricer3.price(S0).optionValue;

    REQUIRE(P_lowVol <= P_midVol);
    REQUIRE(P_midVol <= P_highVol);
}

//TEST 9: we require: DEEP ITM > ATM > OTM  - related to spot monotonicity
TEST_CASE("Put option value is highest deep in the money and lowest out of the money", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 5000;
    config.useAntithetic = true;
    config.numExerciseDates = 50;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    GeometricBrownianMotion process(0.06, 0.2);
    Put_payoff payoff(40.0);
    auto basis = makeTestBasis();

    LSMPricer pricer1(process, payoff, basis, config);
    LSMPricer pricer2(process, payoff, basis, config);
    LSMPricer pricer3(process, payoff, basis, config);

    double P_ITM = pricer1.price(30.0).optionValue;
    double P_ATM = pricer2.price(40.0).optionValue;
    double P_OTM = pricer3.price(50.0).optionValue;

    REQUIRE(P_ITM >= P_ATM);
    REQUIRE(P_ATM >= P_OTM);
}

//TEST: simulatePaths rejects non-positive numExerciseDates and numPaths
TEST_CASE("LSMPricer rejects invalid config in simulatePaths", "[lsm_pricer]") {
    GeometricBrownianMotion process(0.06, 0.2);
    Put_payoff payoff(40.0);
    auto basis = makeTestBasis();

    LSMConfig config;
    config.numPaths = 100;
    config.useAntithetic = false;
    config.numExerciseDates = 10;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    LSMConfig bad = config;
    bad.numExerciseDates = 0;
    LSMPricer pricer1(process, payoff, basis, bad);
    REQUIRE_THROWS_AS(pricer1.price(40.0), std::invalid_argument);

    bad = config;
    bad.numPaths = 0;
    LSMPricer pricer2(process, payoff, basis, bad);
    REQUIRE_THROWS_AS(pricer2.price(40.0), std::invalid_argument);
}

//TEST 10: Imediate exercise lower bound: V^AM(S0) >= (K - S0)+
TEST_CASE("American put value is at least immediate exercise value", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 5000;
    config.useAntithetic = true;
    config.numExerciseDates = 50;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    const double S0 = 35.0;
    const double K = 40.0;

    GeometricBrownianMotion process(0.06, 0.2);
    Put_payoff payoff(K);
    auto basis = makeTestBasis();
    LSMPricer pricer(process, payoff, basis, config);

    double price = pricer.price(S0).optionValue;
    double intrinsic = std::max(K - S0, 0.0);

    REQUIRE(price >= intrinsic);
}