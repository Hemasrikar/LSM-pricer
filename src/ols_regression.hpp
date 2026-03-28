#pragma once

#include <functional>
#include <cmath>
#include <vector>
#include <stdexcept>
// For finding header files necessary used: https://libeigen.gitlab.io/eigen/docs-nightly/group__QuickRefPage.html
#include <Eigen/Dense>
#include "mc_paths.hpp"
#include "option_payoff.hpp"
#include "basis_functions.hpp"

namespace lsm {
    namespace engine {

Eigen::MatrixXd buildDesignMatrix(
    const std::vector<double>& S_t,
    const std::vector<bool>& itm,
    const lsm::core::BasisSet& basis);

std::vector<bool> getITMVector(
    const std::vector<double>& S_t,
    const lsm::core::OptionPayoff& payoff_function);

std::vector<double> buildYVector(
    const std::vector<double>& cashflows,
    const std::vector<bool>& itm,
    double discount_factor);

std::vector<double> Ols_regression(
    const std::vector<std::vector<double>>& paths,
    std::size_t t,
    const std::vector<double>& cashflows,
    const std::vector<bool>& itm,
    double discount_factor,
    const lsm::core::BasisSet& basis);

    }
}
