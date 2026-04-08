#include "lsm_pricer.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <vector>


namespace lsm {
    namespace engine {
    /*-------------------------------------------------------------------------------------------------
     * CONSTRUCT AN LSM PRICER
     *
     * Initialises the pricer with a stochastic process, payoff, regression basis,
     * and numerical configuration. Ownership of the process, payoff, and basis
     * objects is transferred into the class through std::unique_ptr.
     *
     * This design supports:
     *   - Abstraction: the pricer depends on abstract interfaces rather than
     *     concrete model implementations.
     *   - Polymorphism: different derived processes, payoffs, and basis systems
     *     can be supplied without changing the pricing logic.
     *
     * Parameters:
     *    - process  Owned pointer to the stochastic process model.
     *    - payoff   Owned pointer to the option payoff object.
     *    - basis    Owned pointer to the regression basis object.
     *    - config   Numerical configuration for simulation and pricing.
     *
     * Throws: std::invalid_argument if any input pointer is null.
     *
    ---------------------------------------------------------------------------------------------------*/
    LSMPricer::LSMPricer(
    const lsm::core::StochasticProcess& process,
    const lsm::core::OptionPayoff& payoff,
    const lsm::core::BasisSet& basis,
    const lsm::engine::LSMConfig& config)
    : process(process), payoff(payoff), basis(basis), config(config)
    {
    }

    // Define the function simulatePaths with return type as PathData
    // With S0 initial price 
    // const StochasticProcess & process refers back to an object that represents the stochastic model
    PathData simulatePaths(
        double S0,
        const lsm::core::StochasticProcess& process,
        const lsm::engine::LSMConfig& config)
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
            const int N = config.useAntithetic ? config.numPaths - (config.numPaths % 2) : config.numPaths;

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
            lsm::core::RNG  rng(config.rngSeed);


            // Checking if require to use monte carlo or antithetic variates
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
                        p1[t] = process.stepWithNormal(p1[t - 1], dt,  z, rng);

