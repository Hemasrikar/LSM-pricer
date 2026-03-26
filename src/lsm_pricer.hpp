#pragma once
#include <string>
#include <vector>
#include <memory>
#include "underlying_sde.hpp"
#include "option_payoff.hpp"
#include "basis_functions.hpp"
#include "ols_regression.hpp"

namespace lsm {
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
            LSMPricer(
                std::unique_ptr<const lsm::core::StochasticProcess> process,
                std::unique_ptr<const lsm::core::OptionPayoff> payoff,
                std::unique_ptr<lsm::core::BasisSet> basis,
                const lsm::engine::LSMConfig& config);
            
                lsm::engine::SimulationResult price(double S0);
        private:
            std::unique_ptr<const lsm::core::StochasticProcess> process;
            std::unique_ptr<const lsm::core::OptionPayoff>      payoff;
            std::unique_ptr<lsm::core::BasisSet>                basis;

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
