---
title: OLS Regression
layout: default
nav_order: 7
has_children: true
---

OLS regression follows simple OLS logic where we have our design matrix $X \in \mathbb{R}^{n \times m}$ (for $n$ ITM observations and $m$ basis functions), a target vector $Y$ containing the discounted continuation values for each ITM path, and our OLS estimate:

$$\hat{\beta} = (X^\top X)^{-1} X^\top Y$$

The predicted continuation value for each path is then $\hat{C} = X\hat{\beta}$. This tells us, for each path that is currently in the money, what the expected value of continuing to hold the option is. We then compare against the immediate exercise payoff to decide whether to exercise early.

---

## Why Simple Inversion Leads to Instability

The naive approach — forming $X^\top X$ and solving — is numerically problematic. Squaring the matrix **squares the condition number**:

$$\kappa(X^\top X) = \kappa(X)^2$$

A large condition number means small perturbations in the data (floating-point rounding, near-collinear basis functions) produce large errors in $\hat{\beta}$. This is especially relevant here because **Laguerre polynomials evaluated on a narrow range of stock prices** can be nearly collinear, making $X^\top X$ nearly singular. In practice this means the regression coefficients become unreliable, and the continuation value estimates which drive the entire early exercise decision can be wildly off even when the inputs look reasonable.

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

The solver lives in `Ols_regression()` in `src/ols_regression.cpp` (namespace `lsm::engine`). The pipeline works as follows:

**1.** `buildDesignMatrix` evaluates each basis function on $S_t$ for ITM paths only, producing the design matrix $X$. OTM paths are excluded entirely since we only need continuation values where early exercise is actually a decision.

**2.** `buildYVector` discounts the next-period cashflows by $e^{-r\Delta t}$ for ITM paths, producing the target vector $Y$. These are the values we are trying to predict with the regression.

**3.** `Ols_regression` solves $X\hat{\beta} = Y$ via `colPivHouseholderQr().solve(Y)`, returning the fitted coefficients $\hat{\beta}$, which are then used to compute the continuation value for every path.

**4.** OTM paths are assigned $\hat{C}_i = 0$ by default. Since those paths are already out of the money, immediate exercise is not optimal and no regression estimate is needed.

```cpp
// src/ols_regression.cpp (line 94)
Eigen::VectorXd beta = X.colPivHouseholderQr().solve(Y);
```

---

## References

- Longstaff & Schwartz (2001) — *Valuing American Options by Simulation: A Simple Least-Squares Approach*, §1–2
- [Eigen: Least Squares](https://libeigen.gitlab.io/eigen/docs-3.3/group__LeastSquares.html)
- [Eigen: Quick Reference](https://libeigen.gitlab.io/eigen/docs-nightly/group__QuickRefPage.html)
