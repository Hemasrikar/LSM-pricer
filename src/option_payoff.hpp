#pragma once

// Put Functions
class Put_payoff
{
private:
    double K;

public:
    Put_payoff(double strike);
    double payoff(double S) const;
};

// Call Functions
class Call_payoff
{
private:
    double K;

public:
    Call_payoff(double strike);
    double payoff(double S) const;
};

// In-The-Money Functions
bool InTheMoney_Put(double S, double K);
bool InTheMoney_Call(double S, double K);