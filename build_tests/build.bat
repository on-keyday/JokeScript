@echo off
cd clang_build
call build.bat
cd ..\gcc_build
call build.bat
cd ..\cl_build
call build.bat
cd ..
