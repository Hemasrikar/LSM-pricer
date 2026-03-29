---
title: LSM Pricer: Simulate Paths
layout: default
parent: LSM Pricer
nav_order: 1
has_toc: true
---

**Namespace:** `lsm::engine` | **Header:** `lsm_pricer.hpp`

Documents the internal method used by `LSMPricer` to generate simulated paths.

---

## Method

```cpp
PathData simulatePaths(double S0) const;
```

Generates the Monte Carlo sample paths used by the pricing engine.

This method does not itself implement the stochastic simulation scheme. Instead, it serves as an interface between the `LSMPricer` class and the standalone engine-level routine responsible for path generation.

---

## Role of the Method

The `LSMPricer` object already stores:

- the stochastic process `*process`, which determines the asset dynamics,
- and the configuration `config`, which determines the simulation settings.

Rather than requiring these ingredients to be passed again whenever paths are needed, `simulatePaths(double S0)` uses the pricer’s stored state and delegates path generation to the engine-layer simulation routine.

This has two advantages:

1. it guarantees that the paths used for pricing are consistent with the model stored inside the pricer,
2. it keeps the higher-level pricing routine `price(double S0)` clean and modular.

---

## Implementation Logic

### 1. Read the initial asset value

The method takes the initial asset price `S0` as input.

This value provides the starting point for every simulated path.

### 2. Use the stored model and configuration

The method uses:

- `*process`, representing the underlying stochastic process,
- `config`, containing the simulation controls such as path count, maturity, and exercise-date discretisation.

These determine the probabilistic law of the simulated paths and the discretisation grid on which the paths are generated.

### 3. Delegate to the simulation engine

The method calls the standalone path-generation routine:

```cpp
return lsm::engine::simulatePaths(S0, *process, config);
```

This routine returns a `PathData` object containing the simulated asset-price matrix and the associated storage for cashflows.

### 4. Return the generated path data

The method returns the resulting `PathData` directly, without modification.

---

## Returned Value

```cpp
PathData
```

The returned object contains:

- `paths[i][t]`, the simulated asset price on path `i` at time index `t`,
- `cashFlows[i][t]`, a pathwise cashflow container used later during backward induction,
- `numPaths`, the number of simulated paths,
- `numTimeSteps`, the number of time steps / exercise dates.

---

## Interpretation

This method does not perform any valuation logic by itself. Its purpose is to ensure that the simulation stage is correctly tied to the internal state of the `LSMPricer` object.

In particular, it improves encapsulation by making path generation an internal operation of the pricer rather than something that must be orchestrated externally for every pricing call.

As a result, higher-level methods such as `price(double S0)` can treat path generation as a reliable, reusable subroutine.