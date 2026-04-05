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

static LSMConfig makeDefaultConfig() {
    LSMConfig config;
    config.numPaths = 5000;
    config.useAntithetic = true;
    config.numExerciseDates = 50;
    config.maturity = 1.0;
    config.riskFreeRate = 0.06;
    config.rngSeed = 24;
    return config;
}

//Test 1: check if priceWithData() returns correctly shaped paths
TEST_CASE("LSMPricer priceWithData returns correctly shaped path data", "[lsm_pricer]") {
    
    LSMConfig config = makeDefaultConfig();
    config.useAntithetic = false;
    config.numPaths = 100;
    config.numExerciseDates = 10;

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
    LSMConfig config = makeDefaultConfig();
    config.numPaths = 101;   
    config.numExerciseDates = 10;
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
    LSMConfig config = makeDefaultConfig();
    config.numPaths = 2000; //ensures stability
    config.numExerciseDates = 20; 
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
    LSMConfig config = makeDefaultConfig();
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

    LSMConfig config = makeDefaultConfig();

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
    LSMConfig config = makeDefaultConfig();
    config.numPaths = 10000; 
    config.maturity = T;

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

    LSMConfig config = makeDefaultConfig();
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
    LSMConfig config = makeDefaultConfig();
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
    LSMConfig config = makeDefaultConfig();
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

//TEST 10: Imediate exercise lower bound: V^AM(S0) >= (K - S0)+
TEST_CASE("American put value is at least immediate exercise value", "[lsm_pricer]") {
    LSMConfig config = makeDefaultConfig();
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

//TEST CASE 11: test to check toy example. The price and path values are checked.
TEST_CASE("LSM reproduces Longstaff-Schwartz 8-path toy example", "[lsm_pricer][toy_example]") {
    LSMConfig config = makeDefaultConfig();
    config.numExerciseDates = 3;
    config.numPaths = 8;
    config.useAntithetic = false;
    config.maturity = 3.0;
    config.riskFreeRate = 0.06;
    Put_payoff payoff(1.10);

    BasisSet basis;
    basis.makeMonomialSet(2); // constant, x, x^2

    GeometricBrownianMotion process(0.06, 0.2); 
   // process is unused once the test paths are supplied directly
    LSMPricer pricer(process, payoff, basis, config);

    PathData data;
    data.numPaths = 8;
    data.numTimeSteps = 3;
    data.paths = {
        {1.00, 1.09, 1.08, 1.34},
        {1.00, 1.16, 1.26, 1.54},
        {1.00, 1.22, 1.07, 1.03},
        {1.00, 0.93, 0.97, 0.92},
        {1.00, 1.11, 1.56, 1.52},
        {1.00, 0.76, 0.77, 0.90},
        {1.00, 0.92, 0.84, 1.01},
        {1.00, 0.88, 1.22, 1.34}
    };
    data.cashFlows = std::vector<std::vector<double>>(8, std::vector<double>(4, 0.0));

    std::vector<double> pv = pricer.runBackwardInductionForTest(data);

    double price = 0.0;
    for (double x : pv) price += x;
    price /= pv.size();
    //the paper rounds this to 4 s.f. so we choose a margin of 1e-4
    REQUIRE(price == Approx(0.1144).margin(1e-4));

    // check final cashflow matrix implied by the paper
    REQUIRE(data.cashFlows[0][1] == Approx(0.0));
    REQUIRE(data.cashFlows[1][1] == Approx(0.0));
    REQUIRE(data.cashFlows[2][3] == Approx(0.07).margin(1e-8));
    REQUIRE(data.cashFlows[3][1] == Approx(0.17).margin(1e-8));
    REQUIRE(data.cashFlows[4][1] == Approx(0.0));
    REQUIRE(data.cashFlows[5][1] == Approx(0.34).margin(1e-8));
    REQUIRE(data.cashFlows[6][1] == Approx(0.18).margin(1e-8));
    REQUIRE(data.cashFlows[7][1] == Approx(0.22).margin(1e-8));
}