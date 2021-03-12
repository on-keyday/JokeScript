# How to build

## 1.run ```. config``` or ```config.bat```.
## 2.run ```. build``` or ```build.bat```. 
## 3.you can see built executable on ```(compiler)_build/built/src/```
##### note:on linux shell, 'cl_build' is not usable.


# Config Settings

## you can edit files in ./setting

### common_setting
### PROJECT_BASE|PROJECT_BASE_PLUS_WILD_CARD: source code path. default:..\..\src|../../src/*
### CMAKE_GENERATOR:cmake's generator name. default:Ninja
### RUN_GENERATOR:running generator. default:ninja

### vs_setting (for Windows)
### PathToVsDevCmd:path to developer command prompt. default:"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
### PathToVcVars:path to vcvarsXX.bat. default:"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"