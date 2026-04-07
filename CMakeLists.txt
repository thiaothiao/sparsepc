cmake_minimum_required(VERSION 4.1)
project(sparsepc CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Using openMP")
find_package(OpenMP REQUIRED) 
if(OpenMP_CXX_FOUND)
	link_libraries(OpenMP::OpenMP_CXX)
endif()

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)
#add_compile_definitions(JKQTPLOTTER_COMPILE_WITHOUT_PRINTSUPPORT)


set(MY_EIGEN3_INCLUDE_DIR "$ENV{EIGEN3_INCLUDE_DIR}")

message(STATUS "Using Eigen3")
include_directories(${PROJECT_SOURCE_DIR}/include ${MY_EIGEN3_INCLUDE_DIR})

# Add the subdirectories
add_subdirectory(examples)
add_subdirectory(softwares)
