#include "convergence_analyser.hpp"
#include "bs_pricer.hpp"
#include "finite_difference.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <cmath>


namespace lsm{
    namespace analysis{

        // initialise the class
        ConvergenceAnalyser::ConvergenceAnalyser(double s, double rate, double vol, double strike, double maturity)
            : S0(s), r(rate), sigma(vol), K(strike), T(maturity)
        {

        }

        // helper functions
        double ConvergenceAnalyser::getBSPrice() {
            return bs_pricer::price_vanilla_option_european_bs(S0, r, sigma, K, T, false);
        }

        double ConvergenceAnalyser::getFDPrice() {
            lsm::core::GeometricBrownianMotion gbm(r, sigma);
            lsm::core::Put_payoff payoff(K);
            return lsm::fd::FDPricer(gbm, payoff).price(S0, T);
        }

        double ConvergenceAnalyser::getLSMPrice(unsigned seed, int numExerciseDates, int order, int numPaths, bool isLag) {
            lsm::engine::LSMConfig config;
            config.rngSeed = seed;
            config.numExerciseDates = numExerciseDates;
            config.numPaths = numPaths;
            config.riskFreeRate = r;
            config.maturity = T;

            auto process = std::make_unique<lsm::core::GeometricBrownianMotion>(r, sigma);
            auto payoff  = std::make_unique<lsm::core::Put_payoff>(K);
            auto basis   = std::make_unique<lsm::core::BasisSet>();

            if (isLag)
                basis->makeLaguerreSet(order);
            else
                basis->makeMonomialSet(order);

            lsm::engine::LSMPricer myPricer(std::move(process), std::move(payoff), std::move(basis), config);
            return myPricer.price(S0).optionValue;
        }

    // Benchmark: compares BS European, FD American, LSM American across parameter combinations
    void ConvergenceAnalyser::runBenchmark() {
        const double origS0    = S0;
        const double origSigma = sigma;
        const double origT     = T;

        std::vector<double> S0s    = {36, 40, 44};
        std::vector<double> sigmas = {0.2, 0.4};
        std::vector<double> Ts     = {1.0, 2.0};

        std::cout << "\n--- Benchmark ---\n\n";
        std::cout << std::right
                  << std::setw(6)  << "S0"
                  << std::setw(7)  << "Sigma"
                  << std::setw(5)  << "T"
                  << std::setw(14) << "BS European"
                  << std::setw(14) << "FD American"
                  << std::setw(14) << "LSM American"
                  << std::setw(20) << "Early Ex. Premium" << "\n"
                  << std::string(80, '-') << "\n";

        std::ofstream out("csv_output/benchmark.csv");
        out << "S0,Sigma,T,BSPrice,FDPrice,LSMPrice,EarlyExPremium\n";

        for (double s : S0s) {
            for (double vol : sigmas) {
                for (double mat : Ts) {

                    this->S0    = s;
                    this->sigma = vol;
                    this->T     = mat;

                    // get the black scholes, fd and lsm prices
                    double bsPrice  = getBSPrice();
                    double fdPrice  = getFDPrice();
                    double lsmPrice = getLSMPrice(24, 50, 3, 100000, true);
// compute the premium you pay for having early exercise
                    double eeValue  = lsmPrice - bsPrice;

                    std::cout << std::right
                        << std::setw(6)  << s
                        << std::setw(7)  << vol
                        << std::setw(5)  << mat
                        << std::fixed << std::setprecision(4)
                        << std::setw(14) << bsPrice
                        << std::setw(14) << fdPrice
                        << std::setw(14) << lsmPrice
                        << std::setw(20) << eeValue
                        << std::defaultfloat << "\n";

                    out << s << "," << vol << "," << mat << ","
                        << bsPrice << "," << fdPrice << "," << lsmPrice << "," << eeValue << "\n";
                }
            }
        }

        out.close();
        std::cout << "\n  -> Written to: csv_output/benchmark.csv\n";

        S0    = origS0;
        sigma = origSigma;
        T     = origT;
    }

