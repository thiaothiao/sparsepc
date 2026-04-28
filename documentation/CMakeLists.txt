cmake_minimum_required(VERSION 4.1)
project(Documentation)

find_package(Doxygen REQUIRED)

if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)

    set(DOXYGEN_EXCLUDE_PATTERNS "*/test/*" "*/build/*")

    doxygen_add_docs(docs
        ${PROJECT_SOURCE_DIR}/../sparsely
        ${PROJECT_SOURCE_DIR}/../include
        COMMENT "Generating API documentation with Doxygen"
    )
else()
    message(STATUS "Doxygen not found, documentation target will not be created.")
endif()
