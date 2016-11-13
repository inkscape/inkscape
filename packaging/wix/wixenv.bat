@echo Setting environment variables for creating msi installer (adjust if necesssary!)

IF "%WIX_PATH%"=="" set WIX_PATH=C:\Program Files (x86)\WiX Toolset v3.10\bin
IF "%DEVLIBS_PATH%"=="" set DEVLIBS_PATH=c:\devlibs

rem uncomment and adjust the line below if your Inkscape distribution files are in a non-standard location
rem IF "%INKSCAPE_DIST_PATH%"=="" set INKSCAPE_DIST_PATH=..\..\build\inkscape

set PATH=%DEVLIBS_PATH%\python;%WIX_PATH%;%PATH%