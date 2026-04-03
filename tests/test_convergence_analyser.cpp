#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <memory>
#include "underlying_sde.hpp"
#include "convergence_analyser.hpp"

static std::function<std::unique_ptr<lsm::core::StochasticProcess>(double, double)> sdeFactory =
    [](double r, double vol) {
        return std::make_unique<lsm::core::GeometricBrownianMotion>(r, vol);
    };
static lsm::core::Call_payoff call(100.0);
static lsm::core::BasisSet basis = []() {
    lsm::core::BasisSet b;
    b.makeLaguerreSet(3);
    return b;
}();
static std::function<void(lsm::core::BasisSet&, int)> basisFactory = [](lsm::core::BasisSet& b, int order) {
    b.makeLaguerreSet(order);
};
static lsm::analysis::ConvergenceAnalyser analyser(100.0, 0.05, 0.2, 100.0, 1.0, sdeFactory, call, basis, basisFactory, 3, 10000, 50);

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

