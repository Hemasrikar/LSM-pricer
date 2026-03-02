#pragma once

#include <vector>
#include <string>
#include <cstddef>


/*-------------------------------------------------------------------------------------------------
* RANDOM NUMBER GENERATOR
* TYPE: Structure
* Provides sampling from: 
*  - Standard normal distribution N(0,1)
*  - Uniform distribution on [0,1]
* 
* Samples are assumed to be independent
---------------------------------------------------------------------------------------------------*/
struct RNG {

    double normal(); //GENERATES FROM N(0,1)
    double uniform01(); //GENERATES NOS FROM U(O,1)
};
/*-------------------------------------------------------------------------------------------------
* BASE CASE FOR STOCHASTIC PROCESSES
*
* Defines an interface for simulating discrete-time approximations of SDEs
*
* Derived classes implement the dynamics through the step() function.
* Path simulation is performed using successive applications of step()
---------------------------------------------------------------------------------------------------*/
class StochasticProcess{
    public:
        virtual ~StochasticProcess() = default;
        /*------------------------------------------------------------------------------------------
         * step METHOD: ADVANCES PROCESS BY ONE TIME STEP.
         * Parameters:
         * s - Current state of the process
         * dt - Time increment.
         * rng - Randome number generator used for stochastic increments.
         * 
         * Return: The next state of the process after time dt.
         * Derived classes implement the dynamics through the step() function.
         * 
        --------------------------------------------------------------------------------------------*/

        virtual double step(double s, double dt, RNG& rng) const = 0;

         /*------------------------------------------------------------------------------------------
         * simulatePath METHOD: Simulate a sample path of the process.
         * Parameters:
         * S0 - Initial value of the process
         * T - maturity time
         * n - number of time steps
         * rng - Random number generator used for stochastic increments.
         * 
         * Return: A vector containing the simulated path pf length n+1 incluing
         *         initial value S0
        --------------------------------------------------------------------------------------------*/
        std::vector<double> simulatePath(double S0, double T, std::size_t n, RNG& rng) const;

};
/*-------------------------------------------------------------------------------------------------
* GEOMETRIC BROWNIAN MOTION (GBM) process.
*
* Models dynamics of the form:
*      dS_t = r S_t dt + sigma S_t dW_t
*
* The step() function implements the discrete-time approcimation of the exact
* solution over a time increment dt
* This inherits from the class StochasticProcesses
---------------------------------------------------------------------------------------------------*/
class GeometricBrownianMotion final : public StochasticProcess{
public:
    /*------------------------------------------------------------------------------------------
    * Construct a GBM porcess
    * parameter:
    *   - r Drift coefficient
    *   - sigma: colatility parameter (assumed non-negative)
    * 
    --------------------------------------------------------------------------------------------*/
    GeometricBrownianMotion(double r, double sigma);
    /*------------------------------------------------------------------------------------------
    * ADVANCE THE PROCESS BY ONE TIME STEP.
    * 
    * Given the current state s, returns the value at the next time step using the GBM 
    * dynamics over interval dt.
    * 
    * parameter:
    *  - s: current value of the process.
    *  - dt: Time increment
    *  - rng: Random number generator for Brownian increments
    * 
    * return: The next step in the process.
    * 
    --------------------------------------------------------------------------------------------*/
    double step(double s, double dt, RNG& rng) const override;
    // Drift parameter: Returns drift
    double r() const { return r_; }

    // Volatility paramater: Returns volatility
    double sigma() const { return sigma_; }
private:
    double r_; //drift coeff
    double sigma_; //volatility
};
/*-------------------------------------------------------------------------------------------------
* JUMP DIFFUSION PROCESS (MERTON-TYPE MODEL)
*
* Extends GBM by incorporating random jumps. The process evolves as:
* 
*       dS_t = r S_t dt + sigma S_t dW_t - S_t dq
* where:
* - W_t is a BM
* - q_t is a jump process driven by Poisson with intensitiy lambda
---------------------------------------------------------------------------------------------------*/
class JumpDiffusionProcess final : public StochasticProcess {
public:
    /*------------------------------------------------------------------------------------------
    * Construct a jump diffusion process
    * parameter:
    *   - r Drift coefficient
    *   - sigma: colatility parameter (assumed non-negative)
    *   - lambda Jump intensity (expected number of jumps per unit time >= 0)
    * 
    --------------------------------------------------------------------------------------------*/
    JumpDiffusionProcess(double r, double sigma, double lambda);
    /*------------------------------------------------------------------------------------------
    * ADVANCE THE PROCESS BY ONE TIME STEP.
    * 
    *  omputes the next state by combinig:
    *  - a diffusion step (GBM - type increment)
    *  - a jump component, where the jump occurs with prob lambda*dt
    * parameter:
    *  - s: current value of the process.
    *  - dt: Time increment
    *  - rng: Random number generator for Brownian increments
    * 
    * return: The next step in the process.
    * 
    --------------------------------------------------------------------------------------------*/

    double step(double s, double dt, RNG& rng) const override;
    // Drift parameter
    double r() const { return r_; }
    // volatility parameter
    double sigma() const { return sigma_; }
    //jump intensity
    double lambda() const { return lambda_; }

private:
    double r_; //drift coeff 
    double sigma_; //diffusion vol
    double lambda_; //jump intensity.
};
