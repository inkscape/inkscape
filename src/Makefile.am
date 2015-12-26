## Process this file with automake to produce Makefile.in

# ################################################
#  G L O B A L
# ################################################

# Should work in either automake1.7 or 1.8, but 1.6 doesn't
# handle foo/libfoo_a_CPPFLAGS properly (if at all).
# Update: We now avoid setting foo/libfoo_a_CPPFLAGS,
# so perhaps 1.6 will work.
AUTOMAKE_OPTIONS = 1.7 subdir-objects

# Executables compiled by "make" and installed by "make install"
bin_PROGRAMS = inkscape inkview

# Libraries which should be compiled by "make" but not installed.
# Use this only for libraries that are really standalone, rather than for
# source tree subdirectories.

if !WITH_EXT_GDL
internal_GDL = libgdl/libgdl.a
endif


noinst_LIBRARIES =	\
	libcroco/libcroco.a		\
	libavoid/libavoid.a		\
	$(internal_GDL)			\
	libuemf/libuemf.a   		\
	libcola/libcola.a		\
        inkgc/libinkgc.a		\
	libvpsc/libvpsc.a		\
	livarot/libvarot.a		\
	2geom/lib2geom.a		\
	libdepixelize/libdepixelize.a	\
	util/libutil.a			\
	libinkversion.a
#       libinkscape.a

all_libs =			\
	$(noinst_LIBRARIES)	\
	$(INKSCAPE_LIBS)	\
	$(INKSCAPE_CXX_DEPS_LIBS) \
	$(EXIF_LIBS)            \
	$(GNOME_VFS_LIBS)	\
	$(XFT_LIBS)		\
	$(FREETYPE_LIBS)	\
	$(kdeldadd)		\
	$(win32ldflags)		\
	$(LIBWPG_LIBS)		\
	$(LIBVISIO_LIBS)		\
	$(LIBCDR_LIBS)		\
	$(DBUS_LIBS)		\
	$(GDL_LIBS)             \
	$(IMAGEMAGICK_LIBS)     \
	$(X11_LIBS)

# Add sources common for Inkscape and Inkview to this variable.
ink_common_sources =
# Add Inkscape-only sources here.
inkscape_SOURCES =
# Add Inkview-only sources here.
inkview_SOURCES =
# Add sources that are built from meta files
BUILT_SOURCES =
# Extra files to distribute
EXTRA_DIST =

# C++-specific flags defined here
AM_CXXFLAGS = \
	$(INKSCAPE_CXX_DEPS_CFLAGS)

