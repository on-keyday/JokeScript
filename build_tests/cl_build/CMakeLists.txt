cmake_minimum_required (VERSION 3.8)

project ("JokeScript")

if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
message(FATAL_ERROR "in-source-build is not allowed.")
endif()

#set(CMAKE_CXX_FLAGS "-O0 -g")
add_subdirectory("src")