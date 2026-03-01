#include <cmath>
#include <algorithm> 

#include "option_payoff.hpp"

class Put_payoff 
{
private:
    double K;  

public:
    Put_payoff(double strike) : K(strike) {}

    double payoff(double S) const 
    {
        return std::max(K - S, 0.0);
    }
};


class Call_payoff 
{
private:
    double K;  

public:
    Call_payoff(double strike) : K(strike) {}

    double payoff(double S) const 
    {
        return std::max(S - K, 0.0);
    }
};






