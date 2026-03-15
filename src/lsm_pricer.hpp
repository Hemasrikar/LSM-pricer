#pragma once


# include <string>

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

    } // namespace engine
} // namespace lsm
