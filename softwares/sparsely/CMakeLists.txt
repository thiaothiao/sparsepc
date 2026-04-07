project(sparsely CXX)

# some basic configurations
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR})

option(BUILD_PLUGINS "Build a few plugins to test" ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui PrintSupport Svg Xml OpenGl REQUIRED)

message( STATUS )
message( STATUS "=============================================================================" )
message( STATUS "== JKQTPlotterLib:simpletest_cmake Build Information                       ==" )
message( STATUS "=============================================================================" )
message( STATUS "Version:               ${PROJECT_VERSION}")
message( STATUS "Build type:            ${CMAKE_BUILD_TYPE}")
message( STATUS "Build types:           ${CMAKE_CONFIGURATION_TYPES}")
message( STATUS "Using CMake:           ${CMAKE_VERSION}")
message( STATUS "Using CMake Generator: ${CMAKE_GENERATOR}")
message( STATUS "Using compiler:        ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, platform: ${CMAKE_CXX_PLATFORM_ID}" )
message( STATUS "LIB-Architecture:      CXX: ${CMAKE_CXX_LIBRARY_ARCHITECTURE} / BASE: ${CMAKE_LIBRARY_ARCHITECTURE}" )
message( STATUS "System Name:           ${CMAKE_SYSTEM_NAME}" )
message( STATUS "Processor Name:        ${CMAKE_SYSTEM_PROCESSOR}" )
message( STATUS "Using Qt:              ${QT_VERSION}" )
message( STATUS "                       [at ${QT_DIR}]" )

find_package(JKQTPlotter6 REQUIRED)

# add the example executable
add_executable(${PROJECT_NAME} WIN32 main.cpp AtelierMainWindow.cpp AtelierWidget.cpp PreferencesDialog.cpp
    AtelierMainWindow.h AtelierWidget.h PreferencesDialog.h)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Qt6::PrintSupport Qt6::Svg Qt6::Xml)

target_link_libraries(${PROJECT_NAME} PRIVATE JKQTPlotter6::JKQTPlotter6)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/images"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/images"
    COMMENT "Copying images to build directory"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/configurations"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/configurations"
    COMMENT "Copying configurations to build directory"
)

# Plugins
if(BUILD_PLUGINS)
  message(STATUS "Configuring plugins")
  add_subdirectory(plugins)
endif(BUILD_PLUGINS)

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
