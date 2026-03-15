add_executable(my_test ${SRC_FILES} my_test.cpp)
target_include_directories(my_test PUBLIC ${CMAKE_SOURCE_DIR}/extern/catch2 ${CMAKE_SOURCE_DIR}/src)
add_test(NAME my_test COMMAND my_test)
target_link_libraries(my_test Eigen3::Eigen)
# https://stackoverflow.com/questions/12249140/find-package-eigen3-for-cmake#:~:text=find_package%20(Eigen3%203.3,preferred%20whenever%20available.