AM_CPPFLAGS =	\
	-I$(top_srcdir)/cxxtest \
	-I$(builddir)/extension/dbus   \
	$(EXIF_CFLAGS) \
	$(FREETYPE_CFLAGS)	\
	$(GNOME_PRINT_CFLAGS)	\
	$(GNOME_VFS_CFLAGS)	\
	$(IMAGEMAGICK_CFLAGS) \
	$(LIBWPG_CFLAGS) \
	$(LIBVISIO_CFLAGS) \
	$(LIBCDR_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(GDL_CFLAGS)  \
	$(XFT_CFLAGS)	\
	$(LCMS_CFLAGS)	\
	$(POPPLER_CFLAGS)	\
	$(POPPLER_GLIB_CFLAGS)	\
	-DPOTRACE=\"potrace\"	\
	$(INKSCAPE_CFLAGS) \
	$(WIN32_CFLAGS) \
	$(X11_CFLAGS)

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
win32ldflags = -lcomdlg32 -lmscms
mwindows = -mwindows
endif 

# Include all partial makefiles from subdirectories
include Makefile_insert
include display/Makefile_insert
include extension/Makefile_insert
include extension/dbus/Makefile_insert
include extension/implementation/Makefile_insert
include extension/internal/Makefile_insert
include filters/Makefile_insert
include helper/Makefile_insert
include io/Makefile_insert
include libcroco/Makefile_insert
include inkgc/Makefile_insert
include libgdl/Makefile_insert
include libnrtype/Makefile_insert
include libavoid/Makefile_insert
include livarot/Makefile_insert
include live_effects/Makefile_insert
include live_effects/parameter/Makefile_insert
include libvpsc/Makefile_insert
include libcola/Makefile_insert
include libuemf/Makefile_insert
include svg/Makefile_insert
include widgets/Makefile_insert
include debug/Makefile_insert
include xml/Makefile_insert
include ui/Makefile_insert
include ui/cache/Makefile_insert
include ui/dialog/Makefile_insert
include ui/tool/Makefile_insert
include ui/tools/Makefile_insert
include ui/view/Makefile_insert
include ui/widget/Makefile_insert
include util/Makefile_insert
include trace/Makefile_insert
include 2geom/Makefile_insert
include libdepixelize/Makefile_insert

# Extra files not mentioned as sources to include in the source tarball
EXTRA_DIST +=	\
	2geom/makefile.in	\
	debug/makefile.in	\
	display/makefile.in	\
	extension/implementation/makefile.in	\
	extension/internal/makefile.in	\
	extension/makefile.in	\
	filters/makefile.in \
	helper/makefile.in	\
	io/makefile.in	\
	libavoid/makefile.in	\
	libcroco/makefile.in	\
	libgdl/makefile.in	\
	libnrtype/makefile.in	\
	libuemf/makefile.in \
	livarot/makefile.in	\
	live_effects/makefile.in	\
	live_effects/parameter/makefile.in	\
	svg/makefile.in		\
	trace/makefile.in	\
	ui/cache/makefile.in	\
	ui/dialog/makefile.in	\
	ui/makefile.in		\
	ui/view/makefile.in	\
	ui/widget/makefile.in	\
	util/makefile.in	\
	util/makefile.in	\
	widgets/makefile.in	\
	xml/makefile.in		\
	\
	$(top_srcdir)/Doxyfile \
	extension/internal/emf-inout.cpp  \
	extension/internal/emf-inout.h    \
	extension/internal/emf-print.cpp  \
	extension/internal/emf-print.h    \
	helper/sp-marshal.list	\
	io/crystalegg.xml	\
	io/doc2html.xsl 	\
	show-preview.bmp \
	winconsole.cpp \
	libdepixelize/makefile.in	\
	$(CXXTEST_TEMPLATE)

# Extra files to remove when doing "make distclean"
DISTCLEANFILES =	\
	helper/sp-marshal.cpp	\
	helper/sp-marshal.h	\
	inkscape-version.cpp

# ################################################
#  B I N A R I E S
# ################################################

# this should speed up the build
#libinkscape_a_SOURCES = $(ink_common_sources)

inkscape_SOURCES += main.cpp $(ink_common_sources) $(win32_sources)
inkscape_LDADD = $(all_libs)
inkscape_LDFLAGS = $(kdeldflags) $(mwindows)

inkview_SOURCES += inkview.cpp $(ink_common_sources) $(win32_sources)
inkview_LDADD = $(all_libs)
inkview_LDFLAGS = $(mwindows)

# ################################################
#  VERSION REPORTING
# ################################################

libinkversion_a_SOURCES = inkscape-version.cpp inkscape-version.h

if USE_BZR_VERSION
inkscape_version_deps = $(top_srcdir)/.bzr/branch/last-revision
endif

# If this is an BZR snapshot build, regenerate this file every time
# someone updates the BZR working directory.
inkscape-version.cpp: $(inkscape_version_deps)
	VER_PREFIX="$(VERSION)";\
	VER_BZRREV=" r`bzr revno --tree $(top_srcdir)`"; \
	if test ! -z "`bzr status -S -V $(srcdir)`"; then \
	    VER_CUSTOM=" custom"; \
	fi; \
	VERSION="$$VER_PREFIX$$VER_BZRREV$$VER_CUSTOM"; \
	echo "namespace Inkscape { " \
	     "char const *version_string = \"$$VERSION\"; " \
	     "}" > inkscape-version.new.cpp; \
	if cmp -s inkscape-version.new.cpp inkscape-version.cpp; then \
	     rm inkscape-version.new.cpp; \
	else \
	     mv inkscape-version.new.cpp inkscape-version.cpp; \
	fi; \
	echo $$VERSION

# #################################
# ## TESTING STUFF (make check) ###
# #################################

# List of all programs that should be built before testing. Note that this is
# different from TESTS, because some tests can be scripts that don't
# need to be built. There should be one test program per directory.
# automake adds $(EXEEXT) to check_PROGRAMS items but not to TESTS items:
# TESTS items can be scripts etc.
check_PROGRAMS = cxxtests

# streamtest is unfinished and can't handle the relocations done during
# "make distcheck".

# List of all tests to be run.
TESTS = $(check_PROGRAMS)
check-local:
	$(top_srcdir)/share/extensions/test/run-all-extension-tests

# FIXME: Currently, a number of cxxtest tests fail.  These should be fixed and
#        the XFAIL_TESTS build target should be removed.
#        See the following Launchpad bugs:
#
#          LP #1208013 <cxxtest: curve-test.h fails>
#          LP #1208005 <cxxtest: svg-path-geom-test.h fails>
#          LP #1207502 <cxxtest: svg-affine-test.h fails>

XFAIL_TESTS = $(check_PROGRAMS)

# including the testsuites here ensures that they get distributed
cxxtests_SOURCES = cxxtests.cpp $(CXXTEST_TESTSUITES) $(ink_common_sources) $(win32_sources)
cxxtests_LDADD = $(all_libs)

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
