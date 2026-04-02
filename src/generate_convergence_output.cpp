#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <memory>
#include <chrono>
#include <cmath>
#include "bs_pricer.hpp"
#include "finite_difference.hpp"
#include "lsm_pricer.hpp"
#include "option_payoff.hpp"
#include "underlying_sde.hpp"
#include "basis_functions.hpp"

// -- Shared option / market parameters
const double S0 = 36.0;
const double K = 40.0;
const double r = 0.06;
const double sigma = 0.20;
const double T = 1.0;
const bool isCall = false; // false = put, true = call

// -- FD convergence settings
const int fdStockSteps = 1000; // fixed stock steps
const std::vector<int> fdTimeCounts = {10, 50, 100, 500, 1000, 5000, 10000, 40000};
const std::string fdOutputFile = "csv_output/fd_convergence.csv";

// -- LSM convergence settings
const int numExerciseDates = 50;
const int basisOrder = 3;
const bool useLaguerre = true;
const int seed = 30;
const std::vector<int> pathCounts = []() {
    std::vector<int> v;
    for (int i = 6; i <= 17; ++i) // Change the value of i to adjust the range of number of paths (from 2^6 to 2^17 )
        v.push_back(static_cast<int>(std::pow(2.0, i)));
    return v;
}();
const std::string lsmOutputFile = "csv_output/lsm_convergence.csv";



int main() {
    double bsPrice = bs_pricer::price_vanilla_option_european_bs(S0, r, sigma, K, T, isCall);
    lsm::core::GeometricBrownianMotion gbm(r, sigma);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "BS European: " << bsPrice << "\n\n";

    // -- FD convergence table -----------------------------------
    std::cout << "FD Convergence (stock steps = " << fdStockSteps << ")\n";
    std::cout << std::setw(14) << "Time Steps"
              << std::setw(12) << "FD Price"
              << std::setw(10) << "Error"
              << std::setw(12) << "Time (ms)" << "\n"
              << std::string(48, '-') << "\n";

    std::ofstream fdOut(fdOutputFile);
    fdOut << "TimeSteps,BSPrice,FDPrice,Error,Time(ms)\n";

    double fdRefPrice = 0.0; // will use the finest grid run as reference
    for (int timeSteps : fdTimeCounts) {
        auto start = std::chrono::high_resolution_clock::now();
        double fdPrice;
        if (isCall) {
            lsm::core::Call_payoff payoff(K);
            fdPrice = lsm::fd::FDPricer(gbm, payoff, fdStockSteps, timeSteps).price(S0, T);
        } else {
            lsm::core::Put_payoff payoff(K);
            fdPrice = lsm::fd::FDPricer(gbm, payoff, fdStockSteps, timeSteps).price(S0, T);
        }
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();

        fdRefPrice = fdPrice;
        double error = std::abs(bsPrice - fdPrice);

        std::cout << std::setw(14) << timeSteps
                  << std::setw(12) << fdPrice
                  << std::setw(10) << error
                  << std::setw(12) << std::setprecision(2) << ms << " ms"
                  << std::setprecision(4) << "\n";

        fdOut << timeSteps << "," << bsPrice << "," << fdPrice << "," << error << "," << ms << "\n";
    }
    fdOut.close();
    std::cout << "\n-> Written to: " << fdOutputFile << "\n\n";

    // -- LSM convergence table ----------------------------------
    std::cout << "LSM Convergence (reference = FD at " << fdTimeCounts.back() << " time steps)\n";
    std::cout << std::setw(12) << "Paths"
              << std::setw(12) << "LSM Price"
              << std::setw(10) << "Error"
              << std::setw(12) << "Time (ms)" << "\n"
              << std::string(46, '-') << "\n";

    std::ofstream lsmOut(lsmOutputFile);
    lsmOut << "Paths,BSPrice,FDPrice,LSMPrice,Error,Time(ms)\n";

    for (int numPaths : pathCounts) {
        lsm::engine::LSMConfig config;
        config.rngSeed          = seed;
        config.numExerciseDates = numExerciseDates;
        config.numPaths         = numPaths;
        config.riskFreeRate     = r;
        config.maturity         = T;

        auto process = std::make_unique<lsm::core::GeometricBrownianMotion>(r, sigma);
        auto basis   = std::make_unique<lsm::core::BasisSet>();
        std::unique_ptr<lsm::core::OptionPayoff> payoff;

        if (isCall)
            payoff = std::make_unique<lsm::core::Call_payoff>(K);
        else
            payoff = std::make_unique<lsm::core::Put_payoff>(K);

        if (useLaguerre)
            basis->makeLaguerreSet(basisOrder);
        else
            basis->makeMonomialSet(basisOrder);

        lsm::engine::LSMPricer pricer(std::move(process), std::move(payoff), std::move(basis), config);

        auto start = std::chrono::high_resolution_clock::now();
        lsm::engine::SimulationResult result = pricer.price(S0);
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();

        double error = std::abs(fdRefPrice - result.optionValue);

        std::cout << std::setw(12) << numPaths
                  << std::setw(12) << result.optionValue
                  << std::setw(10) << error
                  << std::setw(12) << std::setprecision(2) << ms << " ms"
                  << std::setprecision(4) << "\n";

        lsmOut << numPaths << "," << bsPrice << "," << fdRefPrice << ","
               << result.optionValue << "," << error << "," << ms << "\n";
    }

    lsmOut.close();
    std::cout << "\n-> Written to: " << lsmOutputFile << "\n";

    return 0;
}
