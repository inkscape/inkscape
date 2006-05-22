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
	$(LIBLOUDMOUTH_CFLAGS) \
	$(XFT_CFLAGS)	\
	-DPOTRACE=\"potrace\"	\
	$(INKSCAPE_CFLAGS) \
	-I$(top_srcdir)/cxxtest

include Makefile_insert
include application/Makefile_insert
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
include jabber_whiteboard/Makefile_insert
include libcroco/Makefile_insert
include libnr/Makefile_insert
include libnrtype/Makefile_insert
include libavoid/Makefile_insert
include livarot/Makefile_insert
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
include ui/dialog/Makefile_insert
include ui/view/Makefile_insert
include ui/widget/Makefile_insert
include util/Makefile_insert
include trace/Makefile_insert

bin_PROGRAMS = inkscape inkview

noinst_LIBRARIES =	\
	libinkpre.a	\
	application/libinkapp.a	\
	dialogs/libspdialogs.a	\
	jabber_whiteboard/libjabber_whiteboard.a \
	display/libspdisplay.a	\
	dom/libdom.a	\
	extension/implementation/libimplementation.a	\
	extension/internal/libinternal.a	\
	extension/libextension.a	\
	extension/script/libscript.a	\
	helper/libspchelp.a	\
	io/libio.a	\
	libcroco/libcroco.a	\
	ui/libui.a	\
	ui/dialog/libuidialog.a	\
	ui/view/libuiview.a 	\
	ui/widget/libuiwidget.a	\
	util/libinkutil.a \
	debug/libinkdebug.a \
	$(inkjar_libs)	\
	libnr/libnr.a	\
	libnrtype/libnrtype.a	\
	libavoid/libavoid.a	\
	livarot/libvarot.a	\
	removeoverlap/libremoveoverlap.a	\
	graphlayout/libgraphlayout.a	\
	svg/libspsvg.a	\
	widgets/libspwidgets.a	\
	trace/libtrace.a  \
	xml/libspxml.a	\
	libinkpost.a

check_LIBRARIES =	\
	libnr/libtest-nr.a	\
	svg/libtest-svg.a	\
	xml/libtest-xml.a

DISTCLEANFILES =	\
	helper/sp-marshal.cpp	\
	helper/sp-marshal.h	\
	inkscape_version.h

EXTRA_DIST =	\
	mkdep.pl	\
	mkfiles.pl	\
	make.exclude	\
	Doxyfile	\
	sp-skeleton.cpp sp-skeleton.h	\
	algorithms/makefile.in	\
	application/makefile.in	\
	debug/makefile.in	\
	dialogs/makefile.in	\
	dialogs/filedialog-win32.cpp	\
	display/makefile.in	\
	dom/makefile.in		\
	extension/implementation/makefile.in	\
	extension/internal/makefile.in	\
	extension/makefile.in	\
	extension/plugin/makefile.in	\
	extension/script/makefile.in	\
	helper/makefile.in	\
	inkjar/makefile.in	\
	io/makefile.in	\
	io/crystalegg.xml	\
	io/doc2html.xsl 	\
	jabber_whiteboard/makefile.in	\
	libcroco/makefile.in	\
	libnr/makefile.in	\
	libnrtype/makefile.in	\
	libavoid/makefile.in	\
	livarot/makefile.in	\
	removeoverlap/makefile.in	\
	svg/makefile.in		\
	trace/makefile.in	\
	traits/makefile.in	\
	utest/makefile.in	\
	ui/makefile.in		\
	ui/dialog/makefile.in	\
	ui/view/makefile.in	\
	ui/widget/makefile.in	\
	util/makefile.in	\
	widgets/makefile.in	\
	xml/makefile.in		\
	extension/internal/gnome.cpp	\
	extension/internal/gnome.h	\
	extension/internal/win32.cpp	\
	extension/internal/win32.h	\
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
	test-all$(EXEEXT)	\
	style-test$(EXEEXT)	\
	display/bezier-utils-test$(EXEEXT)	\
	helper/units-test$(EXEEXT)	\
	libnr/in-svg-plane-test$(EXEEXT)	\
	libnr/nr-matrix-test$(EXEEXT)	\
	libnr/nr-point-fns-test$(EXEEXT)	\
	libnr/nr-rotate-test$(EXEEXT)	\
	libnr/nr-rotate-fns-test$(EXEEXT)	\
	libnr/nr-scale-test$(EXEEXT)	\
	libnr/nr-translate-test$(EXEEXT)	\
	libnr/nr-types-test$(EXEEXT)	\
	libnr/test-nr$(EXEEXT)	\
	removeoverlap/remove_rectangle_overlap-test$(EXEEXT)	\
	svg/test-svg$(EXEEXT)	\
	util/list-container-test$(EXEEXT)	\
	xml/test-xml$(EXEEXT)	\
	xml/quote-test$(EXEEXT)	\
	xml/repr-action-test$(EXEEXT)

# streamtest is unfinished and can't handle the relocations done during
# "make distcheck".  Not needed for the 0.41 release.
#	io/streamtest$(EXEEXT)

# automake adds $(EXEEXT) to check_PROGRAMS items but not to TESTS items:
# TESTS items can be scripts etc.

check_PROGRAMS =	\
	test-all	\
	style-test	\
	display/bezier-utils-test	\
	helper/units-test	\
	libnr/in-svg-plane-test	\
	libnr/nr-matrix-test	\
	libnr/nr-point-fns-test	\
	libnr/nr-rotate-test	\
	libnr/nr-rotate-fns-test	\
	libnr/nr-scale-test	\
	libnr/nr-translate-test	\
	libnr/nr-types-test	\
	libnr/test-nr	\
	removeoverlap/remove_rectangle_overlap-test	\
	svg/test-svg	\
	util/list-container-test	\
	xml/test-xml	\
	xml/quote-test	\
	xml/repr-action-test

#	io/streamtest


test-all.cpp: \
		$(libnr_test_nr_a_SOURCES)	\
		$(svg_test_svg_a_SOURCES)	\
		$(xml_test_xml_a_SOURCES)	\
		$(libnr_test_nr_includes)	\
		$(svg_test_svg_includes)	\
		$(xml_test_xml_includes)	\
		$(test_all_includes)
	$(top_srcdir)/cxxtest/cxxtestgen.pl --template=selfname.tpl -root -o test-all.cpp	\
		$(libnr_test_nr_includes)	\
		$(svg_test_svg_includes)	\
		$(xml_test_xml_includes)	\
		$(test_all_includes)

test_all_SOURCES = \
	test-all.cpp $(test_all_includes)


test_all_LDADD =	\
	$(all_libs)	\
	io/libio.a	\
	$(libnr_test_nr_LDADD)	\
	$(svg_test_svg_LDADD)	\
	$(xml_test_xml_LDADD)


# ################################################
#
#  D I S T
#
# ################################################

dist-hook:
	mkdir $(distdir)/pixmaps
	cp $(srcdir)/pixmaps/*xpm $(distdir)/pixmaps

