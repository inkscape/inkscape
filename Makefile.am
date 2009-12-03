## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

SUBDIRS = src doc share po

Graphicsdir = $(datadir)/applications
Graphics_in_files = inkscape.desktop.in
Graphics_DATA = $(Graphics_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

## dist-hook:
##	mkdir $(distdir)/samples
##	cp $(srcdir)/samples/*svg $(distdir)/samples
##	cp $(srcdir)/samples/*png $(distdir)/samples
##	cp $(srcdir)/AUTHORS $(distdir)
##	cp $(srcdir)/NEWS $(distdir)


man_MANS = inkscape.1 \
		inkview.1

EXTRA_DIST = \
	acinclude.m4 \
	fix-roff-punct	\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in	\
	mkinstalldirs		\
	$(Graphics_in_files)	\
	po/check-markup \
	utf8-to-roff \
	inkscape.1		\
	inkscape.fr.1		\
	inkscape.pod		\
	inkscape.fr.pod		\
	inkscape.spec.in	\
	inkscape.spec		\
	Info.plist.in	\
	Info.plist		\
	buildtool.cpp build.xml	\
	README README.ca.txt README.de.txt README.es.txt README.fr.txt README.it.txt	\
	HACKING.txt HACKING.de.txt HACKING.fr.txt HACKING.it.txt	\
	TRANSLATORS \
	inkscape.png \
	inkscape.ico \
	mingwenv.bat \
	src/helper/sp-marshal.cpp.mingw src/helper/sp-marshal.h.mingw \
	src/2geom/angle.h \
	src/inkscape-manifest.xml src/inkview-manifest.xml \
	cxxtest/COPYING	\
	cxxtest/cxxtestgen.pl \
	cxxtest/cxxtestgen.py \
	cxxtest/cxxtest/Descriptions.cpp \
	cxxtest/cxxtest/Descriptions.h \
	cxxtest/cxxtest/DummyDescriptions.cpp \
	cxxtest/cxxtest/DummyDescriptions.h \
	cxxtest/cxxtest/ErrorFormatter.h \
	cxxtest/cxxtest/ErrorPrinter.h \
	cxxtest/cxxtest/Flags.h \
	cxxtest/cxxtest/GlobalFixture.cpp \
	cxxtest/cxxtest/GlobalFixture.h \
	cxxtest/cxxtest/Gui.h \
	cxxtest/cxxtest/LinkedList.cpp \
	cxxtest/cxxtest/LinkedList.h \
	cxxtest/cxxtest/Mock.h \
	cxxtest/cxxtest/ParenPrinter.h \
	cxxtest/cxxtest/QtGui.h \
	cxxtest/cxxtest/RealDescriptions.cpp \
	cxxtest/cxxtest/RealDescriptions.h \
	cxxtest/cxxtest/Root.cpp \
	cxxtest/cxxtest/SelfTest.h \
	cxxtest/cxxtest/StdHeaders.h \
	cxxtest/cxxtest/StdValueTraits.h \
	cxxtest/cxxtest/StdioFilePrinter.h \
	cxxtest/cxxtest/StdioPrinter.h \
	cxxtest/cxxtest/TeeListener.h \
	cxxtest/cxxtest/TestListener.h \
	cxxtest/cxxtest/TestRunner.h \
	cxxtest/cxxtest/TestSuite.cpp \
	cxxtest/cxxtest/TestSuite.h \
	cxxtest/cxxtest/TestTracker.cpp \
	cxxtest/cxxtest/TestTracker.h \
	cxxtest/cxxtest/ValueTraits.cpp \
	cxxtest/cxxtest/ValueTraits.h \
	cxxtest/cxxtest/Win32Gui.h \
	cxxtest/cxxtest/X11Gui.h \
	cxxtest/cxxtest/YesNoRunner.h \
	packaging/autopackage/default.apspec.in	\
	packaging/macosx/dmg_background.png	\
	packaging/macosx/inkscape.ds_store	\
	packaging/macosx/osx-app.sh	\
	packaging/macosx/osx-dmg.sh	\
	packaging/macosx/osx-build.sh	\
	packaging/macosx/Resources/MenuBar.nib/classes.nib	\
	packaging/macosx/Resources/MenuBar.nib/info.nib	\
	packaging/macosx/Resources/MenuBar.nib/objects.xib	\
	packaging/macosx/Resources/application-gimp-gradient.icns \
	packaging/macosx/Resources/application-vnd.ms.xaml.icns \
	packaging/macosx/Resources/image-vnd.sk1.icns \
	packaging/macosx/Resources/application-illustrator.icns \
	packaging/macosx/Resources/application-vnd.corel-draw.icns \
	packaging/macosx/Resources/image-x-eps.icns \
	packaging/macosx/Resources/Inkscape.icns \
	packaging/macosx/Resources/image-vnd.dxf.icns \
	packaging/macosx/Resources/image-svg+xml-compressed.icns \
	packaging/macosx/Resources/application-pdf.icns \
	packaging/macosx/Resources/application-vnd.corel-draw-template.icns \
	packaging/macosx/Resources/application-vnd.corel-draw-compressed.icns \
	packaging/macosx/Resources/image-svg+xml.icns \
	packaging/macosx/Resources/image-vnd.windows-metafile.icns \
	packaging/macosx/Resources/image-x-ps.icns \
	packaging/macosx/Resources/application-vnd.wordperfect-graphic.icns \
	packaging/macosx/Resources/application-illustrator-svg.icns \
	packaging/macosx/Resources/openDoc	\
	packaging/macosx/Resources/script	\
	packaging/macosx/Resources/ProgressWindow.nib/classes.nib	\
	packaging/macosx/Resources/ProgressWindow.nib/info.nib	\
	packaging/macosx/Resources/ProgressWindow.nib/objects.xib	\
	packaging/macosx/Resources/bin/getdisplay.sh	\
	packaging/macosx/Resources/bin/inkscape	\
	packaging/macosx/Resources/etc/fonts/fonts.conf	\
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/pre_gtkrc \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars/stepper-down.png \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars/stepper-left.png \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars/stepper-right.png \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars/stepper-up.png \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars/trough-scrollbar-horiz.png \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars/trough-scrollbar-vert.png \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars_1/copy-slider.sh \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars_1/slider-horiz-prelight.png \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars_1/slider-horiz.png \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars_1/slider-vert-prelight.png \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars_1/slider-vert.png \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars_6/copy-slider.sh \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars_6/slider-horiz-prelight.png \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars_6/slider-horiz.png \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars_6/slider-vert-prelight.png \
	packaging/macosx/Resources/themes/Clearlooks-Quicksilver-OSX/gtk-2.0/Scrollbars_6/slider-vert.png \
	packaging/macosx/ScriptExec/English.lproj/main.nib/classes.nib	\
	packaging/macosx/ScriptExec/English.lproj/main.nib/info.nib	\
	packaging/macosx/ScriptExec/English.lproj/main.nib/objects.xib	\
	packaging/macosx/ScriptExec/English.lproj/InfoPlist.strings	\
	packaging/macosx/ScriptExec/Info.plist	\
	packaging/macosx/ScriptExec/ScriptExec_Prefix.pch	\
	packaging/macosx/ScriptExec/main.c	\
	packaging/macosx/ScriptExec/openDoc	\
	packaging/macosx/ScriptExec/script	\
	packaging/macosx/ScriptExec/version.plist	\
	packaging/macosx/ScriptExec/MenuBar.nib/classes.nib	\
	packaging/macosx/ScriptExec/MenuBar.nib/info.nib	\
	packaging/macosx/ScriptExec/MenuBar.nib/objects.xib	\
	packaging/macosx/ScriptExec/ScriptExec.xcode/project.pbxproj	\
	packaging/macosx/ScriptExec/ScriptExec.xcodeproj/project.pbxproj \
	packaging/win32/inkscape.nsi	\
	packaging/win32/inkscape.nsi.uninstall	\
	packaging/win32/breton.nsh	\
	packaging/win32/catalan.nsh	\
	packaging/win32/czech.nsh	\
	packaging/win32/english.nsh	\
	packaging/win32/finnish.nsh	\
	packaging/win32/french.nsh	\
	packaging/win32/galician.nsh	\
	packaging/win32/german.nsh	\
	packaging/win32/italian.nsh	\
	packaging/win32/japanese.nsh	\
	packaging/win32/polish.nsh	\
	packaging/win32/russian.nsh	\
	packaging/win32/slovak.nsh	\
	packaging/win32/slovenian.nsh	\
	packaging/win32/spanish.nsh	\
	packaging/win32/simpchinese.nsh	\
	packaging/win32/tradchinese.nsh	\
	packaging/win32/header.bmp	\
	packaging/win32/header.svg	\
	packaging/win32/AdvUninstLog.nsh	\
	packaging/win32/AdvUninstLog.txt	\
	packaging/win32/md5dll.dll	\
	packaging/win32/md5dll.txt	\
	packaging/win32/messagebox.dll	\
	packaging/win32/MessageBox.txt	\
	packaging/win32/welcomefinish.bmp	\
	packaging/win32/welcomefinish.svg	\
	packaging/win32/portable/help.html \
	packaging/win32/portable/readme.packaging.txt \
	packaging/win32/portable/App/readme.txt \
	packaging/win32/portable/App/AppInfo/appicon.ico \
	packaging/win32/portable/App/AppInfo/appicon_16.png \
	packaging/win32/portable/App/AppInfo/appicon_32.png \
	packaging/win32/portable/App/AppInfo/appinfo.ini \
	packaging/win32/portable/App/AppInfo/installer.ini \
	packaging/win32/portable/Other/Help/images/donation_button.png \
	packaging/win32/portable/Other/Help/images/favicon.ico \
	packaging/win32/portable/Other/Help/images/help_background_footer.png \
	packaging/win32/portable/Other/Help/images/help_background_header.png \
	packaging/win32/portable/Other/Help/images/help_logo_top.png \
	packaging/win32/portable/Other/Source/InkscapePortable.ini \
	packaging/win32/portable/Other/Source/InkscapePortable.jpg \
	packaging/win32/portable/Other/Source/InkscapePortable.nsi \
	packaging/win32/portable/Other/Source/License.txt \
	packaging/win32/portable/Other/Source/PortableApps.comInstallerCustom.nsh \
	packaging/win32/portable/Other/Source/PortableApps.comLauncherLANG_ENGLISH.nsh \
	packaging/win32/portable/Other/Source/Readme.txt \
	packaging/win32/portable/Other/Source/ReplaceInFileWithTextReplace.nsh \
        po/inkscape.pot \
        po/update_po_files.sh \
	src/inkscape.rc	src/inkview.rc

# We want the built inkscape.1 to go in the distributed .tar.gz so
# that .tar.gz users don't need a full perl distribution to get a man
# page.

DISTCLEANFILES = inkscape.desktop \
                 intltool-extract \
                 intltool-merge \
                 intltool-update

#ACLOCAL_AMFLAGS = -I m4

SUFFIXES = .pod .1
inkscape.1 inkscape.fr.1: fix-roff-punct Makefile.am AUTHORS utf8-to-roff
.pod.1:
	set -e; \
	d=`sed -n 's,/,-,g;s,.*\$$[D]ate: \(..........\).*,\1,p' $<`; \
	pod2man $< \
	  | sed 's/^\.TH .*/.TH INKSCAPE 1 "'"$$d"'" "Inkscape-$(VERSION)" "Inkscape"/' \
	  | perl -pe 'if (/^\[\% .*INCLUDE.*AUTHORS.* \%\]/) { open(FOO, "$(srcdir)/AUTHORS");$$_ = join(",\n", map { chomp; $$_ } <FOO>) . "\n"; }' \
	  | perl $(srcdir)/fix-roff-punct \
	  | perl -CI $(srcdir)/utf8-to-roff	\
	  > tmp.$$$$ \
	 && mv -f tmp.$$$$ $@
# man 7 groff_char for further remarks on charset encoding for man pages.

po/inkscape.pot:
	(cd $(srcdir)/po ; intltool-update --pot --output-file=$(distdir)/po/inkscape.pot)

install-data-local: inkscape.fr.1
	$(mkinstalldirs) $(DESTDIR)$(mandir)/fr/man1
	$(INSTALL_DATA) `if [ -f inkscape.fr.1 ]; then :; else echo $(srcdir)/;fi`inkscape.fr.1 $(DESTDIR)$(mandir)/fr/man1/inkscape.1

.PHONY: warn_markup
noinst_DATA = warn_markup
warn_markup:
	@if perl $(srcdir)/po/check-markup $(srcdir)/po/*.po; [ $$? = 1 ]; then \
	  echo "WARNING: Bad markup found in translations.  Please consider fixing the above problems." >&2; \
	fi

.PHONY: check_markup
check_DATA = check_markup
check_markup:
	perl $(srcdir)/po/check-markup $(srcdir)/po/*.po || [ $$? = 127 ]
#	# `make check' probably shouldn't require perl to be installed, hence ignoring 127.

distcheck-hook:
	perl $(srcdir)/po/check-markup $(srcdir)/po/*.po

test-check-markup:
	if perl -e 42; then \
	  perl $(srcdir)/po/check-markup $(srcdir)/po/bad.po.test 2>&1	\
	    | diff $(srcdir)/po/bad.po.test.exp -;	\
	fi
