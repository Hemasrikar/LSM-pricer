---
title: Basis Function
layout: default
nav_order: 2
has_toc: true
---

**Namespace:** `lsm::core` | **Header:** `basis_functions.hpp`

A small set of basis function classes for least-squares regression, primarily targeting the Longstaff-Schwartz Monte Carlo method for American option pricing.

---

## Class Overview

All concrete classes inherit from `BasisFunction` (defined in the header), which requires two methods:

```cpp
double evaluate(double x) const;
std::string name() const;
```

---

## Classes

### ConstantBasis

Always returns `1.0`. Every `BasisSet` prepends one of these as the intercept term.

---

### MonomialBasis

Computes `x^n` for a given non-negative integer power.

```cpp
MonomialBasis(int power)  // throws std::invalid_argument if power < 0
```

`name()` returns e.g. `"x^2"`.

---

### LaguerrePolynomial

Weighted Laguerre polynomials of the form `e^(-x/2) * Ln(x)`, orders 0–5. Negative inputs are clamped to 0.

```cpp
LaguerrePolynomial(int order)  // throws std::invalid_argument if order < 0 or > 5
```

| Order | Formula |
|---|---|
| 0 | `e^(-x/2)` |
| 1 | `e^(-x/2) * (1 - x)` |
| 2 | `e^(-x/2) * (1 - 2x + x^2/2)` |
| 3 | `e^(-x/2) * (1 - 3x + 3x^2/2 - x^3/6)` |
| 4 | `e^(-x/2) * (1 - 4x + 3x^2 - 2x^3/3 + x^4/24)` |
| 5 | `e^(-x/2) * (1 - 5x + 5x^2 - 5x^3/3 + 5x^4/24 - x^5/120)` |

`name()` returns e.g. `"Laguerre_L2"`.

---

### BasisSet

Class that builds a ready-to-use collection of basis functions via two factory methods.

```cpp
void makeMonomialSet(int numTerms);  // builds { 1, x, x^2, ..., x^numTerms }
void makeLaguerreSet(int numTerms);  // builds { 1, L0, L1, ..., L(numTerms-1) }
```

Both methods prepend a `ConstantBasis` and store ownership in the internal `basis` vector.

> **Note:** The two factory methods interpret `numTerms` differently. `makeMonomialSet` includes `x^numTerms` (inclusive upper bound), whereas `makeLaguerreSet` runs from `L0` to `L(numTerms-1)` — so `numTerms = 3` yields `{ 1, L0, L1, L2 }`, not `{ 1, L0, L1, L2, L3 }`.

**Members**

- `std::vector<std::unique_ptr<BasisFunction>> basis` — owns the allocated basis functions; memory is automatically managed and freed when the `BasisSet` goes out of scope.
- `std::vector<BasisFunction*> basisPtrs() const` — returns a vector of raw (non-owning) pointers, intended for passing to `OLSRegressor` or other consumers that do not take ownership.

> **Note:** Use `basisPtrs()` wherever a non-owning view is needed.
