---
title: LSM Pricer: Compute Option Value
layout: default
parent: LSM Pricer
nav_order: 3
has_toc: true
---

**Namespace:** `lsm::engine` | **Header:** `lsm_pricer.hpp`

Documents the method that converts discounted pathwise cashflows into the final pricing summary.

---

## Method

```cpp
lsm::engine::SimulationResult computeOptionValue(
    const std::vector<double>& pv,
    double europeanValue,
    int N,
    int T) const;
```

Converts the pathwise present values produced by backward induction into the final pricing output.

This method computes:

- the Monte Carlo estimate of the American option value,
- the empirical variance of the pathwise estimator,
- the standard error of the Monte Carlo estimate,
- the early-exercise premium relative to a European benchmark.

It packages these quantities into a `SimulationResult` object.

---

## Role of the Method

The backward induction stage determines the realised discounted payoff on each path. However, those pathwise values are not yet the final pricing output.

`computeOptionValue(...)` performs the statistical aggregation step that converts the pathwise present values into a price estimate and associated diagnostics.

It is therefore the final stage of the pricing pipeline.

---

## Inputs

- `pv`: Vector of discounted pathwise present values returned by `backwardInduction(...)`.

- `europeanValue`: European benchmark value computed from the same simulation framework.

- `N`: Number of simulated paths.

- `T`: Number of exercise dates / time steps.

---

## Computation

### 1. Compute the Monte Carlo mean

The American option value estimate is the sample mean of the discounted pathwise values:

```math
\widehat{V} = \frac{1}{N}\sum_{i=1}^{N} pv_i.
```

This quantity is stored as `optionValue`.

---

### 2. Compute the mean square

The method also computes the sample mean of squared present values:

```math
\frac{1}{N}\sum_{i=1}^{N} pv_i^2.
```

This is used to construct the variance estimate.

---

### 3. Compute the variance estimate

The variance of the pathwise estimator is computed as

```math
\max\left(0,\; \frac{1}{N}\sum_{i=1}^{N} pv_i^2 - \widehat{V}^2\right).
```

The `max(0.0, ...)` guard is included to prevent small negative values caused by floating-point roundoff.

This does not change the underlying statistical idea; it simply ensures numerical robustness.

---

### 4. Compute the standard error

The Monte Carlo standard error is then computed as

```math
\sqrt{\frac{\text{variance}}{N}}.
```

This measures the sampling uncertainty in the estimated option value.

---

### 5. Compute the early-exercise premium

The method computes

```math
\text{earlyExercisePremium} = \text{optionValue} - \text{europeanValue}.
```

This quantity measures the incremental value of early exercise relative to the European benchmark.

---

### 6. Populate the result object

The method fills a `SimulationResult` object with:

- `optionValue`,
- `standardError`,
- `europeanValue`,
- `earlyExercisePremium`,
- `numPaths = N`,
- `numExerciseDates = T`.

It then returns the populated result.

---

## Returned Value

```cpp
SimulationResult
```

The returned object summarises the pricing run and is the final output of `price(double S0)`.

---

## Interpretation

`computeOptionValue(...)` is the final statistical aggregation stage of the Longstaff–Schwartz pricing engine.

Where `backwardInduction(...)` determines the economically realised payoff on each path, `computeOptionValue(...)` converts those individual pathwise outcomes into a Monte Carlo estimate and a set of diagnostic quantities suitable for reporting, benchmarking, and validation.

Including both the European benchmark and the early-exercise premium makes the output more interpretable and more useful for comparing model behaviour across contracts and parameter settings.