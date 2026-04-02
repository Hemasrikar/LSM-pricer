cmake_minimum_required(VERSION 3.10)
project(MyProgram)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra)

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

# Compile shared sources once here at the root for less compiling time
add_library(lsm_core STATIC ${SRC_FILES})
target_include_directories(lsm_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/dependencies/eigen/5.0.1/include/eigen3
)

add_executable(my_program ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(my_program PRIVATE lsm_core)

add_executable(convergence_tool ${CMAKE_SOURCE_DIR}/src/generate_convergence_output.cpp)
target_link_libraries(convergence_tool PRIVATE lsm_core)

enable_testing()
add_subdirectory(tests)