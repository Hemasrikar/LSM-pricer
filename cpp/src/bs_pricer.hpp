#pragma once

namespace bs_pricer {

    double price_vanilla_option_european_bs(
        double S0, 
        double r, 
        double sigma,
        double K, 
        double T,
        bool call
    );

} //namespace bs_pricer