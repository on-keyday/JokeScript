@echo off
cd clang_build
call reconfig.bat
cd ..\gcc_build
call reconfig.bat
cd ..\cl_build
call reconfig.bat
cd ..