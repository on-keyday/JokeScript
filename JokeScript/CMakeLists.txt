#    JokeScript
#
#    Copyright (c) 2021 on-keyday
#
#    This software is released under the MIT License.
#    http://opensource.org/licenses/mit-license.php
#
cmake_minimum_required (VERSION 3.8)

add_library (JokeScript SHARED "JokeScriptType.cpp" "JokeScriptType.h"  "JokeScriptLog.h" "JokeScriptCommon.h" "JokeScriptCommon.cpp" "JokeScript.cpp" "JokeScriptLog.cpp" "JokeScriptLoader.h" "JokeScriptLoader.cpp" "JokeScriptInfoLists.h" "JokeScriptInfoLists.cpp" "JokeScriptVariable.h" "JokeScriptVariable.cpp" "JokeScriptFunction.h" "JokeScriptFunction.cpp" "JokeScriptKeyword.h" "JokeScriptKeyword.cpp" "JokeScriptTree.h" "JokeScriptTree.cpp")

add_executable(Joke "Joke.cpp")
target_link_libraries(Joke JokeScript)


