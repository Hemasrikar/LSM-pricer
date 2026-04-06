---
title: Convergence Analyzer
layout: default
nav_order: 2
parent: Benchmark and Convergence
has_toc: true
---
**Namespace:** `lsm::analysis` | **Header:** `convergence_analyser.hpp`

The **Convergence Analyser** orchestrates a suite of experiments that compare the LSM pricer against analytical and numerical benchmarks. It varies key parameters — number of paths, exercise dates, basis function order, and RNG seed — to assess accuracy and stability of the LSM estimates.

Results are printed to the terminal and written to CSV files in `csv_output/`.

---

## Class: `ConvergenceAnalyser`

### Constructor

```cpp
ConvergenceAnalyser(
    double S0, double r, double sigma, double K, double T,
    std::function<std::unique_ptr<lsm::core::StochasticProcess>(double, double)> sdeFactory,
    const lsm::core::OptionPayoff& payoff,
    const lsm::core::BasisSet& basis,
    std::function<void(lsm::core::BasisSet&, int)> basisFactory,
    int order, int fixedPathCount, int fixedNumDates
);
```

### Parameters

| Parameter        | Type            | Description                                           |
|------------------|-----------------|-------------------------------------------------------|
| `S0`             | `double`        | Initial spot price                                    |
| `r`              | `double`        | Risk-free interest rate (annualised)                  |
| `sigma`          | `double`        | Volatility (annualised)                               |
| `K`              | `double`        | Strike price                                          |
| `T`              | `double`        | Time to maturity (in years)                           |
| `sdeFactory`     | `function`      | Factory that constructs the stochastic process        |
| `payoff`         | `OptionPayoff&` | Payoff type (put or call)                             |
| `basis`          | `BasisSet&`     | Basis function set used in the LSM regression         |
| `basisFactory`   | `function`      | Factory that populates the basis set to a given order |
| `order`          | `int`           | Default polynomial order for the regression           |
| `fixedPathCount` | `int`           | Default number of Monte Carlo paths                   |
| `fixedNumDates`  | `int`           | Default number of exercise dates                      |

---

## Helper Functions

Used internally to retrieve benchmark prices.

| Function | Description |
|----------|-------------|
| `getBSPrice()` | Returns the Black-Scholes European price |
| `getFDPrice()` | Returns the Finite Difference American price |
| `getLSMPrice(seed, numDates, order, numPaths)` | Returns the LSM American option value |
| `getLSMResult(seed, numDates, order, numPaths, antithetic)` | Returns the full `SimulationResult` including standard error, with optional antithetic variance reduction |

---

## Analysis Functions

### `runBenchmark()`

Compares BS European, FD American, and LSM American prices across a parameter grid:
- Spot prices: $S_0 \in \{36, 40, 44\}$
- Volatilities: $\sigma \in \{0.2, 0.4\}$
- Maturities: $T \in \{1, 2\}$

Also reports the **early exercise premium** = LSM price − BS price. Output written to `csv_output/benchmark.csv`.

---

### `runPathConvergence(pathCounts)`

Runs the LSM pricer for each value in `pathCounts`, recording the price, error against the FD benchmark, and runtime. Shows how accuracy improves as the number of Monte Carlo paths increases.

---

### `runDatesConvergence(exerciseDatesList)`

Runs the LSM pricer for each value in `exerciseDatesList`, varying the number of exercise opportunities. Shows how the American option price converges as the time discretisation is refined.

---

### `runOrderConvergence(orders)`

Runs the LSM pricer for each polynomial order in `orders`. Shows how the choice of regression order for the basis functions affects pricing accuracy.

---

### `runFDConvergence(timeCounts, stockSteps)`

Runs the Finite Difference pricer for each time step count in `timeCounts`, comparing against the Black-Scholes price. Shows how the FD method converges as the grid is refined.

---

### `runSeedStability()`

Runs the LSM pricer across 10 fixed seeds (1–10) with all other parameters held constant, comparing **plain Monte Carlo** against **antithetic variance reduction** side by side. Reports the mean, standard deviation, and average standard error for both methods to quantify the reduction in Monte Carlo variance from antithetic sampling.
