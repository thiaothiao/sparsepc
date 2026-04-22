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
        LANGUAGES CXX
)
		
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if the environment variable is defined
if(DEFINED ENV{MY_INSTALL_DIR})
    message(STATUS "MY_INSTALL_DIR is set to: $ENV{MY_INSTALL_DIR}")
    set(MY_INSTALL_PREFIX "$ENV{MY_INSTALL_DIR}")
    set(CMAKE_INSTALL_PREFIX "${MY_INSTALL_PREFIX}" CACHE PATH "Default install path" FORCE)
else()
    message(WARNING "MY_INSTALL_DIR env. variable not found. Using default.")
endif()

include(GNUInstallDirs)

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
	SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..
)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Xml Svg REQUIRED)

CPMAddPackage(
    NAME JKQTPlotter6
    GITHUB_REPOSITORY jkriege2/JKQtPlotter
    GIT_TAG master # Or a specific version/commit
	COMPONENTS JKQTPlotter6 JKQTCommon6 JKQTMath6 JKQTMathText6
	FIND_PACKAGE_ARGUMENTS REQUIRED
	OPTIONS 
		"JKQtPlotter_BUILD_FORCE_NO_PRINTER_SUPPORT ON" 
		"JKQtPlotter_BUILD_TESTS OFF" "JKQtPlotter_BUILD_EXAMPLES OFF" 
		"JKQtPlotter_BUILD_LIB_JKQTFASTPLOTTER OFF"
)

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
file(GLOB headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/*.h)
file(GLOB apprccs CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/*.qrc)

if(WIN32)
	file(GLOB appiconrcs CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/images/*.rc)
    enable_language(RC)
elseif(APPLE)
	set(appiconrcs "${CMAKE_CURRENT_LIST_DIR}/images/logo.icns")
	set(MACOSX_BUNDLE_ICON_FILE "logo.icns")
	set_source_files_properties(${appiconrcs} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

qt_standard_project_setup()
qt_add_resources(sources ${apprccs})
qt_add_executable(${exeName} ${sources} ${appiconrcs} ${headers})

if(WIN32)
	set_target_properties(${exeName} PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
	set_target_properties(${exeName} PROPERTIES 
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_GUI_IDENTIFIER "com.indilo.sparsely"
		MACOSX_BUNDLE_BUNDLE_NAME ${exeName}
		MACOSX_BUNDLE_SHORT_VERSION_STRING ${appVersion}
		MACOSX_BUNDLE_LONG_VERSION_STRING ${appVersion}
	)	
endif()

target_compile_definitions(${exeName} PRIVATE _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(${exeName} PRIVATE _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)

target_link_libraries(${exeName} PRIVATE Eigen3::Eigen)

target_link_libraries(${exeName} PRIVATE sparsepc::sparsepc)

target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTPlotter6)
target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTCommon6)
target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTMath6)
target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTMathText6)

target_link_libraries(${exeName} PRIVATE Qt6::Core)
target_link_libraries(${exeName} PRIVATE Qt6::Widgets)
target_link_libraries(${exeName} PRIVATE Qt6::Gui)
target_link_libraries(${exeName} PRIVATE Qt6::Xml)
target_link_libraries(${exeName} PRIVATE Qt6::Svg)

include(GNUInstallDirs)

include(CPackComponent)
cpack_add_component(${appName}
    DISPLAY_NAME ${appName}
    DESCRIPTION ${appTitle}
    REQUIRED
)

if(WIN32)
    install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/images/"
        DESTINATION "images"
        COMPONENT ${appName}
    )
    install(FILES   $<TARGET_FILE:JKQTPlotter6::JKQTPlotter6>
                    $<TARGET_FILE:JKQTPlotter6::JKQTCommon6>
                    $<TARGET_FILE:JKQTPlotter6::JKQTMath6>
                    $<TARGET_FILE:JKQTPlotter6::JKQTMathText6>
					$<TARGET_FILE:Qt6::Xml>
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT ${appName}
    )
    set(DEPLOY_OPTIONS "--no-translations")	
elseif(APPLE)
    # Define your signing identity
    set(SIGNING_IDENTITY "-")#"Developer ID Application: Your Company (ID)")
    set(DEPLOY_OPTIONS "-codesign=${SIGNING_IDENTITY}" "-hardened-runtime" "-timestamp")
else()# Linux or trying as linux
	# Install the icon to the standard pixmaps directory
    install(FILES "${CMAKE_CURRENT_LIST_DIR}/images/icon.png"
        DESTINATION "/usr/local/share/icons"
        RENAME "sparselyIcon.png"
        COMPONENT ${appName}
    )
    # Install the desktop entry
    install(FILES "${CMAKE_CURRENT_LIST_DIR}/sparsely.desktop"
        DESTINATION "/usr/local/share/applications"
        COMPONENT ${appName}
    )
    set(DEPLOY_OPTIONS "--no-translations")
endif()

qt6_generate_deploy_app_script(
    TARGET ${exeName}
    OUTPUT_SCRIPT deploy_script
    DEPLOY_TOOL_OPTIONS ${DEPLOY_OPTIONS}
)

if (APPLE)
    install(TARGETS ${exeName}
        BUNDLE DESTINATION .
        COMPONENT ${appName}
    )
elseif(WIN32)
    install(TARGETS ${exeName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}    # 'bin'
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}    # 'lib' or 'lib64'
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}    # static libs
        COMPONENT ${appName}
    )
else()
    install(TARGETS ${exeName}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT ${appName}
    )
endif()

install(SCRIPT ${deploy_script}
    COMPONENT ${appName}
)

# 1. Define the Component
#cpack_add_component(QtApplication
#    DISPLAY_NAME "Main Application"
#    DESCRIPTION "The core Qt C++ application"
#    REQUIRED # Ensures this is always checked
#)

#cpack_add_component(Documentation
#    DISPLAY_NAME "Manual"
#    DESCRIPTION "Application documentation"
#    GROUP UserDocumentation
#)

# 2. Map Targets to Components
#install(TARGETS myQtApp
#    DESTINATION bin
#    COMPONENT QtApplication
#)

#install(FILES docs/manual.pdf
#    DESTINATION share/doc
#    COMPONENT Documentation
#)

set(CPACK_GENERATOR "IFW")
set(CPACK_PACKAGE_NAME ${appName})
set(CPACK_PACKAGE_VENDOR ${appVendor})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${appTitle})
set(CPACK_PACKAGE_VERSION ${appVersion})
#set(CPACK_COMPONENTS_ALL ${appName})#Application Runtime Documentation)

set(CPACK_IFW_PACKAGE_LOGO "${CMAKE_CURRENT_LIST_DIR}/images/ico.png")
set(CPACK_IFW_PACKAGE_BANNER "${CMAKE_CURRENT_LIST_DIR}/images/sparselybanner.png")
set(CPACK_IFW_PACKAGE_WINDOW_ICON "${CMAKE_CURRENT_LIST_DIR}/images/iconCopy.png")

# 3. Set the main installer executable icon
if(WIN32)
    set(CPACK_IFW_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images/logo.ico")
elseif(APPLE)
    set(CPACK_IFW_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images/logo.icns")
endif()

set(CPACK_IFW_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/config.xml")

include(CPack)
include(CPackIFW)

# Optional: Add IFW specific information
cpack_ifw_configure_component(${appName}
    NAME "com.${appVendor}.${appName}"
    DISPLAY_NAME ${appName}
    DESCRIPTION ${appTitle}
    VERSION ${appVersion}
    RELEASE_DATE ${buildTime}
)
