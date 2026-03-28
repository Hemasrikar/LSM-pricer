#include "convergence_analyser.hpp"
#include "bs_pricer.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>


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
        // set up the parameter vectors for the benchmark test
            std::vector<double> S0s   = {90.0, 100.0, 110.0};
            std::vector<double> sigmas = {0.2, 0.4};
            std::vector<double> Ts     = {1.0, 2.0};

            std::cout << std::left << std::setw(8)  << "S0"
                << std::setw(8)  << "Sigma"
                << std::setw(8)  << "T"
                << std::setw(15) << "BS European"
                << std::setw(15) << "FD American"
                << std::setw(22) << "Early Exercise Value"
                << std::setw(15) << "LSM American" << std::endl;


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

                        std::cout << std::left << std::setw(8) << s
                            << std::setw(8)  << vol
                            << std::setw(8)  << mat
                            << std::setw(15) << bsPrice
                            << std::setw(15) << fdPrice
                            << std::setw(22) << eeValue
                            << std::setw(15) << lsmPrice << std::endl;
                    }
                }
            }

    }

    void ConvergenceAnalyser::runConvergence(const std::string& mode, bool isLag, bool isCall) {
        std::vector<int> list;
        std::string name;
        std::string filename;

        std::string basisLabel = isLag ? "laguerre" : "monomial";
        std::string callLabel = isCall ? "call" : "put";

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
            // bool call = true;

            double truePrice = getFDPrice(isCall);

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
                out << i << "," << lsmPrice << "," << truePrice << "," << error <<  "," << ms << "\n";

                std::cout << name << std::setw(15) << i 
                                << " | Price: " << std::setw(10) << lsmPrice 
                                << " | Error: " << error
                                << " | Time: " << ms << " ms" << std::endl;
        }

        out.close();
        std::cout << "Convergence data written to " << filename << std::endl;
}


    } //namespace analysis
} //namespace lsm