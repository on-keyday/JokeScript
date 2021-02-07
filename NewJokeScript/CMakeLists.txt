#license
cmake_minimum_required (VERSION 3.8)


add_library (NewJokeScript SHARED "stdcpps.h" "common_structs.cpp" "common_structs.h" "common_io.h" "common_io.cpp" "log.h" "compiler_filereader.h" "compiler_filereader.cpp" "compiler_syntax_analyzer.h" "compiler_identifier_holder.h" "compiler_syntax_analyzer.cpp" "compiler_identifier_holder.cpp" "log.cpp" "compiler_ctype.h" "compiler_ctype.cpp" "compiler_type_analyzer.cpp" "compiler_type_analyzer.h" "common_tools.h" "common_tools.cpp")

add_executable(NewJoke "main.cpp" )
