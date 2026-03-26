#include "convergence_analyser.hpp"
#include "bs_pricer.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>


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

        double ConvergenceAnalyser::getFDPrice() {
            // insert logic to get the fd prices
            return 10.5; //placeholder
        }

        double ConvergenceAnalyser::getLSMPrice(unsigned seed, int numExerciseDates, int order, int numPaths) {
            lsm::engine::LSMConfig config; 
            config.rngSeed = seed;
            config.numExerciseDates = numExerciseDates;
            config.numPaths = numPaths;

            auto process = std::make_unique<lsm::core::GeometricBrownianMotion>(r, sigma);
            auto payoff  = std::make_unique<lsm::core::Call_payoff>(K);
            auto basis   = std::make_unique<lsm::core::BasisSet>();
    
            basis->makeLaguerreSet(order);
            // std::cout << "DEBUG: numTerms in basis = " << basis->basis.size() << std::endl;

            // uncomment once the pricer is complete
            lsm::engine::LSMPricer myPricer(std::move(process), std::move(payoff), std::move(basis), config);
            auto result = myPricer.price(S0);

            return result.optionValue;

            // return 10.7;
        }

    // Code to run a one off check against BS
    void ConvergenceAnalyser::runBenchmark() {
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
                        double fdPrice = getFDPrice();
                        double lsmPrice = getLSMPrice(24, 50, 3, 10000);

                        // compute the premium you pay for having early exercise
                        double eeValue = std::abs(bsPrice - fdPrice); 

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

    void ConvergenceAnalyser::runConvergence(const std::string& mode) {
        std::vector<int> list;
        std::string name;
        std::string filename;

        if (mode == "pathCount"){
            list = {1, 10, 100, 1000, 10000, 100000};
            name = "Number of Paths";
            filename = "csv_output/path_convergence.csv";
        }

        else if(mode == "order"){
            list = {1, 2, 3, 4, 5};
            name = "Order of Basis";
            filename = "csv_output/order_convergence.csv";
        }

        else if (mode == "numExerciseDates"){
            list = {1, 5, 10, 20, 50, 100};
            name = "Number of Exercise Dates";
            filename = "csv_output/exercise_dates_convergence.csv";

        }

            // set up the parameters for the convergence test
            int numExerciseDates = 50;
            int pathCount = 10000;
            int order = 3;
            // bool call = true;

            double truePrice = getFDPrice();

            std::ofstream out(filename);
            out << name << ",LSMPrice,TruePrice,Error\n";

            for (int i : list){
                double lsmPrice;

                if (mode == "pathCount")
                    lsmPrice = getLSMPrice(24, numExerciseDates, order, i);
                else if (mode == "order")
                    lsmPrice = getLSMPrice(24, numExerciseDates, i, pathCount);
                else if (mode == "numExerciseDates")
                    lsmPrice = getLSMPrice(24, i, order, pathCount);
                else {
                    std::cout << "Incorrect mode" << std::endl;
                    break;
                }

                double error = std::abs(truePrice - lsmPrice);
                out << i << "," << lsmPrice << "," << truePrice << "," << error << "\n";

                std::cout << name << std::setw(15) << i 
                                << " | Price: " << std::setw(10) << lsmPrice 
                                << " | Error: " << error << std::endl;
        }

        out.close();
        std::cout << "Convergence data written to " << filename << std::endl;
    }


    } //namespace analysis
} //namespace lsm