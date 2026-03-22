#include "lsm_pricer.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>

// Simulated Paths 
namespace lsm {
    namespace engine {


        PathData simulatePaths(
            double S0,
            const StochasticProcess& process,
            const LSMConfig& config)
        {

            const int T = config.numExerciseDates;
            const int N = config.useAntithetic
                ? config.numPaths - (config.numPaths % 2)
                : config.numPaths;

            const double dt = config.maturity / static_cast<double>(T);

            PathData data;
            data.numPaths = N;
            data.numTimeSteps = T;

            data.paths.resize(N);
            data.cashFlows.resize(N);

            for (int i = 0; i < N; ++i)
            {
                data.paths[i].resize(T + 1);
                data.cashFlows[i].resize(T + 1, 0.0); 
            }

            RNG rng(config.rngSeed);

            if (!config.useAntithetic)
            {
     
                for (int i = 0; i < N; ++i)
                {
                    auto& path = data.paths[i];

                    
                    path[0] = S0;

                    for (int t = 1; t <= T; ++t)
                    {
                        path[t] = process.step(path[t - 1], dt, rng);
                    }
                }
            }

            else
            {
              
                const int half = N / 2;

                for (int i = 0; i < half; ++i)
                {
                    auto& p1 = data.paths[i];
                    auto& p2 = data.paths[i + half];

                    p1[0] = S0;
                    p2[0] = S0;

                    for (int t = 1; t <= T; ++t)
                    {
                        double z = rng.normal();

                        p1[t] = process.stepWithNormal(p1[t - 1], dt,  z);

                        p2[t] = process.stepWithNormal(p2[t - 1], dt, -z);
                    }
                }
            }

            return data;
        }

    } // namespace engine
} // namespace lsm

