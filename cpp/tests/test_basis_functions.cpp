#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "basis_functions.hpp"
#include <cmath>


using namespace lsm::core;


// Constants
double tolerance = 1e-10;


// ConstantBasis

TEST_CASE("ConstantBasis always returns 1.0", "[ConstantBasis]") {
    ConstantBasis consbasis;
    REQUIRE(consbasis.evaluate(0.0) == Approx(1.0));
    REQUIRE(consbasis.evaluate(100.0) == Approx(1.0));
    REQUIRE(consbasis.evaluate(-50.0) == Approx(1.0));
    REQUIRE(consbasis.evaluate(1e12) == Approx(1.0));
}

TEST_CASE("ConstantBasis name is 'Constant'", "[ConstantBasis]") {
    ConstantBasis consbasis;
    REQUIRE(consbasis.name() == "Constant");
}


// MonomialBasis

TEST_CASE("MonomialBasis: power 0 returns 1.0 for all x", "[MonomialBasis]") {
    MonomialBasis m(0);
    REQUIRE(m.evaluate(0.0) == Approx(1.0));
    REQUIRE(m.evaluate(5.0) == Approx(1.0));
    REQUIRE(m.evaluate(-3.0) == Approx(1.0));
}

TEST_CASE("MonomialBasis: power 1 is 1", "[MonomialBasis]") {
    MonomialBasis m(1);
    REQUIRE(m.evaluate(0.0) == Approx(0.0));
    REQUIRE(m.evaluate(3.0) == Approx(3.0));
    REQUIRE(m.evaluate(-2.5) == Approx(-2.5));
}

TEST_CASE("MonomialBasis: power 2 is x^2", "[MonomialBasis]") {
    MonomialBasis m(2);
    REQUIRE(m.evaluate(3.0) == Approx(9.0));
    REQUIRE(m.evaluate(-3.0) == Approx(9.0));
    REQUIRE(m.evaluate(0.0) == Approx(0.0));
}

TEST_CASE("MonomialBasis: power 5 is x^5", "[MonomialBasis]") {
    MonomialBasis m(5);
    REQUIRE(m.evaluate(2.0) == Approx(32.0));
    REQUIRE(m.evaluate(-2.0) == Approx(-32.0));
}

TEST_CASE("MonomialBasis: negative power throws error", "[MonomialBasis]") {
    REQUIRE_THROWS_AS(MonomialBasis(-1), std::invalid_argument);
    REQUIRE_THROWS_AS(MonomialBasis(-100), std::invalid_argument);
}


// LaguerrePolynomial

TEST_CASE("LaguerrePolynomial: order 0 is e^{-x/2}", "[LaguerrePolynomial]") {
    LaguerrePolynomial lagpoly(0);
    for (double x : {0.0, 1.0, 2.0, 5.0, 10.0}) {
        REQUIRE(lagpoly.evaluate(x) == Approx(std::exp(-x / 2.0)).epsilon(tolerance));
    }
}

TEST_CASE("LaguerrePolynomial: order 1", "[LaguerrePolynomial]") {
    LaguerrePolynomial lagpoly(1);
    auto ref = [](double x){ return std::exp(-x / 2.0) * (1.0 - x); };
    for (double x : {0.0, 0.5, 1.0, 2.0, 5.0})
        REQUIRE(lagpoly.evaluate(x) == Approx(ref(x)).epsilon(tolerance));
}

TEST_CASE("LaguerrePolynomial: order 3", "[LaguerrePolynomial]") {
    LaguerrePolynomial lagpoly(3);
    auto ref = [](double x){
        return std::exp(-x / 2.0) * (1.0 - 3.0*x + 1.5*x*x - x*x*x/6.0);
    };
    for (double x : {0.0, 0.5, 1.0, 2.0, 5.0})
        REQUIRE(lagpoly.evaluate(x) == Approx(ref(x)).epsilon(tolerance));
}

TEST_CASE("LaguerrePolynomial: order 4", "[LaguerrePolynomial]") {
    LaguerrePolynomial lagpoly(4);
    auto ref = [](double x){
        return std::exp(-x / 2.0) *
               (1.0 - 4.0*x + 3.0*x*x - (2.0/3.0)*x*x*x + x*x*x*x/24.0);
    };
    for (double x : {0.0, 0.5, 1.0, 2.0, 5.0})
        REQUIRE(lagpoly.evaluate(x) == Approx(ref(x)).epsilon(tolerance));
}

TEST_CASE("LaguerrePolynomial: recurrence consistent across adjacent orders", "[LaguerrePolynomial]") {
    // Three-term recurrence: (n+1)*L_{n+1}(x) = (2n+1-x)*L_n(x) - n*L_{n-1}(x)
    // Verify holds (modulo e^{-x/2} factor) for orders 2..8.
    for (double x : {0.5, 1.0, 3.0, 7.0}) {
        for (int n = 1; n < 8; ++n) {
            double ex = std::exp(-x / 2.0);
            double L_nm1 = LaguerrePolynomial(n - 1).evaluate(x) / ex;
            double L_n = LaguerrePolynomial(n ).evaluate(x) / ex;
            double L_np1 = LaguerrePolynomial(n + 1).evaluate(x) / ex;
            double rhs = ((2.0*n + 1.0 - x) * L_n - n * L_nm1) / (n + 1.0);
            REQUIRE(L_np1 == Approx(rhs).epsilon(1e-9));
        }
    }
}

