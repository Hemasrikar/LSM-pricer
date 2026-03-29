---
title: Option Payoff
layout: default
nav_order: 8
has_toc: true
---

## Overview
The **Option Payoff Module** provides an object-oriented structure for calculating the intrinsic value of a standard options. It is designed to integrate into pricing frameworks such as **Least Squares Monte Carlo (LSM)**, where payoff evaluation must be performed repeatedly across many simulated asset price paths and time steps.

This module supports:
- **Put option payoff**
- **Call option payoff**

The design is flexible and allows future extension to other types of payoff.

---

## Class: `Put_payoff` & `Call_payoff`

### Description
The `Put_payoff` and `Call_payoff` class represents the payoff structure of a standard **put option** and **call option**.  
It inherits from the abstract base class `OptionPayoff` and implements the payoff and In-The-Money (ITM) condition.

---

### Payoff Formula & ITM Condition
- Put Payoff: max(K - S, 0)
- Put ITM: S < K

```cpp
OptionPayoff::OptionPayoff(double strike) : K(strike) {}

// Put_payoff implementations
Put_payoff::Put_payoff(double strike) : OptionPayoff(strike) {}

double Put_payoff::payoff(double S) const 
{
  return std::max(K - S, 0.0);
}

bool Put_payoff::InTheMoney(double S) const
{
  return S < K;
}
```

- Call Payoff: max(S - K, 0)
- Call ITM: S > K

```cpp
OptionPayoff::OptionPayoff(double strike) : K(strike) {}

// Call_payoff implementations
Call_payoff::Call_payoff(double strike) : OptionPayoff(strike) {}

double Call_payoff::payoff(double S) const 
{
  return std::max(S - K, 0.0);
}

bool Call_payoff::InTheMoney(double S) const
{
  return S > K;
}
```

Where:
- K = strike price  
- S = underlying asset price  

---

## Class: `OptionPayoff` (Abstract Base Class)

### Description
`OptionPayoff` represents the general concept of an option payoff. It stores the strike price `K` and defines the two key functions required for payoff evaluation.

### Key Features
- Stores the strike price `K`
- Provides a common interface for all payoff types
- Declares two virtual functions:
  - `payoff(S)` → calculates intrinsic value
  - `InTheMoney(S)` → checks if the option is in-the-money (ITM)

### Code:
```cpp
class OptionPayoff 
{
protected:
    double K;  

public:
    OptionPayoff(double strike);
    virtual ~OptionPayoff() = default;

    virtual double payoff(double S) const = 0;
    virtual bool InTheMoney(double S) const = 0;
};
```

