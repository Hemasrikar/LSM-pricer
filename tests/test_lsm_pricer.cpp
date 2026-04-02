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

static std::unique_ptr<BasisSet> makeTestBasis() {
    auto basis = std::make_unique<BasisSet>();
    basis->makeMonomialSet(2);
    return basis;
}


//TEST 1: constructor throws if process is null.

TEST_CASE("LSMPricer constructor throws if process is null", "[lsm_pricer]") {
    LSMConfig config;

    std::unique_ptr<const StochasticProcess> process = nullptr;
    auto payoff = std::make_unique<Put_payoff>(40.0);
    auto basis = makeTestBasis();

    REQUIRE_THROWS_AS(
        LSMPricer(std::move(process), std::move(payoff), std::move(basis), config),
        std::invalid_argument
    );
}

//TEST 2: Constructor throws if payoff is null
TEST_CASE("LSMPricer constructor throws if payoff is null", "[lsm_pricer]") {
    LSMConfig config;

    auto process = std::make_unique<GeometricBrownianMotion>(0.06, 0.2);
    std::unique_ptr<const OptionPayoff> payoff = nullptr;
    auto basis = makeTestBasis();

    REQUIRE_THROWS_AS(
        LSMPricer(std::move(process), std::move(payoff), std::move(basis), config),
        std::invalid_argument
    );
}

//TEST 3: constructor throws if basis is null
TEST_CASE("LSMPricer constructor throws if basis is null", "[lsm_pricer]") {
    LSMConfig config;

    auto process = std::make_unique<GeometricBrownianMotion>(0.06, 0.2);
    auto payoff = std::make_unique<Put_payoff>(40.0);
    std::unique_ptr<BasisSet> basis = nullptr;

    REQUIRE_THROWS_AS(
        LSMPricer(std::move(process), std::move(payoff), std::move(basis), config),
        std::invalid_argument
    );
}

