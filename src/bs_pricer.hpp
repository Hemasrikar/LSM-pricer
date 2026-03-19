#pragma once

namespace bs_pricer {

    struct OptionPricer {
        double call;
        double put;
    };

    OptionPricer price_vanilla_option_european_bs(
        double S0, 
        double r, 
        double sigma,
        double K, 
        double T
    );

} //namespace bs_pricer