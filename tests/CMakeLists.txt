add_executable(my_test ${SRC_FILES} my_test.cpp)
target_include_directories(my_test PUBLIC ${CMAKE_SOURCE_DIR}/extern/catch2 ${CMAKE_SOURCE_DIR}/src)
add_test(NAME my_test COMMAND my_test)
target_include_directories(my_test PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/eigen/5.0.1/include/eigen3)

# Added the executable for the black scholes pricing test
add_executable(test_bs_pricer ${SRC_FILES} test_bs_pricer.cpp)
target_include_directories(test_bs_pricer PUBLIC ${CMAKE_SOURCE_DIR}/extern/catch2 ${CMAKE_SOURCE_DIR}/src)
target_include_directories(test_bs_pricer PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/eigen/5.0.1/include/eigen3)
add_test(NAME test_bs_pricer COMMAND test_bs_pricer)


add_executable(test_convergence_analyser ${SRC_FILES} test_convergence_analyser.cpp)
target_include_directories(test_convergence_analyser PUBLIC ${CMAKE_SOURCE_DIR}/extern/catch2 ${CMAKE_SOURCE_DIR}/src)
target_include_directories(test_convergence_analyser PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/eigen/5.0.1/include/eigen3)
add_test(NAME test_convergence_analyser COMMAND test_convergence_analyser)

# Tests for Option Payoff Function and ITM
add_executable(OptionPayoff_test ${SRC_FILES} test_option_payoff.cpp)
target_include_directories(OptionPayoff_test PUBLIC ${CMAKE_SOURCE_DIR}/extern/catch2 ${CMAKE_SOURCE_DIR}/src)
add_test(NAME OptionPayoff_test COMMAND OptionPayoff_test)
target_include_directories(OptionPayoff_test PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/eigen/5.0.1/include/eigen3)