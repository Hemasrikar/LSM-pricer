#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "underlying_sde.hpp"
#include "option_payoff.hpp"
#include "basis_functions.hpp"
#include "lsm_pricer.hpp"
#include "ols_regression.hpp"


// Configuration

namespace cfg {
    // Market parameters
    constexpr double initial_spot = 1.0;
    constexpr double risk_free_rate = 0.02;
    constexpr double volatility = 0.15;
    constexpr double strike = 1.1;
    constexpr double maturity = 6.0;
    
    // Simulation parameters
    constexpr int num_paths = 50;
    constexpr int num_exercise_dates = 120;
    
    // Regression parameters
    constexpr int basis_order = 3;
    constexpr int grid_points = 300;
    
    // Output
    constexpr const char* output_dir = "../csv_output";
}

// Utilities

namespace utils {
    double eval_basis(const std::vector<double>& c, const lsm::core::BasisSet& b, double x) {
        double v = 0.0;
        for (std::size_t i = 0; i < c.size(); ++i)
            v += c[i] * b.basis[i]->evaluate(x);
        return v;
    }

    std::ofstream open_file(const std::string& name) {
        std::ofstream f(std::string(cfg::output_dir) + "/" + name);
        if (!f) throw std::runtime_error("Failed to open " + name);
        f << std::fixed << std::setprecision(6);
        return f;
    }
}

// CSV Export

void export_csv(const lsm::engine::PathData& d, const lsm::engine::SimulationResult& res,
                double maturity, double strike, double risk_free_rate) {
    std::string cmd = "mkdir -p " + std::string(cfg::output_dir);
    system(cmd.c_str());
    
    int np = d.numPaths, ns = d.numTimeSteps;
    double dt = maturity / ns, df = std::exp(-risk_free_rate * dt);
    
    // Paths
    {
        auto f = utils::open_file("paths.csv");
        f << "t";
        for (int i = 0; i < np; ++i) f << ",path_" << i;
        f << "\n";
        for (int t = 0; t <= ns; ++t) {
            f << (t * dt);
            for (int i = 0; i < np; ++i) f << "," << d.paths[i][t];
            f << "\n";
        }
    }
    
    // Paths ITM, Payoff, Cashflows
    {
        auto f_itm = utils::open_file("paths_itm.csv");
        auto f_pay = utils::open_file("payoff.csv");
        auto f_cf = utils::open_file("cashflows.csv");
        
        f_itm << "path_id,t,price,itm\n";
        f_pay << "path_id,t,price,payoff\n";
        f_cf << "path_id,t,cashflow\n";
        
        for (int i = 0; i < np; ++i) {
            for (int t = 0; t <= ns; ++t) {
                double s = d.paths[i][t];
                f_itm << i << "," << (t * dt) << "," << s << "," << (s < strike ? 1 : 0) << "\n";
                f_pay << i << "," << (t * dt) << "," << s << "," << std::max(strike - s, 0.0) << "\n";
                
                double cf = d.cashFlows[i][t];
                if (cf != 0.0)
                    f_cf << i << "," << (t * dt) << "," << cf << "\n";
            }
        }
    }
    
    // Exercise times
    {
        auto f = utils::open_file("exercise_times.csv");
        f << "path_id,exercise_t,exercise_price,held_to_maturity\n";
        
        for (int i = 0; i < np; ++i) {
            int ex_step = ns;
            for (int t = 1; t <= ns; ++t) {
                if (d.cashFlows[i][t] > 0.0) { ex_step = t; break; }
            }
            f << i << "," << (ex_step * dt) << "," << d.paths[i][ex_step] 
              << "," << (ex_step == ns ? 1 : 0) << "\n";
        }
    }
    
    // Regression scatter
    {
        auto f = utils::open_file("regression_scatter.csv");
        f << "x,discounted_cf,itm\n";
        
        for (int i = 0; i < np; ++i) {
            double x = d.paths[i][ns - 1], cf = std::max(strike - d.paths[i][ns], 0.0);
            f << x << "," << (cf * df) << "," << (x < strike ? 1 : 0) << "\n";
        }
    }
    
    // Regression grid
    {
        auto f = utils::open_file("regression_grid.csv");
        f << "x,continuation_value,exercise_value\n";
        
        std::vector<bool> itm_mask(np);
        std::vector<double> cf_vec(np);
        double xmin = 1e9, xmax = -1e9;
        
        for (int i = 0; i < np; ++i) {
            double x = d.paths[i][ns - 1];
            xmin = std::min(xmin, x);
            xmax = std::max(xmax, x);
            itm_mask[i] = (x < strike);
            cf_vec[i] = std::max(strike - d.paths[i][ns], 0.0);
        }
        
        lsm::core::BasisSet basis;
        basis.makeLaguerreSet(cfg::basis_order);
        
        auto coeff = lsm::engine::Ols_regression(
            const_cast<std::vector<std::vector<double>>&>(d.paths),
            ns - 1, cf_vec, itm_mask, df, basis);
        
        for (int g = 0; g <= cfg::grid_points; ++g) {
            double x = xmin + (xmax - xmin) * g / cfg::grid_points;
            f << x << "," << utils::eval_basis(coeff, basis, x) 
              << "," << std::max(strike - x, 0.0) << "\n";
        }
    }

    {
        auto f = utils::open_file("price_summary.csv");
        f << "label,value\n"
          << "american," << res.optionValue << "\n"
          << "european," << res.europeanValue << "\n"
          << "early_exercise_premium," << res.earlyExercisePremium << "\n"
          << "std_error," << res.standardError << "\n"
          << "num_paths," << res.numPaths << "\n"
          << "num_exercise_dates," << res.numExerciseDates << "\n";
    }


}

// Main

int main() {
    try {
        // Setup and run pricer
        lsm::engine::LSMConfig cfg_lsm;
        cfg_lsm.numPaths = cfg::num_paths;
        cfg_lsm.useAntithetic = false;
        cfg_lsm.numExerciseDates = cfg::num_exercise_dates;
        cfg_lsm.maturity = cfg::maturity;
        cfg_lsm.riskFreeRate = cfg::risk_free_rate;
        cfg_lsm.rngSeed = 42;

        auto basis = std::make_unique<lsm::core::BasisSet>();
        basis->makeLaguerreSet(cfg::basis_order);

        lsm::engine::LSMPricer pricer(
            std::make_unique<lsm::core::GeometricBrownianMotion>(cfg::risk_free_rate, cfg::volatility),
            std::make_unique<lsm::core::Put_payoff>(cfg::strike),
            std::move(basis),
            cfg_lsm
        );

        std::cout << "\nLongstaff-Schwartz American Put (S0=" << cfg::initial_spot 
                  << ", K=" << cfg::strike << ", T=" << cfg::maturity << " yrs)\n";

        auto [result, path_data] = pricer.priceWithData(cfg::initial_spot);

        std::cout << std::fixed << std::setprecision(6)
                  << "American:  " << result.optionValue << "\n"
                  << "European:  " << result.europeanValue << "\n"
                  << "Premium:   " << result.earlyExercisePremium << "\n"
                  << "Std Err:   " << result.standardError << "\n\n";

        std::cout << "Exporting:\n";
        export_csv(path_data, result, cfg::maturity, cfg::strike, cfg::risk_free_rate);

        std::cout << "\n CSV Exported\n";
        return EXIT_SUCCESS;

    } catch (const std::exception& e) {
        std::cerr << "Error" << e.what() << "\n";
        return EXIT_FAILURE;
    }
}