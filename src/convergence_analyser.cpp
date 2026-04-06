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
#include <sstream>


namespace lsm{
    namespace analysis{

        // initialise the class
        ConvergenceAnalyser::ConvergenceAnalyser(double s, double rate, double vol, double strike, double maturity,
            std::function<std::unique_ptr<lsm::core::StochasticProcess>(double, double)> process,
            const lsm::core::OptionPayoff& payoff,
            const lsm::core::BasisSet& basis,
            std::function<void(lsm::core::BasisSet&, int)> factory,
            int order, int pathCount, int numDates)
            : S0(s), r(rate), sigma(vol), K(strike), T(maturity), sdeFactory(std::move(process)), payoffType(payoff), basisType(basis), basisFactory(std::move(factory)), order(order), fixedPathCount(pathCount), fixedNumDates(numDates)
        {

        }

        // creat the string which will be appended to the end of the file name
        std::string ConvergenceAnalyser::paramString() const {
            std::ostringstream oss;
            oss << "S" << S0
                << "_K" << K
                << "_r" << r
                << "_sig" << sigma
                << "_T" << T
                << "_" << payoffType.name()
                << "_" << basisType.basisPtrs()[1]->name()
                << "_ord" << order
                << "_dates" << fixedNumDates
                << "_paths" << fixedPathCount;
            return oss.str();
        }


        // helper functions
        double ConvergenceAnalyser::getBSPrice() {
            
            // change to payoffType.name()
            return bs_pricer::price_vanilla_option_european_bs(S0, r, sigma, K, T, payoffType.InTheMoney(K + 1.0));
        }

        double ConvergenceAnalyser::getFDPrice() {
            lsm::core::GeometricBrownianMotion gbm(r, sigma);
            return lsm::fd::FDPricer(gbm, payoffType).price(S0, T);
        }

        double ConvergenceAnalyser::getLSMPrice(unsigned seed, int numExerciseDates, int order, int numPaths) {
            lsm::engine::LSMConfig config;
            config.rngSeed = seed;
            config.numExerciseDates = numExerciseDates;
            config.numPaths = numPaths;
            config.riskFreeRate = r;
            config.maturity = T;

            lsm::core::BasisSet basis;
            basisFactory(basis, order);

            auto process = sdeFactory(r, sigma);
            lsm::engine::LSMPricer myPricer(*process, payoffType, basis, config);
            return myPricer.price(S0).optionValue;
        }

