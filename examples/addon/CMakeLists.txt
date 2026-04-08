cmake_minimum_required(VERSION 4.1)
project(UserPluginExample CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(OpenMP REQUIRED) 
if(OpenMP_CXX_FOUND)
	link_libraries(OpenMP::OpenMP_CXX)
endif()

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)


set(MY_EIGEN3_INCLUDE_DIR "$ENV{EIGEN3_INCLUDE_DIR}")

include_directories("../../include" ${MY_EIGEN3_INCLUDE_DIR})

add_library(${PROJECT_NAME} SHARED userplugin.hpp userplugin.cpp)
