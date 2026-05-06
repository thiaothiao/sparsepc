cmake_minimum_required(VERSION 4.1)

project(Sparsepc
        VERSION 0.1.0
		DESCRIPTION "A header-only C++ library for sparse principal components computations"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# PackageProject.cmake will be used to make our target installable
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)

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

# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
  ${PROJECT_NAME} 
  INTERFACE 
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} INTERFACE "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
target_compile_definitions(${PROJECT_NAME} INTERFACE _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(${PROJECT_NAME} INTERFACE _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)

target_link_libraries(${PROJECT_NAME} INTERFACE Eigen3::Eigen)
target_link_libraries(${PROJECT_NAME} INTERFACE Spectra::Spectra)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
#set(VERSION_HEADER_LOCATION ${PROJECT_NAME}/version.hpp )

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
  VERSION_HEADER ${PROJECT_NAME}/version.hpp
  COMPATIBILITY SameMajorVersion
  DEPENDENCIES "Eigen3" "Spectra"
)