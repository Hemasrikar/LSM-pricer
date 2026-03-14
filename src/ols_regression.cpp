#pragma once
#include <functional>
#include <cmath>
#include <vector>
#include <stdexcept>
// For finding header files necessary used: https://libeigen.gitlab.io/eigen/docs-nightly/group__QuickRefPage.html
#include <Eigen/Dense>
#include "mc_paths.hpp"
#include "option_payoff.hpp"
#include "basis_functions.hpp"


// https://libeigen.gitlab.io/eigen/docs-nightly/group__TutorialMatrixClass.html#:~:text=typedef%20Matrix%3Cdouble%2C%20Dynamic%2C%20Dynamic%3E%20MatrixXd%3B
Eigen::MatrixXd datapoints;

// https://stackoverflow.com/questions/40852757/c-how-to-convert-stdvector-to-eigenmatrixxd


// OLS regressors
std::vector<double> ols_parameters;



Eigen::MatrixXd buildDesignMatrix(
    std::vector<double> S_t,
    std::vector<bool> itm, // Boolean to know if it's in the money.
    lms::BasisSet basis)
{
    int K = basis.basis.size();
    int n_itm = 0;
    for (bool b : itm) if (b) ++n_itm;
    Eigen::MatrixXd X(n_itm, K); // https://libeigen.gitlab.io/eigen/docs-nightly/group__TutorialMatrixClass.html#:~:text=this%20page.-,Resizing
    int row = 0;
    for (int i = 0; i < S_t.size(); ++i) {
        if (!itm[i]) continue;
        for (int k = 0; k < K; ++k)
            X(row, k) = basis.basis[k]->evaluate(S_t[i]);
        ++row;
    }
    return X;
}



// std::vector<double> Ols_regression(
//     stochastic_process process, 
//     double simulation_number, 
//     mc_sampling sampler
//     int number_of_intervals, 
//     basis_function basis,
//     double interval_length,
// )
// { 
//     // compute length_of_output(basis)
//     std::vector<Eigen::MatrixXd> OLS_regressors_across_time(number_of_intervals, Eigen::MatrixXd(simulation_number, length_of_output(basis))); // (???) Is that how you size a vector of Eigen matrices???
//     std::vector<Eigen::MatrixXd> 
//     // Need to derive OLS regressors here noting that the design matrix will be made for each time. Since the basis function gives all Beta params
//     // Need to assign 
//     datapoints.resize(number_of_intervals, length_of_output(basis)); //https://libeigen.gitlab.io/eigen/docs-nightly/group__TutorialMatrixClass.html#:~:text=this%20page.-,Resizing,-The%20current%20size
//     std::vector<double> sampled_path;
//     for( i = 0; i < simulation_number; i++){
//         sampled_path = sampler(number_of_intervals, interval_length, process);
//         for(j = 0; j < interval_number; j++) {

//            datapoints(i, j) = basis(sampled_path[j]);

//         }
//     }
// }

// Potential discussion on code performance: https://scicomp.stackexchange.com/questions/3159/is-it-a-good-idea-to-use-vectorvectordouble-to-form-a-matrix-class-for-high
// 