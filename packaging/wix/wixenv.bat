@echo Setting environment variables for creating msi installer

@echo detect OS version
ver | findstr /i "6\.1\." > nul
IF %ERRORLEVEL% EQU 0 goto win7

@rem all other cases
IF "%WIX_PATH%"=="" set WIX_PATH=C:\Programme\WiX Toolset v3.8\bin
IF "%DEVLIBS_PATH%"=="" set DEVLIBS_PATH=c:\devlibs
goto setpath

:win7
IF "%WIX_PATH%"=="" set WIX_PATH=C:\Program Files (x86)\WiX Toolset v3.8\bin
IF "%DEVLIBS_PATH%"=="" set DEVLIBS_PATH=c:\devlibs

:setpath
set PATH=%DEVLIBS_PATH%\python;%WIX_PATH%;%PATH%

:end
