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

Weighted Laguerre polynomials of the form `e^(-x/2) * Ln(x)`. Accepts any non-negative integer order. Negative inputs are clamped to `0`.
```cpp
LaguerrePolynomial(int order)  // throws std::invalid_argument if order < 0
```

Evaluation uses the three-term recurrence relation:
```
L_{n+1}(x) = ((2n + 1 - x) * L_n(x) - n * L_{n-1}(x)) / (n + 1)
```

with `L_0 = 1` and `L_1 = 1 - x` (undamped), and the `e^(-x/2)` damping factor applied once at the end. The `order = 0` case returns `e^(-x/2)` directly.

`name()` returns e.g. `"Laguerre_L2"`.

---

### BasisSet

Class that builds a ready-to-use collection of basis functions via two factory methods.
```cpp
void makeMonomialSet(int numTerms);  // builds { 1, x, x^2, ..., x^numTerms }
void makeLaguerreSet(int numTerms);  // builds { 1, L0, L1, ..., L(numTerms-1) }
```

Both methods clear any existing basis before building, prepend a `ConstantBasis`, and store ownership in the internal `basis_` vector. Both throw `std::invalid_argument` if `numTerms < 1`.

> **Note:** The two factory methods interpret `numTerms` differently. `makeMonomialSet` includes `x^numTerms` (inclusive upper bound), whereas `makeLaguerreSet` runs from `L0` to `L(numTerms-1)` — so `numTerms = 3` yields `{ 1, L0, L1, L2 }`, not `{ 1, L0, L1, L2, L3 }`.

**Copy and move semantics**

`BasisSet` is move-only. Copy construction and copy assignment are explicitly deleted because `std::unique_ptr` members are non-copyable.

**Members**

- `std::vector<std::unique_ptr<BasisFunction>> basis_` *(private)* — owns the allocated basis functions; memory is automatically managed and freed when the `BasisSet` goes out of scope.
- `std::vector<BasisFunction*> basisPtrs() const` — returns a vector of raw (non-owning) pointers, intended for passing to `OLSRegressor` or other consumers that do not take ownership.
- `std::size_t size() const` — returns the number of basis functions currently held.
- `void clear()` — empties the internal basis vector.

> **Note:** Use `basisPtrs()` wherever a non-owning view is needed. The raw pointers are valid only for the lifetime of the owning `BasisSet`.