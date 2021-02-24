#    JokeScript
#
#    Copyright (c) 2021 on-keyday
#
#    This software is released under the MIT License.
#    http://opensource.org/licenses/mit-license.php
#
cmake_minimum_required (VERSION 3.8)

project ("Joker")

if(WIN32)
add_subdirectory ("Joker")
else()
add_subdirectory("src")
endif()