TEST_CASE("LaguerrePolynomial: negative x returns to 0", "[LaguerrePolynomial]") {
    // evaluate(-1) should equal evaluate(0) for all orders
    for (int order : {0, 1, 2, 3, 5}) {
        LaguerrePolynomial lagpoly(order);
        REQUIRE(lagpoly.evaluate(-1.0) == Approx(lagpoly.evaluate(0.0)).epsilon(tolerance));
        REQUIRE(lagpoly.evaluate(-10.0) == Approx(lagpoly.evaluate(0.0)).epsilon(tolerance));
    }
}

TEST_CASE("LaguerrePolynomial: negative order throws error", "[LaguerrePolynomial]") {
    REQUIRE_THROWS_AS(LaguerrePolynomial(-1), std::invalid_argument);
}

TEST_CASE("LaguerrePolynomial: name formatting", "[LaguerrePolynomial]") {
    REQUIRE(LaguerrePolynomial(0).name() == "Laguerre_L0");
    REQUIRE(LaguerrePolynomial(3).name() == "Laguerre_L3");
}


// BasisSet — makeLaguerreSet

TEST_CASE("BasisSet::makeLaguerreSet: structure", "[BasisSet]") {
    BasisSet bs;
    bs.makeLaguerreSet(3);

    REQUIRE(bs.size() == 4);  // constant + L0 + L1 + L2

    auto ptrs = bs.basisPtrs();
    REQUIRE(ptrs[0]->name() == "Constant");
    REQUIRE(ptrs[1]->name() == "Laguerre_L0");
    REQUIRE(ptrs[2]->name() == "Laguerre_L1");
    REQUIRE(ptrs[3]->name() == "Laguerre_L2");
}

TEST_CASE("BasisSet::makeLaguerreSet: constant is first element", "[BasisSet]") {
    BasisSet bs;
    bs.makeLaguerreSet(1);
    REQUIRE(bs.basisPtrs()[0]->evaluate(42.0) == Approx(1.0));
}

TEST_CASE("BasisSet::makeLaguerreSet: numTerms < 1 throws error", "[BasisSet]") {
    BasisSet bs;
    REQUIRE_THROWS_AS(bs.makeLaguerreSet(0),  std::invalid_argument);
    REQUIRE_THROWS_AS(bs.makeLaguerreSet(-1), std::invalid_argument);
}

TEST_CASE("BasisSet::makeLaguerreSet: clears previous contents", "[BasisSet]") {
    BasisSet bs;
    bs.makeLaguerreSet(5);
    REQUIRE(bs.size() == 6);
    bs.makeLaguerreSet(2);
    REQUIRE(bs.size() == 3);  // not 6+3
}


// BasisSet — makeMonomialSet


TEST_CASE("BasisSet::makeMonomialSet: values at x = 2", "[BasisSet]") {
    BasisSet bs;
    bs.makeMonomialSet(4);
    auto ptrs = bs.basisPtrs();
    REQUIRE(ptrs[0]->evaluate(2.0) == Approx(1.0)); // constant
    REQUIRE(ptrs[1]->evaluate(2.0) == Approx(2.0)); // x
    REQUIRE(ptrs[2]->evaluate(2.0) == Approx(4.0)); // x^2
    REQUIRE(ptrs[3]->evaluate(2.0) == Approx(8.0)); // x^3
}

TEST_CASE("BasisSet::makeMonomialSet: numTerms < 1 throws error", "[BasisSet]") {
    BasisSet bs;
    REQUIRE_THROWS_AS(bs.makeMonomialSet(0),  std::invalid_argument);
    REQUIRE_THROWS_AS(bs.makeMonomialSet(-3), std::invalid_argument);
}

TEST_CASE("BasisSet::makeMonomialSet: clears previous contents", "[BasisSet]") {
    BasisSet bs;
    bs.makeMonomialSet(4);
    REQUIRE(bs.size() == 5);
    bs.makeMonomialSet(1);
    REQUIRE(bs.size() == 2);
}


// BasisSet — size and clear

TEST_CASE("BasisSet::clear empties the set", "[BasisSet]") {
    BasisSet bs;
    bs.makeLaguerreSet(3);
    REQUIRE(bs.size() == 4);
    bs.clear();
    REQUIRE(bs.size() == 0);
    REQUIRE(bs.basisPtrs().empty());
}

TEST_CASE("BasisSet: can rebuild after clear", "[BasisSet]") {
    BasisSet bs;
    bs.makeLaguerreSet(2);
    bs.clear();
    bs.makeMonomialSet(3);
    REQUIRE(bs.size() == 4);
}


// Polymorphism via BasisFunction interface

TEST_CASE("MonomialBasis: name formatting", "[MonomialBasis]") {
    REQUIRE(MonomialBasis(0).name() == "x^0");
    REQUIRE(MonomialBasis(1).name() == "x^1");
    REQUIRE(MonomialBasis(3).name() == "x^3");
}

TEST_CASE("BasisFunction interface: virtual dispatch works for all concrete types", "[polymorphism]") {
    std::vector<std::unique_ptr<BasisFunction>> funcs;
    funcs.push_back(std::make_unique<ConstantBasis>());
    funcs.push_back(std::make_unique<MonomialBasis>(2));
    funcs.push_back(std::make_unique<LaguerrePolynomial>(2));

    const double x = 3.0;
    REQUIRE(funcs[0]->evaluate(x) == Approx(1.0));
    REQUIRE(funcs[1]->evaluate(x) == Approx(9.0));
    REQUIRE(funcs[2]->evaluate(x) == Approx(
        std::exp(-x / 2.0) * (1.0 - 2.0*x + 0.5*x*x)));
}