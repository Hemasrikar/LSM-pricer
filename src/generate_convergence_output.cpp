#include <iostream>
#include <fstream>
#include <cmath>
#include "convergence_analyser.hpp"

// market parameters
const double S0 = 36.0;
const double K = 40.0;
const double r = 0.06;
const double sigma = 0.20;
const double T = 1.0;

// fixed parameters used across convergence tests
const int fixedPathCount = 10000;
const int fixedOrder = 3;
const int fixedNumDates = 50;
const int fixedStockSteps = 1000; // for FD convergence

// parameter lists to iterate over in each convergence test
const std::vector<int> orders = {1, 2, 3, 4, 5};
const std::vector<int> exerciseDates = {1, 5, 10, 20, 50, 100};
const std::vector<int> fdTimeCounts = {10, 50, 100, 500, 1000, 5000, 10000, 40000};
const std::vector<int> pathCounts = []() {
    std::vector<int> v;
    for (int i = 6; i <= 17; ++i) // Change the value of i to adjust the range of number of paths (from 2^6 to 2^17 )
        v.push_back(static_cast<int>(std::pow(2.0, i)));
    return v;
}();

// main function which goes through each type of convergence
int main(){

    lsm::core::GeometricBrownianMotion gbm(r, sigma);
    lsm::core::Put_payoff put(K);

    lsm::core::BasisSet basis;
    basis.makeLaguerreSet(fixedOrder);

    std::function<void(lsm::core::BasisSet&, int)> basisFactory = [](lsm::core::BasisSet& bs, int order) {
        bs.makeLaguerreSet(order);
    };

    lsm::analysis::ConvergenceAnalyser analyser(S0, r, sigma, K, T, gbm, put, basis, basisFactory, fixedOrder, fixedPathCount, fixedNumDates);

    analyser.runBenchmark();

    analyser.runPathConvergence(pathCounts);
    analyser.runOrderConvergence(orders);
    analyser.runDatesConvergence(exerciseDates);
    analyser.runFDConvergence(fdTimeCounts, fixedStockSteps);
    analyser.runSeedStability();

    return 0;
}