@echo off

rem This batch file launches Inkscape with a limited PATH.
rem This is to prevent those occurrences when a user might
rem have another Gtk library installation that might conflict
rem with the libs supplied by Inkscape.

rem set the following to the directory where Inkscape is installed.
set INKSCAPE=c:\inkscape

set PATH=%INKSCAPE%;%INKSCAPE%\python;%INKSCAPE%\perl;c:\windows;c:\windows\system32

start inkscape

