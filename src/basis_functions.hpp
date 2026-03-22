#pragma once
#include <cmath>
#include <string>
#include <stdexcept>
#include <memory>
#include <vector>

namespace lsm{
    namespace core{

        // ConstantBasis
        // intercept term, evaluates to 1.0 for any given value
        class ConstantBasis : public BasisFunction {
        public:
            double evaluate(double x) const override;
            std::string name() const override;
        };

        // MonomialBasis
        class MonomialBasis : public BasisFunction{
        public:
            explicit MonomialBasis(int power);
            double evaluate(double x) const override;
            std::string name() const override;
        private:
            int power_;
        };

        // LaguerrePolynomial
        class LaguerrePolynomial : public BasisFunction{
        public:
            explicit LaguerrePolynomial(int order);
            double evaluate(double x) const override;
            std::string name() const override;
        private:
            int order_;
        };

        // BasisSet holds a list of basis functions and provides two ways to fill it.
        // use makeMonomialSet() for polynomial basis functions (1, x, x^2, x^3 and so on)
        // use makeLaguerreSet() for Laguerre polynomial basis functions
        // Owns its basis functions via unique_ptr.
        class BasisSet{
        public:
            std::vector<std::unique_ptr<BasisFunction>> basis;

            // Returns plain (non-owning) pointers for use by OLSRegressor
            std::vector<BasisFunction*> basisPtrs() const;

            void makeLaguerreSet(int numTerms);
            void makeMonomialSet(int numTerms);
        };

        class BasisFunction {
        public:
            virtual ~BasisFunction() = default;
            virtual double evaluate(double x) const = 0;
            virtual std::string name() const = 0;
        };

    } // namespace core
} // namespace lsm