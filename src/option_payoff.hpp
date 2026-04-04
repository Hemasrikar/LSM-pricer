#pragma once
#include <string>

namespace lsm {
    namespace core {

        // Base class
        class OptionPayoff 
        {
        protected:
            double K;  

        public:
            OptionPayoff(double strike);
            virtual ~OptionPayoff() = default;

            double strike() const { return K; }
            virtual double payoff(double S) const = 0;
            virtual bool InTheMoney(double S) const = 0;
            virtual std::string name() const = 0;
        };


        // Put function and ITM
        class Put_payoff : public OptionPayoff 
        {
        public:
            Put_payoff(double strike);

            double payoff(double S) const override;
            bool InTheMoney(double S) const override;
            std::string name() const override;
        };


        //Call function and ITM
        class Call_payoff : public OptionPayoff 
        {
        public:
            Call_payoff(double strike);

            double payoff(double S) const override;
            bool InTheMoney(double S) const override;
            std::string name() const override;
        };

    } //namespace core
} //namespace lsm