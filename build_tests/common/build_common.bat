@echo off
call ..\common\copy_src
setlocal
call ..\setting\common_setting.bat
cd built
%RUN_GENERATOR%
cd ..
exit /b %errorlevel%