        lsm::engine::SimulationResult ConvergenceAnalyser::getLSMResult(unsigned seed, int numExerciseDates, int order, int numPaths, bool antithetic) {
            lsm::engine::LSMConfig config;
            config.rngSeed = seed;
            config.numExerciseDates = numExerciseDates;
            config.numPaths = numPaths;
            config.riskFreeRate = r;
            config.maturity = T;
            config.useAntithetic = antithetic;

            lsm::core::BasisSet basis;
            basisFactory(basis, order);

            auto process = sdeFactory(r, sigma);
            lsm::engine::LSMPricer myPricer(*process, payoffType, basis, config);
            return myPricer.price(S0);
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
                    double lsmPrice = getLSMPrice(24, fixedNumDates, order, fixedPathCount);
                    
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

    void ConvergenceAnalyser::runPathConvergence(std::vector<int> pathCounts) {
        std::string filename = "csv_output/path_convergence_" + paramString() + ".csv";
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
            double lsmPrice = getLSMPrice(24, fixedNumDates, order, i);
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

    void ConvergenceAnalyser::runOrderConvergence(std::vector<int> orders) {
        std::string filename = "csv_output/order_convergence_" + paramString() + ".csv";
        double fdPrice = getFDPrice();

        // keep basis type in title since we run both basis types
        std::cout << "\n--- Order Convergence | " << basisType.basisPtrs()[1]->name() << " ---\n\n";
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
            double lsmPrice = getLSMPrice(24, fixedNumDates, i, fixedPathCount);
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

    void ConvergenceAnalyser::runDatesConvergence(std::vector<int> exerciseDatesList) {
        std::string filename = "csv_output/dates_convergence_" + paramString() + ".csv";
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
            double lsmPrice = getLSMPrice(24, i, order, fixedPathCount);
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
        std::string filename = "csv_output/fd_convergence_" + paramString() + ".csv";
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

        for (int timeSteps : timeCounts) {
            auto start = std::chrono::high_resolution_clock::now();
            double fdPrice = lsm::fd::FDPricer(gbm, payoffType, stockSteps, timeSteps).price(S0, T);
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

    void ConvergenceAnalyser::runSeedStability() {
        std::vector<int> seeds = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        std::string filename = "csv_output/seed_stability_" + paramString() + ".csv";

        std::cout << "\n--- Seed Stability ---\n\n";
        std::cout << std::fixed << std::setprecision(4);
        std::cout << std::right
                  << std::setw(6)  << "Seed"
                  << std::setw(13) << "Plain Price"
                  << std::setw(11) << "Plain SE"
                  << std::setw(17) << "Antithetic Price"
                  << std::setw(15) << "Antithetic SE" << "\n"
                  << std::string(62, '-') << "\n";

        std::ofstream out(filename);
        out << "Seed,PlainPrice,PlainSE,AntitheticPrice,AntitheticSE\n";

        std::vector<double> plainPrices;
        std::vector<double> antitheticPrices;
        double plainSESum = 0.0, antiSESum = 0.0;

        for (int seed : seeds) {
            lsm::engine::SimulationResult plain      = getLSMResult(seed, fixedNumDates, order, fixedPathCount, false);
            lsm::engine::SimulationResult antithetic = getLSMResult(seed, fixedNumDates, order, fixedPathCount, true);

            plainPrices.push_back(plain.optionValue);
            antitheticPrices.push_back(antithetic.optionValue);
            plainSESum += plain.standardError;
            antiSESum  += antithetic.standardError;

            out << seed << ","
                << plain.optionValue << "," << plain.standardError << ","
                << antithetic.optionValue << "," << antithetic.standardError << "\n";

            std::cout << std::setw(6)  << seed
                      << std::setw(13) << plain.optionValue
                      << std::setw(11) << plain.standardError
                      << std::setw(17) << antithetic.optionValue
                      << std::setw(15) << antithetic.standardError << "\n";
        }

        double plainMean = 0.0, antiMean = 0.0;
        for (double p : plainPrices)      plainMean += p;
        for (double p : antitheticPrices) antiMean  += p;
        plainMean /= plainPrices.size();
        antiMean  /= antitheticPrices.size();

        double plainVar = 0.0, antiVar = 0.0;
        for (double p : plainPrices)      plainVar += (p - plainMean) * (p - plainMean);
        for (double p : antitheticPrices) antiVar  += (p - antiMean)  * (p - antiMean);
        double plainStd   = std::sqrt(plainVar / plainPrices.size());
        double antiStd    = std::sqrt(antiVar  / antitheticPrices.size());
        double avgPlainSE = plainSESum / seeds.size();
        double avgAntiSE  = antiSESum  / seeds.size();

        out << "Stats," << plainMean   << "," << plainStd   << "," << antiMean  << "," << antiStd  << "\n";
        out << "AvgSE," << avgPlainSE  << ",," << avgAntiSE << ",\n";
        out.close();

        std::cout << std::string(62, '-') << "\n"
                  << std::setw(6) << "Stats"
                  << std::setw(13) << plainMean  << std::setw(11) << plainStd
                  << std::setw(17) << antiMean   << std::setw(15) << antiStd  << "\n"
                  << std::setw(6) << "AvgSE"
                  << std::setw(13) << avgPlainSE << std::setw(11) << ""
                  << std::setw(17) << avgAntiSE  << "\n"
                  << "\n  -> Written to: " << filename << "\n";
    }


    } //namespace analysis
} //namespace lsm
