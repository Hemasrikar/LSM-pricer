---
title: Tests of the Project
layout: default
nav_order: 7
has_toc: true
---

## Unit Testing

Unit tests are written using the [Catch2](https://github.com/catchorg/Catch2) framework and cover all major components of the LSM pricer.

---

## Basis Functions Testing

Tests cover all classes in `lsm::core` related to basis function construction and evaluation.

#### Running the Tests
```bash
g++ -std=c++17 -I. test_basis_functions.cpp basis_functions.cpp -o test_basis && ./test_basis
```

#### Test Cases and Summary

| Class | Tested on |
|---|---|
| `ConstantBasis` | Returns `1.0` for all inputs and `name()` format check |
| `MonomialBasis` | Powers 0 to 5, negative/zero inputs, and error throw on negative power |
| `LaguerrePolynomial` | Output check for orders 0, 1, 3, 4 and three-term recurrence consistency up to order 8. Negative input clamping and error throw on negative order |
| `BasisSet` | `makeLaguerreSet` and `makeMonomialSet` structure and values. Clear and rebuild check and error throw on `numTerms < 1` |
| Polymorphism | Virtual dispatch through `BasisFunction*` for all three concrete types |

- **Recurrence consistency** — verifies the three-term relation `(n+1) * L_{n+1}(x) = (2n+1-x) * L_n(x) - n * L_{n-1}(x)` holds across orders 2–8
- **Negative input clamping** — confirms `LaguerrePolynomial::evaluate(-x) == evaluate(0)` for all tested orders
- **Clear and rebuild** — ensures calling a factory method twice does not accumulate elements
- **Virtual dispatch** — exercises the `BasisFunction` interface directly via `std::unique_ptr<BasisFunction>`

---

## Black-Scholes Pricer Testing

Tests cover the analytical European option pricer in `bs_pricer`.

#### Running the Tests
```bash
g++ -std=c++17 -I. test_bs_pricer.cpp bs_pricer.cpp -o test_bs && ./test_bs
```

#### Test Cases and Summary

| Test | Tested on |
|---|---|
| Known result | Call and put prices match reference values for ATM option (S=K=100, T=1, r=0.05, σ=0.2) to 4 d.p. |
| Put-call parity | `C - P = S - K * exp(-rT)` holds to within 0.01 |
| Non-negativity | Prices are ≥ 0 across a 3×3 grid of spot and strike combinations |
| Volatility monotonicity | Both call and put prices increase strictly with σ at three volatility levels |
| Spot monotonicity | Call price increases and put price decreases as spot rises |
| Near-expiry intrinsic | Price converges to intrinsic value as `T → 0` for deep ITM call, deep OTM call, and deep ITM put |
| Lower bound | Call price satisfies `C ≥ max(S - K * exp(-rT), 0)` for all tested spot values |

---

## OLS Regression Testing

Tests cover the regression utilities in `lsm::engine` used to estimate continuation values.

#### Running the Tests
```bash
g++ -std=c++17 -I. -I/usr/include/eigen3 test_OLS_regression.cpp basis_functions.cpp ols_regression.cpp mc_paths.cpp option_payoff.cpp underlying_sde.cpp -o test_ols && ./test_ols
```

#### Test Cases and Summary

| Function | Tested on |
|---|---|
| `buildDesignMatrix` (Monomial) | Matrix shape: 3 ITM rows × 4 columns for 5-path input with Monomial basis |
| `buildDesignMatrix` (Laguerre) | Matrix shape: 3 ITM rows × 4 columns for 5-path input with Laguerre basis |
| `generatePaths` | Output dimensions `[N][n+1]` and initial value `S0` on all paths |
| `Ols_regression` | Reproduction of the Longstaff-Schwartz (2001) paper Table 2 continuation values at `t=2` |

- **Paper reproduction** — uses the 8-path toy example from Longstaff & Schwartz (2001), pp. 116–117. The test verifies that OTM paths receive continuation value 0.0, and that ITM paths match the published values (e.g. path 4: `0.1176`, path 6: `0.1520`) to within 1%
- **Design matrix shape** — confirms only ITM rows are included and the number of columns equals the basis size

---

## Option Payoff Testing

Tests cover `Put_payoff` and `Call_payoff` in `lsm::core`.

#### Running the Tests
```bash
g++ -std=c++17 -I. test_option_payoff.cpp option_payoff.cpp -o test_payoff && ./test_payoff
```

#### Test Cases and Summary

| Class | Tested on |
|---|---|
| `Put_payoff` | Payoff values: ITM (`S < K`), ATM (`S = K`), OTM (`S > K`) |
| `Put_payoff` | `InTheMoney`: returns `true` strictly below strike, `false` at and above |
| `Call_payoff` | Payoff values: OTM (`S < K`), ATM (`S = K`), ITM (`S > K`) |
| `Call_payoff` | `InTheMoney`: returns `false` at and below strike, `true` strictly above |
| `OptionPayoff` interface | Virtual destructor exercised via `std::unique_ptr<OptionPayoff>` for both derived types; `strike()` accessor checked |

---
## Underlying SDE Testing

Tests cover `GeometricBrownianMotion`, `JumpDiffusionProcess`, and `RNG` in `lsm::core`.

#### Running the Tests
```bash
g++ -std=c++17 -I. test_underlying_sde.cpp underlying_sde.cpp -o test_sde && ./test_sde
```

#### Test Cases and Summary

| Class | Test # | Tested on |
|---|---|---|
| `GeometricBrownianMotion` | 1 | Constructor throws `std::invalid_argument` for `sigma < 0` |
| `GeometricBrownianMotion` | 2 | `stepWithNormal` throws for `dt ≤ 0` |
| `GeometricBrownianMotion` | 3 | `stepWithNormal` matches exact formula `S * exp((r - σ²/2)dt + σ√dt · z)` |
| `GeometricBrownianMotion` | 4 | Deterministic evolution (`σ = 0`) reduces to `S * exp(r·dt)` regardless of `z` |
| `GeometricBrownianMotion` | 5 | `simulatePath` returns vector of size `n+1` with `path[0] == S0` |
| `GeometricBrownianMotion` | 6 | `simulatePath` throws for `T ≤ 0` |
| `GeometricBrownianMotion` | 17 | `simulatePath` is fully deterministic when `σ = 0`, each step matching `S0 * exp(r·dt·i)` |
| `GeometricBrownianMotion` | 19 | Getters `r()` and `sigma()` return values passed to constructor |
| `JumpDiffusionProcess` | 7 | Constructor throws for `sigma < 0` |
| `JumpDiffusionProcess` | 8 | Constructor throws for `lambda < 0` |
| `JumpDiffusionProcess` | 9 | `step` throws for negative state `s < 0` |
| `JumpDiffusionProcess` | 10 | `step` returns `0.0` when state is zero (absorbing state) |
| `JumpDiffusionProcess` | 11 | `step` throws for `dt ≤ 0` |
| `JumpDiffusionProcess` | 12 | `stepWithNormal` returns `0.0` for `s ≤ 0` |
| `JumpDiffusionProcess` | 13 | `stepWithNormal` throws for `dt ≤ 0` |
| `JumpDiffusionProcess` | 14 | `stepWithNormal` with `lambda = 0` matches GBM exact formula |
| `JumpDiffusionProcess` | 15 | Jump almost surely occurs at `lambda = 1000`, `stepWithNormal` returns `0.0` |
| `JumpDiffusionProcess` | 16 | Fully deterministic when `sigma = 0` and `lambda = 0` |
| `JumpDiffusionProcess` | 18 | Getters `r()`, `sigma()`, `lambda()` return values passed to constructor |
| `JumpDiffusionProcess` | 20 | `step` returns positive value for valid inputs with no jumps (`lambda = 0`) |
| `JumpDiffusionProcess` | 22 | `step` returns `0.0` when jump probability is effectively 1 (`lambda = 1000`) |
| `RNG` | 21 | Default constructor (unseeded) produces finite normal samples |

---