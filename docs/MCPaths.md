---
title: MC Paths
layout: default
parent: OLS Regression
nav_order: 1
has_toc: true
---

**Namespace:** `lsm::engine` | **Header:** `lsm_pricer.hpp`

The `simulatePaths` free function generates the full matrix of Monte Carlo sample paths used by the pricing engine. It is called internally by `LSMPricer::simulatePaths()`.

---

## Function Signature

```cpp
PathData simulatePaths(
    double S0,
    const lsm::core::StochasticProcess& process,
    const lsm::engine::LSMConfig& config
);
```

---

## Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `S0` | `double` | Initial asset price |
| `process` | `const StochasticProcess&` | Stochastic process defining the dynamics of underlying asset|
| `config` | `const LSMConfig&` | Simulation settings: path count, maturity, exercise dates, RNG seed |

---

## Returned Value

A `PathData` object containing:

- `paths[i][t]` — simulated asset price on path `i` at time index `t`, for `t = 0, …, T`
- `cashFlows[i][t]` — zero-initialised cashflow storage, populated later during backward induction
- `numPaths` — number of paths simulated (rounded down to the nearest even number if antithetic is enabled)
- `numTimeSteps` — number of time steps `T = config.numExerciseDates`

---

## Plain Monte Carlo

When `config.useAntithetic = false`, each path is simulated independently:

```
path[i][0] = S0
path[i][t] = process.step(path[i][t-1], dt, rng)   for t = 1, …, T
```

---

## Antithetic Variance Reduction

When `config.useAntithetic = true`, paths are generated in $N/2$ pairs. For each pair, the same standard normal draw $z$ drives path $i$ and its negation $-z$ drives path $i + N/2$:

```
path[i][t]        = process.stepWithNormal(path[i][t-1],        dt,  z, rng)
path[i + N/2][t]  = process.stepWithNormal(path[i + N/2][t-1],  dt, -z, rng)
```

This produces negatively correlated path pairs, reducing Monte Carlo variance without requiring additional process evaluations.

---

## Notes

- The RNG is seeded from `config.rngSeed`, making runs reproducible.
- Throws `std::invalid_argument` if `config.numPaths <= 0` or `config.numExerciseDates <= 0`.
