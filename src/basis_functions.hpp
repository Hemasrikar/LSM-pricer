#pragma once

#include "lsm_types.hpp"
#include <cmath>
#include <string>
#include <stdexcept>
#include <vector>

namespace lms {

// ConstantBasis
// intercept term, evaluates to 1.0 for any given value
class ConstantBasis : public BasisFunction {
public:
    double evaluate(double x) const;
    std::string name() const;
};

// MonomialBasis
class MonomialBasis : public BasisFunction {
public:
    MonomialBasis(int power);

    double evaluate(double x) const;
    std::string name() const;

private:
    int power_;
};

// LaguerrePolynomial
class LaguerrePolynomial : public BasisFunction {
public:
    LaguerrePolynomial(int order);

    double evaluate(double x) const;
    std::string name() const;

private:
    int order_;
};

// BasisSet holds a list of basis functions and provides two ways to fill it
// use makeMonomialSet() for polynomial basis functions (1, x, x^2, x^3 and so on)
// use makeLaguerreSet() for Laguerre polynomial basis functions
class BasisSet {
public:
    std::vector<BasisFunction*> basis;

    void makeLaguerreSet(int numTerms);
    void makeMonomialSet(int numTerms);
};

} // namespace lms