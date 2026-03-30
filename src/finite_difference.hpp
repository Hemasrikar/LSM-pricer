#pragma once
#include <vector>
#include "option_payoff.hpp"
#include "underlying_sde.hpp"

namespace lsm {
namespace fd {

class FDPricer {
public:
    FDPricer(const lsm::core::GeometricBrownianMotion& gbm,
             const lsm::core::OptionPayoff& payoff,
             int stockSteps = 1000,
             int timeStepsPerYear = 40000);

    double price(double S0, double T) const;

private:
    const lsm::core::GeometricBrownianMotion& gbm_;
    const lsm::core::OptionPayoff& payoff_;
    int M_;
    int N_per_year_;
};

} 
} 
