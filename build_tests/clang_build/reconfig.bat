@echo off
call ..\common\reconfig_common
cmake -G Ninja -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ -B ./built .