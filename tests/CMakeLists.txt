add_executable(my_test ${SRC_FILES} my_test.cpp)
target_include_directories(my_test PUBLIC ${CMAKE_SOURCE_DIR}/extern/catch2 ${CMAKE_SOURCE_DIR}/src)
add_test(NAME my_test COMMAND my_test)
target_include_directories(my_test PRIVATE ${EIGEN3_INCLUDE_DIR})
# https://stackoverflow.com/questions/34138879/unable-to-find-eigen3-with-cmake#:~:text=include_directories(%24%7BEIGEN3_INCLUDE_DIR%7D)%0Amessage(%22Found%20Eigen3%20in%3A%20%24%7BEIGEN3_INCLUDE_DIR%7D%22)
# https://stackoverflow.com/questions/31969547/what-is-the-difference-between-include-directories-and-target-include-directorie#:~:text=include_directories(x/y,and%20INTERFACE%20qualifiers.