    void ConvergenceAnalyser::runPathConvergence(bool isLag, std::vector<int> pathCounts, int numExerciseDates, int order) {
        std::string filename = "csv_output/path_convergence.csv";
        double fdPrice = getFDPrice();

        // output the titles of the table to the terminal
        std::cout << "\n--- Path Convergence ---\n\n";
        std::cout << std::fixed << std::setprecision(4);
        std::cout << std::right
                  << std::setw(12) << "Paths"
                  << std::setw(13) << "LSM Price"
                  << std::setw(11) << "Error"
                  << std::setw(12) << "Time (ms)" << "\n"
                  << std::string(48, '-') << "\n";

        std::ofstream out(filename);
        out << "Paths,LSMPrice,FDPrice,Error,Time(ms)\n";

        for (int i : pathCounts){
            auto start = std::chrono::high_resolution_clock::now();
            double lsmPrice = getLSMPrice(24, numExerciseDates, order, i, isLag);
            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();

            double error = std::abs(fdPrice - lsmPrice);
            out << i << "," << lsmPrice << "," << fdPrice << "," << error << "," << ms << "\n";

            // output the values
            std::cout << std::setw(12) << i
                      << std::setw(13) << lsmPrice
                      << std::setw(11) << error
                      << std::setw(11) << std::setprecision(2) << ms << " ms"
                      << std::setprecision(4) << "\n";
        }

        out.close();
        std::cout << "\n  -> Written to: " << filename << "\n";
    }

    void ConvergenceAnalyser::runOrderConvergence(bool isLag, std::vector<int> orders, int numExerciseDates, int pathCount) {
        std::string basisLabel = isLag ? "laguerre" : "monomial";
        std::string basisDisp  = isLag ? "Laguerre" : "Monomial";
        std::string filename   = "csv_output/" + basisLabel + "_order_convergence.csv";
        double fdPrice = getFDPrice();

        // keep basis type in title since we run both basis types
        std::cout << "\n--- Order Convergence | " << basisDisp << " ---\n\n";
        std::cout << std::fixed << std::setprecision(4);
        std::cout << std::right
                  << std::setw(12) << "Order"
                  << std::setw(13) << "LSM Price"
                  << std::setw(11) << "Error"
                  << std::setw(12) << "Time (ms)" << "\n"
                  << std::string(48, '-') << "\n";

        std::ofstream out(filename);
        out << "Order,LSMPrice,FDPrice,Error,Time(ms)\n";

        for (int i : orders){
            auto start = std::chrono::high_resolution_clock::now();
            double lsmPrice = getLSMPrice(24, numExerciseDates, i, pathCount, isLag);
            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();

            double error = std::abs(fdPrice - lsmPrice);
            out << i << "," << lsmPrice << "," << fdPrice << "," << error << "," << ms << "\n";

            std::cout << std::setw(12) << i
                      << std::setw(13) << lsmPrice
                      << std::setw(11) << error
                      << std::setw(11) << std::setprecision(2) << ms << " ms"
                      << std::setprecision(4) << "\n";
        }

        out.close();
        std::cout << "\n  -> Written to: " << filename << "\n";
    }

    void ConvergenceAnalyser::runDatesConvergence(bool isLag, std::vector<int> exerciseDatesList, int order, int pathCount) {
        std::string filename = "csv_output/exercise_dates_convergence.csv";
        double fdPrice = getFDPrice();

        // titles
        std::cout << "\n--- Exercise Dates Convergence ---\n\n";
        std::cout << std::fixed << std::setprecision(4);
        std::cout << std::right
                  << std::setw(12) << "Dates"
                  << std::setw(13) << "LSM Price"
                  << std::setw(11) << "Error"
                  << std::setw(12) << "Time (ms)" << "\n"
                  << std::string(48, '-') << "\n";

        std::ofstream out(filename);
        out << "ExerciseDates,LSMPrice,FDPrice,Error,Time(ms)\n";

        for (int i : exerciseDatesList){
            auto start = std::chrono::high_resolution_clock::now();
            double lsmPrice = getLSMPrice(24, i, order, pathCount, isLag);
            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();

            double error = std::abs(fdPrice - lsmPrice);
            out << i << "," << lsmPrice << "," << fdPrice << "," << error << "," << ms << "\n";

            std::cout << std::setw(12) << i
                      << std::setw(13) << lsmPrice
                      << std::setw(11) << error
                      << std::setw(11) << std::setprecision(2) << ms << " ms"
                      << std::setprecision(4) << "\n";
        }

        out.close();
        std::cout << "\n  -> Written to: " << filename << "\n";
    }

