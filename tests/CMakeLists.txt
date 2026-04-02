# Compile shared sources to compile exactly once
add_library(lsm_core STATIC ${SRC_FILES})
target_include_directories(lsm_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/dependencies/eigen/5.0.1/include/eigen3
)

function(add_lsm_test target source)
    add_executable(${target} ${source})
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/extern/catch2)
    target_link_libraries(${target} PRIVATE lsm_core)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

add_lsm_test(my_test my_test.cpp)
add_lsm_test(BasisFunctions_test test_basis_functions.cpp)
add_lsm_test(test_bs_pricer test_bs_pricer.cpp)
add_lsm_test(test_convergence_analyser test_convergence_analyser.cpp)
add_lsm_test(OptionPayoff_test test_option_payoff.cpp)
add_lsm_test(finite_difference_tests finite_difference_tests.cpp)
add_lsm_test(test_lsm_pricer test_lsm_pricer.cpp)
add_lsm_test(test_underlying_sde test_underlying_sde.cpp)