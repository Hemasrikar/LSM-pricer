---
title: Tests of the Project
layout: default
nav_order: 9
has_toc: true
---


## Basis Functions Testing

Unit tests are written using the [Catch2](https://github.com/catchorg/Catch2) framework and cover all classes in `lsm::core`.

#### Running the Tests
```bash
g++ -std=c++17 -I. test_basis_functions.cpp -o tests && ./tests
```

#### Test Cases and Summary

| Class | Tested on|
|---|---|
| `ConstantBasis` | Returns `1.0` for all inputs and `name()` format check |
| `MonomialBasis` | Powers 0 to 5, negative/zero inputs, and error throw on negative power |
| `LaguerrePolynomial` | Output check for Orders 0, 1, 3, 4 and three term recurrence consistency up to order 8. Then negative input clamping and catching errors on negative order |
| `BasisSet` | `makeLaguerreSet` and `makeMonomialSet` structure and values. Clear and rebuild check and catching errors on `numTerms < 1` |
| Polymorphism | Virtual dispatch through `BasisFunction*` for all three concrete types |

- **Recurrence consistency** - verifies the three-term relation
  `(n+1) * L_{n+1}(x) = (2n+1-x) * L_n(x) - n * L_{n-1}(x)` holds across orders 2–8
- **Negative input clamping** - confirms `LaguerrePolynomial::evaluate(-x) == evaluate(0)` for all tested orders
- **Clear and rebuild** - ensures calling a factory method twice does not accumulate elements
- **Virtual dispatch** - exercises the `BasisFunction` interface directly via `std::unique_ptr<BasisFunction>`