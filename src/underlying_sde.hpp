#pragma once

#include <vector>
#include <string>
#include <cstddef>
#include <random>

namespace lsm {
    namespace core {

//Random number generator
struct RNG {
    std::mt19937_64                        engine{std::random_device{}()};
    std::normal_distribution<double>       norm_dist{0.0, 1.0};
    std::uniform_real_distribution<double> uniform_dist{0.0, 1.0};

    double normal();    //GENERATES FROM N(0,1)
    double uniform01(); //GENERATES FROM U(0,1)
};


//Base class for the stochastic processes and its simulated in discrete time
class StochasticProcess {
    public:
        virtual ~StochasticProcess() = default;
        //Advance by one step given an explicit N(0,1)
        virtual double stepWithNormal(double s, double dt, double z, RNG& rng) const = 0;
        // Advance the process by one time step.
        virtual double step(double s, double dt, RNG& rng) const = 0;

         // Simulate a path of length n+1 over [0, T], starting from S0.
        std::vector<double> simulatePath(double S0, double T, std::size_t n, RNG& rng) const;
};

// Geometric Brownian motion:
//     dS_t = r S_t dt + sigma S_t dW_t
class GeometricBrownianMotion final : public StochasticProcess {
public:
    GeometricBrownianMotion(double r, double sigma);

    // Exact solution: S_{t+dt} = S_t exp((r - sigma^2/2)dt + sigma sqrt(dt) z)
    double stepWithNormal(double s, double dt, double z, RNG& rng) const override;

    double r() const { return r_; }

    // Volatility paramater: Returns volatility
    double sigma() const { return sigma_; }
private:
    double r_; //drift coeff
    double sigma_; //volatility
};

// Jump-diffusion model with diffusion volatility sigma and
// Poisson jump intensity lambda.
class JumpDiffusionProcess final : public StochasticProcess {
public:
    JumpDiffusionProcess(double r, double sigma, double lambda);

    //antithetic step: z drives diffusion; jump detection uses the rng
    double stepWithNormal(double s, double dt, double z, RNG& rng) const override;
    
    //full step: draws z for diffusion and u for jump
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

    }
}
