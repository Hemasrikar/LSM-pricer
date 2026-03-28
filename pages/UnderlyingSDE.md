# Underlying SDEs

**Namespace:** `lsm::core` | **Header:** `underlying_sde.hpp`

A small set of stochastic process classes for simulating one-dimensional asset-price models in Monte Carlo pricing. The module provides a common interface for path generation together with concrete implementations for Geometric Brownian Motion and a jump-diffusion model with killing jumps.

---

## Class Overview

All concrete classes inherit from `StochasticProcess` (defined in the header), which provides the common simulation interface:

```cpp
virtual double step(double s, double dt, RNG& rng) const = 0;
std::vector<double> simulatePath(double s0, double T, std::size_t n, RNG& rng) const;
```
`step()` advances the process by one time step, while `simulatePath()` generates a full path on a uniform grid by repeated application of `step()`.

---
## Random Number Generator

The module uses an `RNG` object to provide random draws required during simulation. It supports:
 
* `normal()` for standard normal draws,
* `uniform01()` for uniform draws on [0,1]

---

## Classes
`Stochastic Process`
Abstract base class for one-dimensional stochastic processes simulated in discrete time. 

### Methods
```cpp
virtual double step(double s, double dt, RNG& rng) const = 0;
std::vector<double> simulatePath(double s0, double T, std::size_t n, RNG& rng) const;
virtual double stepWithNormal(double s, double dt, double z, RNG& rng) const = 0;
```
* `step(...)` is the default one-step update. It draws a standard normal random variable internally and then forwards the update to `stepWithNormal(...)`.

* `stepWithNormal(...)` advances the process by one step using a given normal sample. This is useful if the caller wants explicit control over the Brownian increment for example in antithetical variance reduction where paired paths may be simulated using z and -z.

For some models, additional randomness may still be required. In particular, the jump-diffusion model uses `rng` to simulate jump events . 

### Path generation

For initial value `s0`, maturity `T`, and `n` time steps, the simulated path has length `n+1` and includes the initial point at time `0`. For each time step, `k = 1, \dots, n`:

$$ \text{path}[k] = \text{step(path}[k-1], dt, \text{rng}). $$

If `T <= 0`, `simulatePath()` throws `std::invalid_argument`.


---

## GeometricBrownianMotion

Implements the GBM model 

$$dS_t = rS_t dt + \sigma S_t dW_t$$


where `r` is the drift and `sigma` is the volatility.

### Constructor
```cpp
GeometricBrownianMotion(double r, double sigma);
```
Constructs a GBM process with drift `r` and volatility `sigma`.

Throws `std::invalid_argument` if `sigma < 0`.


### Method: Time-step update with Normal sample
```cpp
double stepWithNormal(double s, double dt, double z, RNG& rng) const override;
```
Uses the exact lognormal solution of the SDE:

$$S_{t+dt} = S_t \exp \left( \left( r - \frac{1}{2}\sigma^2\right)dt + \sigma \sqrt{dt} Z\right), \quad Z \sim N(0,1)$$

where $z$ is supplied standard normal sample

The parameter `rng` is retained for internal consistency, but is not used directly in the GBM implementation since the $z$ is supplied explicitly. 


if `dt <= 0`, `step()` throws `std::invalid_argument`.

### Diffusion one-step update

The inherited `step(...)` method draws a normal sample internally and calls `stepWithNormal(...)`. 

### Accessors

```cpp
double r() const;
double sigma() const;
```
Return the drift and volatility parameters.

---

## JumpDiffusionProcess
Implements a jump-diffusion process with killing jumps. The dynamics used in the implementation are:

$$dS_t = (r + \lambda)S_t dt + \sigma S_t dW_t - S_t dq_t$$


where:

* `r` is the drift parameter
* `sigma` is the diffusion volatility
* `lambda' is the jump intensity
* `q_t is a Poisson process with intensity `lambda`

A jump sends the process value to zero.

### Constructor
```cpp
JumpDiffusionProcess(double r, double sigma, double lambda)
```
Throws `std::invalid_argument` if:
* `sigma` < 0
* `lambda` < 0

### Method: Default time-step update
```cpp
double step(double s, double dt, RNG& rng) const override;
```
The update is performed as follows:

If the current state satisfies `s <= 0`, the method returns `0.0`. The zero state is therefore absorbing. 

Otherwise, over a time step `dt`:

1. Compute the probability of at least one jump in `(t, t+dt]`

$$p_{jump} = 1 - \exp(-\lambda dt)$$


2. Draw `U ~ U(0,1)`
3. If `U <p_jump`, a jump occurs and the process moves to 0
4. If no jump occurs, evolve the process using GBM-style update



$$S_{t+dt} = S_t \exp\left( \left( (r + \lambda) - \frac{1}{2} \sigma^2 \right) dt + \sigma \sqrt{dt} Z \right), \quad Z \sim N(0,1)$$



If `dt <=0`, `step()` throws `std::invalid argument`.

### One-step update with supplied normal sample

```cpp
double stepWithNormal(double s, double dt, double z, RNG& rng) const override;
```
This overload is used when the diffusion is supplied externally. The method samples the jump event interval but it does not generate the Brownian shock itself. Instead, the supplied value `z` is used for diffusion component if no jump occurs. The algorithm is the same but the only difference is we do not sample from the standard normal. This interface is useful for variance reduction methods such as antithetic reduction. The jump event is still generated independently through RNG using a uniform draw. 

### Accessors:
```cpp
double r() const;
double sigma() const;
double lambda() const;
```
---
## Antithetic Variance Reduction
The addition of `stepWithNormal(...)` makes the process interface compatible with antithetic simulation. The idea is:

* Simulate one path using normal sample $z$.
* Simulate a paired path using normal sample $-z$
* average the corresponding payoffs

This creates negatively correlated path pairs and can reduce Monte Carlo variance.

---
## Notes

`simulatePath()` is implemented in the base class and reused by all derived processes. This keeps path-generation logic generic, while allowing each each model to define its own one-step dynamics via `step()`.

The GBM implementation uses an exact discretisation of the continuous-time model. The jump-diffusion implementation combines exact diffusion evolution on no-jump intervals with Bernoulli sampling of Poisson jump arrival over each time step.

Ordinary one-step simulation is done through `step(...)`

Externally controlled Brownian shocks are handled through `stepWithNormal(...)`

---