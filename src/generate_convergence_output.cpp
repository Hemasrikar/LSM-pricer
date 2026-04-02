#include <iostream>
#include <fstream>
#include "convergence_analyser.hpp"


// Market parameters
const double S0    = 36.0;
const double K     = 40.0;
const double r     = 0.06;
const double sigma = 0.20;
const double T     = 1.0;


// Fixed parameters used across convergence tests
const int fixedPathCount       = 10000;
const int fixedOrder           = 3;
const int fixedNumDates        = 50;
const int fixedStockSteps      = 1000; // for FD convergence


// Parameter lists to iterate over in each convergence test
const std::vector<int> orders          = {1, 2, 3, 4, 5};
const std::vector<int> exerciseDates   = {1, 5, 10, 20, 50, 100};
const std::vector<int> fdTimeCounts    = {10, 50, 100, 500, 1000, 5000, 10000, 40000};
const std::vector<int> pathCounts = []() {
    std::vector<int> v;
    for (int i = 6; i <= 17; ++i) // Change the value of i to adjust the range of number of paths (from 2^6 to 2^17 )
        v.push_back(static_cast<int>(std::pow(2.0, i)));
    return v;
}();

int main(){
    std::ofstream params("csv_output/params.csv");
    params << "S0,K,r,sigma,T\n"
           << S0 << "," << K << "," << r << "," << sigma << "," << T << "\n";
    params.close();

    lsm::analysis::ConvergenceAnalyser analyser(S0, r, sigma, K, T);

    analyser.runBenchmark();

    analyser.runPathConvergence(true, pathCounts, fixedNumDates, fixedOrder);
    analyser.runOrderConvergence(true, orders, fixedNumDates, fixedPathCount);   // Laguerre
    analyser.runOrderConvergence(false, orders, fixedNumDates, fixedPathCount);  // Monomial (for comparison)
    analyser.runDatesConvergence(true, exerciseDates, fixedOrder, fixedPathCount);
    analyser.runFDConvergence(fdTimeCounts, fixedStockSteps);
    analyser.runSeedStability(true);

    return 0;
}
