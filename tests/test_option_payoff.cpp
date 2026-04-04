#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "option_payoff.hpp"
#include <cmath>

using namespace lsm::core;

// Put Payoff Test Case

TEST_CASE("Put_payoff returns correct payoff values", "[Put_payoff]") 
{
    Put_payoff put(100.0);

    REQUIRE(put.payoff(50.0) == Approx(50.0));
    REQUIRE(put.payoff(100.0) == Approx(0.0));
    REQUIRE(put.payoff(150.0) == Approx(0.0));
}

TEST_CASE("Put_payoff InTheMoney works correctly", "[Put_payoff]") 
{
    Put_payoff put(100.0);

    REQUIRE(put.InTheMoney(50.0) == true);
    REQUIRE(put.InTheMoney(99.9) == true);
    REQUIRE(put.InTheMoney(100.0) == false);
    REQUIRE(put.InTheMoney(150.0) == false);
}

// Call Payoff Test Case

TEST_CASE("Call_payoff returns correct payoff values", "[Call_payoff]") 
{
    Call_payoff call(100.0);

    REQUIRE(call.payoff(50.0) == Approx(0.0));
    REQUIRE(call.payoff(100.0) == Approx(0.0));
    REQUIRE(call.payoff(150.0) == Approx(50.0));
}

TEST_CASE("Call_payoff InTheMoney works correctly", "[Call_payoff]") 
{
    Call_payoff call(100.0);

    REQUIRE(call.InTheMoney(50.0) == false);
    REQUIRE(call.InTheMoney(100.0) == false);
    REQUIRE(call.InTheMoney(100.1) == true);
    REQUIRE(call.InTheMoney(150.0) == true);
}

TEST_CASE("OptionPayoff virtual destructor via base pointer", "[option_payoff]") {
    // Exercises the virtual ~OptionPayoff() path
    std::unique_ptr<OptionPayoff> put  = std::make_unique<Put_payoff>(100.0);
    std::unique_ptr<OptionPayoff> call = std::make_unique<Call_payoff>(100.0);
    CHECK(put->strike()  == 100.0);
    CHECK(call->strike() == 100.0);
    // both destroyed here
}
