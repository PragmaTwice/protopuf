cmake_minimum_required(VERSION 3.17)
project(protopuf)

set(CMAKE_CXX_STANDARD 20)

add_executable(protopuf test.cpp)
target_include_directories(protopuf PUBLIC include)
