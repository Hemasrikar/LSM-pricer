---
title: LSM Pricer: Price
layout: default
parent: LSM Pricer
nav_order: 4
has_toc: true
---

**Namespace:** `lsm::engine` | **Header:** `lsm_pricer.hpp`

Documents the main entry point of the pricing engine.

---

## Method

```cpp
lsm::engine::SimulationResult price(double S0);
```

Runs the complete Longstaff–Schwartz pricing procedure for an initial asset value `S0`.

This method coordinates all stages of the pricing pipeline:

1. path simulation,
2. European benchmark computation,
3. backward induction for optimal stopping,
4. aggregation of pathwise discounted payoffs.

It is the primary public interface of `LSMPricer`.

---

## Role of the Method

While other methods in the class implement individual components of the algorithm, `price(double S0)` is responsible for **executing the full valuation pipeline end-to-end**.

It ensures that:

- all paths are generated under the correct model,
- the European benchmark is computed consistently,
- backward induction is applied to the same paths,
- and the final result is packaged into a structured output.

In this sense, it acts as the **controller of the pricing workflow**.

---

## Implementation Breakdown

### 1. Simulate asset paths

The method first calls:

```cpp
PathData data = simulatePaths(S0);
```

This generates Monte Carlo sample paths using:

- the stored stochastic process,
- and the configuration parameters.

The returned `PathData` contains:

- the simulated asset paths,
- an initialised cashflow matrix,
- path and time dimensions.

---

### 2. Compute European benchmark

Before applying backward induction, the method computes a European benchmark value.

This is typically done by:

- evaluating the payoff at maturity on each path,
- discounting those terminal payoffs back to time 0,
- averaging across all paths.

Conceptually, this corresponds to:

```math
V_{\text{Euro}} = \frac{1}{N} \sum_{i=1}^N \text{payoff}(S_T^{(i)}) \, e^{-rT}.
```

This benchmark is useful for:

- validating the implementation,
- computing the early-exercise premium,
- and understanding how much value is added by early exercise.

---

### 3. Perform backward induction

The method then calls:

```cpp
std::vector<double> pv = backwardInduction(data);
```

This step:

- determines the optimal stopping time on each path,
- computes the realised exercise payoff,
- discounts that payoff back to time 0.

The result is a vector `pv` where:

- `pv[i]` is the discounted realised payoff on path `i`.

This vector represents the **pathwise American valuation outcomes**.

---

### 4. Aggregate results

Finally, the method calls:

```cpp
return computeOptionValue(pv, europeanValue, data.numPaths, data.numTimeSteps);
```

This step:

- computes the Monte Carlo mean,
- estimates variance and standard error,
- computes the early-exercise premium,
- packages everything into a `SimulationResult`.

---

## Returned Value

```cpp
SimulationResult
```

The returned object contains:

- `optionValue` — estimated American option value,
- `standardError` — Monte Carlo standard error,
- `europeanValue` — European benchmark,
- `earlyExercisePremium` — value of early exercise,
- `numPaths` — number of paths used,
- `numExerciseDates` — number of time steps.

---

## Full Algorithm (Conceptual Flow)

The method implements the following algorithm:

1. Generate simulated paths:
   ```math
   S^{(i)}_t, \quad i = 1, \dots, N
   ```

2. Compute European value:
   ```math
   V_{\text{Euro}} = \frac{1}{N} \sum_{i=1}^N \text{payoff}(S_T^{(i)}) e^{-rT}
   ```

3. Apply backward induction:
   - estimate continuation values via regression,
   - compare with immediate exercise,
   - determine stopping time for each path.

4. Compute discounted realised payoffs:
   ```math
   PV_i
   ```

5. Estimate American value:
   ```math
   \widehat{V} = \frac{1}{N} \sum_{i=1}^N PV_i
   ```

---

## Interpretation

`price(double S0)` is the **top-level implementation of the Longstaff–Schwartz algorithm**.

It brings together:

- stochastic simulation,
- regression-based continuation estimation,
- optimal stopping logic,
- and statistical aggregation.

From a software perspective, it provides a clean abstraction: the user supplies an initial price `S0`, and the method returns a fully computed pricing result.

From a quantitative perspective, it encapsulates the transformation of a high-dimensional optimal stopping problem into a tractable Monte Carlo regression procedure.

---

## Notes

- The method ensures consistency by using the same simulated paths for both:
  - European valuation,
  - and American backward induction.

- The separation into `simulatePaths`, `backwardInduction`, and `computeOptionValue` improves:
  - readability,
  - testability,
  - and modularity of the implementation.