@echo off
setlocal
call ..\setting\common_setting.bat
xcopy %PROJECT_BASE%  pre_src /Y /E /Q /I
rmdir /S /Q src
rename pre_src src
