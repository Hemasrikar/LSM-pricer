#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "convergence_analyser.hpp"

static lsm::core::GeometricBrownianMotion gbm(0.05, 0.2);
static lsm::core::Call_payoff call(100.0);
static lsm::core::BasisSet basis = []() {
    lsm::core::BasisSet b;
    b.makeLaguerreSet(3);
    return b;
}();
static std::function<void(lsm::core::BasisSet&, int)> factory = [](lsm::core::BasisSet& b, int order) {
    b.makeLaguerreSet(order);
};
static lsm::analysis::ConvergenceAnalyser analyser(100.0, 0.05, 0.2, 100.0, 1.0, gbm, call, basis, factory, 3, 10000, 50);



using namespace lsm::analysis;

TEST_CASE("getBSPrice is correct", "[convergence]") {
    REQUIRE(analyser.getBSPrice() == Approx(10.4506).epsilon(0.001));
}