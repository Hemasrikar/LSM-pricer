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

        // defines the functions which can be accessed outside of the class
        public:
            ConvergenceAnalyser(double s, double r, double sigma, double K, double T);

            // helper functions to get the prices

            double getBSPrice();

            double getFDPrice();

            double getLSMPrice(unsigned seed, int numExerciseDates, int order, int numPaths, bool isLag);


            // functions to run the convergence analysis

            void runBenchmark();

            void runPathConvergence(bool isLag, std::vector<int> pathCounts, int numExerciseDates, int order);

            void runDatesConvergence(bool isLag, std::vector<int> exerciseDatesList, int order, int pathCount);

            void runOrderConvergence(bool isLag, std::vector<int> orders, int numExerciseDates, int pathCount);

            void runFDConvergence(std::vector<int> timeCounts, int stockSteps);

            void runSeedStability(bool isLag);
        };
    } // namespace analysis

} // namespace lsm