cmake_minimum_required(VERSION 3.19)
project(conoser LANGUAGES CXX)

add_library(conoser INTERFACE json.hpp type_utils.hpp)
target_include_directories(conoser INTERFACE .)
