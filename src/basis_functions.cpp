#include "basis_functions.hpp"
#include <cmath>
#include <vector>

namespace lms {

// ConstantBasis
double ConstantBasis::evaluate(double x) const {
    return 1.0;
}

std::string ConstantBasis::name() const {
    return "Constant";
}

// MonomialBasis
MonomialBasis::MonomialBasis(int power) {
    if (power < 0) {
        throw std::invalid_argument("power must be greater than 0");
    }
    power_ = power;
}

double MonomialBasis::evaluate(double x) const {
    return std::pow(x, power_);
}

std::string MonomialBasis::name() const {
    return "x^" + std::to_string(power_);
}

// LaguerrePolynomial
LaguerrePolynomial::LaguerrePolynomial(int order) {
    if (order < 0 || order > 5) {
        throw std::invalid_argument("order must be 0 to 5");
    }
    order_ = order;
}

double LaguerrePolynomial::evaluate(double x) const {
    if (x < 0.0) {
        x = 0.0;
    }
    double e = std::exp(-x / 2.0);
    switch (order_) {
        case 0: return e;
        case 1: return e * (1.0 - x);
        case 2: return e * (1.0 - 2.0*x + 0.5*pow(x, 2));
        case 3: return e * (1.0 - 3.0*x + 1.5*pow(x, 2) - pow(x, 3)/6.0);
        case 4: return e * (1.0 - 4.0*x + 3.0*pow(x, 2) - (2.0/3.0)*pow(x, 3) + pow(x, 4)/24.0);
        case 5: return e * (1.0 - 5.0*x + 5.0*pow(x, 2) - (5.0/3.0)*pow(x, 3) + (5.0/24.0)*pow(x, 4) - pow(x, 5)/120.0);
        default: return 0.0;
    }
}

std::string LaguerrePolynomial::name() const {
    return "Laguerre_L" + std::to_string(order_);
}

// BasisSet
void BasisSet::makeLaguerreSet(int numTerms) {
    basis.push_back(new ConstantBasis());
    for (int i = 0; i < numTerms; i++) {
        basis.push_back(new LaguerrePolynomial(i));
    }
}

void BasisSet::makeMonomialSet(int numTerms) {
    basis.push_back(new ConstantBasis());
    for (int i = 1; i <= numTerms; i++) {
        basis.push_back(new MonomialBasis(i));
    }
}

}