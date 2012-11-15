@echo Setting environment variables for creating msi installer
IF "%DEVLIBS_PATH%"=="" set DEVLIBS_PATH=c:\devlibs
IF "%WIX_PATH%"=="" set WIX_PATH=C:\Programme\WiX Toolset v3.6\bin
set PATH=%DEVLIBS_PATH%\python;%WIX_PATH%;%PATH%
