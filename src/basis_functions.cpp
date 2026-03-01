#include "basis_functions.hpp"
#include <cmath>

namespace lms{

//  ConstantBasis
double ConstantBasis::evaluate(double /*x*/) const { return 1.0; }
std::string ConstantBasis::name() const { return "Const"; }

}