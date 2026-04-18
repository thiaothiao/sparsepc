cmake_minimum_required(VERSION 4.1)

set(libraryName "Sparsepc")
set(libName "sparsepc")
set(libVersion "0.1.0")
set(libTitle "Sparse principal component C++ Library")
set(libVendor "Indilo")
set(libDomain "?.com")
string(TIMESTAMP buildTime "%Y-%m-%d %H:%M:%S")

project(${libName}
        VERSION ${libVersion}
        DESCRIPTION ${libTitle}
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)
# Generate the header file
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/infos.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/infos.hpp"
    @ONLY
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# PackageProject.cmake will be used to make our target installable
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)

# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
add_library(${PROJECT_NAME} INTERFACE)

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} INTERFACE "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
target_compile_definitions(${PROJECT_NAME} INTERFACE _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(${PROJECT_NAME} INTERFACE _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)

target_link_libraries(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:Eigen3::Eigen>)

target_include_directories(
  ${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
  VERSION_HEADER "${VERSION_HEADER_LOCATION}"
  COMPATIBILITY SameMajorVersion
  DEPENDENCIES "eigen"
)
