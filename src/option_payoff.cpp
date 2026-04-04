#include <algorithm>
#include "option_payoff.hpp"

namespace lsm {
    namespace core {
        
        // Defining the base class OptionPayoff 
        // Contains the strike price K parameter to initialise the value
        OptionPayoff::OptionPayoff(double strike) : K(strike) {}

// Put_payoff implementations

        // Derived class Put_payoff 
        // Calls the base class OptionPayoff 
        // As well as ensuring the base class strike price K is used
        Put_payoff::Put_payoff(double strike) : OptionPayoff(strike) {}


        // Defining the member function payoff for Put_payoff
        // Takes one argument S, which is the underlying asset price
        // Having const, allows the function to not modify the objects parameter
        // Put payoff formula max(K - S, 0) returns the payoff value
        // bool Put_payoff to check if the option is ITM by returning true/false
        double Put_payoff::payoff(double S) const 
        {
            return std::max(K - S, 0.0);
        }

        bool Put_payoff::InTheMoney(double S) const
        {
            return S < K;
        }

        std::string Put_payoff::name() const { return "Put"; }


// Call_payoff implementations

        // Similar idea to Put_payoff but this time is for Call_payoff
        Call_payoff::Call_payoff(double strike) : OptionPayoff(strike) {}


        // Same working logic as before
        // With Call payoff formula max(S - K, 0) and checking ITM condition
        double Call_payoff::payoff(double S) const 
        {
            return std::max(S - K, 0.0);
        }

        bool Call_payoff::InTheMoney(double S) const
        {
            return S > K;
        }

        std::string Call_payoff::name() const { return "Call"; }

    } //namespace core
} //namespace lsm
