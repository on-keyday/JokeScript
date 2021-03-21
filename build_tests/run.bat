@echo off
cd clang_build
call run.bat
cd ../gcc_build
call run.bat
cd ../cl_build
call run.bat
cd ..
