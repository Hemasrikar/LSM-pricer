#include <cmath>
#include <algorithm>
#include "Finite_Difference.hpp"

namespace lsm {
namespace fd {

FDPricer::FDPricer(const lsm::core::GeometricBrownianMotion& gbm,
                   const lsm::core::OptionPayoff& payoff,
                   int stockSteps,
                   int timeStepsPerYear)
    : gbm_(gbm), payoff_(payoff), M_(stockSteps), N_per_year_(timeStepsPerYear)
{}

double FDPricer::price(double S0, double T) const
{
    double r = gbm_.r();
    double sigma = gbm_.sigma();
    double K = payoff_.strike();
    double S_max = 4.0 * K;
    double dS = S_max / M_;
    int N = static_cast<int>(std::round(N_per_year_ * T));
    double dt = T / N;
    int n = M_ - 1; // number of interior nodes

    // Set terminal payoff values at maturity
    std::vector<double> V(M_ + 1);
    for (int j = 0; j <= M_; ++j)
        V[j] = payoff_.payoff(j * dS);

    // Precompute intrinsic value at each interior node for early exercise
    std::vector<double> intrinsic(n);
    for (int i = 0; i < n; ++i)
        intrinsic[i] = payoff_.payoff((i + 1) * dS);

    // Implicit scheme: alpha_i * f_{i-1} + (1 + beta_i) * f_i + gamma_i * f_{i+1} = f_{i, j+1}
    // where beta_i = sigma^2 * S_i^2 * dt / dS^2 + r * dt,
    //       alpha_i = -0.5 * (sigma^2 * S_i^2 * dt / dS^2 - r * S_i * dt / dS)
    //       gamma_i = -0.5 * (sigma^2 * S_i^2 * dt / dS^2 + r * S_i * dt / dS)
    std::vector<double> alpha(n), beta_diag(n), gamma(n);
    for (int i = 0; i < n; ++i) {
        double S = (i + 1) * dS;
        double xi = sigma * sigma * S * S * dt / (dS * dS);
        double eta = r * S * dt / dS;
        alpha[i] = -0.5 * (xi - eta);
        beta_diag[i] = 1.0 + xi + r * dt;
        gamma[i] = -0.5 * (xi + eta);
    }

    // Neumann boundary condition (d^2V/dS^2 = 0) at both ends
    // Modify first row
    beta_diag[0] += 2.0 * alpha[0];
    gamma[0] -= alpha[0];
    alpha[0] = 0.0;
    // Modify last row
    beta_diag[n - 1] += 2.0 * gamma[n - 1];
    alpha[n - 1] -= gamma[n - 1];
    gamma[n - 1] = 0.0;

    std::vector<double> beta_work(n), d(n);

    for (int step = 0; step < N; ++step) {
        for (int i = 0; i < n; ++i)
            d[i] = V[i + 1];

        // Solve system (Brennan-Schwartz algorithm)
        // Gaussian elimination:
        std::copy(beta_diag.begin(), beta_diag.end(), beta_work.begin());
        for (int i = 1; i < n; ++i) {
            double w = alpha[i] / beta_work[i - 1];
            beta_work[i] -= w * gamma[i - 1];
            d[i] -= w * d[i - 1];
        }

        // Back propagation with early exercise: f*_{i,j} = max(f_{i,j}, payoff(S_i))
        d[n - 1] /= beta_work[n - 1];
        V[n] = std::max(d[n - 1], intrinsic[n - 1]);
        for (int i = n - 2; i >= 0; --i) {
            d[i] = (d[i] - gamma[i] * d[i + 1]) / beta_work[i];
            V[i + 1] = std::max(d[i], intrinsic[i]);
        }

        // Find boundary Neumann condition and apply early exercise
        V[0] = std::max(2.0 * V[1] - V[2], payoff_.payoff(0.0));
        V[M_] = std::max(2.0 * V[M_ - 1] - V[M_ - 2], payoff_.payoff(S_max));
    }

    // Linear interpolation to get price at S0
    double pos = S0 / dS;
    int j0 = static_cast<int>(pos);
    if (j0 >= M_) return V[M_];
    double frac = pos - j0;
    return V[j0] * (1.0 - frac) + V[j0 + 1] * frac;
}

}
}
