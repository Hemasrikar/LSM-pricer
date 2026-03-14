#pragma once

// Base class
class OptionPayoff 
{
protected:
    double K;  

public:
    OptionPayoff(double strike);
    virtual ~OptionPayoff() = default;

    virtual double payoff(double S) const = 0;
    virtual bool InTheMoney(double S) const = 0;
};


// Put function and ITM
class Put_payoff : public OptionPayoff 
{
public:
    Put_payoff(double strike);

    double payoff(double S) const;
    bool InTheMoney(double S) const;
};


//Call function and ITM
class Call_payoff : public OptionPayoff 
{
public:
    Call_payoff(double strike);

    double payoff(double S) const;
    bool InTheMoney(double S) const;
};

