# Minimum CMake version required for this project
cmake_minimum_required(VERSION 3.10)
project(MyProgram VERSION 1.0.0 LANGUAGES CXX)

# Compiler configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable warnings to catch potential issues during build
add_compile_options(-Wall -Wextra)

# Define all source files that form the LSM pricing engine
# These are compiled once into a static library to reduce build times
set(SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/basis_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/convergence_analyser.cpp
    ${CMAKE_SOURCE_DIR}/src/lsm_pricer.cpp
    ${CMAKE_SOURCE_DIR}/src/mc_paths.cpp
    ${CMAKE_SOURCE_DIR}/src/ols_regression.cpp
    ${CMAKE_SOURCE_DIR}/src/option_payoff.cpp
    ${CMAKE_SOURCE_DIR}/src/underlying_sde.cpp
    ${CMAKE_SOURCE_DIR}/src/bs_pricer.cpp
    ${CMAKE_SOURCE_DIR}/src/finite_difference.cpp
)

# Create a static library from all source files
# This allows executables to link against the library instead of recompiling sources
add_library(lsm_core STATIC ${SRC_FILES})
target_include_directories(lsm_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src                                           
    ${CMAKE_SOURCE_DIR}/dependencies/eigen/5.0.1/include/eigen3 
)

# Main pricing executables
add_executable(my_program ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(my_program PRIVATE lsm_core)

# Analysis tool for convergence
add_executable(convergence_tool ${CMAKE_SOURCE_DIR}/src/generate_convergence_output.cpp)
target_link_libraries(convergence_tool PRIVATE lsm_core)

# Enable CMake testing framework and load test definitions from subdirectory
enable_testing()
add_subdirectory(tests)