@echo Setting environment variables for MinGw build of Inkscape
IF "%DEVLIBS_PATH%"=="" set DEVLIBS_PATH=c:\devlibs
IF "%MINGW_PATH%"=="" set MINGW_PATH=c:\mingw
set MINGW_BIN=%MINGW_PATH%\bin
set PKG_CONFIG_PATH=%DEVLIBS_PATH%\lib\pkgconfig
set GS_BIN=C:\latex\gs\gs8.61\bin
set PATH=%DEVLIBS_PATH%\bin;%DEVLIBS_PATH%\python;%MINGW_BIN%;%PATH%;%GS_BIN%
set CMAKE_PREFIX_PATH=%DEVLIBS_PATH%
set GTKMM_BASEPATH=%DEVLIBS_PATH%
