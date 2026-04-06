---
title: Black-Scholes Pricer
layout: default
nav_order: 1
has_toc: true
parent: Benchmark and Convergence
---
**Namespace:** `bs_pricer` | **Header:** `bs_pricer.hpp`

The **Black-Scholes Pricer** provides a closed-form analytical solution for pricing European vanilla call and put options. It is used as the **benchmark** against which the LSM pricer is compared during convergence analysis.

---

## Parameters

| Parameter | Type     | Description                                      |
|-----------|----------|--------------------------------------------------|
| `S0`      | `double` | Current spot price of the underlying asset       |
| `r`       | `double` | Risk-free interest rate (annualised)             |
| `sigma`   | `double` | Volatility of the underlying asset (annualised)  |
| `K`       | `double` | Strike price                                     |
| `T`       | `double` | Time to maturity (in years)                      |
| `call`    | `bool`   | `true` for a call option, `false` for a put      |

---

## Formula

Given spot price $S_0$, strike $K$, risk-free rate $r$, volatility $\sigma$, and time to maturity $T$:

$$d_1 = \frac{\ln(S_0 / K) + (r + \frac{1}{2}\sigma^2)T}{\sigma\sqrt{T}}, \qquad d_2 = d_1 - \sigma\sqrt{T}$$

$$C = S_0 \cdot N(d_1) - K e^{-rT} \cdot N(d_2)$$

$$P = K e^{-rT} \cdot (1 - N(d_2)) - S_0 \cdot (1 - N(d_1))$$

where $N(\cdot)$ is the standard normal CDF, computed via:

```cpp
double normal_CDF(double x)
{
    return 0.5 * erfc(-x * M_SQRT1_2);
}
```

---

## Implementation

```cpp
double price_vanilla_option_european_bs(double S0, double r, double sigma,
                                        double K, double T, bool call) {

    double d1 = (log(S0 / K) + (r + 0.5 * pow(sigma, 2)) * T) / (sigma * sqrt(T));
    double d2 = d1 - sigma * sqrt(T);

    double n_d1 = normal_CDF(d1);
    double n_d2 = normal_CDF(d2);

    double C = S0 * n_d1 - K * exp(-r * T) * n_d2;
    double P = K * exp(-r * T) * (1 - n_d2) - S0 * (1 - n_d1);

    if (call == true) {
        return C;
    }

    return P;
}
```

Returns the call price if `call = true`, otherwise the put price.

