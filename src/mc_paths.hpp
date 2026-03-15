#pragma once

#include <vector>
#include <cstddef>
#include "underlying_sde.hpp"

/*-------------------------------------------------------------------------------------------------
 * GENERATE A MATRIX OF SIMULATED PATHS
 *
 * Simulates N independent sample paths of the given stochastic process on [0, T]
 * using a uniform time grid with n steps.
 *
 * Parameters:
 *    - process: The stochastic process to simulate (GBM, jump-diffusion, etc.)
 *    - s0:      Initial value of the process.
 *    - T:       Terminal time (must be positive).
 *    - n:       Number of time steps per path.
 *    - N:       Number of paths to simulate.
 *    - rng:     Random number generator.
 *
 * Returns: paths[i][t] = stock price on path i at time step t,
 *          where i in [0, N) and t in [0, n].
 *
---------------------------------------------------------------------------------------------------*/
std::vector<std::vector<double>> generatePaths(
    const StochasticProcess& process,
    double s0,
    double T,
    std::size_t n,
    std::size_t N,
    RNG& rng);
