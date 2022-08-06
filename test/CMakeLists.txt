cmake_minimum_required(VERSION 3.19)
project(test_conoser LANGUAGES CXX)

add_executable(test_conoser main.cpp)
target_link_libraries(test_conoser PRIVATE conoser)
