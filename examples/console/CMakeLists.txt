cmake_minimum_required(VERSION 4.1)

project(ConsoleExample LANGUAGES CXX)
		
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(USE_INSTALLED_VERSION "Use the version found by find_package" OFF)

include(../../cmake/tools.cmake)
include(../../cmake/CPM.cmake)

CPMAddPackage(
  NAME eigen
  GITLAB_REPOSITORY libeigen/eigen
  GIT_TAG 5.0.1
  OPTIONS "EIGEN_BUILD_DOC OFF" "BUILD_TESTING OFF"
)

if(USE_INSTALLED_VERSION)
  find_package(sparsepc REQUIRED)
else()
  CPMAddPackage(
    NAME sparsepc
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
endif()

add_executable(${PROJECT_NAME} main.cpp simu.hpp)
target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(${PROJECT_NAME} PUBLIC _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)
target_link_libraries(${PROJECT_NAME} Eigen3::Eigen sparsepc::sparsepc)
