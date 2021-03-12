@echo off
cd clang_build
call clean.bat
cd ..\gcc_build
call clean.bat
cd ..\cl_build
call clean.bat
cd ..
