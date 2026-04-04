project(test CXX)

option(BUILD_PLUGINS "Build a few plugins to test" ON)

# add warnings
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

# Plugin interface
add_library(spcaplugin INTERFACE spca.hpp)
target_include_directories(spcaplugin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Main program
add_executable(spcas spcaloader.hpp spcaloader.cpp main.cpp)
target_link_libraries(spcas PRIVATE spcaplugin)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(spcas PRIVATE dl)
endif()

# Plugins
if(BUILD_PLUGINS)
  message(STATUS "Configuring plugins")
  add_subdirectory(plugins)
endif(BUILD_PLUGINS)
