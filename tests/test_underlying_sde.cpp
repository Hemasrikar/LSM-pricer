#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "underlying_sde.hpp"
#include <cmath>
#include <stdexcept>
#include <vector>

using namespace lsm::core;

// ------------------------------------------------------------
// Geometric Brownian motion tests
// ------------------------------------------------------------

//TEST 1: Check GBM constructor throws for sigma < 0
TEST_CASE("GeometricBrownianMotion constructor throws for negative sigma", "[underlying_sde]") {
    REQUIRE_THROWS_AS(
        GeometricBrownianMotion(0.05, -0.2),
        std::invalid_argument
    );
}

//TEST 2: GBM stepWithNormal throes for dt < 0:
TEST_CASE("GeometricBrownianMotion stepWithNormal throws for non-positive dt", "[underlying_sde]") {
    GeometricBrownianMotion gbm(0.05, 0.2);
    RNG rng(123);

    REQUIRE_THROWS_AS(gbm.stepWithNormal(100.0, 0.0, 0.3, rng), std::invalid_argument);
    REQUIRE_THROWS_AS(gbm.stepWithNormal(100.0, -0.1, 0.3, rng), std::invalid_argument);
}

//TEST 3: check if implementation is correct for GBM stepWithNormal
//by comparing to closed form expression

TEST_CASE("GeometricBrownianMotion stepWithNormal matches exact formula", "[underlying_sde]") {
    const double r = 0.05;
    const double sigma = 0.2;
    const double s = 100.0;
    const double dt = 0.25;
    const double z = 0.3;

    GeometricBrownianMotion gbm(r, sigma);
    RNG rng(123); 

    double result = gbm.stepWithNormal(s, dt, z, rng);

    double expected = s * std::exp((r - 0.5 * sigma * sigma) * dt
                                   + sigma * std::sqrt(dt) * z);

    REQUIRE(result == Approx(expected));
}

//TEST 4: if sigma = 0 , GBM has to be deterministic:
TEST_CASE("GeometricBrownianMotion is deterministic when sigma is zero", "[underlying_sde]") {
    const double r = 0.05;
    const double sigma = 0.0;
    const double s = 100.0;
    const double dt = 0.25;
    const double z = 1.7; // should not matter when sigma = 0

    GeometricBrownianMotion gbm(r, sigma);
    RNG rng(123);

    double result = gbm.stepWithNormal(s, dt, z, rng);
    double expected = s * std::exp(r * dt);

    REQUIRE(result == Approx(expected));
}

//TEST 5: Check if simulatePaths returns correct sie and initial value:
TEST_CASE("simulatePath returns correct size and initial value", "[underlying_sde]") {
    const double S0 = 100.0;
    const double T = 1.0;
    const std::size_t n = 10;

    GeometricBrownianMotion gbm(0.05, 0.2);
    RNG rng(123);

    std::vector<double> path = gbm.simulatePath(S0, T, n, rng);

    REQUIRE(path.size() == n + 1);
    REQUIRE(path[0] == Approx(S0));
}

//TEST 6: Check simulatePath throws for T <0:
TEST_CASE("simulatePath throws if T is non-positive", "[underlying_sde]") {
    GeometricBrownianMotion gbm(0.05, 0.2);
    RNG rng(123);

    REQUIRE_THROWS_AS(gbm.simulatePath(100.0, 0.0, 10, rng), std::invalid_argument);
    REQUIRE_THROWS_AS(gbm.simulatePath(100.0, -1.0, 10, rng), std::invalid_argument);
}
// ------------------------------------------------------------
// Jump diffusion tests
// ------------------------------------------------------------
//TEST 7: contructor throws for sigma < 0
TEST_CASE("JumpDiffusionProcess constructor throws for negative sigma", "[underlying_sde]") {
    REQUIRE_THROWS_AS(
        JumpDiffusionProcess(0.05, -0.2, 0.3),
        std::invalid_argument
    );
}
//TEST 8: Constructor throws for negative lambda
TEST_CASE("JumpDiffusionProcess constructor throws for negative lambda", "[underlying_sde]") {
    REQUIRE_THROWS_AS(
        JumpDiffusionProcess(0.05, 0.2, -0.3),
        std::invalid_argument
    );
}

//TEST 9: Negative state throws in step
TEST_CASE("JumpDiffusionProcess step throws for negative state", "[underlying_sde]") {
    JumpDiffusionProcess process(0.05, 0.2, 0.3);
    RNG rng(123);

    REQUIRE_THROWS_AS(process.step(-1.0, 0.1, rng), std::invalid_argument);
}

