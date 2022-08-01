cmake_minimum_required(VERSION 3.5)

project(conoser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(conoser main.cpp
    json.hpp
    type_utils.hpp)
