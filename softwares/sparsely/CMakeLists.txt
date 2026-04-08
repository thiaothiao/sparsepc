project(sparsely CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui PrintSupport Svg Xml OpenGl REQUIRED)

find_package(JKQTPlotter6 REQUIRED COMPONENTS JKQTPlotter6 JKQTCommon6 JKQTMath6 JKQTMathText6)

qt_standard_project_setup()

add_executable(${PROJECT_NAME} WIN32 lab.cpp labmainwindow.cpp labwidget.cpp preferences.cpp preferencesdialog.cpp
    labmainwindow.h labwidget.h preferences.h preferencesdialog.h)

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
