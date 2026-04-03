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


        // Longstaff-Schwartz Monte Carlo pricer for American-style options.
        //
        // Object-oriented design:
        // - Abstraction: the pricer depends on the abstract interfaces
        //   StochasticProcess, OptionPayoff, and BasisSet.
        // - Polymorphism: different derived models, payoffs, and basis systems
        //   can be supplied at runtime without changing the pricing logic.
        // - Encapsulation: path simulation, backward induction, and valuation
        //   are contained within a single pricing workflow.
        //
        // Resource management:
        // - Dependencies are owned through std::unique_ptr, so their lifetimes
        //   are tied to the lifetime of the pricer.
        // - This follows RAII principles and avoids manual memory management.

        class LSMPricer{
        public:
        // Construct the pricer by transferring ownership of the process,
        // payoff, and basis objects into the class.
        
            LSMPricer(
                const lsm::core::StochasticProcess& process,
                const lsm::core::OptionPayoff& payoff,
                const lsm::core::BasisSet& basis,
                const lsm::engine::LSMConfig& config);
                
                // Price the option from initial asset value S0
                lsm::engine::SimulationResult price(double S0);

                // Price the option and also return the generated path data.
                std::pair<lsm::engine::SimulationResult, lsm::engine::PathData> priceWithData(double S0);
        private:

            const lsm::core::StochasticProcess& process;
            const lsm::core::OptionPayoff& payoff;
            const lsm::core::BasisSet& basis;
            // Numerical configuration stored by value.
            lsm::engine::LSMConfig config;

            // Simulate all asset price paths required by the Monte Carlo stage.
            lsm::engine::PathData simulatePaths(double S0) const;

            // Perform backward induction:
            std::vector<double> backwardInduction(lsm::engine::PathData& data) const;

            // Aggregate discounted pathwise payoffs into final pricing statistics,
            // including standard error and early exercise premium.
            lsm::engine::SimulationResult computeOptionValue(
                const std::vector<double>& pv,
                double europeanValue,
                int N, 
                int T) const;
        };

    } // namespace engine

} // namespace lsm
