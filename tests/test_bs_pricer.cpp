#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <cmath>
#include "bs_pricer.hpp"

// Test for known result
TEST_CASE("Black-Scholes: Option Pricing", "[black_scholes]")
{
    double S = 100.0;
    double K = 100.0;
    double T = 1.0;
    double r = 0.05;
    double sigma = 0.2;

    double call_price = bs_pricer::price_vanilla_option_european_bs(S, r, sigma, K, T, true);
    double put_price = bs_pricer::price_vanilla_option_european_bs(S, r, sigma, K, T, false);


    REQUIRE(call_price == Approx(10.4506).epsilon(0.0001));

    REQUIRE(put_price == Approx(5.5735).epsilon(0.0001));
}

// Put-Call Parity Test
TEST_CASE("BS call and put satisfy put-call parity", "[black_scholes]") {
    double S = 100.0, K = 100.0, r = 0.05, sigma = 0.2, T = 1.0;
    double call_price = bs_pricer::price_vanilla_option_european_bs(S, r, sigma, K, T, true);
    double put_price  = bs_pricer::price_vanilla_option_european_bs(S, r, sigma, K, T, false);
    double parity = S - K * std::exp(-r * T);
    REQUIRE(std::abs((call_price - put_price) - parity) < 0.01);
}

