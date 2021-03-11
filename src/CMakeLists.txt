#    JokeScript
#
#    Copyright (c) 2021 on-keyday
#
#    This software is released under the MIT License.
#    http://opensource.org/licenses/mit-license.php
#

cmake_minimum_required (VERSION 3.8)

if(WIN32)
include_directories("D:/llvm-project/built/include")
else()
include_directories("/lib/llvm-10/include")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} `llvm-config-10 --cxxflags --ldflags --libs --system-libs` -fexceptions")
endif()

add_library (JokeScript SHARED "JokeScript.h" "JokeScript.cpp" "common/stdcpps.h" "common/structs.cpp" "common/structs.h" "common/io.h" "common/io.cpp" "compiler/log.h" "compiler/filereader.h" "compiler/filereader.cpp"  "compiler/log.cpp" "common/ctype.h" "common/ctype.cpp" "common/tools.h" "common/tools.cpp"  "user/user_tools.h" "user/user_tools.cpp" "user/user_action.h" "user/user_action.cpp" "compiler/identifier.h" "compiler/syntax.h"  "compiler/maker.cpp" "compiler/identifier.cpp" "compiler/syntax.cpp" "compiler/type.cpp" "compiler/variable.cpp")




add_executable (Joker "main.cpp")

target_link_libraries(Joker JokeScript)

