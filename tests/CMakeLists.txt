add_executable(my_test ${SRC_FILES} my_test.cpp)
target_include_directories(my_test PUBLIC ${CMAKE_SOURCE_DIR}/extern/catch2 ${CMAKE_SOURCE_DIR}/src)
add_test(NAME my_test COMMAND my_test)
target_include_directories(my_test PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/eigen/5.0.1/include/eigen3)
