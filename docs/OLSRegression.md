---
title: OLS Regression
layout: default
nav_order: 7
---

OLS regression follows simple OLS logic where we have our design matrix $X \in \mathbb{R}^{n \times m}$ (for $n$ ITM observations and $m$ basis functions), a target vector $Y$ (discounted continuation values), and our OLS estimate:

$$\hat{\beta} = (X^\top X)^{-1} X^\top Y$$

The predicted continuation value for each path is then $\hat{C} = X\hat{\beta}$.

---

## Why Simple Inversion Leads to Instability

The naive approach — forming $X^\top X$ and solving — is numerically problematic. Squaring the matrix **squares the condition number**:

$$\kappa(X^\top X) = \kappa(X)^2$$

A large condition number means small perturbations in the data (floating-point rounding, near-collinear basis functions) produce large errors in $\hat{\beta}$. This is especially relevant here because **Laguerre polynomials evaluated on a narrow range of stock prices** can be nearly collinear, making $X^\top X$ nearly singular.

---

## QR Decomposition for Stable Inversion

Instead of forming $X^\top X$, we decompose $X$ directly:

$$X = QR$$

where $Q \in \mathbb{R}^{n \times m}$ has orthonormal columns and $R \in \mathbb{R}^{m \times m}$ is upper triangular. The least-squares solution becomes:

$$\hat{\beta} = R^{-1} Q^\top Y$$

This is solved via back-substitution on $R$, never forming $X^\top X$ explicitly, so the condition number stays at $\kappa(X)$ rather than $\kappa(X)^2$.

### Column-Pivoting for Extra Stability

We use **column-pivoting QR** (`colPivHouseholderQr` from Eigen), which reorders columns of $X$ to place the largest-norm columns first:

$$XP = QR$$

where $P$ is a permutation matrix. This reordering exposes any redundancy in the basis — if two basis functions are nearly collinear, the solver identifies this and still produces a stable $\hat{\beta}$, whereas a non-pivoted solve would silently amplify the near-singularity into large coefficient errors.

---

## Implementation

The solver lives in `Ols_regression()` in `src/ols_regression.cpp` (namespace `lsm::engine`). The pipeline is:

| Step | Function | Description |
|------|----------|-------------|
| 1 | `buildDesignMatrix` | Evaluates each basis function on \( S_t \) for ITM paths only &rarr; \( X \) |
| 2 | `buildYVector` | Discounts next-period cashflows by \( e^{-r\Delta t} \) for ITM paths &rarr; \( Y \) |
| 3 | `Ols_regression` | Solves \( X\hat{\beta} = Y \) via `colPivHouseholderQr().solve(Y)` |
| 4 | — | OTM paths receive \( \hat{C}_i = 0 \) (no extrapolation outside ITM set) |
```cpp
// src/ols_regression.cpp (line 94)
Eigen::VectorXd beta = X.colPivHouseholderQr().solve(Y);
```

Only ITM paths enter the regression (per Longstaff & Schwartz §1). OTM paths are assigned a continuation value of `0.0` by default, since for those paths immediate exercise is already not optimal.

---

## References

- Longstaff & Schwartz (2001) — *Valuing American Options by Simulation: A Simple Least-Squares Approach*, §1–2
- [Eigen: Least Squares](https://libeigen.gitlab.io/eigen/docs-3.3/group__LeastSquares.html)
- [Eigen: Quick Reference](https://libeigen.gitlab.io/eigen/docs-nightly/group__QuickRefPage.html)