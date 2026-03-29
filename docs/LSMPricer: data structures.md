---
title: LSM Pricer: Data Structures
layout: default
parent: LSM Pricer
nav_order: 5
has_toc: true
---


**Namespace:** `lsm::engine` | **Header:** `lsm_pricer.hpp`

This page documents the supporting data structures used by `LSMPricer`.

These structures separate:

- configuration of the pricing run,
- storage of simulated pathwise data,
- and packaging of the final pricing output.

This separation keeps the main pricing logic readable while making intermediate and final data explicit.

---

## `LSMConfig`

Stores the configuration parameters used by the Longstaff–Schwartz pricing engine.

```cpp
struct LSMConfig {
    int numPaths = 100000;
    bool useAntithetic = true;
    int numExerciseDates = 50;
    double maturity = 1.0;
    double riskFreeRate = 0.06;
    unsigned rngSeed = 24;
};
```

### Purpose

`LSMConfig` collects the numerical and simulation settings required by the pricing engine. These values control both the scale of the Monte Carlo experiment and the time discretisation used for the early-exercise problem.

### Fields

- `numPaths`  
  Total number of Monte Carlo sample paths used in the pricing run. A larger number of paths generally reduces Monte Carlo error but increases computational cost.

- `useAntithetic`  
  Indicates whether antithetic variance reduction is enabled during path simulation. This affects the path-generation stage rather than the backward induction logic itself.

- `numExerciseDates`  
  Number of exercise opportunities before maturity. This determines the number of time steps used in the discretised optimal stopping problem.

- `maturity`  
  Option maturity in years. Together with `numExerciseDates`, this determines the time increment
  ```math
  dt = \frac{T}{N_{\text{steps}}}.
  ```

- `riskFreeRate`  
  Continuously compounded risk-free rate used for discounting future cashflows.

- `rngSeed`  
  Seed used to initialise the random number generator. This supports reproducibility of pricing runs.

---

## `PathData`

Stores the simulated asset-price paths together with the cashflow matrix used during backward induction.

```cpp
struct PathData {
    std::vector<std::vector<double>> paths;
    std::vector<std::vector<double>> cashFlows;
    int numPaths = 0;
    int numTimeSteps = 0;
};
```

### Purpose

`PathData` stores the full pathwise state required by the pricing algorithm.

The `paths` matrix contains the simulated realisations of the underlying asset. The `cashFlows` matrix is used during backward induction to record exercise payoffs and to zero out later cashflows once early exercise has occurred.

### Fields

- `paths`  
  Simulated asset prices arranged as `paths[pathIndex][timeIndex]`.

  For example, `paths[i][t]` is the simulated asset price on path `i` at time index `t`.

- `cashFlows`  
  Cashflow matrix used during the backward induction stage.

  Initially this is typically empty or zero-filled apart from terminal values. As the algorithm works backward, exercise cashflows are written into this structure and later cashflows are cleared when early exercise occurs.

- `numPaths`  
  Number of simulated paths stored in the structure.

- `numTimeSteps`  
  Number of time steps / exercise dates represented in each path.

### Interpretation

`PathData` is the key object linking simulation and pricing. It allows the backward induction stage to work directly with realised simulated paths and record the resulting optimal exercise pattern.

---

## `SimulationResult`

Stores the final pricing summary returned by `price(...)`.

```cpp
struct SimulationResult {
    double optionValue          = 0.0;
    double standardError        = 0.0;
    double europeanValue        = 0.0;
    double earlyExercisePremium = 0.0;
    int    numPaths             = 0;
    int    numExerciseDates     = 0;
};
```

### Purpose

`SimulationResult` packages the outputs of the pricing run into a single object suitable for downstream reporting, validation, or comparison.

### Fields

- `optionValue`  
  Estimated American option value produced by Longstaff–Schwartz backward induction.

- `standardError`  
  Monte Carlo standard error of the estimated option value. This provides a measure of sampling uncertainty.

- `europeanValue`  
  European benchmark value computed from the same simulated framework, typically by discounting terminal payoffs without early exercise.

- `earlyExercisePremium`  
  Difference between the American estimate and the European benchmark:
  ```math
  \text{earlyExercisePremium} = \text{optionValue} - \text{europeanValue}.
  ```

- `numPaths`  
  Number of paths used in the run.

- `numExerciseDates`  
  Number of exercise dates / time steps used in the run.

### Interpretation

This structure distinguishes between the American price itself and related diagnostic quantities. In particular, including both the European benchmark and the early-exercise premium makes it easier to interpret whether early exercise is materially valuable in a given experiment.

---

## Summary

Taken together, these structures separate three different layers of the implementation:

- `LSMConfig` specifies how the computation should be run,
- `PathData` stores the simulated and pathwise exercise data needed by the algorithm,
- `SimulationResult` stores the final statistical summary returned to the caller.

