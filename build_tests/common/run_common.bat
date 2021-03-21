@echo off
call ..\setting\common_setting.bat
if exist built\src\%RUNEXECUTABLE%.exe (
  cd built/src
  %RUNEXECUTABLE%
  cd ../..
)
