# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(MyProgram)

# Set C++ standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra)

# Add more source files here if needed
set(SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/basis_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/convergence_analyser.cpp
    ${CMAKE_SOURCE_DIR}/src/lsm_pricer.cpp
    ${CMAKE_SOURCE_DIR}/src/mc_paths.cpp
    ${CMAKE_SOURCE_DIR}/src/ols_regression.cpp
    ${CMAKE_SOURCE_DIR}/src/option_payoff.cpp
    ${CMAKE_SOURCE_DIR}/src/underlying_sde.cpp
    ${CMAKE_SOURCE_DIR}/src/bs_pricer.cpp
    ${CMAKE_SOURCE_DIR}/src/Finite_Difference.cpp

)

add_executable(my_program ${SRC_FILES} ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_include_directories(my_program PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/eigen/5.0.1/include/eigen3)

add_executable(convergence_tool ${SRC_FILES} ${CMAKE_SOURCE_DIR}/src/generate_convergence_output.cpp)
target_include_directories(convergence_tool PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/eigen/5.0.1/include/eigen3)

####################### TESTING STUFF STARTS HERE ###########################################################

enable_testing()
add_subdirectory(tests)
