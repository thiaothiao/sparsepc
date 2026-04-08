cmake_minimum_required(VERSION 4.1)
project(console_example CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED) 
if(OpenMP_CXX_FOUND)
	link_libraries(OpenMP::OpenMP_CXX)
endif()

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)

set(MY_EIGEN3_INCLUDE_DIR "$ENV{EIGEN3_INCLUDE_DIR}")

add_executable(${PROJECT_NAME} main.cpp simu.hpp)

include_directories("../../include" ${CMAKE_CURRENT_SOURCE_DIR} ${MY_EIGEN3_INCLUDE_DIR})
