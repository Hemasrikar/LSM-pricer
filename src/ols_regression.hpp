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
    std::vector<bool> itm, // Boolean to know if it's in the money.
    lms::BasisSet basis);