Quick instructions:
===================

1) install MacPorts from source into $MP_PREFIX (e.g. "/opt/local-x11")
   <https://www.macports.org/install.php>

2) add MacPorts to your PATH environement variable, for example:

$ export PATH="$MP_PREFIX/bin:$MP_PREFIX/sbin:$PATH"

3) add 'ports/' subdirectory as local portfile repository:

$ sudo sed -e '/^rsync:/i\'$'\n'"file://$(pwd)/ports" -i "" "$MP_PREFIX/etc/macports/sources.conf"

4) index the new local portfile repository: 

$ (cd ports && portindex)

5) add default variants for x11-based package to MacPorts' global variants:

$ sudo sed -e '$a\'$'\n''+x11 -quartz -no_x11 +rsvg +Pillow -tkinter +gnome_vfs' -i "" "$MP_PREFIX/etc/macports/sources.conf"

6) install required dependencies: 

$ sudo port install inkscape-packaging

7) compile inkscape, create app bundle and DMG: 

$ LIBPREFIX="$MP_PREFIX" ./osx-build.sh a c b -j 5 i p -s d

8) upload the DMG.
