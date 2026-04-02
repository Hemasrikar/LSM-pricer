#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "convergence_analyser.hpp"

static lsm::analysis::ConvergenceAnalyser analyser(100.0, 0.05, 0.2, 100.0, 1.0, "gbm", "call", "lag", 3, 10000, 50);



using namespace lsm::analysis;

TEST_CASE("getBSPrice is correct", "[convergence]") {
    REQUIRE(analyser.getBSPrice() == Approx(10.4506).epsilon(0.001));
}