#pragma once

#include <vector>
#include <string>
#include "lsm_pricer.hpp"
#include "bs_pricer.hpp"

namespace lsm{
    namespace analysis{

        class ConvergenceAnalyser {
        // defines the variables which only the class can see
        private:
            double S0, r, sigma, K, T;
            bool isCall;

        // defines the functions which can be accessed outside of the class
        public:
            ConvergenceAnalyser(double s, double r, double sigma, double K, double T, bool call);

            // helper functions to get the prices

            double getBSPrice();

            double getFDPrice(bool isCall);

            double getLSMPrice(unsigned seed, int numExerciseDates, int order, int numPaths, bool isLag);

            // functions to run the convergence analysis

            void runBenchmark(bool isCall);

            void runConvergence(const std::string& mode, bool isLag, bool isCall);

            void runSeedStability(const std::string& mode, bool isLag, bool isCall);
        };
    } // namespace analysis

} // namespace lsm