---
title: Main
layout: default
nav_order: 1
has_toc: true
---

**File:** `main.cpp` | **Dependencies:** `lsm_pricer.hpp`, `ols_regression.hpp`, `basis_functions.hpp`, `option_payoff.hpp`, `underlying_sde.hpp`, `Eigen/Dense`

This is the entry point for the LSM pricer. It sets up the parameters, runs the simulation, prints the results, and exports everything to CSV files so you can plot and analyse the output.

---

## Structure Overview

The file is broken into four parts, each in its own namespace, plus `main()` which ties everything together:

| Section | Namespace | What it does |
|---|---|---|
| Configuration | `cfg` | All the market and simulation parameters in one place |
| Utilities | `utils` | Small helpers for evaluating basis functions and opening files |
| CSV Export | *(free function)* | `export_csv(...)` writes all the output files |
| Entry point | *(global)* | `main()` runs the pricer and calls the export |

---

## Configuration

### `cfg` namespace

All the parameters are defined as `constexpr` values here. If needed to run the simulation with different values for analysis, you just change it here and recompile.

```cpp
// Market parameters
constexpr double initial_spot   = 1.0;
constexpr double risk_free_rate = 0.02;
constexpr double volatility     = 0.15;
constexpr double strike         = 1.1;
constexpr double maturity       = 6.0;

// Simulation parameters
constexpr int num_paths          = 50;
constexpr int num_exercise_dates = 120;

// Regression parameters
constexpr int basis_order  = 3;
constexpr int grid_points  = 300;

// Output
constexpr const char* output_dir = "../csv_output";
```

> **Note:** `num_paths` is set to 50 here, which is intentionally small. The main goal of this run is to export the simulation data in a way that's easy to visualise. We clearly can see individual paths and exercise decisions. If an accurate price needed, increase this up to something like 50,000+.

---

## Utilities

### `utils::eval_basis`

A small helper that evaluates a fitted continuation value at a point `x`, given a vector of regression coefficients and a `BasisSet`.

```cpp
double eval_basis(const std::vector<double>& c,
                  const lsm::core::BasisSet& b,
                  double x);
```

It just computes `Σ c[i] * basis[i](x)`. This is used when building the regression grid for the CSV output. We evaluate the fitted curve over a fine grid of stock prices so you can plot what the continuation value looks like.

---

### `utils::open_file`

Opens a file in `cfg::output_dir` and sets fixed-precision formatting on it. Throws a `std::runtime_error` if the file can't be opened, rather than silently producing an empty file.

```cpp
std::ofstream open_file(const std::string& name);
```

---

### `export_csv`

```cpp
void export_csv(const lsm::engine::PathData& d,
                const lsm::engine::SimulationResult& res,
                double maturity,
                double strike,
                double risk_free_rate);
```

This function takes the `PathData` and `SimulationResult` from the pricer and writes everything to disk. It creates the output directory first and then produces six CSV files:

| File | Columns | What's in it |
|---|---|---|
| `paths.csv` | `t, path_0, path_1, ...` | The full simulated stock price paths over time |
| `paths_itm.csv` | `path_id, t, price, itm` | Same paths but with an ITM flag (`1` if the put is in the money at that step) |
| `payoff.csv` | `path_id, t, price, payoff` | The intrinsic put value `max(K - S, 0)` at every time step |
| `cashflows.csv` | `path_id, t, cashflow` | Only the non-zero exercise cash flows — so one entry per path at its exercise time |
| `exercise_times.csv` | `path_id, exercise_t, exercise_price, held_to_maturity` | When each path was exercised and at what price. `held_to_maturity = 1` means no early exercise happened |
| `regression_scatter.csv` | `x, discounted_cf, itm` | Penultimate step price vs discounted terminal payoff — useful for plotting what the regression is fitting |
| `regression_grid.csv` | `x, continuation_value, exercise_value` | The fitted continuation value curve and intrinsic value on a fine grid, for plotting the regression result |
| `price_summary.csv` | `label, value` | The headline numbers: American price, European price, early exercise premium, standard error |

**The regression grid**

For `regression_grid.csv`, we re-run the OLS fit at the penultimate time step using `buildDesignMatrix` and `buildYVector`, solve it with Eigen's `colPivHouseholderQr`, and then evaluate the resulting curve at `cfg::grid_points + 1` evenly spaced points between the min and max stock price at that step. The basis functions are evaluated on `x / strike` to match what `LSMPricer` does internally.

---

### `main()`

```cpp
int main();
```

The setup and execution flow is straightforward:

1. Build an `LSMConfig` from the `cfg` constants. Antithetic variates are turned off here so the exported paths are simpler to look at.
2. Build a `BasisSet` using `makeLaguerreSet(cfg::basis_order)`.
3. Create the `LSMPricer` with a `GeometricBrownianMotion` process and a `Put_payoff`.
4. Call `priceWithData(cfg::initial_spot)` — this runs the full simulation and returns both the `SimulationResult` and the `PathData` we need for the CSV export.
5. Print the key results to stdout.
6. Call `export_csv(...)` to write everything to disk.

The reason we use `priceWithData` rather than `price` is that `price` discards the internal path data after the simulation. Here we need to keep it for the CSV export, so `priceWithData` returns both.

The printed output looks like:
```
Longstaff-Schwartz American Put (S0=1, K=1.1, T=6 yrs)
American:  <value>
European:  <value>
Premium:   <value>
Std Err:   <value>
```

---

### Error Handling

The whole of `main()` is wrapped in a `try/catch(const std::exception&)` block. If anything goes wrong because of a bad config, a file that can't be opened, an exception from the engine, it then catches it, prints the error to `stderr`, and returns `EXIT_FAILURE`. Individual file opens in `export_csv` will throw via `utils::open_file` if the output directory doesn't exist or isn't writable.