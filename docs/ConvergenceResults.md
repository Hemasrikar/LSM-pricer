---
title: Convergence Results
layout: default
nav_order: 3
parent: Benchmark and Convergence
has_toc: true
---

**Namespace:** `lsm::analysis` | **Header:** `convergence_analyser.hpp`

This page presents the numerical results produced by `ConvergenceAnalyser`. Each experiment varies one parameter at a time and compares the LSM price against the Finite Difference or Black-Scholes benchmark.

---

## Benchmark Comparison

### Prices: BS vs FD vs LSM

![Benchmark prices across parameter grid]({{ '/plots/conv_benchmark_prices.png' | relative_url }})

The LSM American price closely tracks the FD American price across all 12 parameter combinations, and consistently exceeds the BS European price — confirming the early exercise premium is being captured correctly.

---

### Early Exercise Premium

![Early exercise premium across parameter grid]({{ '/plots/conv_benchmark_eep.png' | relative_url }})

The early exercise premium (LSM − BS) is largest for deep in-the-money, low-volatility, short-maturity puts, where the benefit of early exercise is most immediate.

---

## Exercise Dates Convergence

### Price vs Number of Exercise Dates

![LSM price vs exercise dates]({{ '/plots/conv_dates_price.png' | relative_url }})

The LSM price converges quickly as exercise dates increase, stabilising by around $M = 20$; adding more dates beyond this offers negligible accuracy improvement.

---

## Polynomial Order Convergence

### Price vs Polynomial Order

![LSM price vs polynomial order]({{ '/plots/conv_order_price.png' | relative_url }})

The price improves from order 1 to 3 and then plateaus, indicating that a Laguerre basis of order 3 is sufficient to approximate the continuation value for this contract.

---

## Finite Difference Convergence

### FD Price vs Time Steps

![FD price vs time steps]({{ '/plots/conv_fd_price.png' | relative_url }})

The FD price converges smoothly to its limit as the number of time steps increases, with most of the gain achieved by around 100 steps.

### Early Exercise Premium vs FD Time Steps

![Early exercise premium vs FD time steps]({{ '/plots/conv_fd_eep.png' | relative_url }})

The FD early exercise premium stabilises at approximately 0.64, consistent with the LSM benchmark result, confirming the two methods agree on the value of early exercise.

---

## Seed Stability

### Price Distribution Across Seeds

![Price distribution across 10 seeds]({{ '/plots/conv_seed_boxplot.png' | relative_url }})

The LSM price is stable across 10 random seeds, with the interquartile range confined to a narrow band around 4.48, demonstrating that the estimator variance is well-controlled.

---

## Runtime Scaling

### Time vs Number of Paths

![Runtime vs path count]({{ '/plots/conv_timing_paths.png' | relative_url }})

Runtime scales as $O(N^{0.96})$, very close to the theoretical $O(N)$, confirming the implementation scales linearly with path count as expected.

### Time vs Number of Exercise Dates

![Runtime vs exercise dates]({{ '/plots/conv_timing_exercisedates.png' | relative_url }})

Runtime scales as $O(M^{1.05})$, consistent with the theoretical $O(M)$ backward induction loop.

### Time vs Polynomial Order

![Runtime vs polynomial order]({{ '/plots/conv_timing_order.png' | relative_url }})

Runtime increases approximately linearly with polynomial order, reflecting the growing cost of constructing and solving the larger design matrix at each time step.

### Time vs FD Time Steps

![Runtime vs FD time steps]({{ '/plots/conv_timing_timesteps.png' | relative_url }})

FD runtime scales as $O(n^{0.97})$, closely matching the expected linear scaling with the number of time steps in the grid.
