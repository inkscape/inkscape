## Process this file with automake to produce Makefile.in


# ################################################
#
#  G L O B A L
#
# ################################################

# Should work in either automake1.7 or 1.8, but 1.6 doesn't
# handle foo/libfoo_a_CPPFLAGS properly (if at all).
# Update: We now avoid setting foo/libfoo_a_CPPFLAGS,
# so perhaps 1.6 will work.
AUTOMAKE_OPTIONS = 1.7 subdir-objects

INCLUDES =	\
	$(PERL_CFLAGS) $(PYTHON_CFLAGS)	\
	$(FREETYPE_CFLAGS)	\
	$(GNOME_PRINT_CFLAGS)	\
	$(GNOME_VFS_CFLAGS)	\
	$(IMAGEMAGICK_CFLAGS) \
	$(INKBOARD_CFLAGS) \
	$(LIBWPG_CFLAGS) \
	$(XFT_CFLAGS)	\
	$(LCMS_CFLAGS)	\
	$(POPPLER_CFLAGS)	\
	$(POPPLER_GLIB_CFLAGS)	\
	-DPOTRACE=\"potrace\"	\
	$(INKSCAPE_CFLAGS) \
	-I$(top_srcdir)/cxxtest \
	$(WIN32_CFLAGS)

include Makefile_insert
include application/Makefile_insert
include bind/Makefile_insert
include dialogs/Makefile_insert
include display/Makefile_insert
include dom/Makefile_insert
include extension/Makefile_insert
include extension/implementation/Makefile_insert
include extension/internal/Makefile_insert
include extension/script/Makefile_insert
include helper/Makefile_insert
include inkjar/Makefile_insert
include io/Makefile_insert
include pedro/Makefile_insert
include jabber_whiteboard/Makefile_insert
include libcroco/Makefile_insert
include libgdl/Makefile_insert
include libnr/Makefile_insert
include libnrtype/Makefile_insert
include libavoid/Makefile_insert
include livarot/Makefile_insert
include live_effects/Makefile_insert
include live_effects/parameter/Makefile_insert
include libvpsc/Makefile_insert
include libcola/Makefile_insert
include removeoverlap/Makefile_insert
include graphlayout/Makefile_insert
include svg/Makefile_insert
include utest/Makefile_insert
include widgets/Makefile_insert
include debug/Makefile_insert
include xml/Makefile_insert
include traits/Makefile_insert
include algorithms/Makefile_insert
include ui/Makefile_insert
include ui/cache/Makefile_insert
include ui/dialog/Makefile_insert
include ui/view/Makefile_insert
include ui/widget/Makefile_insert
include util/Makefile_insert
include trace/Makefile_insert
include 2geom/Makefile_insert

bin_PROGRAMS = inkscape inkview

noinst_LIBRARIES =	\
	libinkpre.a	\
	application/libinkapp.a	\
	dialogs/libspdialogs.a	\
	pedro/libpedro.a \
	jabber_whiteboard/libjabber_whiteboard.a \
	display/libspdisplay.a	\
	dom/libdom.a	\
	extension/implementation/libimplementation.a	\
	extension/internal/libinternal.a	\
	extension/libextension.a	\
	extension/script/libscript.a	\
	bind/libbind.a	\
	helper/libspchelp.a	\
	io/libio.a	\
	libcroco/libcroco.a	\
	libgdl/libgdl.a		\
	live_effects/liblive_effects.a \
	live_effects/parameter/liblpeparam.a \
	ui/libui.a	\
	ui/cache/libuicache.a	\
	ui/dialog/libuidialog.a	\
	ui/view/libuiview.a 	\
	ui/widget/libuiwidget.a	\
	util/libinkutil.a \
	debug/libinkdebug.a \
	$(inkjar_libs)	\
	libnr/libnr.a	\
	libnrtype/libnrtype.a	\
	libavoid/libavoid.a	\
	libvpsc/libvpsc.a	\
	libcola/libcola.a	\
	livarot/libvarot.a	\
	removeoverlap/libremoveoverlap.a	\
	graphlayout/libgraphlayout.a	\
	svg/libspsvg.a	\
	widgets/libspwidgets.a	\
	trace/libtrace.a  \
	xml/libspxml.a	\
	2geom/lib2geom.a	\
	libinkpost.a

check_LIBRARIES =	\
	display/libtest-display.a	\
	helper/libtest-helper.a	\
	libnr/libtest-nr.a	\
	svg/libtest-svg.a	\
	svg/libtest-stubs.a	\
	util/libtest-util.a	\
	xml/libtest-xml.a

DISTCLEANFILES =	\
	helper/sp-marshal.cpp	\
	helper/sp-marshal.h	\
	inkscape_version.h

