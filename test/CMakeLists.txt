cmake_minimum_required(VERSION 4.1)

project(SparsepcTests LANGUAGES CXX)
		
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(MY_DEPLOYMENT_DIR "$ENV{DEPLOYMENT_DIR}")

set(CMAKE_INSTALL_PREFIX "${MY_DEPLOYMENT_DIR}" CACHE PATH "Default install path" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)

#find_package(OpenMP REQUIRED) 
#if(OpenMP_CXX_FOUND)
#	link_libraries(OpenMP::OpenMP_CXX)
#endif()

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

include(../cmake/tools.cmake)
include(../cmake/CPM.cmake)

CPMAddPackage("gh:doctest/doctest@2.5.2")
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")

CPMAddPackage(
  NAME eigen
  GITLAB_REPOSITORY libeigen/eigen
  GIT_TAG 5.0.1
  OPTIONS "EIGEN_BUILD_DOC OFF" "BUILD_TESTING OFF"
)

if(TEST_INSTALLED_VERSION)
  find_package(sparsepc REQUIRED)
else()
  CPMAddPackage(
    NAME sparsepc
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} doctest::doctest Eigen3::Eigen sparsepc::sparsepc)

target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(${PROJECT_NAME} PUBLIC _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(sparsepc INTERFACE -Wall -Wpedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(sparsepc INTERFACE /W4 /WX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()
endif()

enable_testing()

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
# testing frameworks add the tests target instead: add_test(NAME ${PROJECT_NAME} COMMAND
# ${PROJECT_NAME})

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(${PROJECT_NAME})

if(ENABLE_TEST_COVERAGE)
  target_compile_options(sparsepc INTERFACE -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(sparsepc INTERFACE -fprofile-arcs -ftest-coverage)
endif()
