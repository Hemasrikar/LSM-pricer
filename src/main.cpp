#include <iostream>
#include <iomanip>
#include <string>
#include "underlying_sde.hpp"
#include "option_payoff.hpp"
#include "basis_functions.hpp"
#include "lsm_pricer.hpp"

void printRow(double S0, const lsm::engine::SimulationResult& res) {
    std::cout << std::left
              << std::setw(10) << S0
              << std::setw(14) << res.optionValue
              << std::setw(14) << res.europeanValue
              << std::setw(14) << res.earlyExercisePremium
              << std::setw(12) << res.standardError
              << "\n";
}

int main() {
    const double K = 40.0;
    const double r = 0.06;
    const double sigma = 0.20;

    // pricer config
    lsm::engine::LSMConfig cfg;
    cfg.numPaths = 10000;
    cfg.useAntithetic = true;
    cfg.numExerciseDates = 50;
    cfg.maturity = 1.0;
    cfg.riskFreeRate = r;
    cfg.rngSeed = 24;

    const double spots[] = { 36.0, 38.0, 40.0, 42.0, 44.0 };

    // print table header
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Longstaff-Schwartz American Put Pricer\n";
    std::cout << "K=" << K << "  r=" << r << "  sigma=" << sigma
              << "  T=" << cfg.maturity << "  M=" << cfg.numExerciseDates
              << "  N=" << cfg.numPaths << "\n\n";
    std::cout << std::left
              << std::setw(10) << "Spot"
              << std::setw(14) << "American"
              << std::setw(14) << "European"
              << std::setw(14) << "Early Ex."
              << std::setw(12) << "Std.Err."
              << "\n";
    std::cout << std::string(64, '-') << "\n";

    for (double S0 : spots) {
        auto basis = std::make_unique<lsm::core::BasisSet>();
        basis->makeLaguerreSet(3);

        lsm::engine::LSMPricer pricer(
            std::make_unique<lsm::core::GeometricBrownianMotion>(r, sigma),
            std::make_unique<lsm::core::Put_payoff>(K),
            std::move(basis),
            cfg
        );

        lsm::engine::SimulationResult res = pricer.price(S0);
        printRow(S0, res);
    }

    return 0;
}