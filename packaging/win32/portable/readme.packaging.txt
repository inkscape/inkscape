REQUIREMENTS
============
· Inkscape compiled (see http://wiki.inkscape.org/wiki/index.php/Win32Port)
· NSIS Portable
  http://portableapps.com/apps/development/nsis_portable
· PortableApps.com AppCompactor
  http://portableapps.com/apps/utilities/portableapps.com_appcompactor
· PortableApps.com Installer
  http://portableapps.com/apps/development/portableapps.com_installer
· PortableApps.com Launcher
  http://portableapps.com/apps/development/portableapps.com_launcher

INSTRUCTIONS
============
An automated version is planned for later, but currently you'll just have to follow these instructions.  Oh, and it won't do the last half dozen steps automatically.

(0. Compile Inkscape)
1.  Copy everything from the "inkscape" build directory, EXCEPT for inkscape.dbg and gdb.exe, into ./App/Inkscape
2.  Update the version number in the "Version" section of ./App/AppInfo/appinfo.ini
4.  Run the PortableApps.com AppCompactor on ./App/Inkscape
3.  Run the PortableApps.com Launcher on this directory
5.  Run the PortableApps.com Installer on this directory

6.  InkscapePortable_X.XX.paf.exe will now be in packaging/win32, ready for release
7.  Test it (the installer and the installed)
8.  Send it to John T. Haller of PortableApps.com for digital signature
9.  Probably wait a few days...
10. Receive it back, signed
11. Test it again, just to make sure
12. Release it!
