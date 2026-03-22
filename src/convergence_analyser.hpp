#pragma once

#include <vector>
#include <string>
#include "lsm_pricer.hpp"
#include "bs_pricer.hpp"

namespace lsm{
    namespace analysis{

        double getBSPrice(double S0, double r, double sigma, double K, double T);

        double getFDPrice();

        double getLSMPrice(unsigned seed);

        // void runConvergence();

        void runBenchmark();

    } // namespace analysis

} // namespace lsm




