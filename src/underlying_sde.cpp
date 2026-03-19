#include "underlying_sde.hpp"
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <random>

namespace lsm {
    namespace core {

double RNG::normal() {
    return norm_dist(engine);
}

double RNG::uniform01() {
    return uniform_dist(engine);
}

/*-------------------------------------------------------------------------------------------------
 * SIMULATE A DISCRETE-TIME SAMPLE PATH OF THE PROCESS ON [0,T]
 * Generates a path by iteratively applying the one-step update defined by step() using a uniform
 * discretisation.
 *
 * Parameters:
 *    - s0  Initial value of the process.
 *    - T   Terminal time (must be positive).
 *    - n   Number of time steps.
 *    - rng Random number generator.
 *
 * Returns: A vector of size n + 1 containing the simulated path,
 *          including the initial value at time 0.
 *
 * Throws: std::invalid_argument if T <= 0.
 *
 * Uses a uniform grid with dt = T / n.
 *
---------------------------------------------------------------------------------------------------*/
std::vector<double> StochasticProcess::simulatePath(double s0, double T, std::size_t n, RNG& rng) const {
    if (T <= 0.0) {
        throw std::invalid_argument("simulatePath: T must be positive.");
    }
    const double dt = T / static_cast<double>(n);

    // Allocate once with known size to avoid reallocations
    std::vector<double> path(n + 1);
    path[0] = s0;

    // Iteratively apply the one-step update
    for (std::size_t i = 1; i <= n; ++i) {
        path[i] = step(path[i - 1], dt, rng);
    }
    return path;
}

/*-------------------------------------------------------------------------------------------------
 * CONSTRUCT A GBM PROCESS
 *
 * Parameters:
 *    - r:     Drift coefficient.
 *    - sigma: Volatility parameter (must be non-negative).
 *
 * Throws: std::invalid_argument if sigma < 0.
 *
---------------------------------------------------------------------------------------------------*/
GeometricBrownianMotion::GeometricBrownianMotion(double r, double sigma)
    : r_(r), sigma_(sigma)
{
    if (sigma_ < 0.0) {
        throw std::invalid_argument("GeometricBrownianMotion: sigma must be non-negative.");
    }
}

/*-------------------------------------------------------------------------------------------------
 * Advance the GBM process by one time step.
 *
 * Uses the exact solution of the SDE:
 *     S_{t+dt} = S_t * exp((r - 0.5 * sigma^2) * dt + sigma * sqrt(dt) * Z),
 * where Z ~ N(0,1).
 *
 * Parameters:
 *    - s   Current value of the process.
 *    - dt  Time increment (must be positive).
 *    - rng Random number generator providing standard normal samples.
 *
 * Returns: The next state of the process.
 *
 * Throws: std::invalid_argument if dt <= 0.
 *
---------------------------------------------------------------------------------------------------*/
double GeometricBrownianMotion::step(double s, double dt, RNG& rng) const {
    if (dt <= 0.0) {
        throw std::invalid_argument("GeometricBrownianMotion::step: dt must be positive.");
    }
    // Sample standard normal increment
    const double z = rng.normal();
    // Drift and diffusion components of the log-increment
    const double drift = (r_ - 0.5 * sigma_ * sigma_) * dt;
    const double diff  = sigma_ * std::sqrt(dt) * z;
    return s * std::exp(drift + diff);
}

/*-------------------------------------------------------------------------------------------------
 * Construct a jump-diffusion process with killing jumps.
 *
 * Parameters:
 *    - r:      Drift coefficient.
 *    - sigma:  Volatility of the diffusion component (must be non-negative).
 *    - lambda: Jump intensity / Poisson rate (must be non-negative).
 *
 * Throws: std::invalid_argument if sigma < 0 or lambda < 0.
 *
---------------------------------------------------------------------------------------------------*/
JumpDiffusionProcess::JumpDiffusionProcess(double r, double sigma, double lambda)
    : r_(r), sigma_(sigma), lambda_(lambda)
{
    if (sigma_ < 0.0) {
        throw std::invalid_argument("JumpDiffusionProcess: sigma must be non-negative.");
    }
    if (lambda_ < 0.0) {
        throw std::invalid_argument("JumpDiffusionProcess: lambda must be non-negative.");
    }
}

/*-------------------------------------------------------------------------------------------------
 * Advance the jump-diffusion process by one time step.
 *
 * The process follows:
 *     dS = (r + lambda) * S * dt + sigma * S * dW - S * dq,
 * where q is a Poisson process with intensity lambda. The drift is compensated
 * by +lambda to offset the expected loss from jumps (E[dq] = lambda * dt).
 *
 * Over a time step dt:
 *  - With probability p = 1 - exp(-lambda * dt), a jump occurs and S -> 0.
 *  - Otherwise, the process evolves as GBM with compensated drift (r + lambda).
 *
 * Parameters:
 *    - s   Current value of the process.
 *    - dt  Time increment (must be positive).
 *    - rng Random number generator.
 *
 * Returns: The next state of the process.
 *
 * Throws: std::invalid_argument if dt <= 0 or s < 0.
 *
 * The state S = 0 is absorbing: once reached, it remains zero.
 *
---------------------------------------------------------------------------------------------------*/
double JumpDiffusionProcess::step(double s, double dt, RNG& rng) const {
    if (s < 0.0) {
        // FIX: negative price is a logic error, not a valid absorbing state
        throw std::invalid_argument("JumpDiffusionProcess::step: s must be non-negative.");
    }
    if (s == 0.0) return 0.0;  // Absorbing state: once defaulted, remains at zero
    if (dt <= 0.0) {
        throw std::invalid_argument("JumpDiffusionProcess::step: dt must be positive.");
    }

    // Probability of at least one jump in (t, t+dt]
    const double p_jump = 1.0 - std::exp(-lambda_ * dt);

    // Jump event: process defaults to zero
    if (rng.uniform01() < p_jump) {
        return 0.0;
    }

    // No jump: exact GBM evolution with compensated drift (r + lambda)
    const double z     = rng.normal();
    const double drift = ((r_ + lambda_) - 0.5 * sigma_ * sigma_) * dt;
    const double diff  = sigma_ * std::sqrt(dt) * z;

    return s * std::exp(drift + diff);
}

    }
}
