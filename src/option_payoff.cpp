#include <algorithm> 
#include "option_payoff.hpp"


OptionPayoff::OptionPayoff(double strike) : K(strike) {}

// Put_payoff implementations
Put_payoff::Put_payoff(double strike) : OptionPayoff(strike) {}

double Put_payoff::payoff(double S) const 
{
    return std::max(K - S, 0.0);
}

bool Put_payoff::InTheMoney(double S) const
{
    return S < K;
}


// Call_payoff implementations
Call_payoff::Call_payoff(double strike) : OptionPayoff(strike) {}

double Call_payoff::payoff(double S) const 
{
    return std::max(S - K, 0.0);
}

bool Call_payoff::InTheMoney(double S) const
{
    return S > K;
}