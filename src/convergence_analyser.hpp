#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "lsm_pricer.hpp"
#include "bs_pricer.hpp"
#include "underlying_sde.hpp"
#include "option_payoff.hpp"
#include "basis_functions.hpp"

namespace lsm{
    namespace analysis{

        class ConvergenceAnalyser {
        // defines the variables which only the class can see
        private:
            double S0, r, sigma, K, T;
            std::function<std::unique_ptr<lsm::core::StochasticProcess>(double, double)> sdeFactory;
            const lsm::core::OptionPayoff& payoffType;
            const lsm::core::BasisSet& basisType;
            std::function<void(lsm::core::BasisSet&, int)> basisFactory;
            int order, fixedPathCount, fixedNumDates;

            std::string paramString() const;

        // defines the functions which can be accessed outside of the class
        public:
            ConvergenceAnalyser(double s, double r, double sigma, double K, double T,
                std::function<std::unique_ptr<lsm::core::StochasticProcess>(double, double)> sdeFactory,
                const lsm::core::OptionPayoff& payoff,
                const lsm::core::BasisSet& basis,
                std::function<void(lsm::core::BasisSet&, int)> basisFactory,
                int order, int fixedPathCount, int fixedNumDates);

            // helper functions to get the prices

            double getBSPrice();

            double getFDPrice();

            double getLSMPrice(unsigned seed, int numExerciseDates, int order, int numPaths);


            // functions to run the convergence analysis

            void runBenchmark();

            void runPathConvergence(std::vector<int> pathCounts);

            void runDatesConvergence(std::vector<int> exerciseDatesList);

            void runOrderConvergence(std::vector<int> orders);

            void runFDConvergence(std::vector<int> timeCounts, int stockSteps);

            void runSeedStability();
        };
    } // namespace analysis

} // namespace lsm