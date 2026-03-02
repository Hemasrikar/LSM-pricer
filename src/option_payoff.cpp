#include <cmath>
#include <algorithm> 

#include "option_payoff.hpp"

// Payoff functions of both put and call options
// K = strike price
// S = underlying price

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

// Functions to check if the call and put payoff functions are in-the-money or out-the-money
// Use bool to check if the payoff functions is in-the-money
// Otherwise, it will return as false which implies it is out-the-money


bool InTheMoney_Put(double S, double K) 
{
    return S < K;
}

bool InTheMoney_Call(double S, double K) 
{
    return S > K;
}







