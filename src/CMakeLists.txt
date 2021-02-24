#    JokeScript
#
#    Copyright (c) 2021 on-keyday
#
#    This software is released under the MIT License.
#    http://opensource.org/licenses/mit-license.php
#

cmake_minimum_required (VERSION 3.8)



add_library (JokeScript SHARED "JokeScript.h" "JokeScript.cpp" "stdcpps.h" "common_structs.cpp" "common_structs.h" "common_io.h" "common_io.cpp" "log.h" "compiler_filereader.h" "compiler_filereader.cpp" "compiler_syntax_analyzer.h" "compiler_identifier_holder.h" "compiler_syntax_analyzer.cpp" "compiler_identifier_holder.cpp" "log.cpp" "compiler_ctype.h" "compiler_ctype.cpp" "common_tools.h" "common_tools.cpp" "compiler_id_analyzer.h" "compiler_id_analyzer.cpp" "user_tools.h" "user_tools.cpp" "user_action.h" "user_action.cpp")


add_executable (Joker "main.cpp")

target_link_libraries(Joker JokeScript)

