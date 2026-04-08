#include "mc_paths.hpp"

namespace lsm {
    namespace engine {

/*-------------------------------------------------------------------------------------------------
 * GENERATE A MATRIX OF SIMULATED PATHS
 *
 * Calls simulatePath() N times, storing each resulting path as a row in the output matrix.
 * Each path has n + 1 entries: [S_0, S_1, ..., S_n].
 *
 * Parameters:
 *    - process: The stochastic process to simulate.
 *    - s0:      Initial value of the process.
 *    - T:       Terminal time.
 *    - n:       Number of time steps per path.
 *    - N:       Number of paths to simulate.
 *    - rng:     Random number generator.
 *
 * Returns: paths[i][t] = stock price on path i at time step t.
 *
---------------------------------------------------------------------------------------------------*/
std::vector<std::vector<double>> generatePaths(
    const lsm::core::StochasticProcess& process,
    double s0,
    double T,
    std::size_t n,
    std::size_t N,
    lsm::core::RNG& rng)
{
    std::vector<std::vector<double>> paths(N);
    for (std::size_t i = 0; i < N; ++i) {
        paths[i] = process.simulatePath(s0, T, n, rng);
    }
    return paths;
}

    }
}
