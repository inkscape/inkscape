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

CXXTEST_TEMPLATE = $(srcdir)/cxxtest-template.tpl
CXXTESTGENFLAGS = --root --have-eh --template=$(CXXTEST_TEMPLATE)
CXXTESTGEN = $(top_srcdir)/cxxtest/cxxtestgen.pl $(CXXTESTGENFLAGS)
# Add test cases to this variable
CXXTEST_TESTSUITES =

# ################################################
#
#  E X T R A 
#
# ################################################

if PLATFORM_WIN32
win32_sources = winmain.cpp registrytool.cpp registrytool.h
win32ldflags = -lcomdlg32
endif 

if INKJAR
inkjar_dir = inkjar
inkjar_libs = inkjar/libinkjar.a
endif

inkscape_private_libs =	\
	libinkpre.a		\
	application/libinkapp.a	\
	ui/dialog/libuidialog.a	\
	dialogs/libspdialogs.a	\
	ui/cache/libuicache.a	\
	jabber_whiteboard/libjabber_whiteboard.a \
	pedro/libpedro.a	\
	trace/libtrace.a	\
	svg/libspsvg.a		\
	filters/libfilters.a	\
	widgets/libspwidgets.a	\
	display/libspdisplay.a	\
	helper/libspchelp.a	\
	libcroco/libcroco.a	\
	libnrtype/libnrtype.a	\
	libavoid/libavoid.a	\
	livarot/libvarot.a	\
	live_effects/liblive_effects.a	\
	live_effects/parameter/liblpeparam.a	\
	ui/view/libuiview.a	\
	ui/libui.a		\
	ui/widget/libuiwidget.a	\
	libgdl/libgdl.a		\
	graphlayout/libgraphlayout.a	\
	removeoverlap/libremoveoverlap.a		\
	libcola/libcola.a	\
	libvpsc/libvpsc.a	\
	extension/libextension.a	\
	extension/implementation/libimplementation.a	\
	extension/internal/libinternal.a	\
	libnr/libnr.a		\
	extension/script/libscript.a	\
	bind/libbind.a	\
	dom/libdom.a            \
	xml/libspxml.a		\
	2geom/lib2geom.a	\
	util/libinkutil.a	\
	io/libio.a	        \
	$(inkjar_libs)		\
	libinkpost.a		\
	debug/libinkdebug.a

all_libs =			\
	$(inkscape_private_libs)	\
	$(INKSCAPE_LIBS)	\
	$(GNOME_VFS_LIBS)	\
	$(XFT_LIBS)		\
	$(FREETYPE_LIBS)	\
	$(kdeldadd)		\
	$(win32ldflags)		\
	$(CARBON_LDFLAGS)	\
	$(PERL_LIBS)		\
	$(PYTHON_LIBS)		\
	$(INKBOARD_LIBS)	\
	$(LIBWPG_LIBS)		\
	$(IMAGEMAGICK_LIBS)

# Include all partial makefiles from subdirectories
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
include filters/Makefile_insert
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

# Executables compiled by "make" and installed by "make install"
bin_PROGRAMS = inkscape inkview

# Libraries which should be compiled by "make" but not installed
# Currently we use one library per source directory.
noinst_LIBRARIES = $(inkscape_private_libs)

# Extra files to remove when doing "make distclean"
DISTCLEANFILES =	\
	helper/sp-marshal.cpp	\
	helper/sp-marshal.h	\
	inkscape_version.h

# Extra files not mentioned as sources to include in the source tarball
EXTRA_DIST =	\
	$(top_srcdir)/Doxyfile	\
	sp-skeleton.cpp sp-skeleton.h	\
	algorithms/makefile.in	\
	application/makefile.in	\
	bind/makefile.in	\
	debug/makefile.in	\
	dialogs/makefile.in	\
	display/makefile.in	\
	dom/makefile.in		\
	extension/implementation/makefile.in	\
	extension/internal/makefile.in	\
	extension/makefile.in	\
	extension/script/makefile.in	\
	filters/makefile.in \
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


# #################################
# ## TESTING STUFF (make check) ###
# #################################

# List of all programs that should be built before testing. Note that this is
# different from TESTS, because some tests can be scripts that don't
# need to be built. There should be one test program per directory.
# automake adds $(EXEEXT) to check_PROGRAMS items but not to TESTS items:
# TESTS items can be scripts etc.
check_PROGRAMS =	\
	cxxtests	\
	style-test
# streamtest is unfinished and can't handle the relocations done during
# "make distcheck".

# List of all tests to be run.
TESTS = $(check_PROGRAMS)

# including the the testsuites here ensures that they get distributed
cxxtests_SOURCES = cxxtests.cpp $(CXXTEST_TESTSUITES)
cxxtests_LDADD = libnr/nr-compose-reference.o $(all_libs)

cxxtests.cpp: $(CXXTEST_TESTSUITES) $(CXXTEST_TEMPLATE)
	$(CXXTESTGEN) -o cxxtests.cpp $(CXXTEST_TESTSUITES)

# ################################################
#  D I S T
# ################################################

dist-hook:
	mkdir $(distdir)/pixmaps
	cp $(srcdir)/pixmaps/*xpm $(distdir)/pixmaps

distclean-local:
	rm -f cxxtests.xml cxxtests.log
