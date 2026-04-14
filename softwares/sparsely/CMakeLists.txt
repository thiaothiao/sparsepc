set(appName "Sparsely")
set(exeName "sparsely")
set(appVersion "1.0.0")
set(appTitle "SPARSELY Software - SPARSE principal component LaboratorY")
set(appVendor "Indilo")
set(appDomain "?.com")
string(TIMESTAMP buildTime "%Y-%m-%d %H:%M:%S")

project(${exeName}
        VERSION ${appVersion}
        DESCRIPTION ${appTitle}
        LANGUAGES CXX)

# Generate the header file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
    @ONLY
)		
		
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui PrintSupport Svg Xml REQUIRED)
find_package(JKQTPlotter6 REQUIRED COMPONENTS JKQTPlotter6 JKQTCommon6 JKQTMath6 JKQTMathText6 REQUIRED)

set(sources main.cpp labmainwindow.cpp labwidget.cpp labcontroller.cpp labmodel.cpp labpreferences.cpp labpreferencesdialog.cpp labenums.h labmainwindow.h labwidget.h labcontroller.h labmodel.h labpreferences.h labpreferencesdialog.h)
set(appiconrc images/appicon.rc)
set(apprcc resources.qrc)

if(WIN32)
    message(STATUS "Running on Windows")
    enable_language(RC)
endif()

qt_standard_project_setup()
qt_add_resources(sources ${apprcc})
qt_add_executable(${exeName} ${sources} ${appiconrc})

set_target_properties(${exeName} PROPERTIES
    WIN32_EXECUTABLE ON
)

target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTPlotter6)
target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTCommon6)
target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTMath6)
target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTMathText6)

target_link_libraries(${exeName} PRIVATE Qt6::Core)
target_link_libraries(${exeName} PRIVATE Qt6::Widgets)
target_link_libraries(${exeName} PRIVATE Qt6::Gui)
target_link_libraries(${exeName} PRIVATE Qt6::PrintSupport)
target_link_libraries(${exeName} PRIVATE Qt6::Svg)
target_link_libraries(${exeName} PRIVATE Qt6::Xml)

add_custom_command(TARGET ${exeName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/images"
        "${CMAKE_INSTALL_PREFIX}/images"
    COMMENT "Copying images to build directory"
)

add_custom_command(TARGET ${exeName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/configurations"
        "${CMAKE_INSTALL_PREFIX}/configurations"
    COMMENT "Copying configurations to build directory"
)

include(GNUInstallDirs)

if (LINUX)
    message(STATUS "Running on Linux")
	# Install the icon to the standard pixmaps directory
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/images/logo_transparent.png"
			DESTINATION "/usr/local/share/icons"
			RENAME "sparselyIcon.png")

	# Install the desktop entry
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/sparsely.desktop"
			DESTINATION "/usr/local/share/applications")
endif()

install(TARGETS ${exeName}
    DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES   $<TARGET_FILE:JKQTPlotter6::JKQTPlotter6>
                $<TARGET_FILE:JKQTPlotter6::JKQTCommon6>
                $<TARGET_FILE:JKQTPlotter6::JKQTMath6>
                $<TARGET_FILE:JKQTPlotter6::JKQTMathText6>
    DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY   "${CMAKE_SOURCE_DIR}/include/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
	
install(DIRECTORY   "${CMAKE_CURRENT_SOURCE_DIR}/addons"
    DESTINATION ${CMAKE_INSTALL_BINDIR})

qt_generate_deploy_app_script(
    TARGET ${exeName}
    OUTPUT_SCRIPT deploy_script
    NO_TRANSLATIONS
)

install(SCRIPT ${deploy_script})
