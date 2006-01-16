REM BEGIN
@echo off
REM edit %GSDIR% to match the ghostscript installation directory
set GSDIR=%PROGRAMFILES%\gs\gs8.51
set GSBINDIR=%GSDIR%\bin
set GSLIBDIR=%GSDIR%\lib
set PATH=%GSBINDIR%;%GSLIBDIR%;%PATH%
echo %PATH%
ps2pdf.bat %1 -
REM END