                        // Antithetic path
                        p2[t] = process.stepWithNormal(p2[t - 1], dt, -z, rng);
                    }
                }
            }

            return data;
        }
    /*-------------------------------------------------------------------------------------------------
    * PERFORM BACKWARD INDUCTION FOR THE LSM ALGORITHM
    *
    * Implements the core Longstaff-Schwartz recursion. Starting from maturity,
    * the method moves backwards through the exercise dates and determines whether
    * early exercise is optimal on each simulated path.
    *
    * At each time step:
    *   - in-the-money paths are identified,
    *   - discounted future cashflows are used in the regression step,
    *   - continuation values are estimated via OLS,
    *   - immediate exercise is compared against continuation.
    *
    * If early exercise is optimal, the pathwise cashflow is updated and all
    * later cashflows on that path are set to zero.
    *
    * Parameters:
    *    - data  PathData object containing simulated paths and pathwise cashflows.
    *
    * Returns: A vector of present values at time 0, one for each simulated path.
    *
    * Notes:
    *    - Terminal payoffs initialise the recursion.
    *    - Only in-the-money paths enter the exercise decision.
    *    - Regression coefficients / continuation values are supplied by
    *      Ols_regression using the chosen basis system.
    *
    ---------------------------------------------------------------------------------------------------*/
    
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

    //initialise the maturity payoff
    // At maturity there is no continuation decision left, so the payoff is
    // just the terminal payoff

    for (int i = 0; i < numPaths; i++){
        cashflow[i] = payoff.payoff(data.paths[i][numTimes]);
        data.cashFlows[i][numTimes] = cashflow[i];
    }

    //implementing backward induction by moving backwards in time
    for (int t = numTimes - 1; t >= 1; t--){

        //Mark which paths are in the monye at time t
        std::vector<bool> itm(numPaths, false);

        for (int i = 0; i < numPaths; ++i){
            double exerciseValue = payoff.payoff(data.paths[i][t]);
            if (exerciseValue > 0.0) {
                itm[i] = true;
            }
        }
        //Run Regression to estimate continuation coeffs
        const double strike = payoff.strike();
        
        std::vector<double> coeffs = lsm::engine::Ols_regression(
                data.paths,
                static_cast<std::size_t>(t),
                cashflow,
                itm,
                discountFactor,
                basis,
                strike
            );

        //compare execise value with continuation value
        for (int i = 0; i < numPaths; ++i){
            double St = data.paths[i][t];
            double exerciseValue = payoff.payoff(St);
            double continuationValue = coeffs[i];
            if (itm[i] && exerciseValue > continuationValue){
                cashflow[i] = exerciseValue;
                data.cashFlows[i][t] = exerciseValue;

                for(int s= t+1; s<= numTimes; ++s){
                    data.cashFlows[i][s] = 0.0;
                }
            }
            else{
                cashflow[i] = cashflow[i]*discountFactor;
                data.cashFlows[i][t] = 0;
            }
        }
        }  
        std::vector<double> presentValue(numPaths, 0.0);
        //find present value: // cashflow is now the time-1 value, so discount once more to time 0
        for (int i = 0; i < numPaths; ++i) {
        presentValue[i] = discountFactor * cashflow[i];
        }   
        return presentValue;
    }
    /*-------------------------------------------------------------------------------------------------
    * COMPUTE SUMMARY PRICING STATISTICS
    *
    * Aggregates pathwise present values into the final Monte Carlo estimate of
    * the American option price. Also computes the standard error, European
    * benchmark, and early exercise premium.
    *
    * Parameters:
    *    - pv             Vector of discounted pathwise option values at time 0.
    *    - europeanValue  European benchmark value.
    *    - N              Number of simulated paths.
    *    - T              Number of exercise dates.
    *
    * Returns: A SimulationResult object containing the option value and
    *          associated summary statistics.
    *
    * Notes:
    *    - The option value is the sample mean of the pathwise present values.
    *    - The standard error is computed from the sample variance.
    *
    ---------------------------------------------------------------------------------------------------*/

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

        double variance, standardError;

        if (config.useAntithetic) {
            // SE is computed from the N/2 pairwise averages rather than treating all N paths as independent
            const int half = N / 2;
            double pairSum = 0.0, pairSumSq = 0.0;
            for (int i = 0; i < half; ++i) {
                double y = (pv[i] + pv[i + half]) / 2.0;
                pairSum += y;
                pairSumSq += y * y;
            }
            double pairMean = pairSum / half;
            variance = std::max(0.0, pairSumSq / half - pairMean * pairMean);
            standardError = std::sqrt(variance / half);
        } else {
            variance = std::max(0.0, sumSq / N - optionValue * optionValue);
            standardError = std::sqrt(variance / N); //standard error
        }

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
    /*-------------------------------------------------------------------------------------------------
    * SIMULATE PATHS USING THE PRICER'S STORED MODEL AND CONFIGURATION
    *
    * This member function provides a class-level wrapper around the standalone
    * simulation routine. It uses the stochastic process owned by the pricer
    * together with the stored numerical configuration.
    *
    * Parameters:
    *    - S0  Initial asset price.
    *
    * Returns: A PathData object containing simulated paths and zero-initialised
    *          cashflow storage.
    *
    ---------------------------------------------------------------------------------------------------*/
    PathData LSMPricer::simulatePaths(double S0) const
    {
        return lsm::engine::simulatePaths(S0, process, config);
    }

    SimulationResult LSMPricer::price(double S0) {
        // simulate paths
        PathData data = simulatePaths(S0);
        const int N = data.numPaths;
        const int T = data.numTimeSteps;

        // european benchmark - mean discounted terminal payoff
        const double totalDisc = std::exp(-config.riskFreeRate * config.maturity);
        double euSum = 0.0;
        for (int i = 0; i < N; ++i)
            euSum += payoff.payoff(data.paths[i][T]);
        const double europeanValue = (euSum / N) * totalDisc;

        // backward induction returns per path PV at time 0
        std::vector<double> pv = backwardInduction(data);

        // compute statistics
        return computeOptionValue(pv, europeanValue, N, T);
    }
    
    std::pair<lsm::engine::SimulationResult, lsm::engine::PathData> LSMPricer::priceWithData(double S0) {
        // simulate paths
        PathData data = simulatePaths(S0);
        const int N = data.numPaths;
        const int T = data.numTimeSteps;

        // european benchmark - mean discounted terminal payoff
        const double totalDisc = std::exp(-config.riskFreeRate * config.maturity);
        double euSum = 0.0;
        for (int i = 0; i < N; ++i)
            euSum += payoff.payoff(data.paths[i][T]);
        const double europeanValue = (euSum / N) * totalDisc;

        // backward induction returns per path PV at time 0
        std::vector<double> pv = backwardInduction(data);

        // compute statistics
        SimulationResult result = computeOptionValue(pv, europeanValue, N, T);
        
        return std::make_pair(result, data);
    }
    
    } // namespace engine
} // namespace lsm
