#include "lsm_pricer.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace lsm{
namespace engine{
    std::vector<double> LSMPricer::backwardInduction(PathData& data) const
    {
    //num of simulated paths
    int numPaths = data.numPaths;
    
    //num of exercise dates
    int numTimes = data.numTimeSteps;

    //num of exercise dates
    double dt = config.maturity / numTimes;

    //one-step discount factor
    double discountFactor = std::exp(-config.riskFreeRate * dt);

    //stores payoff for each path
    std::vector<double> cashflow(numPaths);

    //stores exercise time for each path
    std::vector<int> exerciseTime(numPaths);

    //initialise the maturity payoff
    // At maturity there is no continuation decision left, so the payoff is
    // just the terminal payoff

    for (int i = 0; i < numPaths; i++){
        cashflow[i] = payoff->payoff(data.paths[i][numTimes]);
        exerciseTime[i] = numTimes;
        data.cashFlows[i][numTimes] = cashflow[i];
    }

    //implementing backward induction by moving backwards in time
    for (int t = numTimes - 1; t >= 1; t--){

        //Mark which paths are in the monye at time t
        std::vector<bool> itm(numPaths, false);

        for (int i = 0; i < numPaths; ++i){
            double exerciseValue = payoff->payoff(data.paths[i][t]);
            if (exerciseValue > 0.0) {
                itm[i] = true;
            }
        }

        //Run Regression to estimate continuation coeffs
        std::vector<double> coeffs = lsm::engine::Ols_regression(
                data.paths, 
                static_cast<std::size_t>(t),
                cashflow, 
                itm, 
                discountFactor, 
                *basis
            );
        
        //compare execise value with continuation value
        for (int i = 0; i < numPaths; ++i){
            if(!itm[i]){
                continue;
            }
            double St = data.paths[i][t];
            double exerciseValue = payoff->payoff(St);

            //continuation value = fitted regression at S_t
            if (coeffs.size() > basis->basis.size()){
                throw std::runtime_error("Regression returned more coefficients than available basis functions");
            }
            double continuationValue = 0.0;
            for (std::size_t j = 0; j < coeffs.size(); ++j){
                continuationValue += coeffs[j] * basis->basis[j]->evaluate(St);
            }
            if (exerciseValue > continuationValue){
                cashflow[i] = exerciseValue;
                exerciseTime[i] = t;
                data.cashFlows[i][t] = exerciseValue;

                for(int s= t+1; s<= numTimes; ++s){
                    data.cashFlows[i][s] = 0.0;
                }
            }
        }
        }  
        std::vector<double> presentValue(numPaths, 0.0);
        
        for(int i = 0; i < numPaths; ++i){
            presentValue[i] = cashflow[i] * std::exp(-config.riskFreeRate*dt*exerciseTime[i]);
        }
        return presentValue;
    }

    SimulationResult LSMPricer::computeOptionValue(
        const std::vector<double>& pv, 
        double europeanValue,
        int N, 
        int T) const
    {
        double sum = 0.0; //will store sum of all pathwise present vals

        double sumSq = 0.0; //will store the sum of squares of all pathwise vals

        for (int i = 0; i < N; i++){
            sum += pv[i];
            sumSq += pv[i] * pv[i];
        }

        double optionValue = sum/N; //monte carlo estimate of the option val

        double variance = std::max(0.0, sumSq/N - optionValue * optionValue);

        double standardError = std::sqrt(variance / N); //standard error

        //store everything in the result obj
        SimulationResult result;
        result.optionValue = optionValue;
        result.standardError = standardError;
        result.europeanValue = europeanValue;
        result.earlyExercisePremium = optionValue - europeanValue;
        result.numPaths = N;
        result.numExerciseDates = T;

        return result;

    }
}

}

// Simulated Paths 
namespace lsm {
    namespace engine {

        // Define the function simulatePaths with return type as PathData
        // With S0 initial price 
        // const StochasticProcess & process refers back to an object that represents the stochastic model
        PathData simulatePaths(
            double S0,
            const StochasticProcess& process,
            const LSMConfig& config)
        {

            // Validation checks
            if (config.numExerciseDates <= 0)
                throw std::invalid_argument("Number of exercise dates must be positive");

            if (config.numPaths <= 0)
                throw std::invalid_argument("Number of paths must be positive");

           // Setting up: 
           // The number of Time Steps T 
           // The number of Monte Carlo Paths N 
           // The size of the step dt
            const int T = config.numExerciseDates;

            // Ensure even number of paths when using antithetic variates
            const int N = config.useAntithetic
                ? config.numPaths - (config.numPaths % 2)
                : config.numPaths;

            const double dt = config.maturity / static_cast<double>(T);

            // Creating a result to store the data
            PathData data;
            data.numPaths = N;
            data.numTimeSteps = T;

            // Allocating the memory and time steps
            data.paths.resize(N);
            data.cashFlows.resize(N);

            for (int i = 0; i < N; ++i)
            {
                data.paths[i].resize(T + 1);
                data.cashFlows[i].resize(T + 1, 0.0); 
            }

            // Setting up the random number generator 
            // Checking if require to use monte carlo or antithetic variates
            RNG rng(config.rngSeed);


            // Monte Carlo Simulation part
            if (!config.useAntithetic)
            {
                // Monte Carlo loop reference path 
                // Setting the price S0 as starting value of the path
                for (int i = 0; i < N; ++i)
                {
                    auto& path = data.paths[i];

                    
                    path[0] = S0;

                    // Time evolution for each time step
                    for (int t = 1; t <= T; ++t)
                    {
                        path[t] = process.step(path[t - 1], dt, rng);
                    }
                }
            }

            // Antithetic Variates Simulation part
            else
            {
                // Spliting the paths into half 
                // Loop over the two paths and receive the path references
                // We set the price as S0 for each of the path p1 and p2
                const int half = N / 2;

                for (int i = 0; i < half; ++i)
                {
                    auto& p1 = data.paths[i];
                    auto& p2 = data.paths[i + half];

                    p1[0] = S0;
                    p2[0] = S0;

                    // Time evolution and random shock z standard normal 
                    // For both paths which includes the antithetic path p2
                    for (int t = 1; t <= T; ++t)
                    {
                        double z = rng.normal();

                        // Normal path
                        p1[t] = process.stepWithNormal(p1[t - 1], dt,  z);

                        // Antithetic path
                        p2[t] = process.stepWithNormal(p2[t - 1], dt, -z);
                    }
                }
            }

            return data;
        }

    } // namespace engine
} // namespace lsm
