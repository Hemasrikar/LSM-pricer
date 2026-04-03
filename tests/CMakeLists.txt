
# To avoids code duplication and makes it easy to add new tests
function(add_lsm_test test_name test_source)
    # Create an executable from the test source file
    add_executable(${test_name} ${test_source})
    
    # Include the Catch2 header-only library for testing framework
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/extern/catch2)
    
    # Link LSM project files
    target_link_libraries(${test_name} PRIVATE lsm_core)
    
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()


# Tests for the main files of the LSM pricer

add_lsm_test(OLS_regression_test my_test.cpp)
add_lsm_test(BasisFunctions_test test_basis_functions.cpp)
add_lsm_test(OptionPayoff_test test_option_payoff.cpp)
add_lsm_test(finite_difference_tests finite_difference_tests.cpp)


# Tests for Black-Scholes and LSM Monte Carlo
add_lsm_test(test_bs_pricer test_bs_pricer.cpp)
add_lsm_test(test_lsm_pricer test_lsm_pricer.cpp)

# Tests for the underlying SDE and convergence analysis tools
add_lsm_test(test_underlying_sde test_underlying_sde.cpp)
add_lsm_test(test_convergence_analyser test_convergence_analyser.cpp)