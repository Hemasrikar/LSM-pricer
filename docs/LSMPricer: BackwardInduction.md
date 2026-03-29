---
title: LSM Pricer: Backward Induction
layout: default
parent: LSM Pricer
nav_order: 2
has_toc: true
---

**Namespace:** `lsm::engine` | **Header:** `lsm_pricer.hpp`

Documents the core Longstaff–Schwartz backward induction procedure implemented by `LSMPricer`.

---

## Method

```cpp
std::vector<double> backwardInduction(lsm::engine::PathData& data) const;
```

Performs the backward induction stage of the Longstaff–Schwartz Least Squares Monte Carlo algorithm.

The method works backward through the exercise dates, estimates continuation values by regression, compares continuation value with immediate exercise value, and determines the realised exercise policy path by path.

It returns a vector containing one discounted present value for each simulated path.

---

## Role of the Method

This method is the core of the American option pricer.

For a European option, valuation only requires discounting terminal payoffs. For an American option, however, the holder may exercise early, so the value at an intermediate date depends on whether immediate exercise is preferable to continuation.

The Longstaff–Schwartz method addresses this by estimating the continuation value through regression on basis functions. `backwardInduction(...)` is the implementation of that idea.

---

## Main Idea

At maturity, there is no exercise choice remaining, so the value on each path is simply the terminal payoff.

At earlier times, for paths that are in the money, the method compares:

- the payoff obtained by exercising immediately,
- the estimated continuation value from holding the option.

If immediate exercise is larger, the option is exercised on that path at that time. Otherwise, the path continues.

This produces a realised stopping time and realised cashflow for each path.

---

## Implementation Outline

### 1. Read dimensions and define discounting

The method begins by reading:

- `numPaths = data.numPaths`,
- `numTimes = data.numTimeSteps`.

It then defines the time step

```math
dt = \frac{\text{config.maturity}}{\text{numTimes}}
```

and the one-step discount factor

```math
\exp(-r\,dt),
```

where `r = config.riskFreeRate`.

These quantities are used throughout the backward recursion.

---

### 2. Initialise terminal cashflows

At maturity, each path is assigned the terminal payoff

```math
\text{payoff}(S_T).
```

The implementation initialises:

- the current realised cashflow on each path with the terminal payoff,
- the exercise time of each path with the maturity time index,
- `data.cashFlows[i][numTimes]` with the same terminal payoff.

Thus, before any backward exercise decisions are made, every path is treated as if exercise occurs at maturity.

---

### 3. Move backward through exercise dates

The method then loops backward from time index `numTimes - 1` down to `1`.

At each time index `t`, it evaluates the immediate exercise payoff on every path and identifies the in-the-money paths, i.e. those for which the immediate payoff is strictly positive.

Only these in-the-money paths are relevant for the exercise decision, since out-of-the-money paths would never be exercised immediately.

---

### 4. Estimate continuation values by regression

For the current time step, the method calls the regression routine to estimate continuation values using the basis functions stored in the pricer.

Conceptually, the continuation value is approximated as

```math
C_t(S_t) \approx \sum_{j=1}^{m} \beta_j \phi_j(S_t),
```

where:

- `\phi_j` are the basis functions,
- `\beta_j` are regression coefficients estimated from simulated data.

The regression uses:

- the asset prices at time `t`,
- the future pathwise cashflows currently stored,
- the in-the-money indicator,
- the one-step discount factor,
- and the basis set.

This is the step that replaces the unknown conditional expectation in the optimal stopping problem with a tractable regression approximation.

---

### 5. Evaluate fitted continuation values

Once the regression coefficients have been obtained, the method computes the fitted continuation value on each in-the-money path via

```math
\widehat{C}_t(S_t) = \sum_{j=1}^{m} \beta_j \phi_j(S_t).
```

Before evaluating this sum, the implementation checks that the number of regression coefficients is compatible with the number of available basis functions.

If the coefficient vector is larger than the available basis dimension, the method throws a `std::runtime_error`. This protects against invalid regression output and avoids silent out-of-bounds access.

---

### 6. Compare exercise with continuation

For each in-the-money path, the method compares:

- immediate exercise value,
- fitted continuation value.

If immediate exercise exceeds the continuation value, the option is exercised immediately on that path.

Otherwise, the path is left unchanged and continues to its previously stored future exercise time.

This comparison is the pathwise implementation of the American optimal stopping rule.

---

### 7. Update the realised exercise decision

When exercise occurs on a path at time `t`, the implementation:

- replaces the stored realised cashflow with the immediate payoff,
- updates the stored exercise time to `t`,
- records the exercise payoff in `data.cashFlows[i][t]`,
- sets all later cashflows on that path to zero.

This last step is important: once the option has been exercised, no later payoff can occur on that path.

If exercise is not optimal, the previously stored future cashflow and exercise time remain unchanged.

---

### 8. Discount realised cashflows to time 0

After the backward loop is complete, each path has:

- a realised exercise time,
- and a realised exercise cashflow.

The method discounts this realised cashflow back to time 0.

For path `i`, the present value is

```math
PV_i = \text{cashflow}_i \exp(-r\,dt\,\text{exerciseTime}_i).
```

These discounted pathwise values are collected into a vector and returned.

---

## Returned Value

```cpp
std::vector<double>
```

Entry `i` of the returned vector is the discounted realised option payoff on path `i`.

This vector is the direct Monte Carlo input for the final aggregation stage.

---

## Interpretation

`backwardInduction(...)` is the computational heart of the Longstaff–Schwartz algorithm.

Its purpose is not merely to move backward through time, but to approximate the continuation region of the American option by regression and then enforce the resulting exercise rule on each simulated path.

From a mathematical perspective, it is the step that turns the American pricing problem into a regression-based Monte Carlo method.

From a software perspective, it is the step that transforms simulated path data into economically meaningful realised pathwise payoffs.