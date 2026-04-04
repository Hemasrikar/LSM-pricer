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

// Both call and put prices must be >= 0
TEST_CASE("BS prices are non-negative", "[black_scholes]") {
    double r = 0.05, sigma = 0.2, T = 1.0;
    for (double S : {50.0, 100.0, 150.0}) {
        for (double K : {70.0, 100.0, 130.0}) {
            REQUIRE(bs_pricer::price_vanilla_option_european_bs(S, r, sigma, K, T, true)  >= 0.0);
            REQUIRE(bs_pricer::price_vanilla_option_european_bs(S, r, sigma, K, T, false) >= 0.0);
        }
    }
}

// Higher sigma => higher option price
TEST_CASE("BS prices increase with volatility", "[black_scholes]") {
    double S = 100.0, K = 100.0, r = 0.05, T = 1.0;
    double low  = bs_pricer::price_vanilla_option_european_bs(S, r, 0.1, K, T, true);
    double mid  = bs_pricer::price_vanilla_option_european_bs(S, r, 0.2, K, T, true);
    double high = bs_pricer::price_vanilla_option_european_bs(S, r, 0.4, K, T, true);
    REQUIRE(low < mid);
    REQUIRE(mid < high);

    low  = bs_pricer::price_vanilla_option_european_bs(S, r, 0.1, K, T, false);
    mid  = bs_pricer::price_vanilla_option_european_bs(S, r, 0.2, K, T, false);
    high = bs_pricer::price_vanilla_option_european_bs(S, r, 0.4, K, T, false);
    REQUIRE(low < mid);
    REQUIRE(mid < high);
}

// Call increases with S, put decreases with S
TEST_CASE("BS call increases and put decreases with spot price", "[black_scholes]") {
    double K = 100.0, r = 0.05, sigma = 0.2, T = 1.0;
    double call_low  = bs_pricer::price_vanilla_option_european_bs(80.0,  r, sigma, K, T, true);
    double call_mid  = bs_pricer::price_vanilla_option_european_bs(100.0, r, sigma, K, T, true);
    double call_high = bs_pricer::price_vanilla_option_european_bs(120.0, r, sigma, K, T, true);
    REQUIRE(call_low < call_mid);
    REQUIRE(call_mid < call_high);

    double put_low  = bs_pricer::price_vanilla_option_european_bs(80.0,  r, sigma, K, T, false);
    double put_mid  = bs_pricer::price_vanilla_option_european_bs(100.0, r, sigma, K, T, false);
    double put_high = bs_pricer::price_vanilla_option_european_bs(120.0, r, sigma, K, T, false);
    REQUIRE(put_low > put_mid);
    REQUIRE(put_mid > put_high);
}

// Price converges to intrinsic value as T -> 0
TEST_CASE("BS price converges to intrinsic value near expiry", "[black_scholes]") {
    const double r = 0.05, sigma = 0.2, T = 1e-5;

    // Deep ITM call (S >> K): price ~ S - K
    double itm_call = bs_pricer::price_vanilla_option_european_bs(150.0, r, sigma, 100.0, T, true);
    REQUIRE(itm_call == Approx(50.0).margin(0.01));

    // Deep OTM call (S << K): price ~ 0
    double otm_call = bs_pricer::price_vanilla_option_european_bs(50.0, r, sigma, 100.0, T, true);
    REQUIRE(otm_call == Approx(0.0).margin(0.01));

    // Deep ITM put (K >> S): price ~ K - S
    double itm_put = bs_pricer::price_vanilla_option_european_bs(50.0, r, sigma, 100.0, T, false);
    REQUIRE(itm_put == Approx(50.0).margin(0.01));
}

// Lower bound: call price (from ...)
TEST_CASE("BS call price satisfies lower bound", "[black_scholes]") {
    double r = 0.05, sigma = 0.2, T = 1.0;
    for (double S : {80.0, 100.0, 120.0}) {
        double K = 100.0;
        double call = bs_pricer::price_vanilla_option_european_bs(S, r, sigma, K, T, true);
        double lower_bound = std::max(S - K * std::exp(-r * T), 0.0);
        REQUIRE(call >= lower_bound - 1e-10);
    }
}
