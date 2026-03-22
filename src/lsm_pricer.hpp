#pragma once

#include <vector>

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

    } // namespace engine

} // namespace lsm