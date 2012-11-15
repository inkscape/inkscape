@echo set environment parameter ...
rem call wixenv.bat

@echo todo implement custom icon and artwork
@echo todo insert files from ../source/release

@echo call wix compiler ...
candle inkscape.wxs -ext WiXUtilExtension
@if NOT %ERRORLEVEL% == 0 goto theend

candle files.wxs
@if NOT %ERRORLEVEL% == 0 goto theend

@echo call wix linker ...
light -ext WixUIExtension -ext WiXUtilExtension inkscape.wixobj files.wixobj -o inkscape.msi
@if NOT %ERRORLEVEL% == 0 goto theend

@echo the installer is now created
goto theend

@echo install ...
msiexec /i inkscape.msi /l*v inkscape.log

pause the program is now installed. press any key to run uninstaller ...
@echo deinstall ...
msiexec /x inkscape.msi

@echo ... finished

:theend
