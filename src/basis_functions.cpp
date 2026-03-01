#include "basis_functions.hpp"
#include <cmath>

namespace lms{

//  ConstantBasis
double ConstantBasis::evaluate(double /*x*/) const { return 1.0; }
std::string ConstantBasis::name() const { return "Const"; }

}

// Monomial Basis Function

MonomialBasis::MonomialBasis(int power) {
    if (power < 0) {
        throw std::invalid_argument("power must be >= 0");
    }
    power_ = power;
}

double MonomialBasis::evaluate(double x) const {
    return std::pow(x, power_);
}

std::string MonomialBasis::name() const {
    return "x^" + std::to_string(power_);
}