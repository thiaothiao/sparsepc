project(sparsely 
        VERSION 1.0.0 
        DESCRIPTION "SPARSELY Software - SPARSE principal component LaboratorY"
        LANGUAGES CXX)
		

# Optional: Add custom metadata
set(PROJECT_VENDOR "Indilo")
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")

# Generate the header file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
    @ONLY
)		
		
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui PrintSupport Svg Xml OpenGl REQUIRED)

find_package(JKQTPlotter6 REQUIRED COMPONENTS JKQTPlotter6 JKQTCommon6 JKQTMath6 JKQTMathText6)

set(sources main.cpp labmainwindow.cpp labwidget.cpp labcontroller.cpp labmodel.cpp labpreferences.cpp labpreferencesdialog.cpp labenums.h labmainwindow.h labwidget.h labcontroller.h labmodel.h labpreferences.h labpreferencesdialog.h)
set(appiconrc images/appicon.rc)

if(WIN32)
    enable_language(RC)
endif()

qt_standard_project_setup()
qt_add_resources(sources resources.qrc)
qt_add_executable(${PROJECT_NAME} ${sources} ${appiconrc})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Gui)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::PrintSupport)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Svg)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Xml)

target_link_libraries(${PROJECT_NAME} PRIVATE JKQTPlotter6::JKQTPlotter6)
target_link_libraries(${PROJECT_NAME} PRIVATE JKQTPlotter6::JKQTCommon6)
target_link_libraries(${PROJECT_NAME} PRIVATE JKQTPlotter6::JKQTMath6)
target_link_libraries(${PROJECT_NAME} PRIVATE JKQTPlotter6::JKQTMathText6)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/addons"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/addons"
    COMMENT "Copying addons to build directory"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/images"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/images"
    COMMENT "Copying images to build directory"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/configurations"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/configurations"
    COMMENT "Copying configurations to build directory"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:JKQTPlotter6::JKQTPlotter6>
        $<TARGET_FILE:JKQTPlotter6::JKQTCommon6>
        $<TARGET_FILE:JKQTPlotter6::JKQTMath6>
        $<TARGET_FILE:JKQTPlotter6::JKQTMathText6>
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    COMMENT "Copying my_lib to the new directory"
)

qt_generate_deploy_app_script(
    TARGET ${PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    NO_TRANSLATIONS
)

install(SCRIPT ${deploy_script})