//Test 4: check if priceWithData() returns correctly shaped paths
TEST_CASE("LSMPricer priceWithData returns correctly shaped path data", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 100;
    config.useAntithetic = false;
    config.numExerciseDates = 10;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    auto process = std::make_unique<GeometricBrownianMotion>(0.06, 0.2);
    auto payoff = std::make_unique<Put_payoff>(40.0);
    auto basis = makeTestBasis();

    LSMPricer pricer(std::move(process), std::move(payoff), std::move(basis), config);

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

//TEST 5: Check whether LSMPricer enforces even number of pahs when using antithetic 
//variation reduction

TEST_CASE("LSMPricer enforces even number of paths when using antithetic variates", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 101;              // odd number
    config.useAntithetic = true;
    config.numExerciseDates = 10;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    auto process = std::make_unique<GeometricBrownianMotion>(0.06, 0.2);
    auto payoff = std::make_unique<Put_payoff>(40.0);
    auto basis = makeTestBasis();

    LSMPricer pricer(std::move(process), std::move(payoff), std::move(basis), config);

    const double S0 = 40.0;
    auto resultPair = pricer.priceWithData(S0);
    const PathData& data = resultPair.second;

    // Should drop 1 path to make it even
    REQUIRE(data.numPaths == 100);
    REQUIRE(data.paths.size() == 100);
    REQUIRE(data.cashFlows.size() == 100);
}

//TEST 6: Check whether LSM returns finite and consistent results

TEST_CASE("LSMPricer returns finite and consistent results", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 2000;          // large to ensure stability
    config.useAntithetic = true;
    config.numExerciseDates = 20;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    auto process = std::make_unique<GeometricBrownianMotion>(0.06, 0.2);
    auto payoff = std::make_unique<Put_payoff>(40.0);
    auto basis = makeTestBasis();

    LSMPricer pricer(std::move(process), std::move(payoff), std::move(basis), config);

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

//TEST 7: American put has to be at least European value
TEST_CASE("American put value is at least European value", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 5000;
    config.useAntithetic = true;
    config.numExerciseDates = 50;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    auto process = std::make_unique<GeometricBrownianMotion>(0.06, 0.2);
    auto payoff = std::make_unique<Put_payoff>(40.0);
    auto basis = makeTestBasis();

    LSMPricer pricer(std::move(process), std::move(payoff), std::move(basis), config);

    const double S0 = 40.0;
    SimulationResult result = pricer.price(S0);

    REQUIRE(result.optionValue >= Approx(result.europeanValue).margin(1e-10));
}

//TEST 8: Put value decreases as spot increases, i.e., S1 < S2 -> P(S1) >= P(S2)
TEST_CASE("Put option value decreases as spot increases", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 5000;
    config.useAntithetic = true;
    config.numExerciseDates = 50;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    auto process = std::make_unique<GeometricBrownianMotion>(0.06, 0.2);
    auto payoff = std::make_unique<Put_payoff>(40.0);
    auto basis = makeTestBasis();

    LSMPricer pricer1(
        std::make_unique<GeometricBrownianMotion>(0.06, 0.2),
        std::make_unique<Put_payoff>(40.0),
        makeTestBasis(),
        config
    );

    LSMPricer pricer2(
        std::make_unique<GeometricBrownianMotion>(0.06, 0.2),
        std::make_unique<Put_payoff>(40.0),
        makeTestBasis(),
        config
    );

    LSMPricer pricer3(
        std::make_unique<GeometricBrownianMotion>(0.06, 0.2),
        std::make_unique<Put_payoff>(40.0),
        makeTestBasis(),
        config
    );

    double P_low  = pricer1.price(30.0).optionValue;
    double P_mid  = pricer2.price(40.0).optionValue;
    double P_high = pricer3.price(50.0).optionValue;

    REQUIRE(P_low >= P_mid);
    REQUIRE(P_mid >= P_high);
}

//TEST 9: LSM must be approximately Finite difference benchmark
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
    LSMPricer lsm_pricer(
        std::make_unique<GeometricBrownianMotion>(r, sigma),
        std::make_unique<Put_payoff>(K),
        makeTestBasis(),
        config
    );

    double lsm_price = lsm_pricer.price(S0).optionValue;

    // --- FD benchmark ---
    GeometricBrownianMotion gbm(r, sigma);
    Put_payoff put(K);
    lsm::fd::FDPricer fd_pricer(gbm, put);

    double fd_price = fd_pricer.price(S0, T);

    // --- Comparison ---
    REQUIRE(lsm_price == Approx(fd_price).margin(0.5));
}

//TEST 10: Put increases as strike increases
TEST_CASE("Put option value increases as strike increases", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 5000;
    config.useAntithetic = true;
    config.numExerciseDates = 50;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    LSMPricer pricer1(
        std::make_unique<GeometricBrownianMotion>(0.06, 0.2),
        std::make_unique<Put_payoff>(35.0),
        makeTestBasis(),
        config
    );

    LSMPricer pricer2(
        std::make_unique<GeometricBrownianMotion>(0.06, 0.2),
        std::make_unique<Put_payoff>(40.0),
        makeTestBasis(),
        config
    );

    LSMPricer pricer3(
        std::make_unique<GeometricBrownianMotion>(0.06, 0.2),
        std::make_unique<Put_payoff>(45.0),
        makeTestBasis(),
        config
    );

    const double S0 = 40.0;

    double P_lowK  = pricer1.price(S0).optionValue;
    double P_midK  = pricer2.price(S0).optionValue;
    double P_highK = pricer3.price(S0).optionValue;

    REQUIRE(P_lowK <= P_midK);
    REQUIRE(P_midK <= P_highK);
}

//TEST 11: Put value increases as volatility increases
TEST_CASE("Put option value increases as volatility increases", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 5000;
    config.useAntithetic = true;
    config.numExerciseDates = 50;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    LSMPricer pricer1(
        std::make_unique<GeometricBrownianMotion>(0.06, 0.15),
        std::make_unique<Put_payoff>(40.0),
        makeTestBasis(),
        config
    );

    LSMPricer pricer2(
        std::make_unique<GeometricBrownianMotion>(0.06, 0.25),
        std::make_unique<Put_payoff>(40.0),
        makeTestBasis(),
        config
    );

    LSMPricer pricer3(
        std::make_unique<GeometricBrownianMotion>(0.06, 0.35),
        std::make_unique<Put_payoff>(40.0),
        makeTestBasis(),
        config
    );

    const double S0 = 40.0;

    double P_lowVol  = pricer1.price(S0).optionValue;
    double P_midVol  = pricer2.price(S0).optionValue;
    double P_highVol = pricer3.price(S0).optionValue;

    REQUIRE(P_lowVol <= P_midVol);
    REQUIRE(P_midVol <= P_highVol);
}

//TEST 12: we require: DEEP ITM > ATM > OTM  - related to spot monotonicity
TEST_CASE("Put option value is highest deep in the money and lowest out of the money", "[lsm_pricer]") {
    LSMConfig config;
    config.numPaths = 5000;
    config.useAntithetic = true;
    config.numExerciseDates = 50;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;

    LSMPricer pricer1(
        std::make_unique<GeometricBrownianMotion>(0.06, 0.2),
        std::make_unique<Put_payoff>(40.0),
        makeTestBasis(),
        config
    );

    LSMPricer pricer2(
        std::make_unique<GeometricBrownianMotion>(0.06, 0.2),
        std::make_unique<Put_payoff>(40.0),
        makeTestBasis(),
        config
    );

    LSMPricer pricer3(
        std::make_unique<GeometricBrownianMotion>(0.06, 0.2),
        std::make_unique<Put_payoff>(40.0),
        makeTestBasis(),
        config
    );

    double P_ITM = pricer1.price(30.0).optionValue;
    double P_ATM = pricer2.price(40.0).optionValue;
    double P_OTM = pricer3.price(50.0).optionValue;

    REQUIRE(P_ITM >= P_ATM);
    REQUIRE(P_ATM >= P_OTM);
}

//TEST 13: Imediate exercise lower bound: V^AM(S0) >= (K - S0)+
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

    LSMPricer pricer(
        std::make_unique<GeometricBrownianMotion>(0.06, 0.2),
        std::make_unique<Put_payoff>(K),
        makeTestBasis(),
        config
    );

    double price = pricer.price(S0).optionValue;
    double intrinsic = std::max(K - S0, 0.0);

    REQUIRE(price >= intrinsic);
}