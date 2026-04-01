add_executable(main main.cpp simu.hpp)

add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE_DIR:main>/$<TARGET_FILE_NAME:main>
            ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:main>
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
