cmake_minimum_required(VERSION 3.9)
project(uni)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread -fpermissive")

add_executable(uni main.cpp)