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
if WITH_INKBOARD
libpedro = pedro/libpedro.a
endif
noinst_LIBRARIES =	\
	libinkscape.a		\
	dom/libdom.a		\
	libcroco/libcroco.a	\
	libavoid/libavoid.a	\
	libgdl/libgdl.a		\
	libcola/libcola.a	\
	libvpsc/libvpsc.a	\
	livarot/libvarot.a	\
	2geom/lib2geom.a	\
	$(libpedro)		\
	libinkversion.a

all_libs =			\
	$(noinst_LIBRARIES)	\
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

# Add sources common for Inkscape and Inkview to this variable.
ink_common_sources =
# Add Inkscape-only sources here.
inkscape_SOURCES =
# Add Inkview-only sources here.
inkview_SOURCES =

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
	$(WIN32_CFLAGS) \
	-I$(srcdir)/bind/javainc \
	-I$(srcdir)/bind/javainc/linux \
	$(AM_CPPFLAGS)

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
	traits/copy.h	\
	traits/function.h	\
	traits/list-copy.h	\
	traits/reference.h \
	$(jabber_whiteboard_SOURCES)

# Extra files to remove when doing "make distclean"
DISTCLEANFILES =	\
	helper/sp-marshal.cpp	\
	helper/sp-marshal.h	\
	inkscape-version.cpp

# ################################################
#  B I N A R I E S
# ################################################

# this should speed up the build
libinkscape_a_SOURCES = $(ink_common_sources)

inkscape_SOURCES += main.cpp $(win32_sources)
inkscape_LDADD = $(all_libs)
inkscape_LDFLAGS = --export-dynamic $(kdeldflags)

inkview_SOURCES += inkview.cpp $(win32_sources)
inkview_LDADD = $(all_libs)

# ################################################
#  VERSION REPORTING
# ################################################

libinkversion_a_SOURCES = inkscape-version.cpp inkscape-version.h

if USE_SVN_VERSION
inkscape_version_deps = $(top_srcdir)/.svn/entries
endif

# If this is an SVN snapshot build, regenerate this file every time
# someone updates the SVN working directory.
inkscape-version.cpp: $(inkscape_version_deps)
	VER_PREFIX="$(VERSION)"; \
	if test -x "$(srcdir)/.svn" -a ! -z `which svn`; then \
	    VER_SVNREV=" r`svn info $(srcdir) | sed -n -e '/^Revision:/s/Revision: \(.*\)/\1/p'`"; \
	    if test ! -z "`svn status -q $(srcdir)`"; then \
	         VER_CUSTOM=" custom"; \
	    fi; \
	fi; \
	VERSION="$$VER_PREFIX$$VER_SVNREV$$VER_CUSTOM"; \
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

# including the the testsuites here ensures that they get distributed
cxxtests_SOURCES = cxxtests.cpp $(CXXTEST_TESTSUITES)
cxxtests_LDADD = libnr/nr-compose-reference.o $(ink_common_sources)

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
