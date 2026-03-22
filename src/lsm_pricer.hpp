#pragma once

#include <vector>

#include <string>
#include <vector>
#include "underlying_sde.hpp"
#include "option_payoff.hpp"
#include "basis_functions.hpp"
#include "ols_regression.hpp"

namespace lsm {
   /*  I dont think this is required... since we are including underlying SDE
    // RNG part
    class RNG {
    public:
        explicit RNG(unsigned int seed);

        double normal();
    };
    
    // Stochastic process 
    class StochasticProcess {
    public:
        virtual ~StochasticProcess() = default;

        virtual double step(double S, double dt, RNG& rng) const = 0;

        virtual double stepWithNormal(double S, double dt, double z) const = 0;
    };
    */ 

    namespace engine {

        // Configuration for LSMPricer
        struct LSMConfig {
            int numPaths = 100000;          // total simulation paths
            bool useAntithetic = true;      // antithetic variance reduction
            int numExerciseDates = 50;      // early exercise opportunities
            double maturity = 1.0;          // option maturity in years
            double riskFreeRate = 0.06;     // continuously compounded risk-free rate
            unsigned rngSeed = 24;          // RNG seed
        };

        // Simulated paths and recorded cash flows
        struct PathData {
            std::vector<std::vector<double>> paths;      // [numPaths][numTimeSteps+1]
            std::vector<std::vector<double>> cashFlows;  // [numPaths][numTimeSteps+1]
            int numPaths = 0;
            int numTimeSteps = 0;
        };

        // Path simulation function
        PathData simulatePaths(
            double S0,
            const lsm::core::StochasticProcess& process,
            const lsm::engine::LSMConfig& config
        );

        //output of LSMPricer::price()
        struct SimulationResult {
        double optionValue          = 0.0;  // American option value (lower bound)
        double standardError        = 0.0;  // Monte Carlo standard error
        double europeanValue        = 0.0;  // European (no early-exercise) benchmark
        double earlyExercisePremium = 0.0;  // American − European
        int    numPaths             = 0;
        int    numExerciseDates     = 0;
        };

        class LSMPricer{
        public:
            LSMPricer(lsm::core::StochasticProcess* process,
              lsm::core::OptionPayoff* payoff,
              lsm::core::BasisSet* basis,
              const lsm::engine::LSMConfig& config);
            
              lsm::engine::SimulationResult price(double S0);
        private:
            lsm::core::StochasticProcess* process;
            lsm::core::OptionPayoff* payoff;
            lsm::core::BasisSet* basis;

            lsm::engine::LSMConfig config;

            lsm::engine::PathData simulatePaths(double S0) const;

            std::vector<double> backwardInduction(lsm::engine::PathData& data) const;

            lsm::engine::SimulationResult computeOptionValue(
                const std::vector<double>& pv,
                double europeanValue,
                int N, 
                int T) const;
        };

    } // namespace engine

} // namespace lsm
