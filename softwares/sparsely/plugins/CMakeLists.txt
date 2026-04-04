add_library(dca SHARED dca.hpp dca.cpp)
target_link_libraries(dca PRIVATE spcaplugin)