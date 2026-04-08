#pragma once
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace lsm {
    namespace core {

        // BasisFunction, abstract interface
        class BasisFunction {
        public:
            virtual ~BasisFunction() = default;
            virtual double evaluate(double x) const = 0;
            virtual std::string name() const = 0;
        };

        // ConstantBasis — always returns 1.0; used as the intercept term
        class ConstantBasis : public BasisFunction {
        public:
            double evaluate(double x) const override;
            std::string name() const override;
        };

        // MonomialBasis — evaluates x^power
        class MonomialBasis : public BasisFunction {
        public:
            explicit MonomialBasis(int power);
            double evaluate(double x) const override;
            std::string name() const override;
        private:
            int power_;
        };

        // LaguerrePolynomial — evaluates e^{-x/2} * L_order(x) using three-term
        class LaguerrePolynomial : public BasisFunction {
        public:
            explicit LaguerrePolynomial(int order);
            double evaluate(double x) const override;
            std::string name() const override;
        private:
            int order_;
        };


        // BasisSet owns a collection of basis functions and provides factory
        // methods to populate it
        //makeLaguerreSet(n) : builds {1, L_0, L_1,.., L_{n-1}}
        //makeMonomialSet(n) : builds {1, x, x^2,.., x^n}
        // Both methods clear any existing basis before building.
        // basisPtrs() returns non owning pointers for use by OLS Regressor.
        class BasisSet {
        public:
            BasisSet() = default;

            // unique_ptr members make copyable later so we should be
            // deleting explicitly surfaces the error at the class definition.
            BasisSet(const BasisSet&) = delete;
            BasisSet& operator=(const BasisSet&) = delete;

            // Moves are fine — unique_ptr supports them.
            BasisSet(BasisSet&&) = default;
            BasisSet& operator=(BasisSet&&) = default;

            void makeLaguerreSet(int numTerms);
            void makeMonomialSet(int numTerms);

            std::vector<BasisFunction*> basisPtrs() const;
            std::size_t size() const;
            void clear();

        private:
            std::vector<std::unique_ptr<BasisFunction>> basis_;
        };

    } // namespace core
} // namespace lsm