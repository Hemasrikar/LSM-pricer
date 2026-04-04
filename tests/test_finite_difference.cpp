#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "basis_functions.hpp"
#include "ols_regression.hpp"
#include "mc_paths.hpp"
#include "option_payoff.hpp"
#include "finite_difference.hpp"

using namespace lsm::core;
using namespace lsm::engine;

TEST_CASE("0 == 0", "[tests]")
{
	REQUIRE(0 == 0);
}

TEST_CASE("1 + 1 == 2", "[tests]")
{
	REQUIRE(1 + 1 == 2);
}


// FD values from the paper (table 1, p.127): implicit scheme, K=40, r=0.06,
// 40,000 time steps per year, 1,000 stock steps as stated
TEST_CASE("FD: American put matches paper Table 1 (T=1, sigma=0.20)", "[fd]")
{
lsm::core::GeometricBrownianMotion gbm(0.06, 0.20);
lsm::core::Put_payoff put(40.0);
lsm::fd::FDPricer pricer(gbm, put);

REQUIRE(pricer.price(36.0, 1.0) == Approx(4.478).epsilon(0.01));
REQUIRE(pricer.price(40.0, 1.0) == Approx(2.314).epsilon(0.01));
REQUIRE(pricer.price(44.0, 1.0) == Approx(1.110).epsilon(0.01));
}

TEST_CASE("FD: American put matches paper Table 1 (T=1, sigma=0.40)", "[fd]")
{
lsm::core::GeometricBrownianMotion gbm(0.06, 0.40);
lsm::core::Put_payoff put(40.0);
lsm::fd::FDPricer pricer(gbm, put);

REQUIRE(pricer.price(36.0, 1.0) == Approx(7.101).epsilon(0.01));
}

TEST_CASE("FD: American put matches paper Table 1 (T=2, sigma=0.20)", "[fd]")
{
lsm::core::GeometricBrownianMotion gbm(0.06, 0.20);
lsm::core::Put_payoff put(40.0);
lsm::fd::FDPricer pricer(gbm, put);

REQUIRE(pricer.price(36.0, 2.0) == Approx(4.840).epsilon(0.01));
REQUIRE(pricer.price(40.0, 2.0) == Approx(2.885).epsilon(0.01));
}
