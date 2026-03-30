project(helloworld CXX)

# some basic configurations
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR})

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

#qt_standard_project_setup()

#add_executable(helloworld WIN32 main.cpp AtelierMainWindow.cpp AtelierWidget.cpp
#    AtelierMainWindow.h AtelierWidget.h)
#qt_add_executable(helloworld main.cpp AtelierMainWindow.cpp AtelierWidget.cpp
#AtelierMainWindow.h AtelierWidget.h)

find_package(JKQTPlotter6 REQUIRED)

# add the example executable
add_executable(${PROJECT_NAME} WIN32 main.cpp AtelierMainWindow.cpp AtelierWidget.cpp
    AtelierMainWindow.h AtelierWidget.h simu.hpp)

target_link_libraries(${PROJECT_NAME} PUBLIC Qt6::Core Qt6::Widgets Qt6::Gui Qt6::PrintSupport Qt6::Svg Qt6::Xml)

target_link_libraries(${PROJECT_NAME} PUBLIC JKQTPlotter6::JKQTPlotter6)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

#target_link_libraries(helloworld PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Qt6::PrintSupport JKQTPlotter6_Debug)#JKQTPlotter6::JKQTPlotter6)

#add_custom_command(TARGET helloworld POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy
#            $<TARGET_FILE_DIR:helloworld>/$<TARGET_FILE_NAME:helloworld>
#            ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:helloworld>)



# Locate windeployqt
#get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
#get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
#find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

# Add post-build command
#add_custom_command(
#    TARGET helloworld POST_BUILD
#    COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:helloworld>"
#    COMMENT "Running windeployqt..."
#)
