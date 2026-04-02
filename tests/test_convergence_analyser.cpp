#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "convergence_analyser.hpp"

// constructor: S0=36, r=0.06, sigma=0.20, K=40, T=1 (Longstaff-Schwartz put parameters)
static lsm::analysis::ConvergenceAnalyser analyser(36.0, 0.06, 0.2, 40.0, 1.0);

using namespace lsm::analysis;

TEST_CASE("getBSPrice returns a positive price", "[convergence]") {
    REQUIRE(analyser.getBSPrice() > 0.0);
}

TEST_CASE("getFDPrice returns a positive price", "[convergence]") {
    REQUIRE(analyser.getFDPrice() > 0.0);
}

TEST_CASE("getLSMPrice returns a positive price", "[convergence]") {
    REQUIRE(analyser.getLSMPrice(24, 50, 3, 1000, true) > 0.0);
}

TEST_CASE("FD price is close to BS price for put", "[convergence]") {
    double bs = analyser.getBSPrice();
    double fd = analyser.getFDPrice();
    REQUIRE(std::abs(bs - fd) < 0.1);
}
