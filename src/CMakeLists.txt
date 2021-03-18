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

add_library (JokeScript SHARED "public/JokeScript.h" "public/JokeScript.cpp" "common/stdcpps.h" "common/structs.cpp" "common/structs.h" "common/io.h" "common/io.cpp" "common/log.h" "common/filereader.h" "common/filereader.cpp"  "common/log.cpp" "common/ctype.h" "common/ctype.cpp" "common/tools.h" "common/tools.cpp"  "user/json_tools.h" "user/json_tools.cpp" "user/user_action.h" "user/user_action.cpp" "compiler/data/identifier.h" "compiler/parser/syntax.h"  "compiler/data/maker.cpp" "compiler/data/identifier.cpp" "compiler/parser/syntax.cpp" "compiler/parser/type.cpp" "compiler/parser/variable.cpp" "compiler/parser/type.h" "compiler/parser/variable.h")

add_executable (Joker "executable/main.cpp")

target_link_libraries(Joker JokeScript)
