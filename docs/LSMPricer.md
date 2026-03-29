---
title: LSM Pricer
layout: default
nav_order: 5
has_children: true
---

**Namespace:** `lsm::engine` | **Header:** `lsm_pricer.hpp`

Implements the pricing stage of the Longstaff–Schwartz Least Squares Monte Carlo method for American-style options.

The class combines:

- a stochastic process model for the underlying asset,
- an option payoff object,
- a regression basis set,
- and a pricing configuration,

to produce a Monte Carlo estimate of the American option value.

Unlike the path-generation layer, which is responsible only for simulating trajectories of the underlying, `LSMPricer` is responsible for the **valuation logic** itself. In particular, it:

1. obtains simulated asset paths,
2. computes terminal and continuation values,
3. determines optimal early-exercise decisions by backward induction,
4. discounts realised pathwise cashflows to time 0,
5. aggregates the resulting present values into a final pricing summary.

---

## Core Interface

```cpp
class LSMPricer {
public:
    LSMPricer(lsm::core::StochasticProcess* process,
              lsm::core::OptionPayoff* payoff,
              lsm::core::BasisSet* basis,
              const lsm::engine::LSMConfig& config);

    lsm::engine::SimulationResult price(double S0);

private:
    lsm::core::StochasticProcess* process;
    lsm::core::OptionPayoff* payoff;
    lsm::core::BasisSet* basis;

    lsm::engine::LSMConfig config;

    lsm::engine::PathData simulatePaths(double S0) const;

    std::vector<double> backwardInduction(lsm::engine::PathData& data) const;

    lsm::engine::SimulationResult computeOptionValue(
        const std::vector<double>& pv,
        double europeanValue,
        int N,
        int T) const;
};
```

---

## Class Role

`LSMPricer` acts as the central pricing engine of the Longstaff–Schwartz framework.

Its responsibility is not to define the model, payoff, or basis functions themselves. Instead, it coordinates these components into a complete valuation procedure. This separation is important:

- the stochastic process determines how paths evolve,
- the payoff determines exercise value,
- the basis set determines how continuation values are approximated,
- the pricer determines when exercise is optimal and how the final estimate is computed.

Accordingly, the class encapsulates the **optimal stopping** part of the algorithm rather than the simulation mechanics alone.

---

## Constructor

```cpp
LSMPricer(lsm::core::StochasticProcess* process,
          lsm::core::OptionPayoff* payoff,
          lsm::core::BasisSet* basis,
          const lsm::engine::LSMConfig& config);
```

Constructs an `LSMPricer` from:

- a stochastic process for the underlying asset,
- a payoff object,
- a regression basis set,
- and a simulation / pricing configuration.

The constructor stores pointers to the supplied process, payoff, and basis objects, together with a copy of the configuration.

This design allows the pricer to reuse the same model components throughout the full valuation pipeline without repeatedly passing them between internal routines.

---

## Public Method

```cpp
lsm::engine::SimulationResult price(double S0);
```

Runs the complete pricing procedure for initial asset value `S0`.

At a conceptual level, `price(double S0)` performs the following sequence:

1. call `simulatePaths(S0)` to generate sample paths under the stored model and configuration,
2. compute a European benchmark from discounted terminal payoffs,
3. call `backwardInduction(...)` to determine the realised American cashflow on each path,
4. call `computeOptionValue(...)` to aggregate those pathwise present values into a final Monte Carlo estimate.

The return value is a `SimulationResult` object containing:

- the American option value estimate,
- the Monte Carlo standard error,
- the European benchmark,
- the early-exercise premium,
- and the run metadata.

See: [LSMPricer: Price](/docs/LSMPricer: Price.md)

---

## Pricing Workflow

### 1. Top-level pricing method

The method `price(double S0)` is the public entry point of the pricing engine. It coordinates the full Longstaff–Schwartz valuation pipeline.

See: [LSMPricer: Price](/docs/LSMPricer: Price.md)

### 2. Path Simulation

The method `simulatePaths(double S0)` generates Monte Carlo sample paths using the stored stochastic process and configuration.

See: [LSMPRicer: SimulatPaths](/docs/LSMPRicer: SimulatPaths.md)

### 3. Backward Induction

The method `backwardInduction(PathData& data)` implements the Longstaff–Schwartz regression-based optimal stopping procedure.

See: [LSMPricer: BackwardInduction](/docs/LSMPricer: BackwardInduction.md)

### 4. Price Aggregation

The method `computeOptionValue(...)` converts discounted pathwise realised cashflows into the final pricing summary.

See: [LSMPricer: Optionvalue](/docs/LSMPricer: Optionvalue.md)

---

## Supporting Data Structures

The class relies on the following supporting structures:

- `LSMConfig`
- `PathData`
- `SimulationResult`

See: [LSMPricer DataStructures](/docs/LSMPricer: data structures.md)

---

## Interpretation

`LSMPricer` is the computational core of the Longstaff–Schwartz implementation.

The key mathematical role of the class is to approximate the continuation value at each exercise date by regression and then compare that continuation value with immediate exercise value. In this way, the class transforms the American pricing problem from a high-dimensional optimal stopping problem into a tractable Monte Carlo regression procedure.
