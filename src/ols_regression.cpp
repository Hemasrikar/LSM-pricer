#include <functional>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <Eigen/Dense>
#include "mc_paths.hpp"
#include "lsm_pricer.hpp"
#include "option_payoff.hpp"
#include "basis_functions.hpp"
#include "underlying_sde.hpp"

namespace lsm {
    namespace engine {

        Eigen::MatrixXd buildDesignMatrix(
            const std::vector<double>& S_t,
            const std::vector<bool>& itm, // Boolean to know if it's in the money.
            const lsm::core::BasisSet& basis,
            double strike)
        {
            auto K = basis.size();
            int n_itm = 0;
            for (bool b : itm) if (b) ++n_itm;
            Eigen::MatrixXd X(n_itm, K); 
            auto ptrs = basis.basisPtrs();
            int row = 0;
            for (size_t i = 0; i < S_t.size(); ++i) {
                if (!itm[i]) continue;
                for (size_t k = 0; k < K; ++k)
                    X(row, k) = ptrs[k]->evaluate(S_t[i] / strike);
                ++row;
            }
            return X;
        }

        std::vector<bool> getITMVector(
            const std::vector<double>& S_t,
            const lsm::core::OptionPayoff& payoff_function
        )
        {
            std::vector<bool> itm(S_t.size());
            for (size_t i = 0; i < S_t.size(); ++i) {
                itm[i] = payoff_function.InTheMoney(S_t[i]); // Use the OptionPayoff's InTheMoney method to determine ITM status
            }
            return itm;
        };

        std::vector<double> buildYVector(
            const std::vector<double>& cashflows, // Cashflows from t+1 (assume to be already updated by previous backward step)
            const std::vector<bool>& itm,         // Boolean to know if it's in the money.
            const double discount_factor         // exp(-r * dt)
        )
        {
            std::vector<double> y;
            for (size_t i = 0; i < itm.size(); ++i) {
                if (itm[i]) {
                    y.push_back(cashflows[i] * discount_factor);
                }
            }
            return y;
        };


        std::vector<double> Ols_regression(
        const std::vector<std::vector<double>>& paths,
            const std::size_t t,
            const std::vector<double>& cashflows,
            const std::vector<bool>& itm,
            double discount_factor,
            const lsm::core::BasisSet& basis,
            double strike)
        {
            auto N = paths.size();
            auto K = basis.size();

            // Extract stock prices at time t across all paths
            std::vector<double> S_t(N);
            for (size_t i = 0; i < N; ++i)
                S_t[i] = paths[i][t];

            // Build design matrix X (ITM paths only) and discounted Y vector
            Eigen::MatrixXd X = buildDesignMatrix(S_t, itm, basis, strike);
            std::vector<double> Y_vec = buildYVector(cashflows, itm, discount_factor);
            Eigen::VectorXd Y = Eigen::Map<Eigen::VectorXd>(Y_vec.data(), Y_vec.size());

            Eigen::VectorXd beta = X.colPivHouseholderQr().solve(Y);  // Solve OLS via QR decomposition: beta = (X'X)^{-1} X'Y

            auto ptrs = basis.basisPtrs();
            std::vector<double> C_hat(N, 0.0);
            for (size_t i = 0; i < N; ++i) {
                if (!itm[i]) continue;
                double val = 0.0;
                for (size_t k = 0; k < K; ++k)
                    val += beta[k] * ptrs[k]->evaluate(S_t[i] / strike);
                C_hat[i] = val;
            }
            return C_hat;
        }

    } // namespace engine
} // namespace lsm
