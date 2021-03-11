#    JokeScript
#
#    Copyright (c) 2021 on-keyday
#
#    This software is released under the MIT License.
#    http://opensource.org/licenses/mit-license.php
#
cmake_minimum_required (VERSION 3.8)

project ("JokeScript")


if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
message(FATAL_ERROR "in-source-build is not allowed.")
endif()

if(EXISTS Jcript)
add_subdirectory ("Jcript")
elseif(EXISTS src)
add_subdirectory("src")
else()
message(FATAL_ERROR "directory not found.")
endif()