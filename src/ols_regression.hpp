#include <functional>
#include <cmath>
#include <vector>
#include <stdexcept>
// For finding header files necessary used: https://libeigen.gitlab.io/eigen/docs-nightly/group__QuickRefPage.html
#include <Eigen/Dense>
#include "mc_paths.hpp"
#include "option_payoff.hpp"
#include "basis_functions.hpp"


Eigen::MatrixXd buildDesignMatrix(
    std::vector<double> S_t,
    std::vector<bool> itm,
    lms::BasisSet basis);

std::vector<bool> getITMVector(
    std::vector<double> S_t,
    OptionPayoff* payoff_function);

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
    lms::BasisSet basis);