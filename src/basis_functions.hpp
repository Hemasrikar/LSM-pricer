#pragma once

#include "lsm_types.hpp"
#include <cmath>
#include <string>
#include <stdexcept>


//  ConstantBasis  
// - intercept term; evaluates to 1.0 for any given values

class ConstantBasis : public BasisFunction {
public:
    double evaluate(double x) const;
    std::string name() const;
};