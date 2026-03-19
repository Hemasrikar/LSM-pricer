#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include "bs_pricer.hpp"


namespace bs_pricer {


	double normal_CDF(double x)
	{
		return 0.5 * erfc(-x * M_SQRT1_2);
	}

	double price_vanilla_option_european_bs(double S0, double r, double sigma,
												double K, double T, bool call) {
		
		double d1 = (log(S0 / K) + (r+0.5*pow(sigma,2))*T) / (sigma * sqrt(T));
		double d2 = d1 - sigma*sqrt(T);

		double n_d1 = normal_CDF(d1);
		double n_d2 = normal_CDF(d2);
		
		double C = S0*n_d1 - K*exp(-r*T)*n_d2;
		double P = K*exp(-r*T)*(1-n_d2) - S0*(1-n_d1);

		if (call == true){
			return C;
		}

		return P;

		}

	} //namespace bs_pricer