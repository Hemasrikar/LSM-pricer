---
layout: home
title: LSM American Option Pricer
---


## Module Overview
 
| Module | Files | Role |
|---|---|---|
| [Underlying SDE](pages/UnderlyingSDE.md) | `underlying_sde.hpp/.cpp` | Stochastic processes (GBM, jump-diffusion) and RNG |
| [Option Payoff](pages/OptionPayoff.md) | `option_payoff.hpp/.cpp` | Payoff functions and ITM conditions |
| [Basis Functions](pages/BasisFunction.md) | `basis_functions.hpp/.cpp` | Polynomial and Laguerre basis sets for regression |
| [Monte Carlo Paths](pages/MCPaths.md) | `mc_paths.hpp/.cpp` | Path simulation matrix |
| [OLS Regression](pages/OLSRegression.md) | `ols_regression.hpp/.cpp` | Least-squares regression via Eigen QR decomposition |
| [LSM Pricer](pages/LSM Pricer.md) | `lsm_pricer.hpp/.cpp` | Core pricing engine — simulation, backward induction, statistics |
| [BS Pricer](pages/BSPricer.md) | `bs_pricer.hpp/.cpp` | Black–Scholes closed-form European benchmark |
| [Convergence Analyser](pages/ConvergenAnalyzer.md) | `convergence_analyser.hpp/.cpp` | Benchmarking and convergence studies |
 
---