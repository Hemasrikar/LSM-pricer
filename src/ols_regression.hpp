#pragma once

#include <functional>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <Eigen/Dense>
#include "mc_paths.hpp"
#include "option_payoff.hpp"
#include "basis_functions.hpp"

namespace lsm {
    namespace engine {

Eigen::MatrixXd buildDesignMatrix(
    std::vector<double> S_t,
    std::vector<bool> itm,
    lsm::core::BasisSet& basis);

std::vector<bool> getITMVector(
    std::vector<double> S_t,
    lsm::core::OptionPayoff* payoff_function);

std::vector<double> buildYVector(
    std::vector<double> cashflows,
    std::vector<bool> itm,
    double discount_factor);

std::vector<double> Ols_regression(
    std::vector<std::vector<double>>& paths,
    std::size_t t,
    std::vector<double>& cashflows,
    std::vector<bool>& itm,
    double discount_factor,
    lsm::core::BasisSet& basis);

    }
}
