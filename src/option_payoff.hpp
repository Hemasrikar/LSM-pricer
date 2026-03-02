#pragma once

// Put Functions and ITM Functions
class Put_payoff
{
private:
    double K;

public:
    Put_payoff(double strike);
    double payoff(double S) const;
    bool InTheMoney_Put(double S) const;
};

// Call Functions and ITM Functions
class Call_payoff
{
private:
    double K;

public:
    Call_payoff(double strike);
    double payoff(double S) const;
    bool InTheMoney_Call(double S) const;
};