EXTRA_DIST =	\
	Doxyfile	\
	sp-skeleton.cpp sp-skeleton.h	\
	algorithms/makefile.in	\
	application/makefile.in	\
	bind/makefile.in	\
	debug/makefile.in	\
	dialogs/makefile.in	\
	dialogs/filedialog-win32.cpp	\
	display/makefile.in	\
	dom/makefile.in		\
	extension/implementation/makefile.in	\
	extension/internal/makefile.in	\
	extension/makefile.in	\
	extension/script/makefile.in	\
	helper/makefile.in	\
	inkjar/makefile.in	\
	io/makefile.in	\
	io/crystalegg.xml	\
	io/doc2html.xsl 	\
	pedro/makefile.in	\
	jabber_whiteboard/makefile.in	\
	libgdl/makefile.in	\
	libcroco/makefile.in	\
	libnr/makefile.in	\
	libnrtype/makefile.in	\
	libavoid/makefile.in	\
	livarot/makefile.in	\
	live_effects/makefile.in	\
	live_effects/parameter/makefile.in	\
	removeoverlap/makefile.in	\
	svg/makefile.in		\
	trace/makefile.in	\
	traits/makefile.in	\
	utest/makefile.in	\
	ui/makefile.in		\
	ui/cache/makefile.in	\
	ui/dialog/makefile.in	\
	ui/view/makefile.in	\
	ui/widget/makefile.in	\
	util/makefile.in	\
	widgets/makefile.in	\
	xml/makefile.in		\
	2geom/makefile.in	\
	extension/internal/win32.cpp	\
	extension/internal/win32.h	\
	extension/internal/emf-win32-inout.cpp	\
	extension/internal/emf-win32-inout.h	\
	extension/internal/emf-win32-print.cpp	\
	extension/internal/emf-win32-print.h	\
	helper/sp-marshal.list	\
	utest/utest.h	\
	utest/test-1ary-cases.h	\
	traits/copy.h	\
	traits/function.h	\
	traits/list-copy.h	\
	traits/reference.h \
	$(jabber_whiteboard_SOURCES)

EXTRA_PROGRAMS =	\
	inkview 	\
	libnr/testnr

TESTS =	\
	test-src$(EXEEXT)	\
	style-test$(EXEEXT)	\
	display/bezier-utils-test$(EXEEXT)	\
	display/test-display$(EXEEXT)	\
	helper/units-test$(EXEEXT)	\
	helper/test-helper$(EXEEXT)	\
	libnr/in-svg-plane-test$(EXEEXT)	\
	libnr/nr-matrix-test$(EXEEXT)	\
	libnr/nr-point-fns-test$(EXEEXT)	\
	libnr/nr-rotate-test$(EXEEXT)	\
	libnr/nr-rotate-fns-test$(EXEEXT)	\
	libnr/nr-scale-test$(EXEEXT)	\
	libnr/nr-translate-test$(EXEEXT)	\
	libnr/nr-types-test$(EXEEXT)	\
	libnr/test-nr$(EXEEXT)	\
	svg/test-svg$(EXEEXT)	\
	util/list-container-test$(EXEEXT)	\
	util/test-util$(EXEEXT)	\
	xml/test-xml$(EXEEXT)	\
	xml/quote-test$(EXEEXT)	\
	xml/repr-action-test$(EXEEXT)

# As of 2007-01-13, these are failing...
#XFAIL_TESTS = \
#	test-src$(EXEEXT)	\
#	style-test$(EXEEXT)

# streamtest is unfinished and can't handle the relocations done during
# "make distcheck".  Not needed for the 0.41 release.
#	io/streamtest$(EXEEXT)

# automake adds $(EXEEXT) to check_PROGRAMS items but not to TESTS items:
# TESTS items can be scripts etc.

check_PROGRAMS =	\
	test-src	\
	style-test	\
	display/bezier-utils-test	\
	display/test-display	\
	helper/units-test	\
	helper/test-helper	\
	libnr/in-svg-plane-test	\
	libnr/nr-matrix-test	\
	libnr/nr-point-fns-test	\
	libnr/nr-rotate-test	\
	libnr/nr-rotate-fns-test	\
	libnr/nr-scale-test	\
	libnr/nr-translate-test	\
	libnr/nr-types-test	\
	libnr/test-nr	\
	svg/test-svg	\
	util/list-container-test	\
	util/test-util	\
	xml/test-xml	\
	xml/quote-test	\
	xml/repr-action-test

#	io/streamtest


test-src.cpp: \
		$(test_src_includes)
		$(top_srcdir)/cxxtest/cxxtestgen.pl --have-eh --template=$(srcdir)/selfname.tpl -root -o test-src.cpp	\
		$(test_src_includes)

test_src_SOURCES = \
	test-src.cpp $(test_src_includes)


test_src_LDADD =	\
	$(all_libs)	\
	io/libio.a	


# ################################################
#
#  D I S T
#
# ################################################

dist-hook:
	mkdir $(distdir)/pixmaps
	cp $(srcdir)/pixmaps/*xpm $(distdir)/pixmaps

distclean-local:
	rm -f display/test-display.xml display/test-display.log helper/test-helper.xml helper/test-helper.log libnr/test-nr.xml libnr/test-nr.log svg/test-svg.xml svg/test-svg.log util/test-util.xml util/test-util.log xml/test-xml.xml xml/test-xml.log test-src.xml test-src.log