//TEST 10: JumpDiffusion step returns 0 when we are in 0 state:
TEST_CASE("JumpDiffusionProcess step returns zero when state is zero", "[underlying_sde]") {
    JumpDiffusionProcess process(0.05, 0.2, 0.3);
    RNG rng(123);

    REQUIRE(process.step(0.0, 0.1, rng) == Approx(0.0));
}
//TEST 11: non-positive dt throws in step:
TEST_CASE("JumpDiffusionProcess step throws for non-positive dt", "[underlying_sde]") {
    JumpDiffusionProcess process(0.05, 0.2, 0.3);
    RNG rng(123);

    REQUIRE_THROWS_AS(process.step(100.0, 0.0, rng), std::invalid_argument);
    REQUIRE_THROWS_AS(process.step(100.0, -0.1, rng), std::invalid_argument);
}
//TEST 12: stepWithNormal returns 0 when s<=0
TEST_CASE("JumpDiffusionProcess stepWithNormal returns zero for non-positive state", "[underlying_sde]") {
    JumpDiffusionProcess process(0.05, 0.2, 0.3);
    RNG rng(123);

    REQUIRE(process.stepWithNormal(0.0, 0.1, 0.5, rng) == Approx(0.0));
    REQUIRE(process.stepWithNormal(-1.0, 0.1, 0.5, rng) == Approx(0.0));
}

//TEST 13: non-positive dt throwws in stepWithNormal
TEST_CASE("JumpDiffusionProcess stepWithNormal throws for non-positive dt", "[underlying_sde]") {
    JumpDiffusionProcess process(0.05, 0.2, 0.3);
    RNG rng(123);

    REQUIRE_THROWS_AS(process.stepWithNormal(100.0, 0.0, 0.5, rng), std::invalid_argument);
    REQUIRE_THROWS_AS(process.stepWithNormal(100.0, -0.1, 0.5, rng), std::invalid_argument);
}
//TEST 14: if lambda = 0, must be diffusion:
TEST_CASE("JumpDiffusionProcess stepWithNormal matches diffusion formula when lambda is zero", "[underlying_sde]") {
    const double r = 0.05;
    const double sigma = 0.2;
    const double lambda = 0.0;
    const double s = 100.0;
    const double dt = 0.25;
    const double z = 0.3;

    JumpDiffusionProcess process(r, sigma, lambda);
    RNG rng(123);

    double result = process.stepWithNormal(s, dt, z, rng);

    double expected = s * std::exp((r - 0.5 * sigma * sigma) * dt
                                   + sigma * std::sqrt(dt) * z);

    REQUIRE(result == Approx(expected));
}

//TEST 15: Jump almost surely occurs

TEST_CASE("JumpDiffusionProcess jumps to zero when jump probability is effectively one", "[underlying_sde]") {
    const double r = 0.05;
    const double sigma = 0.2;
    const double lambda = 1000.0;
    const double s = 100.0;
    const double dt = 1.0;
    const double z = 0.3;

    JumpDiffusionProcess process(r, sigma, lambda);
    RNG rng(123);

    double result = process.stepWithNormal(s, dt, z, rng);

    REQUIRE(result == Approx(0.0));
}

//TEST 16: sigma = 0 and lambda = 0 => deterministic
TEST_CASE("JumpDiffusionProcess is deterministic when sigma and lambda are zero", "[underlying_sde]") {
    const double r = 0.05;
    const double sigma = 0.0;
    const double lambda = 0.0;
    const double s = 100.0;
    const double dt = 0.25;
    const double z = 1.7; // should not matter

    JumpDiffusionProcess process(r, sigma, lambda);
    RNG rng(123);

    double result = process.stepWithNormal(s, dt, z, rng);
    double expected = s * std::exp(r * dt);

    REQUIRE(result == Approx(expected));
}

//TEST 17: simulatePath should be deterministic when sigma = 0
TEST_CASE("simulatePath is deterministic when sigma is zero", "[underlying_sde]") {
    const double S0 = 100.0;
    const double T = 1.0;
    const std::size_t n = 4;
    const double r = 0.05;
    const double dt = T / static_cast<double>(n);

    GeometricBrownianMotion gbm(r, 0.0);
    RNG rng(123);

    std::vector<double> path = gbm.simulatePath(S0, T, n, rng);

    REQUIRE(path.size() == n + 1);
    for (std::size_t i = 0; i <= n; ++i) {
        double expected = S0 * std::exp(r * dt * static_cast<double>(i));
        REQUIRE(path[i] == Approx(expected));
    }
}