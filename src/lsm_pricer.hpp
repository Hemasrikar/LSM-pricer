#pragma once


#include <string>

#include <vector>

//  BasisFunction  
// — Abstract: strategy for regression basis

namespace lsm{
    namespace core{

        // BasisFunction — Abstract strategy for regression basis
        class BasisFunction {
        public:
            virtual ~BasisFunction() = default;
            virtual double evaluate(double x) const = 0;
            virtual std::string name() const = 0;
        };

    } // namespace core

    namespace engine{

        // Configuration for LSMPricer
        struct LSMConfig {
            int numPaths = 100000;  // total simulation paths
            bool useAntithetic = true;  // Antithetic Variance reduction
            int numExerciseDates = 50;  // Early exercise opportunities (t=1..T)
            double maturity = 1.0;     // Option maturity in years
            double riskFreeRate = 0.06;  // Continuously compounded risk-free rate
            unsigned rngSeed = 24;      // Seed value
        };

        //simulated paths and recorded cash flows
        struct PathData {
        std::vector<std::vector<double>> paths;      // [numPaths][numTimeSteps+1]
        std::vector<std::vector<double>> cashFlows;  // [numPaths][numTimeSteps+1]
        int numPaths     = 0;
        int numTimeSteps = 0;
        };

        //output of LSMPricer::price()
        struct SimulationResult {
        double optionValue          = 0.0;  // American option value (lower bound)
        double standardError        = 0.0;  // Monte Carlo standard error
        double europeanValue        = 0.0;  // European (no early-exercise) benchmark
        double earlyExercisePremium = 0.0;  // American − European
        int    numPaths             = 0;
        int    numExerciseDates     = 0;
        };

    } // namespace engine
} // namespace lsm
