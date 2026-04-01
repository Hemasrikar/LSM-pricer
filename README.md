# LSM American Option Pricer

This project implements the **Longstaff-Schwartz Least Squares Monte Carlo (LSM)** algorithm for pricing American options in C++20.

The core idea is to use regression on Monte Carlo simulated paths to estimate the continuation value at each exercise date, working backwards from maturity. This gives a provably convergent lower bound on the true option price.

> Longstaff, F.A. & Schwartz, E.S. (2001). *Valuing American Options by Simulation: A Simple Least-Squares Approach.* Review of Financial Studies, 14(1), 113–147.

### Configuration: LSMConfig

The `LSMConfig` struct controls all parameters for the Longstaff-Schwartz pricer. The lsm_pricer.hpp provides defaults and values can be changed through main.cpp.

**Default Configuration**
```cpp
struct LSMConfig {
    int numPaths = 100000;  // total simulation paths
    bool useAntithetic = true;    // antithetic-variate variance reduction
    int numExerciseDates = 50; // early-exercise opportunities (t=1..T)
    double maturity = 1.0;     // option maturity in years
    double   riskFreeRate = 0.06;    // continuously compounded risk-free rate
    unsigned rngSeed = 24;  // seed value
};
```

## Architecture

Three layers:

- **`lsm::core`** - stochastic processes, payoffs, basis functions, RNG
- **`lsm::engine`** - LSM algorithm: path simulation, backward induction, OLS regression
- **`lsm::analysis`** - convergence studies and benchmarking via `ConvergenceAnalyser`
