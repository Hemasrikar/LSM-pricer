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
