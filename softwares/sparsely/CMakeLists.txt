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

set(sources main.cpp labmainwindow.cpp labwidget.cpp labcontroller.cpp labmodel.cpp labpreferences.cpp labpreferencesdialog.cpp labenums.h labmainwindow.h labwidget.h labcontroller.h labmodel.h labpreferences.h labpreferencesdialog.h)
set(appiconrc images/appicon.rc)
set(apprcc resources.qrc)

if(WIN32)
    enable_language(RC)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(${exeName} WIN32 ${sources} ${apprcc} ${appiconrc})

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui PrintSupport Svg Xml REQUIRED)
target_link_libraries(${exeName} PRIVATE Qt6::Core)
target_link_libraries(${exeName} PRIVATE Qt6::Widgets)
target_link_libraries(${exeName} PRIVATE Qt6::Gui)
target_link_libraries(${exeName} PRIVATE Qt6::PrintSupport)
target_link_libraries(${exeName} PRIVATE Qt6::Svg)
target_link_libraries(${exeName} PRIVATE Qt6::Xml)

find_package(JKQTPlotter6 REQUIRED COMPONENTS JKQTPlotter6 JKQTCommon6 JKQTMath6 JKQTMathText6 REQUIRED)
target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTPlotter6)
target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTCommon6)
target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTMath6)
target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTMathText6)

add_custom_command(TARGET ${exeName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/addons"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/addons"
    COMMENT "Copying addons to build directory"
)

add_custom_command(TARGET ${exeName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/images"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/images"
    COMMENT "Copying images to build directory"
)

add_custom_command(TARGET ${exeName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/configurations"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/configurations"
    COMMENT "Copying configurations to build directory"
)

add_custom_command(TARGET ${exeName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/include"
    COMMENT "Copying include to build directory"
)

install(FILES   $<TARGET_FILE:Qt6::Core>
                $<TARGET_FILE:Qt6::Widgets>
                $<TARGET_FILE:Qt6::Gui>
                $<TARGET_FILE:Qt6::PrintSupport>
                $<TARGET_FILE:Qt6::Svg>
                $<TARGET_FILE:Qt6::Xml>
    DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

install(FILES   $<TARGET_FILE:JKQTPlotter6::JKQTPlotter6>
                $<TARGET_FILE:JKQTPlotter6::JKQTCommon6>
                $<TARGET_FILE:JKQTPlotter6::JKQTMath6>
                $<TARGET_FILE:JKQTPlotter6::JKQTMathText6>
    DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

install(DIRECTORY   ${CMAKE_PREFIX_PATH}/plugins/platforms
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins)
