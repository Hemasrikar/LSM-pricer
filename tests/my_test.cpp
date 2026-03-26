#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "basis_functions.hpp"
#include "ols_regression.hpp"
#include "mc_paths.hpp"
#include "option_payoff.hpp"

using namespace lsm::core;
using namespace lsm::engine;

TEST_CASE("0 == 0", "[tests]")
{
	REQUIRE(0 == 0);
}

TEST_CASE("1 + 1 == 2", "[tests]")
{
	REQUIRE(1 + 1 == 2);
}


TEST_CASE("OLS: DesignMatrixTest with Monomial Basis", "ols")
{
std::vector<double> S_t = {100, 110, 90, 95, 105};
std::vector<bool> itm = {false, true, true, true, false};
lsm::core::BasisSet basis;
basis.makeMonomialSet(3);

Eigen::MatrixXd X = buildDesignMatrix(S_t, itm, basis);

std::cout << "Design Matrix X: \n" << X << std::endl;

REQUIRE(X.rows() == 3); // Only 3 in-the-money points
REQUIRE(X.cols() == 4); // 4 basis functions (including constant)
}


TEST_CASE("OLS: DesignMatrixTest with Laguerre Polynomials", "ols1")
{
std::vector<double> S_t = {100, 110, 90, 95, 105};
std::vector<bool> itm = {false, true, true, true, false};
lsm::core::BasisSet basis;
basis.makeLaguerreSet(3);

Eigen::MatrixXd X = buildDesignMatrix(S_t, itm, basis);

std::cout << "Design Matrix X:\n" << X << std::endl;

REQUIRE(X.rows() == 3); // Only 3 in-the-money points
REQUIRE(X.cols() == 4); // 4 basis functions (including constant)
}


TEST_CASE("MC: generatePaths produces correct dimensions", "[mc_paths]")
{
GeometricBrownianMotion gbm(0.05, 0.2);
RNG rng;
std::size_t N = 100; // number of paths
std::size_t n = 50;  // number of time steps
double S0 = 1.0; // initial stock price
double T  = 1.0; // Length of time horizon

auto paths = generatePaths(gbm, S0, T, n, N, rng);

std::cout << "paths[0]: ";
for (double v : paths[0]) std::cout << v << " ";
std::cout << std::endl;

REQUIRE(paths.size() == N);        // N paths
REQUIRE(paths[0].size() == n + 1); // n+1 points per path (includes S0)
REQUIRE(paths[0][0] == S0);        // all paths start at S0
REQUIRE(paths[N-1][0] == S0);
}


// Numerical example from Longstaff-Schwartz (2001) paper, pages 116-117.
// 8 paths, strike K=1.10, r=6%, dt=1 year, monomial basis {1, X, X^2}.
// At t=2 the paper runs the regression and gives known continuation values.
// This test verifies fitAndPredict reproduces those values.
TEST_CASE("OLS: fitAndPredict matches paper example at t=2", "[ols]")
{
// Stock price paths from Table 1 of the paper (paths[i][t])
// We only need columns t=2 and t=3 for this regression step
std::vector<std::vector<double>> paths = {
    {1.00, 1.09, 1.08, 1.34}, // path 1
    {1.00, 1.16, 1.26, 1.54}, // path 2
    {1.00, 1.22, 1.07, 1.03}, // path 3
    {1.00, 0.93, 0.97, 0.92}, // path 4
    {1.00, 1.11, 1.56, 1.52}, // path 5
    {1.00, 0.76, 0.77, 0.90}, // path 6
    {1.00, 0.92, 0.84, 1.01}, // path 7
    {1.00, 0.88, 1.22, 1.34}  // path 8
};

// Cashflows at t=3: payoff = max(1.10 - S, 0)
std::vector<double> cashflows = {0.00, 0.00, 0.07, 0.18, 0.00, 0.20, 0.09, 0.00};

double r              = 0.06;
double dt             = 1.0;
double discount_factor = std::exp(-r * dt); // 0.94176

// Use getITMVector with Put_payoff to build itm — this is how lsm_pricer will call it
Put_payoff put(1.10);
std::vector<double> S_t2 = {1.08, 1.26, 1.07, 0.97, 1.56, 0.77, 0.84, 1.22};
std::vector<bool> itm = getITMVector(S_t2, put);

// Basis {1, X, X^2} i.e. monomial degree 2 gives 3 functions (paper uses constant + X + X^2)
lsm::core::BasisSet basis;
basis.makeMonomialSet(2);

std::vector<double> C_hat = Ols_regression(paths, 2, cashflows, itm, discount_factor, basis);

std::cout << "C_hat: ";
for (double v : C_hat) std::cout << v << " ";
std::cout << std::endl;

// OTM paths (2, 5, 8) i.e. index 1, 4, 7 must be 0.0
REQUIRE(C_hat[1] == 0.0);
REQUIRE(C_hat[4] == 0.0);
REQUIRE(C_hat[7] == 0.0);

// ITM paths check against paper's continuation values (table 2, p.117)
REQUIRE(C_hat[0] == Approx(0.0369).epsilon(0.01));
REQUIRE(C_hat[2] == Approx(0.0461).epsilon(0.01));
REQUIRE(C_hat[3] == Approx(0.1176).epsilon(0.01));
REQUIRE(C_hat[5] == Approx(0.1520).epsilon(0.01));
REQUIRE(C_hat[6] == Approx(0.1565).epsilon(0.01));
}