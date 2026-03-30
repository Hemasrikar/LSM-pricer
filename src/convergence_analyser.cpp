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
        ConvergenceAnalyser::ConvergenceAnalyser(double s, double rate, double vol, double strike, double maturity, bool call) 
            : S0(s), r(rate), sigma(vol), K(strike), T(maturity), isCall(call) 
        {
            
        }

        // helper functions
        double ConvergenceAnalyser::getBSPrice() {
            return bs_pricer::price_vanilla_option_european_bs(S0, r, sigma, K, T, isCall);
        }

        double ConvergenceAnalyser::getFDPrice(bool isCall) {
            lsm::core::GeometricBrownianMotion gbm(r, sigma);
            if (isCall) {
                lsm::core::Call_payoff payoff(K);
                return lsm::fd::FDPricer(gbm, payoff).price(S0, T);
            } else {
                lsm::core::Put_payoff payoff(K);
                return lsm::fd::FDPricer(gbm, payoff).price(S0, T);
            }
            // insert logic to get the fd prices
            return isCall ? 10.45 : 6.08;
        }

        double ConvergenceAnalyser::getLSMPrice(unsigned seed, int numExerciseDates, int order, int numPaths, bool isLag) {
            lsm::engine::LSMConfig config; 
            config.rngSeed = seed;
            config.numExerciseDates = numExerciseDates;
            config.numPaths = numPaths;
            config.riskFreeRate = r;
            config.maturity = T;

            auto process = std::make_unique<lsm::core::GeometricBrownianMotion>(r, sigma);
            std::unique_ptr<lsm::core::OptionPayoff> payoff;
            if (isCall)
                payoff = std::make_unique<lsm::core::Call_payoff>(K);
            else
                payoff = std::make_unique<lsm::core::Put_payoff>(K);
            auto basis   = std::make_unique<lsm::core::BasisSet>();

            if (isLag == true){
                basis->makeLaguerreSet(order);
            }
            else{
                basis->makeMonomialSet(order);
            }

            lsm::engine::LSMPricer myPricer(std::move(process), std::move(payoff), std::move(basis), config);
            auto result = myPricer.price(S0);

            return result.optionValue;
        }

    // Code to run a one off check against BS
    void ConvergenceAnalyser::runBenchmark(bool isCall) {
        // save and restore original parameters so the benchmark loop doesn't corrupt state
        const double origS0    = S0;
        const double origSigma = sigma;
        const double origT     = T;

        // set up the parameter vectors for the benchmark test
            std::vector<double> S0s   = {36, 38, 40, 42, 44};
            std::vector<double> sigmas = {0.2, 0.4};
            std::vector<double> Ts     = {1.0, 2.0};

        std::string callDisp = isCall ? "Call" : "Put";
        std::string title = " Benchmark  |  " + callDisp + " ";
        std::string border(title.size() + 2, '=');
        std::cout << "\n+" << border << "+\n"
                  << "|  " << title << "  |\n"
                  << "+" << border << "+\n\n";

        std::cout << std::right
                  << std::setw(6)  << "S0"
                  << std::setw(7)  << "Sigma"
                  << std::setw(5)  << "T"
                  << std::setw(14) << "BS European"
                  << std::setw(14) << "FD American"
                  << std::setw(14) << "LSM American"
                  << std::setw(20) << "Early Ex. Premium" << "\n"
                  << std::string(80, '-') << "\n";

            for (double s : S0s) {
                for (double vol : sigmas) {
                    for (double mat : Ts) {

                        this->S0 = s;
                        this->sigma = vol;
                        this->T = mat;

                        // get the black scholes, fd and lsm prices
                        double bsPrice = getBSPrice();
                        double fdPrice = getFDPrice(isCall);
                        double lsmPrice = getLSMPrice(24, 50, 3, 100000, true);

                        // compute the premium you pay for having early exercise
                        double eeValue = lsmPrice - bsPrice;

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
                    }
                }
            }

        S0    = origS0;
        sigma = origSigma;
        T     = origT;
    }

    void ConvergenceAnalyser::runConvergence(const std::string& mode, bool isLag, bool isCall) {
        std::vector<int> list;
        std::string name;
        std::string filename;

        std::string basisLabel = isLag ? "laguerre" : "monomial";
        std::string basisDisp  = isLag ? "Laguerre" : "Monomial";
        std::string callLabel  = isCall ? "call" : "put";
        std::string callDisp   = isCall ? "Call" : "Put";

        if (mode == "pathCount"){
            list = {10, 50, 100, 500, 1000, 5000, 10000, 50000, 100000};
            name = "Number of Paths";
            filename = "csv_output/" + callLabel + "_" + basisLabel + "_path_convergence.csv";
        }
        else if(mode == "order"){
            list = {1, 2, 3, 4, 5};
            name = "Order of Basis";
            filename = "csv_output/" + callLabel + "_" + basisLabel + "_order_convergence.csv";
        }
        else if (mode == "numExerciseDates"){
            list = {1, 5, 10, 20, 50, 100};
            name = "Number of Exercise Dates";
            filename = "csv_output/" + callLabel + "_" + basisLabel + "_exercise_dates_convergence.csv";
        }

        // set up the parameters for the convergence test
        int numExerciseDates = 50;
        int pathCount = 10000;
        int order = 3;

        double truePrice = getFDPrice(isCall);

        // print box header
        std::string title = " " + name + "  |  " + basisDisp + "  |  " + callDisp + " ";
        std::string border(title.size() + 2, '=');
        std::cout << "\n+" << border << "+\n"
                  << "|  " << title << "  |\n"
                  << "+" << border << "+\n\n";

        // print column headers
        std::cout << std::fixed << std::setprecision(4);
        std::cout << std::right
                  << std::setw(12) << name
                  << std::setw(13) << "LSM Price"
                  << std::setw(11) << "Error"
                  << std::setw(12) << "Time (ms)" << "\n"
                  << std::string(48, '-') << "\n";

        std::ofstream out(filename);
        out << name << ",LSMPrice,TruePrice,Error,Time(ms)\n";

        for (int i : list){
            double lsmPrice;

            auto start = std::chrono::high_resolution_clock::now();

            if (mode == "pathCount")
                lsmPrice = getLSMPrice(24, numExerciseDates, order, i, isLag);
            else if (mode == "order")
                lsmPrice = getLSMPrice(24, numExerciseDates, i, pathCount, isLag);
            else if (mode == "numExerciseDates")
                lsmPrice = getLSMPrice(24, i, order, pathCount, isLag);
            else {
                std::cout << "Incorrect mode" << std::endl;
                break;
            }

            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();

            double error = std::abs(truePrice - lsmPrice);
            out << i << "," << lsmPrice << "," << truePrice << "," << error << "," << ms << "\n";

            std::cout << std::setw(12) << i
                      << std::setw(13) << lsmPrice
                      << std::setw(11) << error
                      << std::setw(11) << std::setprecision(2) << ms << " ms"
                      << std::setprecision(4) << "\n";
        }

        out.close();
        std::cout << "\n  -> Written to: " << filename << "\n";
    }

    void ConvergenceAnalyser::runSeedStability(bool isLag, bool isCall) {
        std::vector<int> seeds = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        std::string basisLabel = isLag ? "laguerre" : "monomial";
        std::string basisDisp  = isLag ? "Laguerre" : "Monomial";
        std::string callLabel  = isCall ? "call" : "put";
        std::string callDisp   = isCall ? "Call" : "Put";
        std::string filename   = "csv_output/" + callLabel + "_" + basisLabel + "_seed_stability.csv";

        int numExerciseDates = 50;
        int pathCount        = 10000;
        int order            = 3;

        double truePrice = getFDPrice(isCall);

        // print box header
        std::string title = " Seed Stability  |  " + basisDisp + "  |  " + callDisp + " ";
        std::string border(title.size() + 2, '=');
        std::cout << "\n+" << border << "+\n"
                  << "|  " << title << "  |\n"
                  << "+" << border << "+\n\n";

        // print column headers
        std::cout << std::fixed << std::setprecision(4);
        std::cout << std::right
                  << std::setw(6)  << "Seed"
                  << std::setw(13) << "LSM Price"
                  << std::setw(11) << "Error"
                  << std::setw(12) << "Time (ms)" << "\n"
                  << std::string(42, '-') << "\n";

        std::ofstream out(filename);
        out << "Seed,LSMPrice,TruePrice,Error,Time(ms)\n";

        std::vector<double> prices;

        for (int seed : seeds) {
            auto start      = std::chrono::high_resolution_clock::now();
            double lsmPrice = getLSMPrice(seed, numExerciseDates, order, pathCount, isLag);
            auto end        = std::chrono::high_resolution_clock::now();
            double ms       = std::chrono::duration<double, std::milli>(end - start).count();

            double error = std::abs(truePrice - lsmPrice);
            prices.push_back(lsmPrice);
            out << seed << "," << lsmPrice << "," << truePrice << "," << error << "," << ms << "\n";

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