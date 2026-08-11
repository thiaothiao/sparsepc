cmake_minimum_required(VERSION 4.1)

project(ConsoleExample LANGUAGES CXX)
		
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(../../cmake/tools.cmake)
include(../../cmake/CPM.cmake)

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

if(NOT TARGET Sparsepc::Sparsepc)
    CPMAddPackage(
		NAME Sparsepc
		SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..
		DOWNLOAD_ONLY YES
    )

    if(Sparsepc_ADDED)
        add_library(Sparsepc::Sparsepc INTERFACE IMPORTED)
        target_include_directories(Sparsepc::Sparsepc INTERFACE ${Sparsepc_SOURCE_DIR}/include)
    endif()
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(${PROJECT_NAME} PUBLIC _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)
target_link_libraries(${PROJECT_NAME} Eigen3::Eigen Spectra::Spectra Sparsepc::Sparsepc)
