@echo off
call ..\common\copy_src
cd built
ninja
cd ..