    void ConvergenceAnalyser::runFDConvergence(std::vector<int> timeCounts, int stockSteps) {
        std::string filename = "csv_output/fd_convergence.csv";
        double bsPrice = getBSPrice();

        // titles
        std::cout << "\n--- FD Convergence ---\n\n";
        std::cout << std::fixed << std::setprecision(4);
        std::cout << std::right
                  << std::setw(12) << "Time Steps"
                  << std::setw(12) << "FD Price"
                  << std::setw(11) << "Error"
                  << std::setw(12) << "Time (ms)" << "\n"
                  << std::string(48, '-') << "\n";

        std::ofstream out(filename);
        out << "TimeSteps,BSPrice,FDPrice,Error,Time(ms)\n";

        // generate the stock price and the payoff with gbm
        lsm::core::GeometricBrownianMotion gbm(r, sigma);
        lsm::core::Put_payoff payoff(K);

        for (int timeSteps : timeCounts) {
            auto start = std::chrono::high_resolution_clock::now();
            double fdPrice = lsm::fd::FDPricer(gbm, payoff, stockSteps, timeSteps).price(S0, T);
            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();

            double error = std::abs(bsPrice - fdPrice);
            out << timeSteps << "," << bsPrice << "," << fdPrice << "," << error << "," << ms << "\n";

            std::cout << std::setw(12) << timeSteps
                      << std::setw(12) << fdPrice
                      << std::setw(11) << error
                      << std::setw(11) << std::setprecision(2) << ms << " ms"
                      << std::setprecision(4) << "\n";
        }

        out.close();
        std::cout << "\n  -> Written to: " << filename << "\n";
    }

    void ConvergenceAnalyser::runSeedStability(bool isLag) {
        std::vector<int> seeds = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        std::string filename   = "csv_output/seed_stability.csv";

        int numExerciseDates = 50;
        int pathCount        = 10000;
        int order            = 3;

        double fdPrice = getFDPrice();

        std::cout << "\n--- Seed Stability ---\n\n";
        std::cout << std::fixed << std::setprecision(4);
        std::cout << std::right
                  << std::setw(6)  << "Seed"
                  << std::setw(13) << "LSM Price"
                  << std::setw(11) << "Error"
                  << std::setw(12) << "Time (ms)" << "\n"
                  << std::string(42, '-') << "\n";

        std::ofstream out(filename);
        out << "Seed,LSMPrice,FDPrice,Error,Time(ms)\n";

        std::vector<double> prices;

        for (int seed : seeds) {
            auto start      = std::chrono::high_resolution_clock::now();
            double lsmPrice = getLSMPrice(seed, numExerciseDates, order, pathCount, isLag);
            auto end        = std::chrono::high_resolution_clock::now();
            double ms       = std::chrono::duration<double, std::milli>(end - start).count();

            double error = std::abs(fdPrice - lsmPrice);
            prices.push_back(lsmPrice);
            out << seed << "," << lsmPrice << "," << fdPrice << "," << error << "," << ms << "\n";

            std::cout << std::setw(6)  << seed
                      << std::setw(13) << lsmPrice
                      << std::setw(11) << error
                      << std::setw(11) << std::setprecision(2) << ms << " ms"
                      << std::setprecision(4) << "\n";
        }

        double mean = 0.0;
        for (double p : prices) mean += p;
        mean /= prices.size();

        double variance = 0.0;
        for (double p : prices) variance += (p - mean) * (p - mean);
        double stddev = std::sqrt(variance / prices.size());

        out << "Mean,"   << mean   << ",,,\n";
        out << "StdDev," << stddev << ",,,\n";
        out.close();

        std::cout << std::string(42, '-') << "\n"
                  << std::setw(6)  << "Mean"   << std::setw(13) << mean   << "\n"
                  << std::setw(6)  << "StdDev" << std::setw(13) << stddev << "\n"
                  << "\n  -> Written to: " << filename << "\n";
    }


    } //namespace analysis
} //namespace lsm
