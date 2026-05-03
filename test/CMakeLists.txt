cmake_minimum_required(VERSION 4.1)

project(SparsepcTests LANGUAGES CXX)
		
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

include(../cmake/tools.cmake)
include(../cmake/CPM.cmake)

CPMAddPackage("gh:doctest/doctest@2.5.2")
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")

if(NOT TARGET Eigen3::Eigen)
	CPMAddPackage(
		NAME Eigen3
		GITLAB_REPOSITORY libeigen/eigen
		GIT_TAG 5.0.1
		# Eigen's CMakelists are not intended for library use
		DOWNLOAD_ONLY YES
		#OPTIONS "EIGEN_BUILD_DOC OFF" "BUILD_TESTING OFF"
	)

    if(Eigen3_ADDED)
        add_library(Eigen3::Eigen INTERFACE IMPORTED)
        target_include_directories(Eigen3::Eigen INTERFACE ${Eigen3_SOURCE_DIR})
    endif()
endif()

if(NOT TARGET Spectra::Spectra)
    CPMAddPackage(
		NAME Spectra
		GITHUB_REPOSITORY yixuan/spectra
		GIT_TAG v1.2.0
		DOWNLOAD_ONLY YES
    )

    if(Spectra_ADDED)
        add_library(Spectra::Spectra INTERFACE IMPORTED)
        target_include_directories(Spectra::Spectra INTERFACE ${Spectra_SOURCE_DIR}/include)
    endif()
endif()

enable_testing()

if(TEST_INSTALLED_VERSION)
  find_package(sparsepc REQUIRED)
else()
	if(NOT TARGET sparsepc::sparsepc)
		CPMAddPackage(
			NAME sparsepc
			SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..
			DOWNLOAD_ONLY YES
		)

		if(sparsepc_ADDED)
			add_library(sparsepc::sparsepc INTERFACE IMPORTED)
			target_include_directories(sparsepc::sparsepc INTERFACE ${sparsepc_SOURCE_DIR}/include)
			if(NOT TEST_INSTALLED_VERSION)
			  target_compile_options(sparsepc::sparsepc INTERFACE
			      # MSVC warnings
			      $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
			      # GCC/Clang warnings
			      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
			  )
		        endif()

			if(ENABLE_TEST_COVERAGE)
			  target_compile_options(sparsepc::sparsepc INTERFACE -O0 -g -fprofile-arcs -ftest-coverage)
			  target_link_options(sparsepc::sparsepc INTERFACE -fprofile-arcs -ftest-coverage)
		        endif()
		endif()
	endif()
endif()

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} doctest::doctest Eigen3::Eigen Spectra::Spectra sparsepc::sparsepc)
target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(${PROJECT_NAME} PUBLIC _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
  if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()
endif()

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
# testing frameworks add the tests target instead: add_test(NAME ${PROJECT_NAME} COMMAND
# ${PROJECT_NAME})

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(${PROJECT_NAME})
