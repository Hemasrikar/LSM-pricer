---
title: LSM American Option Pricer
layout: default
nav_order: 1
has_toc: true
---


## Module Overview
 
| Module | Files | Role |
|---|---|---|
| [Underlying SDE](/docs/UnderlyingSDE.md) | `underlying_sde.hpp/.cpp` | Stochastic processes (GBM, jump-diffusion) and RNG |
| [Option Payoff](/docs/OptionPayoff.md) | `option_payoff.hpp/.cpp` | Payoff functions and ITM conditions |
| [Basis Functions](/docs/BasisFunction.md) | `basis_functions.hpp/.cpp` | Polynomial and Laguerre basis sets for regression |
| [Monte Carlo Paths](/docs/MCPaths.md) | `mc_paths.hpp/.cpp` | Path simulation matrix |
| [OLS Regression](/docs/OLSRegression.md) | `ols_regression.hpp/.cpp` | Least-squares regression via Eigen QR decomposition |
| [LSM Pricer](/docs/LSMPricer.md) | `lsm_pricer.hpp/.cpp` | Core pricing engine — simulation, backward induction, statistics |
| [BS Pricer](/docs/BSPricer.md) | `bs_pricer.hpp/.cpp` | Black–Scholes closed-form European benchmark |
| [Convergence Analyser](/docs/ConvergenAnalyzer.md) | `convergence_analyser.hpp/.cpp` | Benchmarking and convergence studies |
 
---
[UML Diagram for the project](/docs/uml_diagram.md)