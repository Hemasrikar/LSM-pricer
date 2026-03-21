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

    //stores payoff for each path
    std::vector<double> cashflow(numPaths);

    //stores exercise time for each path
    std::vector<int> exerciseTime(numPaths);

    //initialise the maturity payoff
    // At maturity there is no continuation decision left, so the payoff is
    // just the terminal payoff

    for (int i = 0; i < numPaths; i++){
        cashflow[i] = (*payoff)(data.paths[i][numTimes]);
        exerciseTime[i] = numTimes;
        data.cashFlows[i][numTimes] = cashflow[i];
    }

    //implementing backward induction by moving backwards in time
    for (int t = numTimes - 1; t >= 1; t--){
        // X will store current stock prices for in-the-money paths
        std::vector<double> X;
        // Y will store discounted future cashflows for those same paths
        std::vector<double> Y;
        // pathIndex remembers which original path each row belongs to
        std::vector<int> pathIndex;

        for (int i = 0; i < numPaths; i++) {
            // Value if we exercise immediately at time t
            double exerciseValue = (*payoff)(data.paths[i][t]);
            //We are only using paths that are in the money for regression
            if (exerciseValue > 0.0) {
                // Discount the currently stored future cashflow back to time t
                double discountedFutureCashflow =
                    cashflow[i] * std::exp(-config.riskFreeRate * dt * (exerciseTime[i] - t));
                // Current stock price goes into X
                X.push_back(data.paths[i][t]);
                // Discounted future payoff goes into Y
                Y.push_back(discountedFutureCashflow);
                // Remember which path this came from
                pathIndex.push_back(i);
            }
        }
        // Need at least as many data points as basis functions
        if (X.size() >= basis ->basis.size()) {
            // Fit continuation value as a function of current stock price
            std::vector<double> coeffs = regressor.fit(X, Y);
            //compare exercise and continuation
            for (int k = 0; k < static_cast<int>(pathIndex.size()); k++) {
                int i = pathIndex[k];

                double exerciseValue = (*payoff)(data.paths[i][t]);
                 // Estimated continuation value from the regression
                double continuationValue = regressor.predict(data.paths[i][t], coeffs);

                if (exerciseValue > continuationValue) {
                    // Replace the old future payoff with payoff from exercising now
                    cashflow[i] = exerciseValue;

                    // Record that exercise now happens at time t
                    exerciseTime[i] = t;
                    // Store the exercise payoff in the cash-flow matrix
                    data.cashFlows[i][t] = exerciseValue;
                    // Remove any later cashflows because the option is dead after exercise
                    for (int s = t + 1; s <= numTimes; s++) {
                        data.cashFlows[i][s] = 0.0;
                    }
                }
            }
        }
    }
   //discount all chosen cashflows back to time 0
    std::vector<double> presentValue(numPaths);

    for (int i = 0; i < numPaths; i++) {
    presentValue[i] = cashflow[i] * std::exp(-config.riskFreeRate * dt * exerciseTime[i]);
    }   

    //return present value of path
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
