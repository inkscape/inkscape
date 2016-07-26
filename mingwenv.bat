@echo off

REM -----------------------------------------------------------------------------
REM The paths in this file are used if you want to build Inkscape from the
REM the Windows built-in command line.
REM -----------------------------------------------------------------------------

REM Directory containing the precompiled Inkscape libraries. Usually c:\devlibs or c:\devlibs64
if "%DEVLIBS_PATH%"=="" set DEVLIBS_PATH=c:\devlibs64

REM Directory containing the MinGW instance used for compilation. Usually c:\mingw or c:\mingw64
REM Note: Make sure there are no whitespaces in the path. MinGW doesn't like that.. ;)
if "%MINGW_PATH%"=="" set MINGW_PATH=c:\mingw64

REM Directory containing the (optional) Ghostscript installation.
if "%GS_PATH%"=="" set GS_PATH=C:\latex\gs\gs8.61

REM -----------------------------------------------------------------------------
@echo Setting environment variables for MinGW build of Inkscape..

@echo  DEVLIBS_PATH: %DEVLIBS_PATH%
@echo  MINGW_PATH:   %MINGW_PATH%
@echo  GS_PATH:      %GS_PATH%

REM Include the MinGW environment in the path to prevent error messages during CMake configure.
set PATH=%DEVLIBS_PATH%\bin;%DEVLIBS_PATH%\python;%MINGW_PATH%\bin;%PATH%;%GS_PATH%\bin

REM Also set the pkgconfig path to prevent error messages during CMake configure.
set PKG_CONFIG_PATH=%DEVLIBS_PATH%\lib\pkgconfig