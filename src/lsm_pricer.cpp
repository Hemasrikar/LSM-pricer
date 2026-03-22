#include "lsm_pricer.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>

// Simulated Paths 
namespace lsm {
    namespace engine {

        // Define the function simulatePaths with return type as PathData
        // With S0 initial price 
        // const StochasticProcess & process refers back to an object that represents the stochastic model
        PathData simulatePaths(
            double S0,
            const StochasticProcess& process,
            const LSMConfig& config)
        {

            // Validation checks
            if (config.numExerciseDates <= 0)
                throw std::invalid_argument("Number of exercise dates must be positive");

            if (config.numPaths <= 0)
                throw std::invalid_argument("Number of paths must be positive");

           // Setting up: 
           // The number of Time Steps T 
           // The number of Monte Carlo Paths N 
           // The size of the step dt
            const int T = config.numExerciseDates;

            // Ensure even number of paths when using antithetic variates
            const int N = config.useAntithetic
                ? config.numPaths - (config.numPaths % 2)
                : config.numPaths;

            const double dt = config.maturity / static_cast<double>(T);

            // Creating a result to store the data
            PathData data;
            data.numPaths = N;
            data.numTimeSteps = T;

            // Allocating the memory and time steps
            data.paths.resize(N);
            data.cashFlows.resize(N);

            for (int i = 0; i < N; ++i)
            {
                data.paths[i].resize(T + 1);
                data.cashFlows[i].resize(T + 1, 0.0); 
            }

            // Setting up the random number generator 
            // Checking if require to use monte carlo or antithetic variates
            RNG rng(config.rngSeed);


            // Monte Carlo Simulation part
            if (!config.useAntithetic)
            {
                // Monte Carlo loop reference path 
                // Setting the price S0 as starting value of the path
                for (int i = 0; i < N; ++i)
                {
                    auto& path = data.paths[i];

                    
                    path[0] = S0;

                    // Time evolution for each time step
                    for (int t = 1; t <= T; ++t)
                    {
                        path[t] = process.step(path[t - 1], dt, rng);
                    }
                }
            }

            // Antithetic Variates Simulation part
            else
            {
                // Spliting the paths into half 
                // Loop over the two paths and receive the path references
                // We set the price as S0 for each of the path p1 and p2
                const int half = N / 2;

                for (int i = 0; i < half; ++i)
                {
                    auto& p1 = data.paths[i];
                    auto& p2 = data.paths[i + half];

                    p1[0] = S0;
                    p2[0] = S0;

                    // Time evolution and random shock z standard normal 
                    // For both paths which includes the antithetic path p2
                    for (int t = 1; t <= T; ++t)
                    {
                        double z = rng.normal();

                        // Normal path
                        p1[t] = process.stepWithNormal(p1[t - 1], dt,  z);

                        // Antithetic path
                        p2[t] = process.stepWithNormal(p2[t - 1], dt, -z);
                    }
                }
            }

            return data;
        }

    } // namespace engine
} // namespace lsm
