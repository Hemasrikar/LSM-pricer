# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(MyProgram)

# Set C++ standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra)

find_package(Eigen3 REQUIRED)

# Add more source files here if needed
set(SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/basis_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/convergence_analyser.cpp
    ${CMAKE_SOURCE_DIR}/src/lsm_pricer.cpp
    ${CMAKE_SOURCE_DIR}/src/mc_paths.cpp
    ${CMAKE_SOURCE_DIR}/src/ols_regression.cpp
    ${CMAKE_SOURCE_DIR}/src/option_payoff.cpp
    ${CMAKE_SOURCE_DIR}/src/underlying_sde.cpp

)

add_executable(my_program ${SRC_FILES} ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(my_program Eigen3::Eigen)

####################### TESTING STUFF STARTS HERE ###########################################################

enable_testing()
add_subdirectory(tests)
https://stackoverflow.com/questions/12249140/find-package-eigen3-for-cmake#:~:text=find_package%20(Eigen3%203.3,preferred%20whenever%20available.
