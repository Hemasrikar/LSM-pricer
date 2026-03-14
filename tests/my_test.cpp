#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "basis_functions.hpp"
#include "ols_regression.hpp"

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
lms::BasisSet basis;
basis.makeMonomialSet(3);

Eigen::MatrixXd X = buildDesignMatrix(S_t, itm, basis);

std::cout << "Design Matrix X: \n" << X << std::endl;

REQUIRE(X.rows() == 3); // Only 3 in-the-money points
REQUIRE(X.cols() == 5); // 4 basis functions (including constant)
}


TEST_CASE("OLS: DesignMatrixTest with Laguerre Polynomials", "ols1")
{
std::vector<double> S_t = {100, 110, 90, 95, 105};
std::vector<bool> itm = {false, true, true, true, false};
lms::BasisSet basis;
basis.makeLaguerreSet(3);

Eigen::MatrixXd X = buildDesignMatrix(S_t, itm, basis);

std::cout << "Design Matrix X:\n" << X << std::endl;

REQUIRE(X.rows() == 3); // Only 3 in-the-money points
REQUIRE(X.cols() == 5); // 4 basis functions (including constant)
}