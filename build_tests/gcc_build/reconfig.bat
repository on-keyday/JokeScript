@echo off
call ..\common\reconfig_common
cmake -G %CMAKE_GENERATOR% -D CMAKE_C_COMPILER=gcc -D CMAKE_CXX_COMPILER=g++ -B ./built .