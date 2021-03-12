@echo off
call ..\common\reconfig_common
setlocal
call vcvars_call
cmake -G Ninja -D CMAKE_C_COMPILER=cl -D CMAKE_CXX_COMPILER=cl -B ./built .