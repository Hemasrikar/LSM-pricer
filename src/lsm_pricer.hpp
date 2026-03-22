#pragma once

#include <vector>

#include <string>
#include <vector>
#include "underlying_sde.hpp"
#include "option_payoff.hpp"
#include "basis_functions.hpp"
#include "ols_regression.hpp"

namespace lsm 

    // Configuration for LSM
    struct LSMConfig {
        int numPaths = 0;
        int numExerciseDates = 0;
        double maturity = 0.0;
        bool useAntithetic = false;
        unsigned int rngSeed = 42;
    };

    // Path storage structure
    struct PathData {
        int numPaths = 0;
        int numTimeSteps = 0;

        std::vector<std::vector<double>> paths;

        std::vector<std::vector<double>> cashFlows;
    };

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

    namespace engine {

        // Path simulation function
        PathData simulatePaths(
            double S0,
            const StochasticProcess& process,
            const LSMConfig& config
        );

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