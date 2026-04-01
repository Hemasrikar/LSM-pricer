---
title: Tests of the Project
layout: default
nav_order: 7
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


## Option Payoff Testing
This section describes the tests written for the `Put_payoff` and `Call_payoff` classes and to verify that:
- The payoff formulas are implemented correctly.
- The `InTheMoney()` logic correctly identifies whether an option is ITM (in-the-money).

---

Here we have included the test cases for both the put payoff and call payoff. As well as checking if the put option and call option is in-the-money (ITM). We have chosen the strike price `K = 100` and along with different set of values for the underlying asset price `S`.

### Payoff Tests
```cpp
TEST_CASE("Put_payoff returns correct payoff values", "[Put_payoff]") 
{
    Put_payoff put(100.0);

    REQUIRE(put.payoff(50.0) == Approx(50.0));
    REQUIRE(put.payoff(100.0) == Approx(0.0));
    REQUIRE(put.payoff(150.0) == Approx(0.0));
}

TEST_CASE("Call_payoff returns correct payoff values", "[Call_payoff]") 
{
    Call_payoff call(100.0);

    REQUIRE(call.payoff(50.0) == Approx(0.0));
    REQUIRE(call.payoff(100.0) == Approx(0.0));
    REQUIRE(call.payoff(150.0) == Approx(50.0));
}
```
This test checks whether the put payoff and call payoff functions correctly follows their payoff formula:

- Put Payoff = max(K − S,0) 
- Call Payoff = max(S - K,0)

Where:
- K = strike price
- S = underlying asset price

### In-The-Money Tests
```cpp
TEST_CASE("Put_payoff InTheMoney works correctly", "[Put_payoff]") 
{
    Put_payoff put(100.0);

    REQUIRE(put.InTheMoney(50.0) == true);
    REQUIRE(put.InTheMoney(99.9) == true);
    REQUIRE(put.InTheMoney(100.0) == false);
    REQUIRE(put.InTheMoney(150.0) == false);
}

TEST_CASE("Call_payoff InTheMoney works correctly", "[Call_payoff]") 
{
    Call_payoff call(100.0);

    REQUIRE(call.InTheMoney(50.0) == false);
    REQUIRE(call.InTheMoney(100.0) == false);
    REQUIRE(call.InTheMoney(100.1) == true);
    REQUIRE(call.InTheMoney(150.0) == true);
}
```
This test validates the logic for determining if a put option and call option is in-the-money (ITM).

ITM Condition:
- Put option: S < K
- Call option: S > K


Where:
- K = strike price
- S = underlying asset price