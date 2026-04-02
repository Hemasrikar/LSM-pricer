#include "basis_functions.hpp"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>

namespace lsm {
    namespace core{

        // ConstantBasis
        double ConstantBasis::evaluate(double /*x*/) const {
            return 1.0;
        }

        std::string ConstantBasis::name() const {
            return "Constant";
        }

        // MonomialBasis
        MonomialBasis::MonomialBasis(int power) : power_(power) {
            if (power < 0)
                throw std::invalid_argument("MonomialBasis: power must be non-negative");
        }

        double MonomialBasis::evaluate(double x) const {
            double result = 1.0;
            for (int i = 0; i < power_; ++i)
                result *= x;
            return result;
        }

        std::string MonomialBasis::name() const {
            return "x^" + std::to_string(power_);
        }

        // LaguerrePolynomial
        LaguerrePolynomial::LaguerrePolynomial(int order) : order_(order) {
            if (order < 0)
                throw std::invalid_argument("LaguerrePolynomial: order must be non-negative");
        }

        double LaguerrePolynomial::evaluate(double x) const {
            // Stock prices are non-negative; clamp defensively.
            if (x < 0.0) x = 0.0;

            // Three-term recurrence: L_{n+1}(x) = ((2n+1-x)*L_n - n*L_{n-1}) / (n+1)
            if (order_ == 0) return std::exp(-x / 2.0);

            double L_prev = 1.0;       // L_0 (undamped)
            double L_curr = 1.0 - x;  // L_1 (undamped)

            for (int n = 1; n < order_; ++n) {
                double L_next = ((2.0*n + 1.0 - x) * L_curr - static_cast<double>(n) * L_prev)
                                / static_cast<double>(n + 1);
                L_prev = L_curr;
                L_curr = L_next;
            }

            // Apply the e^{-x/2} damping factor once at the end.
            return std::exp(-x / 2.0) * L_curr;
        }

        std::string LaguerrePolynomial::name() const {
            return "Laguerre_L" + std::to_string(order_);
        }

        // BasisSet
        std::vector<BasisFunction*> BasisSet::basisPtrs() const {
            std::vector<BasisFunction*> ptrs;
            ptrs.reserve(basis_.size());
            for (const auto& b : basis_)
                ptrs.push_back(b.get());
            return ptrs;
        }

        std::size_t BasisSet::size() const {
            return basis_.size();
        }

        void BasisSet::clear() {
            basis_.clear();
        }

        // Builds {1, L_0, L_1, ..., L_{numTerms-1}}.
        // numTerms counts only the Laguerre terms, not the constant intercept.
        void BasisSet::makeLaguerreSet(int numTerms) {
            if (numTerms < 1)
                throw std::invalid_argument("makeLaguerreSet: numTerms must be at least 1");
            basis_.clear();
            basis_.push_back(std::make_unique<ConstantBasis>());
            for (int i = 0; i < numTerms; ++i)
                basis_.push_back(std::make_unique<LaguerrePolynomial>(i));
        }

        // Builds {1, x, x^2, ..., x^numTerms}.
        // numTerms is the highest power included, not counting the constant intercept.
        void BasisSet::makeMonomialSet(int numTerms) {
            if (numTerms < 1)
                throw std::invalid_argument("makeMonomialSet: numTerms must be at least 1");
            basis_.clear();
            basis_.push_back(std::make_unique<ConstantBasis>());
            for (int i = 1; i <= numTerms; ++i)
                basis_.push_back(std::make_unique<MonomialBasis>(i));
        }

    } // namespace core
} // namespace lsm