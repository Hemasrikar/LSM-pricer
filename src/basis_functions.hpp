#pragma once
#include <cmath>
#include <string>
#include <stdexcept>
#include <memory>
#include <vector>

namespace lsm{
    namespace core{

        class BasisFunction {
        public:
            virtual ~BasisFunction() = default;
            virtual double evaluate(double x) const = 0;
            virtual std::string name() const = 0;
        };

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
        
        // makeLaguerreSet(n): builds {1, L0, L1, ..., L_{n-1}} — a constant intercept
        //   plus n weighted Laguerre polynomials
        //   Clears any existing basis before building. Throws if n < 1 or n > 5.

        // makeMonomialSet(n): builds {1, x, x^2, ..., x^n} — a constant intercept
        // plus monomials up to degree n. Clears any existing basis before building.
        // Owns its basis functions via unique_ptr.
        class BasisSet{
        public:
            std::vector<std::unique_ptr<BasisFunction>> basis;

            // Returns plain (non-owning) pointers for use by OLSRegressor
            std::vector<BasisFunction*> basisPtrs() const;

            void makeLaguerreSet(int numTerms);
            void makeMonomialSet(int numTerms);
        };

    } // namespace core
} // namespace lsm