## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

SUBDIRS = src doc share po

Graphicsdir = $(datadir)/applications
Graphics_in_files = inkscape.desktop.in
Graphics_DATA = $(Graphics_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

appdatadir = $(datadir)/appdata
appdata_in_files = inkscape.appdata.xml.in
appdata_DATA = $(appdata_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

## dist-hook:
##	mkdir $(distdir)/samples
##	cp $(srcdir)/samples/*svg $(distdir)/samples
##	cp $(srcdir)/samples/*png $(distdir)/samples
##	cp $(srcdir)/AUTHORS $(distdir)
##	cp $(srcdir)/NEWS $(distdir)

DISTCHECK_CONFIGURE_FLAGS = --enable-localinstall

man_MANS = \
	inkscape.1 \
	inkscape.de.1		\
	inkscape.el.1		\
	inkscape.fr.1		\
	inkscape.ja.1		\
	inkscape.sk.1		\
	inkscape.zh_TW.1		\
	inkview.1

EXTRA_DIST = \
	acinclude.m4 \
	autogen.sh   \
	GPL2.txt \
	GPL3.txt \
	LGPL2.1.txt \
	fix-roff-punct	\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in	\
	$(Graphics_in_files)	\
	$(appdata_in_files)	\
	po/check-markup \
	utf8-to-roff \
	inkscape.1		\
	inkscape.de.1		\
	inkscape.el.1		\
	inkscape.fr.1		\
	inkscape.ja.1		\
	inkscape.sk.1		\
	inkscape.zh_TW.1		\
	inkscape.pod		\
	inkscape.de.pod		\
	inkscape.el.pod		\
	inkscape.fr.pod		\
	inkscape.sk.pod		\
	inkscape.zh_TW.pod		\
	inkscape.spec.in	\
	inkscape.spec		\
	Info.plist.in	\
	Info.plist		\
	buildtool.cpp build.xml	build-x64.xml \
	README README.ca.txt README.de.txt README.es.txt README.fr.txt README.it.txt README.osx.txt README.pt_BR.txt README.sk.txt README.sr.txt	\
	TRANSLATORS \
	inkscape.png \
	inkscape.ico \
	mingwenv.bat \
	src/helper/sp-marshal.cpp.mingw src/helper/sp-marshal.h.mingw \
	src/2geom/angle.h \
	src/inkscape-manifest.xml src/inkview-manifest.xml \
	src/inkscape-manifest-x64.xml src/inkview-manifest-x64.xml \
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
	packaging/macosx/README.txt \
	packaging/macosx/Resources/Inkscape-Generic.icns \
	packaging/macosx/Resources/Inkscape.icns \
	packaging/macosx/Resources/MenuBar.nib/classes.nib \
	packaging/macosx/Resources/MenuBar.nib/info.nib \
	packaging/macosx/Resources/MenuBar.nib/objects.xib \
	packaging/macosx/Resources/ProgressWindow.nib/classes.nib \
	packaging/macosx/Resources/ProgressWindow.nib/info.nib \
	packaging/macosx/Resources/ProgressWindow.nib/objects.xib \
	packaging/macosx/Resources/alert_fccache.sh \
	packaging/macosx/Resources/application-gimp-gradient.icns \
	packaging/macosx/Resources/application-illustrator-svg.icns \
	packaging/macosx/Resources/application-illustrator.icns \
	packaging/macosx/Resources/application-pdf.icns \
	packaging/macosx/Resources/application-vnd.corel-draw-compressed.icns \
	packaging/macosx/Resources/application-vnd.corel-draw-template.icns \
	packaging/macosx/Resources/application-vnd.corel-draw.icns \
	packaging/macosx/Resources/application-vnd.ms.xaml.icns \
	packaging/macosx/Resources/application-vnd.wordperfect-graphic.icns \
	packaging/macosx/Resources/bin/gimp-wrapper.sh \
	packaging/macosx/Resources/bin/inkscape \
	packaging/macosx/Resources/etc/fonts/fonts.conf \
	packaging/macosx/Resources/etc/gtk-2.0/gtkrc \
	packaging/macosx/Resources/image-svg+xml-compressed.icns \
	packaging/macosx/Resources/image-svg+xml.icns \
	packaging/macosx/Resources/image-vnd.dxf.icns \
	packaging/macosx/Resources/image-vnd.sk1.icns \
	packaging/macosx/Resources/image-vnd.windows-metafile.icns \
	packaging/macosx/Resources/image-x-eps.icns \
	packaging/macosx/Resources/image-x-ps.icns \
	packaging/macosx/Resources/openDoc \
	packaging/macosx/Resources/script \
	packaging/macosx/Resources/xdg_setup.sh \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/application-exit.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/dialog-information.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/document-new.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/document-open-recent.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/document-open.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/document-print-preview.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/document-print.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/document-properties.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/document-revert-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/document-revert-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/document-save-as.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/document-save.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/drive-harddisk.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/edit-clear.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/edit-copy.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/edit-cut.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/edit-delete.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/edit-find-replace.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/edit-find.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/edit-paste.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/edit-redo-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/edit-redo-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/edit-select-all.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/edit-undo-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/edit-undo-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/folder-remote.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/folder.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/format-indent-less-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/format-indent-less-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/format-indent-more-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/format-indent-more-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/format-justify-center.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/format-justify-fill.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/format-justify-left.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/format-justify-right.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/format-text-bold.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/format-text-italic.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/format-text-strikethrough.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/format-text-underline.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-bottom.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-down.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-first-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-first-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-home.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-jump-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-jump-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-last-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-last-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-next-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-next-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-previous-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-previous-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-top.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/go-up.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-caps-lock-warning.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-color-picker.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-connect.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-convert.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-disconnect.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-edit.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-font.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-index.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-orientation-landscape.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-orientation-portrait.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-orientation-reverse-landscape.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-orientation-reverse-portrait.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-page-setup.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-preferences.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-select-color.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-select-font.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-undelete-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/gtk-undelete-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/help-about.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/help-contents.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/image-missing.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/list-add.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/list-remove.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-floppy.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-optical.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-playback-pause.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-playback-start-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-playback-start-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-playback-stop.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-record.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-seek-backward-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-seek-backward-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-seek-forward-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-seek-forward-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-skip-backward-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-skip-backward-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-skip-forward-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/media-skip-forward-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/network-idle.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/printer-error.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/printer-info.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/printer-paused.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/printer-warning.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/process-stop.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/system-run.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/text-x-generic.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/tools-check-spelling.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/user-desktop.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/user-home.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/view-fullscreen.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/view-refresh.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/view-restore.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/view-sort-ascending.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/view-sort-descending.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/window-close.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/zoom-fit-best.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/zoom-in.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/zoom-original.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/16/zoom-out.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/20/gtk-apply.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/20/gtk-cancel.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/20/gtk-no.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/20/gtk-ok.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/20/gtk-yes.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/20/window-close.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/application-exit.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/audio-volume-high.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/audio-volume-low.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/audio-volume-medium.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/audio-volume-muted.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/dialog-information.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/document-new.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/document-open-recent.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/document-open.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/document-print-preview.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/document-print.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/document-properties.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/document-revert-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/document-revert-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/document-save-as.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/document-save.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/drive-harddisk.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/edit-clear.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/edit-copy.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/edit-cut.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/edit-delete.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/edit-find-replace.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/edit-find.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/edit-paste.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/edit-redo-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/edit-redo-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/edit-select-all.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/edit-undo-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/edit-undo-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/folder-remote.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/folder.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/format-indent-less-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/format-indent-less-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/format-indent-more-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/format-indent-more-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/format-justify-center.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/format-justify-fill.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/format-justify-left.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/format-justify-right.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/format-text-bold.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/format-text-italic.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/format-text-strikethrough.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/format-text-underline.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-bottom.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-down.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-first-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-first-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-home.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-jump-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-jump-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-last-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-last-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-next-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-next-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-previous-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-previous-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-top.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/go-up.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-caps-lock-warning.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-color-picker.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-connect.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-convert.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-disconnect.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-edit.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-font.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-index.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-orientation-landscape.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-orientation-portrait.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-orientation-reverse-landscape.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-orientation-reverse-portrait.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-page-setup.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-preferences.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-select-color.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-select-font.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-undelete-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/gtk-undelete-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/help-about.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/help-contents.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/image-missing.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/list-add.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/list-remove.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-floppy.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-optical.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-playback-pause.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-playback-start-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-playback-start-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-playback-stop.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-record.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-seek-backward-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-seek-backward-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-seek-forward-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-seek-forward-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-skip-backward-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-skip-backward-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-skip-forward-ltr.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/media-skip-forward-rtl.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/network-idle.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/printer-error.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/printer-info.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/printer-paused.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/printer-warning.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/process-stop.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/system-run.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/text-x-generic.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/tools-check-spelling.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/user-desktop.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/user-home.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/view-fullscreen.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/view-refresh.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/view-restore.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/view-sort-ascending.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/view-sort-descending.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/window-close.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/zoom-fit-best.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/zoom-in.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/zoom-original.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/24/zoom-out.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/32/gtk-dnd-multiple.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/32/gtk-dnd.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/48/dialog-error.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/48/dialog-information.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/48/dialog-password.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/48/dialog-question.png \
	packaging/macosx/Resources-extras/src/icons/stock-icons/48/dialog-warning.png \
	packaging/macosx/ScriptExec/English.lproj/InfoPlist.strings \
	packaging/macosx/ScriptExec/English.lproj/main.nib/classes.nib \
	packaging/macosx/ScriptExec/English.lproj/main.nib/info.nib \
	packaging/macosx/ScriptExec/English.lproj/main.nib/objects.xib \
	packaging/macosx/ScriptExec/Info.plist \
	packaging/macosx/ScriptExec/MenuBar.nib/classes.nib \
	packaging/macosx/ScriptExec/MenuBar.nib/info.nib \
	packaging/macosx/ScriptExec/MenuBar.nib/objects.xib \
	packaging/macosx/ScriptExec/ScriptExec.xcodeproj/project.pbxproj \
	packaging/macosx/ScriptExec/ScriptExec_Prefix.pch \
	packaging/macosx/ScriptExec/launcher-quartz-no-macintegration.sh \
	packaging/macosx/ScriptExec/main.c \
	packaging/macosx/ScriptExec/openDoc \
	packaging/macosx/ScriptExec/script \
	packaging/macosx/ScriptExec/version.plist \
	packaging/macosx/create-stock-icon-theme.sh \
	packaging/macosx/dmg_background.png \
	packaging/macosx/dmg_background.svg \
	packaging/macosx/dmg_set_style.scpt \
	packaging/macosx/inkscape.ds_store \
	packaging/macosx/osx-app.sh \
	packaging/macosx/osx-build.sh \
	packaging/macosx/osx-dmg.sh \
	packaging/macosx/ports/_resources/port1.0/group/python-1.0.tcl \
	packaging/macosx/ports/devel/dbus-python/Portfile \
	packaging/macosx/ports/devel/inkscape-packaging/Portfile \
	packaging/macosx/ports/lang/python25/Portfile \
	packaging/macosx/ports/lang/python25/files/_localemodule.c.ed \
	packaging/macosx/ports/lang/python25/files/locale.py.ed \
	packaging/macosx/ports/lang/python25/files/patch-64bit.diff \
	packaging/macosx/ports/lang/python25/files/patch-FSIORefNum.diff \
	packaging/macosx/ports/lang/python25/files/patch-Lib-cgi.py.diff \
	packaging/macosx/ports/lang/python25/files/patch-Lib-distutils-dist.py.diff \
	packaging/macosx/ports/lang/python25/files/patch-Makefile.pre.in.diff \
	packaging/macosx/ports/lang/python25/files/patch-Misc-setuid-prog.c.diff \
	packaging/macosx/ports/lang/python25/files/patch-Modules-posixmodule.c.diff \
	packaging/macosx/ports/lang/python25/files/patch-configure.diff \
	packaging/macosx/ports/lang/python25/files/patch-fwrapv.diff \
	packaging/macosx/ports/lang/python25/files/patch-libedit.diff \
	packaging/macosx/ports/lang/python25/files/patch-mac_ver.diff \
	packaging/macosx/ports/lang/python25/files/patch-pyconfig.h.in.diff \
	packaging/macosx/ports/lang/python25/files/patch-setup.py-disabled_modules.diff \
	packaging/macosx/ports/lang/python25/files/patch-setup.py.diff \
	packaging/macosx/ports/lang/python25/files/pyconfig.ed \
	packaging/macosx/ports/lang/python25/files/pyconfig.h-universal.ed \
	packaging/macosx/ports/lang/python25/files/python25 \
	packaging/macosx/ports/lang/python26/Portfile \
	packaging/macosx/ports/lang/python26/files/patch-Lib-cgi.py.diff \
	packaging/macosx/ports/lang/python26/files/patch-Lib-distutils-dist.py.diff \
	packaging/macosx/ports/lang/python26/files/patch-Mac-IDLE-Makefile.in.diff \
	packaging/macosx/ports/lang/python26/files/patch-Mac-Makefile.in.diff \
	packaging/macosx/ports/lang/python26/files/patch-Mac-PythonLauncher-Makefile.in.diff \
	packaging/macosx/ports/lang/python26/files/patch-Mac-Tools-Doc-setup.py.diff \
	packaging/macosx/ports/lang/python26/files/patch-Makefile.pre.in.diff \
	packaging/macosx/ports/lang/python26/files/patch-setup.py.diff \
	packaging/macosx/ports/lang/python26/files/pyconfig.ed \
	packaging/macosx/ports/lang/python26/files/python26 \
	packaging/macosx/ports/lang/python26/files/version.plist \
	packaging/macosx/ports/python/py-Pillow/Portfile \
	packaging/macosx/ports/python/py-Pillow/files/patch-setup.py.diff \
	packaging/macosx/ports/python/py-gobject/Portfile \
	packaging/macosx/ports/python/py-gobject/files/00git_class_init.patch \
	packaging/macosx/ports/python/py-gobject/files/00git_fix_qdata_warning.patch \
	packaging/macosx/ports/python/py-gobject/files/00git_gio_flag_type.patch \
	packaging/macosx/ports/python/py-gobject/files/patch-pygi-info.c-p1.diff \
	packaging/macosx/ports/python/py-numpy/Portfile \
	packaging/macosx/ports/python/py-numpy/files/patch-f2py_setup.py.diff \
	packaging/macosx/ports/python/py-numpy/files/patch-fcompiler_g95.diff \
	packaging/macosx/ports/python/py-numpy/files/patch-numpy_core_setup.py.diff \
	packaging/macosx/ports/python/py-numpy/files/patch-numpy_linalg_setup.py.diff \
	packaging/macosx/ports/python/py-numpy/files/wrapper-template \
	packaging/macosx/ports/python/py-sk1libs/Portfile \
	packaging/macosx/ports/python/py-sk1libs/files/patch-src-imaging-libimagingft-_imagingft.c.diff \
	packaging/macosx/ports/python/py-sk1libs/files/patch-src-utils-fs.py.diff \
	packaging/macosx/ports/python/py-uniconvertor/Portfile \
	packaging/macosx/ports/python/py25-Pillow/Portfile \
	packaging/macosx/ports/python/py25-Pillow/files/patch-_imagingft.c.diff \
	packaging/macosx/ports/python/py25-Pillow/files/patch-setup.py-v1.7.8.diff \
	packaging/macosx/ports/python/py25-lxml/Portfile \
	packaging/macosx/ports/python/py25-nose/Portfile \
	packaging/macosx/ports/python/py25-nose/files/nosetests24 \
	packaging/macosx/ports/python/py25-nose/files/nosetests25 \
	packaging/macosx/ports/python/py25-nose/files/nosetests26 \
	packaging/macosx/ports/python/py25-nose/files/nosetests27 \
	packaging/macosx/ports/python/py25-nose/files/nosetests31 \
	packaging/macosx/ports/python/py25-nose/files/nosetests32 \
	packaging/macosx/ports/python/py25-nose/files/nosetests33 \
	packaging/macosx/ports/python/py25-nose/files/nosetests34 \
	packaging/macosx/ports/python/py25-numpy/Portfile \
	packaging/macosx/ports/python/py25-numpy/files/patch-f2py_setup.py.diff \
	packaging/macosx/ports/python/py25-numpy/files/patch-fcompiler_g95.diff \
	packaging/macosx/ports/python/py25-numpy/files/patch-numpy_distutils_fcompiler___init__.py.diff \
	packaging/macosx/ports/python/py25-numpy/files/patch-numpy_linalg_setup.py.diff \
	packaging/macosx/ports/python/py25-numpy/files/patch-setup.py.diff \
	packaging/macosx/ports/python/py25-numpy/files/wrapper-template \
	packaging/macosx/ports/python/py25-setuptools/Portfile \
	packaging/macosx/ports/python/py26-cython/Portfile \
	packaging/macosx/ports/python/py26-cython/files/cython24 \
	packaging/macosx/ports/python/py26-cython/files/cython25 \
	packaging/macosx/ports/python/py26-cython/files/cython26 \
	packaging/macosx/ports/python/py26-cython/files/cython27 \
	packaging/macosx/ports/python/py26-cython/files/cython31 \
	packaging/macosx/ports/python/py26-cython/files/cython32 \
	packaging/macosx/ports/python/py26-cython/files/cython33 \
	packaging/macosx/ports/python/py26-cython/files/cython34 \
	packaging/macosx/ports/python/py26-lxml/Portfile \
	packaging/macosx/ports/python/py26-nose/Portfile \
	packaging/macosx/ports/python/py26-nose/files/nosetests24 \
	packaging/macosx/ports/python/py26-nose/files/nosetests25 \
	packaging/macosx/ports/python/py26-nose/files/nosetests26 \
	packaging/macosx/ports/python/py26-nose/files/nosetests27 \
	packaging/macosx/ports/python/py26-nose/files/nosetests31 \
	packaging/macosx/ports/python/py26-nose/files/nosetests32 \
	packaging/macosx/ports/python/py26-nose/files/nosetests33 \
	packaging/macosx/ports/python/py26-nose/files/nosetests34 \
	packaging/macosx/ports/python/py26-numpy/Portfile \
	packaging/macosx/ports/python/py26-numpy/files/patch-f2py_setup.py.diff \
	packaging/macosx/ports/python/py26-numpy/files/patch-fcompiler_g95.diff \
	packaging/macosx/ports/python/py26-numpy/files/patch-numpy_core_setup.py.diff \
	packaging/macosx/ports/python/py26-numpy/files/patch-numpy_linalg_setup.py.diff \
	packaging/macosx/ports/python/py26-numpy/files/wrapper-template \
	packaging/macosx/ports/python/py26-setuptools/Portfile \
	packaging/win32/inkscape.nsi	\
	packaging/win32/inkscape.nsi.uninstall	\
	packaging/win32/languages/Breton.nsh	\
	packaging/win32/languages/Catalan.nsh	\
	packaging/win32/languages/Czech.nsh	\
	packaging/win32/languages/Danish.nsh	\
	packaging/win32/languages/Dutch.nsh	\
	packaging/win32/languages/English.nsh	\
	packaging/win32/languages/Finnish.nsh	\
	packaging/win32/languages/French.nsh	\
	packaging/win32/languages/Galician.nsh	\
	packaging/win32/languages/German.nsh	\
	packaging/win32/languages/Greek.nsh	\
	packaging/win32/languages/Hebrew.nsh	\
	packaging/win32/languages/Indonesian.nsh \
	packaging/win32/languages/Icelandic.nsh \
	packaging/win32/languages/Italian.nsh	\
	packaging/win32/languages/Japanese.nsh	\
	packaging/win32/languages/Polish.nsh	\
	packaging/win32/languages/Portuguese.nsh \
	packaging/win32/languages/PortugueseBR.nsh \
	packaging/win32/languages/Romanian.nsh	\
	packaging/win32/languages/Russian.nsh	\
	packaging/win32/languages/Slovak.nsh	\
	packaging/win32/languages/Slovenian.nsh	\
	packaging/win32/languages/Spanish.nsh	\
	packaging/win32/languages/SimpChinese.nsh	\
	packaging/win32/languages/TradChinese.nsh	\
	packaging/win32/languages/Ukrainian.nsh	\
	packaging/win32/header.bmp	\
	packaging/win32/header.svg	\
	packaging/win32/AdvUninstLog.nsh	\
	packaging/win32/AdvUninstLog.txt	\
	packaging/win32/ifexist.nsh \
	packaging/win32/md5dll.dll	\
	packaging/win32/md5dll.txt	\
	packaging/win32/messagebox.dll	\
	packaging/win32/MessageBox.txt	\
	packaging/win32/RequireLatestNSIS.nsh \
	packaging/win32/VersionCompleteXXXX.nsh \
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
	packaging/wix/README \
	packaging/wix/inkscape.wxs \
	packaging/wix/gpl-2.0.rtf \
	packaging/wix/install.bat \
	packaging/wix/files.py \
	packaging/wix/version.py \
	packaging/wix/next_uuid.py \
	packaging/wix/wixenv.bat \
	packaging/wix/Bitmaps/banner.bmp \
	packaging/wix/Bitmaps/banner.svg \
	packaging/wix/Bitmaps/dialog.bmp \
	packaging/wix/Bitmaps/dialog.svg \
	po/inkscape.pot \
	po/update_po_files.sh \
	src/inkscape.rc	src/inkview.rc	src/inkscape-x64.rc	src/inkview-x64.rc

# We want the built inkscape.1 to go in the distributed .tar.gz so
# that .tar.gz users don't need a full perl distribution to get a man
# page.

DISTCLEANFILES = inkscape.appdata.xml \
                 inkscape.desktop \
                 intltool-extract \
                 intltool-merge \
                 intltool-update

#ACLOCAL_AMFLAGS = -I m4

SUFFIXES = .pod .1
inkscape.1 inkscape.de.1 inkscape.el.1 inkscape.fr.1 inkscape.ja.1 inkscape.sk.1 inkscape.zh_TW.1 : fix-roff-punct Makefile.am AUTHORS
.pod.1:
	set -e; \
	d=`sed -n 's,/,-,g;s,.*\$$[D]ate: \(..........\).*,\1,p' $<`; \
	pod2man --utf8 $< \
	  | sed 's/^\.TH .*/.TH INKSCAPE 1 "'"$$d"'" "Inkscape-$(VERSION)" "Inkscape"/' \
	  | perl -pe 'if (/^\[\% .*INCLUDE.*AUTHORS.* \%\]/) { open(FOO, "$(srcdir)/AUTHORS");$$_ = join(",\n", map { chomp; $$_ } <FOO>) . "\n"; }' \
	  | perl $(srcdir)/fix-roff-punct \
	  > tmp.$$$$ \
	 && mv -f tmp.$$$$ $@
# ut8-to-roff removed (groff now supports utf8).
# 	  | perl -CI $(srcdir)/utf8-to-roff	\
# man 7 groff_char for further remarks on charset encoding for man pages.

po/inkscape.pot:
	(cd $(srcdir)/po ; intltool-update --pot --output-file=$(distdir)/po/inkscape.pot)

mande_DATA = inkscape.de.1
mandedir = $(mandir)/de/man1

manel_DATA = inkscape.el.1
maneldir = $(mandir)/el/man1

manfr_DATA = inkscape.fr.1
manfrdir = $(mandir)/fr/man1

manja_DATA = inkscape.ja.1
manjadir = $(mandir)/ja/man1

mansk_DATA = inkscape.ja.1
manskdir = $(mandir)/sk/man1

manzhtw_DATA = inkscape.zh_TW.1
manzhtwdir = $(mandir)/zh_TW/man1

#.PHONY: warn_markup
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
