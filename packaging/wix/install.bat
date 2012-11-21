
@echo build files.wxs
python files.py

@echo call wix compiler ...
candle inkscape.wxs -ext WiXUtilExtension
@if NOT %ERRORLEVEL% == 0 goto theend

candle files.wxs
@if NOT %ERRORLEVEL% == 0 goto theend

@echo call wix linker ...
light -ext WixUIExtension -ext WiXUtilExtension inkscape.wixobj files.wixobj -o inkscape.msi
@if NOT %ERRORLEVEL% == 0 goto theend

@echo the installer is now created
@rem uncomment following line if you want to test the installer
goto theend

@echo install ...
msiexec /i inkscape.msi /l*v inkscape.log

pause the program is now installed. press any key to run uninstaller ...
@echo deinstall ...
msiexec /x inkscape.msi

@echo ... finished

:theend
