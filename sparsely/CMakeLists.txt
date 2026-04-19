cmake_minimum_required(VERSION 4.1)

set(appName "Sparsely")
set(exeName "sparsely")
set(appVersion "0.1.0")
set(appTitle "SPARSELY Software - SPARSE principal component LaboratorY")
set(appVendor "Indilo")
set(appDomain "?.com")
string(TIMESTAMP buildTime "%Y-%m-%d %H:%M:%S")

project(${exeName}
        VERSION ${appVersion}
        DESCRIPTION ${appTitle}
        LANGUAGES CXX)
		
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if the environment variable is defined
if(DEFINED ENV{MY_INSTALL_DIR})
    message(STATUS "MY_INSTALL_DIR is set to: $ENV{MY_INSTALL_DIR}")
    # Use it to set a CMake variable
    set(MY_INSTALL_PREFIX "$ENV{MY_INSTALL_DIR}")
    set(CMAKE_INSTALL_PREFIX "${MY_INSTALL_PREFIX}" CACHE PATH "Default install path" FORCE)
else()
    message(WARNING "MY_INSTALL_DIR env. variable not found. Using default.")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

include(../cmake/tools.cmake)
include(../cmake/CPM.cmake)

# Generate the header file
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/infos.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/infos.h"
    @ONLY
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

CPMAddPackage(
  NAME eigen
  GITLAB_REPOSITORY libeigen/eigen
  GIT_TAG 5.0.1
  OPTIONS "EIGEN_BUILD_DOC OFF" "BUILD_TESTING OFF"
)

CPMAddPackage(
    NAME sparsepc
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

#CPMFindPackage(
#  NAME Qt6
#  VERSION 6.11.0
#  COMPONENTS Core Widgets Gui Xml
#  FIND_PACKAGE_ARGUMENTS REQUIRED
#)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Xml Svg REQUIRED)

CPMAddPackage(
    NAME JKQTPlotter6
    GITHUB_REPOSITORY jkriege2/JKQtPlotter
    GIT_TAG master # Or a specific version/commit
	COMPONENTS JKQTPlotter6 JKQTCommon6 JKQTMath6 JKQTMathText6
	FIND_PACKAGE_ARGUMENTS REQUIRED
	OPTIONS "JKQtPlotter_BUILD_FORCE_NO_PRINTER_SUPPORT ON" "JKQtPlotter_BUILD_TESTS OFF" "JKQtPlotter_BUILD_EXAMPLES OFF" "JKQtPlotter_BUILD_LIB_JKQTFASTPLOTTER OFF"
)

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
file(GLOB headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/*.h)
file(GLOB appiconrcs CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/images/*.rc)
file(GLOB apprccs CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/*.qrc)

if(WIN32)
    message(STATUS "Running on Windows")
    enable_language(RC)
endif()

qt_standard_project_setup()
qt_add_resources(sources ${apprccs})
qt_add_executable(${exeName} ${sources} ${appiconrcs} ${headers})

set_target_properties(${exeName} PROPERTIES
    WIN32_EXECUTABLE ON
)

target_compile_definitions(${exeName} PRIVATE _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(${exeName} PRIVATE _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)
target_compile_definitions(${exeName} PRIVATE JKQTPLOTTER_COMPILE_WITHOUT_PRINTSUPPORT)

if(eigen_ADDED)
    target_link_libraries(${exeName} PRIVATE Eigen3::Eigen)
endif()

if(sparsepc_ADDED)
    target_link_libraries(${exeName} PRIVATE sparsepc::sparsepc)
endif()

if(JKQTPlotter6_ADDED)
    target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTPlotter6)
    target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTCommon6)
    target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTMath6)
    target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTMathText6)
endif()

target_link_libraries(${exeName} PRIVATE Qt6::Core)
target_link_libraries(${exeName} PRIVATE Qt6::Widgets)
target_link_libraries(${exeName} PRIVATE Qt6::Gui)
target_link_libraries(${exeName} PRIVATE Qt6::Xml)
target_link_libraries(${exeName} PRIVATE Qt6::Svg)

add_custom_command(TARGET ${exeName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_LIST_DIR}/images"
        "${CMAKE_INSTALL_PREFIX}/images"
    COMMENT "Copying images to build directory"
)

add_custom_command(TARGET ${exeName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_LIST_DIR}/configurations"
        "${CMAKE_INSTALL_PREFIX}/configurations"
    COMMENT "Copying configurations to build directory"
)

install(TARGETS ${exeName}
    DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

if(WIN32)
    install(FILES   $<TARGET_FILE:JKQTPlotter6::JKQTPlotter6>
                    $<TARGET_FILE:JKQTPlotter6::JKQTCommon6>
                    $<TARGET_FILE:JKQTPlotter6::JKQTMath6>
                    $<TARGET_FILE:JKQTPlotter6::JKQTMathText6>
        DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

    install(FILES   "${CMAKE_CURRENT_LIST_DIR}/installscript.iss"
        DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()
	
install(DIRECTORY   "${CMAKE_CURRENT_LIST_DIR}/addons/"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/addons")

#install(DIRECTORY   "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
#    DESTINATION "${CMAKE_INSTALL_PREFIX}/toto")

if (LINUX)
    # Install the icon to the standard pixmaps directory
    install(FILES "${CMAKE_CURRENT_LIST_DIR}/images/logo_transparent.png"
                    DESTINATION "/usr/local/share/icons"
                    RENAME "sparselyIcon.png")

    # Install the desktop entry
    install(FILES "${CMAKE_CURRENT_LIST_DIR}/sparsely.desktop"
                    DESTINATION "/usr/local/share/applications")
endif()

qt6_generate_deploy_app_script(
    TARGET ${exeName}
    OUTPUT_SCRIPT deploy_script
    NO_TRANSLATIONS
)

install(SCRIPT ${deploy_script})

if (LINUX)
    #fixing weird missing PrintSupport, Xml after installation
    install(FILES   $<TARGET_FILE:Qt6::Xml>
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
endif()
