# LSM American Option Pricer

This project implements the **Longstaff-Schwartz Least Squares Monte Carlo (LSM)** algorithm for pricing American options in C++20.

The core idea is to use regression on Monte Carlo simulated paths to estimate the continuation value at each exercise date, working backwards from maturity. This gives a provably convergent lower bound on the true option price.

> Longstaff, F.A. & Schwartz, E.S. (2001). *Valuing American Options by Simulation: A Simple Least-Squares Approach.* Review of Financial Studies, 14(1), 113–147.

### Configuration: LSMConfig

The `LSMConfig` struct controls all parameters for the Longstaff-Schwartz pricer. The lsm_pricer.hpp provides defaults and values can be changed through main.cpp.

**Default Configuration for LSM Pricer**
```cpp
struct LSMConfig {
    int numPaths = 100000;  // total simulation paths
    bool useAntithetic = true;    // antithetic-variate variance reduction
    int numExerciseDates = 50; // early-exercise opportunities (t=1..T)
    double maturity = 1.0;     // option maturity in years
    double riskFreeRate = 0.06;    // continuously compounded risk-free rate
    unsigned rngSeed = 24;  // seed value
};
```

**Deafult Configuration for Main.cpp for Data Visual Analysis** 
```cpp
namespace cfg {
    constexpr int seed = 24;

    // Market parameters
    constexpr double initial_spot = 1.0;
    constexpr double risk_free_rate = 0.02;
    constexpr double volatility = 0.15;
    constexpr double strike = 1.1;
    constexpr double maturity = 6.0;
    
    // Simulation parameters
    constexpr int num_paths = 50;
    constexpr int num_exercise_dates = 120;
    
    // Regression parameters
    constexpr int basis_order = 3;
    constexpr int grid_points = 300;
    
    // Output
    constexpr const char* output_dir = "../csv_output";
}
```
---

## Architecture

Three layers:

- **`lsm::core`** - stochastic processes, payoffs, basis functions, RNG
- **`lsm::engine`** - LSM algorithm: path simulation, backward induction, OLS regression
- **`lsm::analysis`** - convergence studies and benchmarking via `ConvergenceAnalyser`
---
## Testing

To run the unit tests, open VSCode and use either the **CMake** tab or the **Testing** tab in the left sidebar.

---

> Why we chose Unicorn as a team name: Unicorn was a memorable mascot for our team project, while also being quick and easy to type in command terminals. The term is also used in the tech industry to denote fast-growing startups valued at over US$1 billion, a testament to the naive ambition and enthusiasm with which we began the project.


<div align="center">
  <img src="extern/unicorn cutout.png" alt="Unicorn Mascot" width="300">
</div>
