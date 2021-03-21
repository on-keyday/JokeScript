@echo off
cd clang_build
call run
cd ../gcc_build
call run
cd ../cl_build
call run
cd ..
