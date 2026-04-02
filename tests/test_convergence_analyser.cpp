#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "convergence_analyser.hpp"


static lsm::analysis::ConvergenceAnalyser analyser(100.0, 0.05, 0.2, 100.0, 1.0, "gbm", "call", "lag", 3, 10000, 50);

using namespace lsm::analysis;

TEST_CASE("getBSPrice returns a positive price", "[convergence]") {
    REQUIRE(analyser.getBSPrice() > 0.0);
}

TEST_CASE("getFDPrice returns a positive price", "[convergence]") {
    REQUIRE(analyser.getFDPrice() > 0.0);
}

TEST_CASE("getLSMPrice returns a positive price", "[convergence]") {
    REQUIRE(analyser.getLSMPrice(24, 50, 3, 1000) > 0.0);
}

