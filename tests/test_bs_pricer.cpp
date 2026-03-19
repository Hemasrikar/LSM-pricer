#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "bs_pricer.hpp"


TEST_CASE("Black-Scholes: Option Pricing", "[black_scholes]")
{
    double S = 100.0;
    double K = 100.0;
    double T = 1.0;
    double r = 0.05;
    double sigma = 0.2;

    auto [call, put] = bs_pricer::price_vanilla_option_european_bs(S, r, sigma, K, T);

    REQUIRE(call == Approx(10.4506).epsilon(0.0001));

    REQUIRE(put == Approx(5.5735).epsilon(0.0001));
}





