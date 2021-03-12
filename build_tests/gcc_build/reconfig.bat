@echo off
call ..\common\reconfig_common
cmake -G Ninja -D CMAKE_C_COMPILER=gcc -D CMAKE_CXX_COMPILER=g++ -B ./built .