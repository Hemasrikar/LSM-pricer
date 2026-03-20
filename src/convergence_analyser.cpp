#include "convergence_analyser.hpp"
#include "bs_pricer.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

namespace lsm{
    namespace analysis{

        // defined functions to get the prices from each of the different calculation methodologies
        double getBSPrice(double S0, double r, double sigma, double K, double T, bool call) {
            return bs_pricer::price_vanilla_option_european_bs(S0, r, sigma, K, T, call);
        }

        double getFDPrice() {
            // insert logic to get the fd prices
            return 10.5; //placeholder
        }

        double getLSMPrice() {
            // engine::LSMConfig config;
            // config.rngSeed = seed;
            // insert logic for the lsm prices
            return 10.5; //placeholder
        }

    // Code to run a one off check against BS
    void runBenchmark() {
        // set up the parameters for the benchmark test
            double S0 = 100.0;
            double r = 0.05;
            double sigma = 0.2;
            double K = 100.0;
            double T = 1.0;
            bool call = true;

            // get the black scholes, fd and lsm prices
            double bsPrice = getBSPrice(S0, r, sigma, K, T, call);
            double fdPrice = getFDPrice();
            double lsmPrice = getLSMPrice();

            // compute the premium you pay for having early exercise
            double eeValue = std::abs(bsPrice - fdPrice); 

            std::cout << "--- BENCHMARK REPORT ---" << std::endl;
            std::cout << std::left << std::setw(15) << "BS European"
                << std::setw(15) << "FD American"
                << std::setw(25) << "Early Exercise Value"
                << std::setw(15) << "LSM American" << std::endl;

            std::cout << std::left << std::setw(15) << bsPrice
                << std::setw(15) << fdPrice
                << std::setw(25) << eeValue
                << std::setw(15) << lsmPrice << std::endl;

    }

    //     // Code to run the convergence
    //     void runConvergence() {

    //         // set up the parameters for the benchmark test
    //         double S0 = 100.0;
    //         double r = 0.05;
    //         double sigma = 0.2;
    //         double K = 100.0;
    //         double T = 1.0;
    //         bool call = true;

    //         // get the true price from the black scholes model
    //         double truePrice = getBSPrice(S0, r, sigma, K, T, call);

    //         // define seeds to test
    //         std::vector<unsigned> seeds = {24, 42, 123, 777, 2026};

    //         // iterate 5 times to get 5 different prices
    //         for (unsigned seed : seeds) {
    //             double lsmPrice = getLSMPrice(seed);

    //             double error = std::abs(truePrice - lsmPrice);
    //             std::cout << "Error: " << error << std::endl;

    //         }
    // }


    } //namespace analysis
} //namespace lsm