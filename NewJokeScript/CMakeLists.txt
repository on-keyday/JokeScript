#    JokeScript
#
#    Copyright (c) 2021 on-keyday
#
#    This software is released under the MIT License.
#    http://opensource.org/licenses/mit-license.php
#
cmake_minimum_required (VERSION 3.8)

include_directories("D:/llvm-project/llvm-project-llvmorg-11.0.1/llvm/include")

add_library (NewJokeScript SHARED "stdcpps.h" "common_structs.cpp" "common_structs.h" "common_io.h" "common_io.cpp" "log.h" "compiler_filereader.h" "compiler_filereader.cpp" "compiler_syntax_analyzer.h" "compiler_identifier_holder.h" "compiler_syntax_analyzer.cpp" "compiler_identifier_holder.cpp" "log.cpp" "compiler_ctype.h" "compiler_ctype.cpp" "compiler_id_analyzer.cpp" "compiler_id_analyzer.h" "common_tools.h" "common_tools.cpp" "compiler_tree_to_llvm.h" "llvms.h" "JokeScript.h" "JokeScript.cpp" "compiler_tree_to_llvm.cpp")

add_executable(NewJoke "main.cpp" )
