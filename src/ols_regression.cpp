#include <functional>
#include <cmath>
#include <vector>
#include <stdexcept>
// For finding header files necessary used: https://libeigen.gitlab.io/eigen/docs-nightly/group__QuickRefPage.html
#include <Eigen/Dense>
#include "mc_paths.hpp"
#include "lsm_pricer.hpp"
#include "option_payoff.hpp"
#include "basis_functions.hpp"
#include "underlying_sde.hpp"

namespace lsm {
    namespace engine {

// https://libeigen.gitlab.io/eigen/docs-nightly/group__TutorialMatrixClass.html#:~:text=typedef%20Matrix%3Cdouble%2C%20Dynamic%2C%20Dynamic%3E%20MatrixXd%3B
Eigen::MatrixXd datapoints;

// https://stackoverflow.com/questions/40852757/c-how-to-convert-stdvector-to-eigenmatrixxd

// OLS regressors
std::vector<double> ols_parameters;


Eigen::MatrixXd buildDesignMatrix(
    std::vector<double> S_t,
    std::vector<bool> itm, // Boolean to know if it's in the money.
    lsm::core::BasisSet& basis)
{
    int K = basis.basis.size();
    int n_itm = 0;
    for (bool b : itm) if (b) ++n_itm;
    Eigen::MatrixXd X(n_itm, K); // https://libeigen.gitlab.io/eigen/docs-nightly/group__TutorialMatrixClass.html#:~:text=this%20page.-,Resizing
    int row = 0;
    for (int i = 0; i < static_cast<int>(S_t.size()); ++i) {
        if (!itm[i]) continue;
        for (int k = 0; k < K; ++k)
            X(row, k) = basis.basis[k]->evaluate(S_t[i]);
        ++row;
    }
    return X;
}

std::vector<bool> getITMVector(
    std::vector<double> S_t,
    lsm::core::OptionPayoff* payoff_function
)
{
    std::vector<bool> itm(S_t.size());
    for (size_t i = 0; i < S_t.size(); ++i) {
        itm[i] = payoff_function->InTheMoney(S_t[i]); // Use the OptionPayoff's InTheMoney method to determine ITM status
    }
    return itm;
};

std::vector<double> buildYVector(
    std::vector<double> cashflows, // Cashflows from t+1 (assume to be already updated by previous backward step)
    std::vector<bool> itm,         // Boolean to know if it's in the money.
    double discount_factor         // exp(-r * dt)
)
{
    std::vector<double> y;
    for (int i = 0; i < static_cast<int>(itm.size()); ++i) {
        if (itm[i]) {
            y.push_back(cashflows[i] * discount_factor);
        }
    }
    return y;
};


std::vector<double> Ols_regression(
    std::vector<std::vector<double>>& paths,
    std::size_t t,
    std::vector<double>& cashflows,
    std::vector<bool>& itm,
    double discount_factor,
    lsm::core::BasisSet& basis)
{
    std::size_t N = paths.size();
    int K = basis.basis.size();

    // Extract stock prices at time t across all paths
    std::vector<double> S_t(N);
    for (std::size_t i = 0; i < N; ++i)
        S_t[i] = paths[i][t];

    // Build design matrix X (ITM paths only) and discounted Y vector
    Eigen::MatrixXd X = buildDesignMatrix(S_t, itm, basis);
    std::vector<double> Y_vec = buildYVector(cashflows, itm, discount_factor);
    Eigen::VectorXd Y = Eigen::Map<Eigen::VectorXd>(Y_vec.data(), static_cast<int>(Y_vec.size()));

    // https://libeigen.gitlab.io/eigen/docs-3.3/group__LeastSquares.html#:~:text=and%20decompositions%20.-,Using%20the%20QR%20decomposition,-The%20solve()%20method
    Eigen::VectorXd beta = X.colPivHouseholderQr().solve(Y);  // Solve OLS via QR decomposition: beta = (X'X)^{-1} X'Y

    std::vector<double> C_hat(N, 0.0); // Predict continuation values for all paths; OTM paths get 0.0
    for (std::size_t i = 0; i < N; ++i) {
        if (!itm[i]) continue;
        double val = 0.0;
        for (int k = 0; k < K; ++k)
            val += beta(k) * basis.basis[k]->evaluate(S_t[i]);
        C_hat[i] = val;
    }
    return C_hat;
}
// Potential discussion on code performance: https://scicomp.stackexchange.com/questions/3159/is-it-a-good-idea-to-use-vectorvectordouble-to-form-a-matrix-class-for-high

    }
}
