@echo off
cd clang_build&&call build.bat&&cd ..\gcc_build&&call build.bat&&cd ..\cl_build&&call build.bat&&cd ..

if %errorlevel% neq 0 goto ERR
exit
:ERR
echo error occurred.
echo if you did not run config.bat
echo please run it.
pause
exit /b 1