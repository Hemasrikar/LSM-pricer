 // Implements various basis functions used for polynomial regression in the 
 // Least Squares Monte Carlo (LSM) pricing method. These basis functions serve
 // as regressors to approximate the continuation value of American options.

#include "basis_functions.hpp"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>

namespace lsm {
    namespace core{

        // ConstantBasis: Represents a constant function, always returns 1
        // Used as an intercept term in regression models to shift the fitted surface.
        
        double ConstantBasis::evaluate(double /*x*/) const {
            // Returns a constant value of 1, regardless of input x
            return 1.0;
        }

        std::string ConstantBasis::name() const {
            // Returns the name identifier for this basis function
            return "Constant";
        }

        // MonomialBasis: Represents polynomial terms (x^power)
        // Used to build polynomial basis functions such as x, x^2, x^3

        // Constructor: creates a monomial basis with a given exponent
        // param power: the exponent, must be non-negative
        // throws std::invalid_argument if power < 0
        MonomialBasis::MonomialBasis(int power) : power_(power) {
            if (power < 0)
                throw std::invalid_argument("MonomialBasis: power must be non-negative");
        }

        // Evaluates x raised to the power specified in the constructor
        // Uses iterative multiplication to compute x^power accurately
        double MonomialBasis::evaluate(double x) const {
            double result = 1.0;
            // Multiply x by itself 'power' times to compute x^power
            for (int i = 0; i < power_; ++i)
                result *= x;
            return result;
        }

        // Returns a string representation of this basis function
        std::string MonomialBasis::name() const {
            return "x^" + std::to_string(power_);
        }

        // LaguerrePolynomial: Implements damped Laguerre polynomials
        // Constructor: creates a Laguerre polynomial of specified order
        // param order: the polynomial order (must be non-negative)
        // throws std::invalid_argument if order < 0
        LaguerrePolynomial::LaguerrePolynomial(int order) : order_(order) {
            if (order < 0)
                throw std::invalid_argument("LaguerrePolynomial: order must be non-negative");
        }

        // Evaluates the damped Laguerre polynomial at a given point
        // Uses the three term recurrence relation for numerical stability
        double LaguerrePolynomial::evaluate(double x) const {
            if (x < 0.0) x = 0.0;

            // Special case: order 0 returns just the damping factor e^{-x/2}
            if (order_ == 0) return std::exp(-x / 2.0);

            // Initialize the recurrence with base cases for undamped Laguerre polynomials
            double L_prev = 1.0;       
            double L_curr = 1.0 - x;  

            // Apply the three term recurrence to compute higher-order polynomials
            // This iterates from order 1 up to the desired order
            for (int n = 1; n < order_; ++n) {
                // Three-term recurrence: L̃_{n+1}(x) = ((2n+1-x)*L̃_n - n*L̃_{n-1}) / (n+1)
                double L_next = ((2.0*n + 1.0 - x) * L_curr - static_cast<double>(n) * L_prev)
                                / static_cast<double>(n + 1);
                // Shift for next iteration
                L_prev = L_curr;
                L_curr = L_next;
            }

            // Apply the e^{-x/2} damping factor once at the end.
            return std::exp(-x / 2.0) * L_curr;
        }

        // Returns a string representation of this basis function
        std::string LaguerrePolynomial::name() const {
            return "Laguerre_L" + std::to_string(order_);
        }


        // BasisSet for managing a collection of basis functions
        // This class manages multiple basis functions as a set, allowing construction
        // of basis sets such as {1, L_0(x), L_1(x), ...} or {1, x, x^2, ...}.
        // It uses unique_ptr to handle memory management automatically.

        // Returns raw pointers to all basis functions in the set
        std::vector<BasisFunction*> BasisSet::basisPtrs() const {
            std::vector<BasisFunction*> ptrs;
            ptrs.reserve(basis_.size());
            for (const auto& b : basis_)
                ptrs.push_back(b.get());
            return ptrs;
        }

        // Returns the number of basis functions in the set
        std::size_t BasisSet::size() const {
            return basis_.size();
        }

        // Clears all basis functions from the set
        void BasisSet::clear() {
            basis_.clear();
        }


        // makeLaguerreSet: Creates a basis set with Laguerre polynomials
        // Constructs a set: {1, L_0(x), L_1(x), ..., L_{numTerms-1}(x)}
        // The basis always includes a constant intercept term (1).
        // @param numTerms: number of Laguerre polynomial terms to include (not counting the constant intercept)
        // throws std::invalid_argument if numTerms < 1

        void BasisSet::makeLaguerreSet(int numTerms) {
            if (numTerms < 1)
                throw std::invalid_argument("makeLaguerreSet: numTerms must be at least 1");
            basis_.clear();
            // Add the constant intercept term
            basis_.push_back(std::make_unique<ConstantBasis>());
            // Add Laguerre polynomial terms from order 0 to numTerms-1
            for (int i = 0; i < numTerms; ++i)
                basis_.push_back(std::make_unique<LaguerrePolynomial>(i));
        }

        // makeMonomialSet: Creates a basis set with monomial polynomials
        // Constructs a set: {1, x, x^2, x^3, ..., x^numTerms}
        // The basis always includes a constant intercept term (1).
        // param numTerms: the highest power of x to include in the basis
        // throws std::invalid_argument if numTerms < 1

        void BasisSet::makeMonomialSet(int numTerms) {
            if (numTerms < 1)
                throw std::invalid_argument("makeMonomialSet: numTerms must be at least 1");
            basis_.clear();
            // Add the constant intercept term
            basis_.push_back(std::make_unique<ConstantBasis>());
            // Add monomial terms from x^1 to x^numTerms
            for (int i = 1; i <= numTerms; ++i)
                basis_.push_back(std::make_unique<MonomialBasis>(i));
        }

    } // namespace core
} // namespace lsm