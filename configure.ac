dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])

# Always use 0.xx+devel instead of 0.xxdevel for the version, e.g. 0.46+devel.
# Rationale: (i) placate simple version comparison software such as
# `dpkg --compare-versions'.  (ii) We don't always know what the next
# version is going to be called until about the time we release it
# (whereas we always know what the previous version was called).
AC_INIT([inkscape], [0.91+devel],
        [http://bugs.launchpad.net/inkscape/+filebug],
        [inkscape],
        [http://inkscape.org/])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_HOST

# We need version 1.9 of Automake or higher since we no longer distribute the
# obsolete mkinstalldirs script
AM_INIT_AUTOMAKE([-Wall dist-zip dist-bzip2 tar-pax 1.9])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR]) dnl Workaround for Automake 1.12

AC_ARG_ENABLE([lsb], AS_HELP_STRING([--enable-lsb], [LSB-compatible build configuration]), [
  prefix=/opt/inkscape
  PATH="/opt/lsb/bin:$PATH"
  CC=lsbcc
  CXX=lsbc++
  export CC CXX
])

AC_LANG(C++)
AC_PROG_CXX
AC_PROG_CC
AM_PROG_AS
# Initialize libtool
LT_PREREQ([2.2])
LT_INIT
AC_HEADER_STDC
INK_BZR_SNAPSHOT_BUILD

dnl If automake 1.11 shave the output to look nice
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

dnl *********************************************************
dnl Configure a strict set of build rules to prevent usage of 
dnl deprecated features in external libraries and code that
dnl triggers compiler warnings.
dnl *********************************************************
AC_ARG_ENABLE(strict-build,
	      [AS_HELP_STRING([--enable-strict-build], [Enable strict build configuration [[default=yes]].  Usage of most deprecated symbols is forbidden by default.  Set the argument to "high" to introduce very strict checking that will cause the build to fail on many systems.])],
	      [enable_strict_build=$enableval],
	      [enable_strict_build=yes])

if test "x$enable_strict_build" = "xno"; then
	AC_MSG_WARN([Strict build options disabled])
elif test "x$enable_strict_build" = "xhigh"; then
	AC_MSG_WARN([Strictest build options enabled.  This will cause build failure on most systems])
else
	AC_MSG_RESULT([Strict build options enabled])
fi

dnl These next few lines are needed only while libcroco is in our source tree.
AM_PROG_CC_C_O
if test "$GCC" = "yes"; then
  # Enable some warnings from gcc.
  AC_LANG_PUSH(C)

  ####
  # Generic cpp flags...

  # What is just plain "-W" ?
  # Fortify source requires -O2 or higher, which is handled with newer autoconf
  CPPFLAGS="-W -D_FORTIFY_SOURCE=2 $CPPFLAGS"
  # Enable format and format security warnings
  CPPFLAGS="-Wformat -Wformat-security $CPPFLAGS"
  # Enable all default warnings
  CPPFLAGS="-Wall $CPPFLAGS"

  # Permit only top-level Glib headers to be used.
  # 
  # TODO: This is already used in Glib >= 2.32 so this flag can be 
  #       dropped when all targeted distros use higher Glib versions.
  CPPFLAGS="-DG_DISABLE_SINGLE_INCLUDES $CPPFLAGS"

  # Ensure that private GTK+ fields are not accessible. This strictly
  # enforced in Gtk+ 3, so it is important to check this in Gtk+ 2 builds
  CPPFLAGS="-DGSEAL_ENABLE $CPPFLAGS"

  # Unfortunately, we cannot (yet) build with -Werror, so we have to manually
  # change a ton of warnings into errors.
  # After some more work on fixing warning-inducing code, we can change this set into
  # it's complement and use -Wno-error=...
  # Test for -Werror=... (introduced some time post-4.0)
  AC_MSG_CHECKING([compiler support for -Werror=...])
  ink_svd_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="-Werror=format-security -Wswitch -Werror=return-type $CPPFLAGS"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])], [ink_opt_ok=yes], [ink_opt_ok=no])
  AC_MSG_RESULT([$ink_opt_ok])
  if test "x$ink_opt_ok" != "xyes"; then
    CPPFLAGS="$ink_svd_CPPFLAGS"
  fi

  # Uncomment for SVG2 stuff
  CPPFLAGS="-DWITH_MESH -DWITH_CSSBLEND -DWITH_CSSCOMPOSITE -DWITH_SVG2 $CPPFLAGS"

  # Uncomment for LPE Tool and All LPEs
  CPPFLAGS="-DWITH_LPETOOL -DLPE_ENABLE_TEST_EFFECTS $CPPFLAGS"

  ####
  # C-specific flags...

  # -Wno-pointer-sign is probably new in gcc 4.0; certainly it isn't accepted
  # by gcc 2.95.
  AC_MSG_CHECKING([compiler support for -Wno-pointer-sign])
  ink_svd_CFLAGS="$CFLAGS"
  CFLAGS="-Wno-pointer-sign $CFLAGS"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])], [ink_opt_ok=yes], [ink_opt_ok=no])
  AC_MSG_RESULT([$ink_opt_ok])
  if test "x$ink_opt_ok" != "xyes"; then
    CFLAGS="$ink_svd_CFLAGS"
  fi

  
  # This Automake variable is only used to suppress warnings in our internal
  # fork of GDL.  Once we get rid of our GDL fork, we can delete this test
  AC_MSG_CHECKING([compiler support for -Wno-unused-but-set-variable])
  ink_svd_CFLAGS="$CFLAGS"
  CFLAGS="-Wno-unused-but-set-variable $CFLAGS"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])], [ink_opt_ok=yes], [ink_opt_ok=no])
  AC_MSG_RESULT([$ink_opt_ok])
  CFLAGS="$ink_svd_CFLAGS"
  AM_CONDITIONAL(CC_WNO_UNUSED_BUT_SET_VARIABLE_SUPPORTED, test "$ink_opt_ok" = "yes")
  

  ####
  # Linker flags...

  # Have linker produce read-only relocations, if it knows how
  AC_MSG_CHECKING([linker tolerates -z relro])
  ink_svd_LDFLAGS="$LDFLAGS"
  LDFLAGS="-Wl,-z,relro $LDFLAGS"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([])], [ink_opt_ok=yes], [ink_opt_ok=no])
  AC_MSG_RESULT([$ink_opt_ok])
  if test "x$ink_opt_ok" != "xyes"; then
    LDFLAGS="$ink_svd_LDFLAGS"
  fi

  AC_LANG_POP

  # C++-specific flags are defined further below.  Look for CXXFLAGS...
fi

# Detect a working version of unordered containers.
# First try looking for native support (C++11 feature)
AC_MSG_CHECKING([native support for unordered_set])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <unordered_set>]],
[[
  std::unordered_set<int> i, j;
  i = j;
]])],
[native_unordered_set_works=yes], [native_unordered_set_works=no])

if test "x$native_unordered_set_works" = "xyes"; then
	AC_MSG_RESULT([ok])
	AC_DEFINE(HAVE_NATIVE_UNORDERED_SET, 1, [Has working native unordered_set])
else
	AC_MSG_RESULT([not working])
fi

AC_MSG_CHECKING([TR1 unordered_set usability])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <tr1/unordered_set>]],
[[
  std::tr1::unordered_set<int> i, j;
  i = j;
]])],
[tr1_unordered_set_works=yes], [tr1_unordered_set_works=no])

if test "x$tr1_unordered_set_works" = "xyes"; then
	AC_MSG_RESULT([ok])
	AC_DEFINE(HAVE_TR1_UNORDERED_SET, 1, [Has working standard TR1 unordered_set])
else
	AC_MSG_RESULT([not working])
fi

AC_CHECK_HEADER([boost/unordered_set.hpp], [AC_DEFINE(HAVE_BOOST_UNORDERED_SET, 1, [Boost unordered_set (Boost >= 1.36)])], [])

# Test whether GCC emits a spurious warning when using boost::optional
# If yes, turn off strict aliasing warnings to reduce noise
# and allow the legitimate warnings to stand out
AC_MSG_CHECKING([for overzealous strict aliasing warnings])
ignore_strict_aliasing=no
CXXFLAGS_SAVE=$CXXFLAGS
CXXFLAGS="$CXXFLAGS -Werror=strict-aliasing"
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include <boost/optional.hpp>
boost::optional<int> x;
int func() {
  return *x;
}
])], [ignore_strict_aliasing=no], [ignore_strict_aliasing=yes])
AC_MSG_RESULT($ignore_strict_aliasing)
CXXFLAGS=$CXXFLAGS_SAVE
if test "x$ignore_strict_aliasing" = "xyes"; then
  CXXFLAGS="$CXXFLAGS -Wno-strict-aliasing"
fi

dnl ******************************
dnl Gettext stuff
dnl ******************************
IT_PROG_INTLTOOL([0.40.0])

AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])

GETTEXT_PACKAGE="AC_PACKAGE_NAME"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",[Translation domain used])

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test "x$PKG_CONFIG" = "xno"; then
	AC_MSG_ERROR(You have to install pkg-config to compile inkscape.)
fi

dnl Find msgfmt.  Without this, po/Makefile fails to set MSGFMT on some platforms.
AC_PATH_PROG(MSGFMT, msgfmt, msgfmt)
AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)

dnl ******************************
dnl Check for OpenMP 
dnl ******************************
AC_OPENMP
if test "x$ac_cv_prog_cxx_openmp" != "xunsupported"; then
	openmp_ok=yes
	dnl We have it, now set up the flags
	CXXFLAGS="$CXXFLAGS $OPENMP_CXXFLAGS"
	AC_DEFINE(HAVE_OPENMP, 1, [Use OpenMP])
fi

dnl ******************************
dnl Check for libexif
dnl ******************************
PKG_CHECK_MODULES(EXIF, libexif, exif_ok=yes, exif_ok=no)
if test "x$exif_ok" = "xyes"; then
   AC_DEFINE(HAVE_EXIF, 1, [Use libexif])
fi
AC_SUBST(EXIF_LIBS)
AC_SUBST(EXIF_CFLAGS)

dnl ******************************
dnl Check for libjpeg
dnl ******************************
AC_CHECK_LIB(jpeg, jpeg_CreateDecompress, [AC_CHECK_HEADER(jpeglib.h, jpeg_ok=yes, jpeg_ok=no)], jpeg_ok=no)
if test "x$jpeg_ok" = "xyes"; then
   LIBS="-ljpeg $LIBS"
   AC_DEFINE(HAVE_JPEG, 1, [Use libjpeg])
fi

dnl This check is to get a FIONREAD definition on Solaris 8
AC_CHECK_HEADERS([sys/filio.h])


AC_CHECK_HEADERS([malloc.h])
AC_CHECK_FUNCS([mallinfo], [
	AC_CHECK_MEMBERS([struct mallinfo.usmblks,
			  struct mallinfo.fsmblks,
			  struct mallinfo.uordblks,
			  struct mallinfo.fordblks,
			  struct mallinfo.hblkhd],,,
			 [#include <malloc.h>])
])

AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, no)
if test "x$FREETYPE_CONFIG" = "xno"; then
	AC_MSG_ERROR([Cannot find freetype-config])
fi
FREETYPE_CFLAGS=`$FREETYPE_CONFIG --cflags`
FREETYPE_LIBS=`$FREETYPE_CONFIG --libs`
AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)

dnl ******************************
dnl Win32
dnl ******************************
AC_MSG_CHECKING([for Win32 platform])
case "$host" in
  *-*-mingw*)
    platform_win32=yes
    WIN32_CFLAGS="-mms-bitfields -DLIBXML_STATIC"
    ;;
  *)
    platform_win32=no
    ;;
esac
AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform_win32" = "yes")
AC_SUBST(WIN32_CFLAGS)

dnl TODO - switch to a linker/libtool/feature check, not OS check:
dnl ******************************
dnl MacOS X
dnl ******************************
AC_MSG_CHECKING([for OSX platform])
if test "x$build_vendor" = "xapple" ; then
  platform_osx=yes
else
  platform_osx=no
fi
AC_MSG_RESULT([$platform_osx])
	
AC_MSG_CHECKING([for Solaris platform])
case "$host" in
  *-solaris2.*)
    platform_solaris=yes
    solaris_version=`echo $host|sed -e 's/^.*-solaris2\.//' -e s'/\..*$//'`
    CFLAGS="$CFLAGS -DSOLARIS=$solaris_version"
    CXXFLAGS="$CXXFLAGS -DSOLARIS=$solaris_version"
    ;;
  *)
    platform_solaris=no
    ;;
esac
AC_MSG_RESULT([$platform_solaris])
AM_CONDITIONAL(PLATFORM_SOLARIS, test "$platform_solaris" = "yes")

dnl ******************************
dnl gnome vfs checking
dnl ******************************

AC_ARG_WITH(gnome-vfs,
	AS_HELP_STRING([--with-gnome-vfs],[use gnome vfs for loading files]),
	[with_gnome_vfs=$withval], [with_gnome_vfs=auto])

if test "x$with_gnome_vfs" = "xno"; then
	dnl Asked to ignore gnome-vfs
	gnome_vfs=no
else
        dnl Have to test gnome-vfs presence
	PKG_CHECK_MODULES(GNOME_VFS, gnome-vfs-2.0 >= 2.0, gnome_vfs=yes, gnome_vfs=no)
	if test "x$gnome_vfs" != "xyes"; then
	        dnl No gnome-vfs found
		if test "x$with_gnome_vfs" = "xyes"; then
		        dnl Gnome-VFS was explicitly asked for, so stop
			AC_MSG_ERROR([--with-gnome-vfs was specified, but appropriate libgnomevfs development packages could not be found])
		else
			# gnome-vfs is no, tell us for the log file
			AC_MSG_RESULT($gnome_vfs)
		fi
	fi
fi

AM_CONDITIONAL(USE_GNOME_VFS, test "x$gnome_vfs" = "xyes")
if test "x$gnome_vfs" = "xyes"; then
	AC_DEFINE(WITH_GNOME_VFS, 1, [Use gnome vfs file load functionality])
fi

AC_SUBST(GNOME_VFS_CFLAGS)
AC_SUBST(GNOME_VFS_LIBS)

dnl ******************************
dnl libinkjar checking
dnl ******************************

AC_ARG_WITH(inkjar,
	AS_HELP_STRING([--without-inkjar],[disable openoffice files (SVG jars)]),[with_ij=$withval], [with_ij=yes])

if test "x$with_ij" = "xyes"; then
	AC_DEFINE(WITH_INKJAR, 1, [enable openoffice files (SVG jars)])
	AC_C_BIGENDIAN
	AC_CHECK_HEADERS(zlib.h)
	ij=yes
else
	ij=no
fi
AM_CONDITIONAL(INKJAR, test "$with_ij" = "yes")

dnl ******************************
dnl LittleCms checking
dnl ******************************

AC_ARG_ENABLE(lcms,
	AS_HELP_STRING([--enable-lcms],[enable LittleCms for color management]),
	[enable_lcms=$enableval], [enable_lcms=yes])

if test "x$enable_lcms" = "xno"; then
    # Asked to ignore LittleCms
    lcms=no
    have_lcms2=no
else
    # Have to test LittleCms presence
    PKG_CHECK_MODULES(LCMS2, lcms2, have_lcms2="yes", have_lcms2="no")

    if test "x${have_lcms2}" = "xyes"; then
        LIBS="$LIBS $LCMS2_LIBS"
        AC_DEFINE(HAVE_LIBLCMS2, 1, [define to 1 if you have lcms version 2.x])
	AC_SUBST(LCMS2_CFLAGS)
	AC_SUBST(LCMS2_LIBS)
    else
        PKG_CHECK_MODULES(LCMS, lcms >= 1.13, lcms=yes, lcms=no)
        if test "x$lcms" = "xyes"; then
            LIBS="$LIBS $LCMS_LIBS"
            AC_DEFINE(HAVE_LIBLCMS1, 1, [define to 1 if you have lcms version 1.x])
	    AC_SUBST(LCMS_CFLAGS)
	    AC_SUBST(LCMS_LIBS)
        else
            # No lcms found. LittleCms was explicitly asked for, so stop
            AC_MSG_ERROR([--enable-lcms was specified, but appropriate LittleCms development packages could not be found])
        fi
    fi
fi

dnl ******************************
dnl Libpoppler checking
dnl ******************************

AC_ARG_ENABLE(poppler-cairo,
	AS_HELP_STRING([--enable-poppler-cairo],[Enable libpoppler-cairo for rendering PDF preview]),
	[enable_poppler_cairo=$enableval], [enable_poppler_cairo=yes])

POPPLER_CFLAGS=""
PKG_CHECK_MODULES(POPPLER, poppler >= 0.8.3, poppler=yes, poppler=no)

if test "x$poppler" = "xyes"; then
	dnl Working libpoppler
	dnl Have to test libpoppler-glib presence
	PKG_CHECK_MODULES(POPPLER_GLIB, poppler-glib >= 0.8.3, poppler_glib=yes, poppler_glib=no)
	if test "x$poppler_glib" = "xyes"; then
		dnl Working libpoppler-glib found
		dnl Check whether the Cairo SVG backend is available
		PKG_CHECK_MODULES(CAIRO_SVG, cairo-svg, cairo_svg=yes, cairo_svg=no)
		if test "x$cairo_svg" = "xyes"; then
			POPPLER_LIBS="$POPPLER_LIBS $POPPLER_GLIB_LIBS "
		fi
	fi
	if test "x$enable_poppler_cairo" = "xyes"; then
		dnl Have to test libpoppler-cairo presence for PDF preview
		dnl AC_CHECK_HEADER(Magick++.h, magick_ok=yes, magick_ok=no)
		PKG_CHECK_MODULES(POPPLER_CAIRO, poppler-cairo >= 0.8.3, poppler_cairo=yes, poppler_cairo=no)
		if test "x$poppler_glib" = "xyes" -a "x$poppler_cairo" = "xyes" -a \
			"x$cairo_svg" = "xno"
		then
			POPPLER_LIBS="$POPPLER_LIBS $POPPLER_CAIRO_LIBS "
		fi
	fi
fi

if test "x$poppler" = "xyes"; then
	LIBS="$LIBS $POPPLER_LIBS"
	AC_DEFINE(HAVE_POPPLER, 1, [Use libpoppler for direct PDF import])
fi
if test "x$poppler_cairo" = "xyes" -a "x$poppler_glib" = "xyes"; then
	AC_DEFINE(HAVE_POPPLER_CAIRO, 1, [Use libpoppler-cairo for rendering PDF preview])
fi
if test "x$poppler_glib" = "xyes" -a "x$cairo_svg" = "xyes"; then
	AC_DEFINE(HAVE_POPPLER_GLIB, 1, [Use libpoppler-glib and Cairo-SVG for PDF import])
fi
AC_SUBST(POPPLER_CFLAGS)
AC_SUBST(POPPLER_LIBS)

PKG_CHECK_MODULES(POPPLER_NEWERRORAPI, poppler >= 0.20.0, popplernewerror=yes, popplernewerror=no)
if test "x$popplernewerror" = "xyes"; then
	AC_DEFINE(POPPLER_NEW_ERRORAPI, 1, [Use new error API from Poppler >= 0.20.0])
fi


ink_svd_CPPFLAGS=$CPPFLAGS
ink_svd_LIBS=$LIBS
CPPFLAGS="$CPPFLAGS $POPPLER_CFLAGS"
LIBS="$LIBS $POPPLER_LIBS"

AC_MSG_CHECKING(for new color space API in Poppler)
popplercolor="no"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <GfxState.h>]],
[[
  typedef GfxColorSpace *(*parse_p)(Object *, Gfx *);
  parse_p p = &GfxColorSpace::parse;
]])],
[popplercolor=yes])

if test "x$popplercolor" = "xyes"; then
	AC_DEFINE(POPPLER_NEW_COLOR_SPACE_API, 1, [Use color space API from Poppler >= 0.12.2])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

PKG_CHECK_MODULES(POPPLER_EVEN_NEWER_COLOR_SPACE_API, poppler >= 0.26.0, popplernewercolorspaceapi=yes, popplernewercolorspaceapi=no)
if test "x$popplernewercolorspaceapi" = "xyes"; then
	AC_DEFINE(POPPLER_EVEN_NEWER_COLOR_SPACE_API, 1, [Use even newer color space API from Poppler >= 0.26.0])
fi

PKG_CHECK_MODULES(POPPLER_EVEN_NEWER_NEW_COLOR_SPACE_API, poppler >= 0.29.0, popplernewernewcolorspaceapi=yes, popplernewernewcolorspaceapi=no)
if test "x$popplernewernewcolorspaceapi" = "xyes"; then
	AC_DEFINE(POPPLER_EVEN_NEWER_NEW_COLOR_SPACE_API, 1, [Use even newer new color space API from Poppler >= 0.29.0])
fi

# Poppler's b604a008 commit changes this
AC_MSG_CHECKING([whether GfxPatch in Poppler no longer uses GfxColor])
popplergfxcolor="no"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
		  [[#include <GfxState.h>]],
[[
  GfxPatch::ColorValue color = {c: {0}};
  GfxPatch patch;
  patch.color[0][0] = color;
]])
], [popplergfxcolor=yes])
if test "x$popplergfxcolor" = "xyes"; then
	AC_DEFINE(POPPLER_NEW_GFXPATCH, 1, [GfxPatch no longer uses GfxColor in >= 0.15.1])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

CPPFLAGS=$ink_svd_CPPFLAGS
LIBS=$ink_svd_LIBS

dnl ******************************
dnl Check for libwpg for extension
dnl ******************************

AC_ARG_ENABLE(wpg,
       AS_HELP_STRING([--disable-wpg], [compile without support for WordPerfect Graphics]),
       enable_wpg=$enableval,enable_wpg=yes)

with_libwpg=no

if test "x$enable_wpg" = "xyes"; then
        dnl **************************************************************
        dnl Try using librevenge framework first. Fall back to old libs
        dnl if unavailable.
        dnl TODO: Drop subsequent tests once this is widespread in distros
        dnl **************************************************************
	PKG_CHECK_MODULES(LIBWPG03, libwpg-0.3 librevenge-0.0 librevenge-stream-0.0, with_libwpg03=yes, with_libwpg03=no)
	if test "x$with_libwpg03" = "xyes"; then
		AC_DEFINE(WITH_LIBWPG03,1,[Build using libwpg 0.3.x])
		with_libwpg=yes
		AC_SUBST(LIBWPG_LIBS, $LIBWPG03_LIBS)
		AC_SUBST(LIBWPG_CFLAGS, $LIBWPG03_CFLAGS)
	else
		PKG_CHECK_MODULES(LIBWPG02, libwpg-0.2 libwpd-0.9 libwpd-stream-0.9, with_libwpg02=yes, with_libwpg02=no)
		if test "x$with_libwpg02" = "xyes"; then
			AC_DEFINE(WITH_LIBWPG02,1,[Build using libwpg 0.2.x])
			with_libwpg=yes
			AC_SUBST(LIBWPG_LIBS, $LIBWPG02_LIBS)
			AC_SUBST(LIBWPG_CFLAGS, $LIBWPG02_CFLAGS)
		fi
	fi

	if test "x$with_libwpg" = "xyes"; then
		AC_DEFINE(WITH_LIBWPG,1,[Build in libwpg])
	fi
fi
AM_CONDITIONAL(WITH_LIBWPG03, test "x$with_libwpg03" = "xyes")
AM_CONDITIONAL(WITH_LIBWPG02, test "x$with_libwpg02" = "xyes")
AM_CONDITIONAL(WITH_LIBWPG, test "x$with_libwpg" = "xyes")

dnl ********************************
dnl Check for libvisio for extension
dnl ********************************

AC_ARG_ENABLE(visio,
       AS_HELP_STRING([--disable-visio], [compile without support for Microsoft Visio Diagrams]),
       enable_visio=$enableval,enable_visio=yes)

with_libvisio=no

if test "x$enable_visio" = "xyes"; then
        dnl **************************************************************
        dnl Try using librevenge framework first. Fall back to old libs
        dnl if unavailable.
        dnl TODO: Drop subsequent tests once this is widespread in distros
        dnl **************************************************************
	PKG_CHECK_MODULES(LIBVISIO01, libvisio-0.1 librevenge-0.0 librevenge-stream-0.0, with_libvisio01=yes, with_libvisio01=no)
	if test "x$with_libvisio01" = "xyes"; then
		AC_DEFINE(WITH_LIBVISIO01,1,[Build using libvisio 0.1.x])
		with_libvisio=yes
		AC_SUBST(LIBVISIO_LIBS, $LIBVISIO01_LIBS)
		AC_SUBST(LIBVISIO_CFLAGS, $LIBVISIO01_CFLAGS)
	else
		PKG_CHECK_MODULES(LIBVISIO00, libvisio-0.0 >= 0.0.20 libwpd-0.9 libwpd-stream-0.9 libwpg-0.2, with_libvisio00=yes, with_libvisio00=no)
		if test "x$with_libvisio00" = "xyes"; then
			AC_DEFINE(WITH_LIBVISIO00,1,[Build using libvisio 0.0.x])
			with_libvisio=yes
			AC_SUBST(LIBVISIO_LIBS, $LIBVISIO00_LIBS)
			AC_SUBST(LIBVISIO_CFLAGS, $LIBVISIO00_CFLAGS)
		fi
	fi

	if test "x$with_libvisio" = "xyes"; then
		AC_DEFINE(WITH_LIBVISIO,1,[Build in libvisio])
	fi
fi
AM_CONDITIONAL(WITH_LIBVISIO01, test "x$with_libvisio01" = "xyes")
AM_CONDITIONAL(WITH_LIBVISIO00, test "x$with_libvisio00" = "xyes")
AM_CONDITIONAL(WITH_LIBVISIO, test "x$with_libvisio" = "xyes")

dnl ********************************
dnl Check for libcdr for extension
dnl ********************************

AC_ARG_ENABLE(cdr,
       AS_HELP_STRING([--disable-cdr], [compile without support for CorelDRAW Diagrams]),
       enable_cdr=$enableval,enable_cdr=yes)

with_libcdr=no

if test "x$enable_cdr" = "xyes"; then
        dnl **************************************************************
        dnl Try using librevenge framework first. Fall back to old libs
        dnl if unavailable.
        dnl TODO: Drop subsequent tests once this is widespread in distros
        dnl **************************************************************
	PKG_CHECK_MODULES(LIBCDR01, libcdr-0.1 librevenge-0.0 librevenge-stream-0.0, with_libcdr01=yes, with_libcdr01=no)
	if test "x$with_libcdr01" = "xyes"; then
		AC_DEFINE(WITH_LIBCDR01,1,[Build using libcdr 0.1.x])
		with_libcdr=yes
		AC_SUBST(LIBCDR_LIBS, $LIBCDR01_LIBS)
		AC_SUBST(LIBCDR_CFLAGS, $LIBCDR01_CFLAGS)
	else
		PKG_CHECK_MODULES(LIBCDR00, libcdr-0.0 >= 0.0.3 libwpd-0.9 libwpd-stream-0.9 libwpg-0.2, with_libcdr00=yes, with_libcdr00=no)
		if test "x$with_libcdr00" = "xyes"; then
			AC_DEFINE(WITH_LIBCDR00,1,[Build using libcdr 0.0.x])
			with_libcdr=yes
			AC_SUBST(LIBCDR_LIBS, $LIBCDR00_LIBS)
			AC_SUBST(LIBCDR_CFLAGS, $LIBCDR00_CFLAGS)
		fi
	fi

	if test "x$with_libcdr" = "xyes"; then
		AC_DEFINE(WITH_LIBCDR,1,[Build in libcdr])
	fi
fi
AM_CONDITIONAL(WITH_LIBCDR01, test "x$with_libcdr01" = "xyes")
AM_CONDITIONAL(WITH_LIBCDR00, test "x$with_libcdr00" = "xyes")
AM_CONDITIONAL(WITH_LIBCDR, test "x$with_libcdr" = "xyes")

dnl ******************************
dnl Support doing a local install
dnl   (mostly for distcheck)
dnl ******************************

with_localinstall="no"
AC_ARG_ENABLE(localinstall, AS_HELP_STRING([--enable-localinstall], [install system files in the local path (for distcheck)]), with_localinstall=$enableval, with_localinstall=no)

dnl ******************************
dnl Check for dbus functionality
dnl ******************************

AC_ARG_ENABLE(dbusapi,
       AS_HELP_STRING([--enable-dbusapi], [compile with support for DBus interface]),
       enable_dbusapi=$enableval,enable_dbusapi=no)

with_dbus="no"
if test "x$enable_dbusapi" = "xyes"; then
	PKG_CHECK_MODULES(DBUS, dbus-glib-1, with_dbus=yes, with_dbus=no)
	if test "x$with_dbus" = "xyes"; then
		if test "x$with_localinstall" = "xyes"; then
			DBUSSERVICEDIR="${datadir}/dbus-1/services/"
		else
			DBUSSERVICEDIR=`$PKG_CONFIG --variable=session_bus_services_dir dbus-1`
		fi
		AC_SUBST(DBUSSERVICEDIR)
		AC_DEFINE(WITH_DBUS,1,[Build in dbus])
	fi
fi
AC_SUBST(DBUS_LIBS)
AC_SUBST(DBUS_CFLAGS)
AM_CONDITIONAL(WITH_DBUS, test "x$with_dbus" = "xyes")

dnl ******************************
dnl Check for ImageMagick Magick++ 
dnl ******************************

PKG_CHECK_MODULES(IMAGEMAGICK, ImageMagick++, magick_ok=yes, magick_ok=no)
if test "x$magick_ok" = "xyes"; then
      AC_DEFINE(WITH_IMAGE_MAGICK,1,[Image Magick++ support for bitmap effects])
fi
AM_CONDITIONAL(USE_IMAGE_MAGICK, test "x$magick_ok" = "xyes")

AC_SUBST(IMAGEMAGICK_LIBS)
AC_SUBST(IMAGEMAGICK_CFLAGS)

dnl ******************************
dnl   Unconditional dependencies
dnl ******************************
PKG_CHECK_MODULES(INKSCAPE,
		  bdw-gc      >= 7.1
		  cairo       >= 1.10
		  cairomm-1.0 >= 1.9.8
		  glib-2.0    >= 2.28
		  glibmm-2.4  >= 2.28
		  giomm-2.4
		  gsl
		  gthread-2.0 >= 2.0
		  libpng      >= 1.2
		  libxml-2.0  >= 2.6.11
		  libxslt     >= 1.0.15
		  pango       >= 1.24
		  pangoft2    >= 1.24
		  sigc++-2.0  >= 2.0.12
		  )

dnl Check whether we need to use new Glib::Thread::Mutex API.
dnl TODO: Drop this test once we bump the required version of glibmm to 2.32
PKG_CHECK_MODULES(GLIBMM_2_32, glibmm-2.4 >= 2.32, with_glibmm_2_32=yes, with_glibmm_2_32=no)

if test "x$with_glibmm_2_32" = "xyes"; then
	AC_DEFINE(WITH_GLIBMM_2_32,1,[Build with Glibmm 2.32 or higher])
fi

dnl *********************************
dnl   Allow experimental GTK+3 build
dnl *********************************
AC_ARG_ENABLE(gtk3-experimental,
	AS_HELP_STRING([--enable-gtk3-experimental], [enable compilation with GTK+3 (EXPERIMENTAL!)]),
	[enable_gtk3=$enableval], [enable_gtk3=no])

with_gtkmm_3_0="no"
if test "x$enable_gtk3" = "xyes"; then

	ink_spell_pkg=
	if pkg-config --exists gtkspell-3.0; then
		ink_spell_pkg=gtkspell-3.0
		AC_DEFINE(WITH_GTKSPELL, 1, [enable gtk spelling widget])
	fi

	PKG_CHECK_MODULES(GTK,
			  gtkmm-3.0 >= 3.2
			  gdkmm-3.0 >= 3.2
			  gtk+-3.0  >= 3.2
			  gdk-3.0   >= 3.2
			  gdl-3.0   > 3.3.4
			  $ink_spell_pkg,
			  with_gtkmm_3_0=yes,
			  with_gtkmm_3_0=no)

	if test "x$with_gtkmm_3_0" = "xyes"; then
		AC_MSG_RESULT([Using EXPERIMENTAL Gtkmm 3 build])
		AC_DEFINE(WITH_GTKMM_3_0,1,[Build with Gtkmm 3.0.x or higher])
		
		AC_MSG_RESULT([Using external GDL])
		AC_DEFINE(WITH_EXT_GDL,1,[Build with external GDL])
	else
		AC_MSG_ERROR([Some dependencies were not fulfilled for the experimental GTK+ 3 build. One possible cause for this is a new dependency on the gdl-3.0 development package.])
	fi

	# Check whether we can use new features in Gtkmm 3.10
	# TODO: Drop this test and bump the version number in the GTK test above
	#       as soon as all supported distributions provide Gtkmm >= 3.10
	PKG_CHECK_MODULES(GTKMM_3_10,
			  gtkmm-3.0 >= 3.10,
			  with_gtkmm_3_10=yes,
			  with_gtkmm_3_10=no)

	if test "x$with_gtkmm_3_10" = "xyes"; then
		AC_MSG_RESULT([Using Gtkmm 3.10 build])
		AC_DEFINE(WITH_GTKMM_3_10,1,[Build with Gtkmm 3.10.x or higher])
	fi

	# Check whether we are using Gdl >= 3.6.  This version introduced an API/ABI change.
	# TODO: We should drop support for older versions of Gdl once all supported distros
	#       provide Gdl 3.6 or higher
	PKG_CHECK_MODULES(GDL_3_6, gdl-3.0 >= 3.6, with_gdl_3_6=yes, with_gdl_3_6=no)

	if test "x$with_gdl_3_6" = "xyes"; then
		AC_MSG_RESULT([Using Gdl 3.6 or higher])
		AC_DEFINE(WITH_GDL_3_6,1,[Build with Gdl 3.6 or higher])
	fi

	dnl The following test is only defined if Gtk+ 3 development libraries
	dnl are installed on the system.  Therefore, it is guarded by an 
	dnl m4_ifdef statement.  The ifdef can be probably be removed once we 
	dnl switch to Gtk+ 3 as a hard dependency

	# Check whether we are using the X11 backend target for Gtk+ 3.
	m4_ifdef([GTK_CHECK_BACKEND],
		 [GTK_CHECK_BACKEND([x11], , [have_x11=yes], [have_x11=no])])

	# Enable strict build options that should work on most systems unless
	# the build has been configured not to do so
	if test "x$enable_strict_build" != "xno"; then
		# Add build flags here as soon as Inkscape trunk can build
		# against Gtk+ 3 with the option enabled
		echo ""
	fi

	# Enable strict build options that are known to cause failure in
	# Gtk+ 3 builds
	if test "x$enable_strict_build" = "xhigh"; then
		# Disable deprecated Gtk+ symbols that have been removed since
		# Gtk+ 3.
		CPPFLAGS="-DGTKMM_DISABLE_DEPRECATED $CPPFLAGS"
		CPPFLAGS="-DGTK_DISABLE_DEPRECATED $CPPFLAGS"
		CPPFLAGS="-DGDK_DISABLE_DEPRECATED $CPPFLAGS"
	fi
else

	ink_spell_pkg=
	if pkg-config --exists gtkspell-2.0; then
		ink_spell_pkg=gtkspell-2.0
		AC_DEFINE(WITH_GTKSPELL, 1, [enable gtk spelling widget])
	fi

	PKG_CHECK_MODULES(GTK,
			  gtk+-2.0  >= 2.24
			  $ink_spell_pkg
			  gdkmm-2.4 >= 2.24
			  gtkmm-2.4 >= 2.24)

	# Check whether we are using the X11 backend for Gtk+ 2.
	AC_MSG_CHECKING([if Gtk+ 2.0 is using the X11 backend target])
	if test "x`$PKG_CONFIG --variable=target gtk+-2.0`" = "xx11"; then
		have_x11=yes
	else
		have_x11=no
	fi
	AC_MSG_RESULT($have_x11)
	
	# Enable build strict options that should work on most systems unless
	# the build has been configured explicitly not to do so
	if test "x$enable_strict_build" != "xno"; then
		# Prevent usage of deprecated Gtk+ symbols.  These have all
		# been removed in Gtk+ 3 so these checks are important.
		CPPFLAGS="-DGTKMM_DISABLE_DEPRECATED $CPPFLAGS"
		CPPFLAGS="-DGTK_DISABLE_DEPRECATED $CPPFLAGS"
		
		# Allow only top-level GTK+ headers to be used.  This is mandatory
		# for GTK+ >= 3.0 so there is no need to apply the flag in GTK+ 3
		# builds.
		CPPFLAGS="-DGTK_DISABLE_SINGLE_INCLUDES $CPPFLAGS"
	fi

	# Optionally enable strict build options that are known to cause build
	# failure in many/most systems
	if test "x$enable_strict_build" == "xhigh"; then
		# FIXME: This causes build failure because our internal
		# copy of GDL uses deprecated GDK symbols.
		# 
		# This specific issue isn't a problem for GTK+ 3 builds because 
		# we build against external GDL >= 3.3.4 rather than using
		# the deprecated internal code
		CPPFLAGS="-DGDK_DISABLE_DEPRECATED $CPPFLAGS"
	fi
fi

INKSCAPE_CFLAGS="$GTK_CFLAGS $INKSCAPE_CFLAGS"
INKSCAPE_LIBS="$GTK_LIBS $INKSCAPE_LIBS"

dnl Configure x11 library if Gtk+ uses it as a backend.
dnl Note that this is only here because we directly use X11 functionality.  We
dnl wouldn't need this check if we were only using X as the Gtk+ backend.
if test "x$have_x11" = "xyes"; then
	PKG_CHECK_MODULES(X11, x11)
fi
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

AM_CONDITIONAL(WITH_EXT_GDL, test "x$with_gtkmm_3_0" = "xyes")

# Prevent usage of deprecated library symbols unless strict build
# checking has been disabled
if test "x$enable_strict_build" != "xno"; then
	CPPFLAGS="-DGDKMM_DISABLE_DEPRECATED $CPPFLAGS"
	
	# Ensure that no deprecated glibmm symbols are introduced.
	# lp:inkscape builds cleanly with this option at r10957
	CPPFLAGS="-DGLIBMM_DISABLE_DEPRECATED $CPPFLAGS"
	
	dnl Pango 1.32.4 uses a deprecated Glib symbol:
	dnl   https://bugzilla.gnome.org/show_bug.cgi?id=689843
	dnl 
	dnl TODO: Get rid of this check once we are sure that all targeted
	dnl       platforms have got rid of this Pango version.  Apply the
	dnl       G_DISABLE_DEPRECATED flag to all builds.
	pango_uses_deprecated_glib_symbols=no

	PKG_CHECK_MODULES(PANGO_USES_DEPRECATED_GLIB_SYMBOLS,
			  pango = 1.32.4,
			  pango_uses_deprecated_glib_symbols=yes,
			  pango_uses_deprecated_glib_symbols=no)

	# Don't disable deprecated Glib symbols if it will break stuff in an
	# external library header that we use
	if test "x$pango_uses_deprecated_glib_symbols" = "xyes"; then
		AC_MSG_WARN([The available version of Pango uses deprecated Glib symbols.  Deprecated Glib symbol usage will be allowed])
	else
		CPPFLAGS="-DG_DISABLE_DEPRECATED $CPPFLAGS"
	fi
fi

# Disable all deprecated symbols if very strict build is requested.
# TODO: Make this a default option for Gtk+ 2 or 3 builds as soon as
#       possible.
if test "x$enable_strict_build" = "xhigh"; then
        CPPFLAGS="-Werror=deprecated-declarations $CPPFLAGS"
fi

# Check for Apple Mac OS X Carbon framework
carbon_ok=no
AC_MSG_CHECKING([for Mac OS X Carbon support])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>]], [])
], [carbon_ok=yes])
AC_MSG_RESULT($carbon_ok)
if test "x$carbon_ok" = "xyes"; then
  AC_DEFINE(HAVE_CARBON, 1, [define to 1 if Carbon is available])
  CARBON_LDFLAGS="-framework Carbon"
  AC_SUBST(CARBON_LDFLAGS)
fi
AM_CONDITIONAL(HAVE_CARBON, test "x$carbon_ok" = "xyes")

# Check for some boost header files
AC_CHECK_HEADERS([boost/concept_check.hpp], [], AC_MSG_ERROR([You need the boost package (e.g. libboost-dev)]))

PKG_CHECK_MODULES(CAIRO_PDF, cairo-pdf, cairo_pdf=yes, cairo_pdf=no)
if test "x$cairo_pdf" = "xyes"; then
  AC_DEFINE(HAVE_CAIRO_PDF, 1, [Whether the Cairo PDF backend is available])
fi

dnl Shouldn't we test for libz?
INKSCAPE_LIBS="$INKSCAPE_LIBS -lz"
if test "x$openmp_ok" = "xyes"; then
	INKSCAPE_LIBS="$INKSCAPE_LIBS -lgomp"
fi

AC_CHECK_HEADER(popt.h,
		[INKSCAPE_LIBS="$INKSCAPE_LIBS -lpopt"],
		AC_MSG_ERROR([libpopt is required]))

dnl **************************
dnl Check for aspell 
dnl ******************************
AC_CHECK_LIB(aspell, new_aspell_config, [AC_CHECK_HEADER(aspell.h, aspell_ok=yes, aspell_ok=no)], aspell_ok=no, -lz -lm)
if test "x$aspell_ok" = "xyes"; then
	AC_DEFINE(HAVE_ASPELL, 1, [Use aspell for built-in spellchecker])
  INKSCAPE_LIBS="$INKSCAPE_LIBS -laspell"
else
	AC_MSG_CHECKING([Aspell not found, spell checker will be disabled])
fi

dnl Check for bind_textdomain_codeset, including -lintl if GLib brings it in.
sp_save_LIBS=$LIBS
LIBS="$LIBS $INKSCAPE_LIBS"
AC_CHECK_FUNCS(bind_textdomain_codeset)
dnl Check for gtk_window_fullscreen in gtk (>= 2.2)
AC_CHECK_FUNCS(gtk_window_set_default_icon_from_file)
AC_CHECK_FUNCS(gtk_window_fullscreen)
LIBS=$sp_save_LIBS


dnl Check for binary relocation support
dnl Hongli Lai <h.lai@chello.nl>

AC_ARG_ENABLE(binreloc,
       AS_HELP_STRING([--enable-binreloc], [compile with binary relocation support]),
       enable_binreloc=$enableval,enable_binreloc=no)

AC_MSG_CHECKING(whether binary relocation support should be enabled)
if test "$enable_binreloc" = "yes"; then
       AC_MSG_RESULT(yes)
       AC_MSG_CHECKING(for linker mappings at /proc/self/maps)
       if test -e /proc/self/maps; then
               AC_MSG_RESULT(yes)
       else
               AC_MSG_RESULT(no)
               AC_MSG_ERROR(/proc/self/maps is not available. Binary relocation cannot be enabled.)
               enable_binreloc="no"
       fi

elif test "$enable_binreloc" = "auto"; then
       AC_MSG_RESULT(yes when available)
       AC_MSG_CHECKING(for linker mappings at /proc/self/maps)
       if test -e /proc/self/maps; then
               AC_MSG_RESULT(yes)
               enable_binreloc=yes

               AC_MSG_CHECKING(whether everything is installed to the same prefix)
               if test "$bindir" = '${exec_prefix}/bin' -a "$sbindir" = '${exec_prefix}/sbin' -a \
                       "$datadir" = '${prefix}/share' -a "$libdir" = '${exec_prefix}/lib' -a \
                       "$libexecdir" = '${exec_prefix}/libexec' -a "$sysconfdir" = '${prefix}/etc'
               then
                       AC_MSG_RESULT(yes)
               else
                       AC_MSG_RESULT(no)
                       AC_MSG_NOTICE(Binary relocation support will be disabled.)
                       enable_binreloc=no
               fi

       else
               AC_MSG_RESULT(no)
               enable_binreloc=no
       fi

elif test "$enable_binreloc" = "no"; then
       AC_MSG_RESULT(no)
else
       AC_MSG_RESULT([no (unknown value "$enable_binreloc")])
       enable_binreloc=no
fi
AC_DEFINE(BR_PTHREADS,[0],[Use binreloc thread support?])
if test "$enable_binreloc" = "yes"; then
   AC_DEFINE(ENABLE_BINRELOC,,[Use AutoPackage?])
fi

AC_ARG_ENABLE(osxapp,
       AS_HELP_STRING([--enable-osxapp], [compile with OSX .app data dir paths]),
       enable_osxapp=$enableval,enable_osxapp=no)
if test "$enable_osxapp" = "yes"; then
   AC_DEFINE(ENABLE_OSX_APP_LOCATIONS,,[Build with OSX .app data dir paths?])
   LDFLAGS="$LDFLAGS -headerpad_max_install_names"
fi

dnl ******************************
dnl   Reported by autoscan
dnl ******************************
AC_CHECK_FUNCS(pow)
# if we did not find pow(), see if it's in libm.
if test x"$ac_cv_func_pow" = x"no" ; then
	AC_CHECK_LIB(m,pow)
fi
AC_CHECK_FUNCS(sqrt)
AC_CHECK_FUNCS(floor)
AC_CHECK_FUNCS(gettimeofday)
AC_CHECK_FUNCS(memmove)
AC_CHECK_FUNCS(memset)
AC_CHECK_FUNCS(mkdir)
AC_CHECK_FUNCS(strncasecmp)
AC_CHECK_FUNCS(strpbrk)
AC_CHECK_FUNCS(strrchr)
AC_CHECK_FUNCS(strspn)
AC_CHECK_FUNCS(strstr)
AC_CHECK_FUNCS(strtoul)
AC_CHECK_FUNCS(fpsetmask)
AC_CHECK_FUNCS(ecvt)
AC_CHECK_HEADERS(ieeefp.h)
AC_CHECK_HEADERS(fcntl.h)
AC_CHECK_HEADERS(libintl.h)
AC_CHECK_HEADERS(stddef.h)
AC_CHECK_HEADERS(sys/time.h)
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_HEADER_STAT
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_MODE_T

dnl Work around broken gcc 3.3 (seen on OSX) where "ENABLE_NLS" isn't
dnl set correctly because the gettext function isn't noticed.
if test "$ac_cv_header_libintl_h" = "yes" &&
   test "$ac_cv_func_bind_textdomain_codeset" = "yes"  &&
   test "$gt_cv_func_have_gettext" != "yes"; then
    AC_DEFINE([ENABLE_NLS], [], [Description])
fi

dnl ******************************
dnl   Compilation warnings
dnl ******************************
if test "$GXX" = "yes"; then
  # Enable some warnings from g++.

  # Rationale: a number of bugs in inkscape have been fixed by enabling g++
  # warnings and addressing the produced warnings.  Usually the committing
  # developer is the best person to address the warnings.

  ink_svd_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="-Wno-unused-parameter $CXXFLAGS"
  # -Wno-unused-parameter isn't accepted by gcc 2.95.
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([int dummy;])
], , CXXFLAGS="-Wno-unused $ink_svd_CXXFLAGS",)
  # Note: At least one bug has been caught from unused parameter warnings,
  # so it might be worth trying not to disable it.
  # One way of selectively disabling the warnings (i.e. only where the
  # programmer deliberately isn't using the parameter, e.g. for a callback)
  # is to remove the parameter name (leaving just its type), as is done
  # in src/seltrans.cpp:sp_seltrans_handle_event; this indicates that the
  # programmer deliberately has an unused parameter (e.g. because it's used
  # as a callback or similar function pointer use).

  # Add even more stuff
  CXXFLAGS="-Wpointer-arith -Wcast-align -Wsign-compare -Woverloaded-virtual -Wswitch $CXXFLAGS"

fi

dnl ******************************
dnl   libinkscape
dnl ******************************
dnl
dnl AC_ARG_ENABLE(libinkscape, AS_HELP_STRING([--enable-libinkscape],[Compile dynamic library (experimental)]), [splib=$enableval], [splib=no])
dnl
dnl AM_CONDITIONAL(ENABLE_LIBINKSCAPE, test "x$splib" != "xno")
dnl

AC_SUBST(INKSCAPE_CFLAGS)
AC_SUBST(INKSCAPE_LIBS)


dnl Check to see if we have a newer (2.32.0+) glibmm
TMP_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$GTK_CFLAGS $CPPFLAGS"
AC_CHECK_HEADERS([glibmm/threads.h])
CPPFLAGS="$TMP_CPPFLAGS"


dnl Define our data paths for config.h
AC_DEFINE_DIR([INKSCAPE_DATADIR], [datadir], [Base data directory])
AC_DEFINE_DIR([PACKAGE_LOCALE_DIR], [localedir], [Locatization directory])

AC_CONFIG_FILES([
Makefile
src/Makefile
src/check-header-compile
src/debug/makefile
src/display/makefile
src/extension/implementation/makefile
src/extension/internal/makefile
src/extension/makefile
src/extension/dbus/wrapper/inkdbus.pc
src/filters/makefile
src/helper/makefile
src/io/makefile
src/libcroco/makefile
src/libgdl/makefile
src/libnrtype/makefile
src/libavoid/makefile
src/libuemf/makefile
src/livarot/makefile
src/live_effects/makefile
src/live_effects/parameter/makefile
src/svg/makefile
src/trace/makefile
src/ui/cache/makefile
src/ui/dialog/makefile
src/ui/makefile
src/ui/view/makefile
src/ui/widget/makefile
src/util/makefile
src/widgets/makefile
src/xml/makefile
src/2geom/makefile
doc/Makefile
po/Makefile.in
share/Makefile
share/attributes/Makefile
share/branding/Makefile
share/examples/Makefile
share/extensions/Makefile
share/extensions/alphabet_soup/Makefile
share/extensions/Barcode/Makefile
share/extensions/Poly3DObjects/Makefile
share/extensions/scour/Makefile
share/extensions/test/Makefile
share/extensions/xaml2svg/Makefile
share/extensions/ink2canvas/Makefile
share/filters/Makefile
share/fonts/Makefile
share/gradients/Makefile
share/icons/Makefile
share/icons/application/Makefile
share/icons/application/16x16/Makefile
share/icons/application/22x22/Makefile
share/icons/application/24x24/Makefile
share/icons/application/32x32/Makefile
share/icons/application/48x48/Makefile
share/icons/application/256x256/Makefile
share/keys/Makefile
share/markers/Makefile
share/palettes/Makefile
share/patterns/Makefile
share/screens/Makefile
share/symbols/Makefile
share/templates/Makefile
share/tutorials/Makefile
share/ui/Makefile
packaging/autopackage/default.apspec
inkscape.spec
Info.plist
inkview.1
])

AH_BOTTOM([


])

AC_OUTPUT

echo "
Configuration:

        Source code location:     ${srcdir}
        Destination path prefix:  ${prefix}
        Compiler:                 ${CXX}
        CPPFLAGS:                 ${CPPFLAGS}
        CXXFLAGS:                 ${CXXFLAGS}
        CFLAGS:                   ${CFLAGS}
        LDFLAGS:                  ${LDFLAGS}

        Use gnome-vfs:            ${gnome_vfs}
        Use openoffice files:     ${ij}
        Use relocation support:   ${enable_binreloc}
        Enable LittleCms:         ${enable_lcms}
        Enable DBUS:              ${with_dbus} 
        Enable Poppler-Cairo:     ${enable_poppler_cairo}
        ImageMagick Magick++:     ${magick_ok}
        Libwpg:                   ${with_libwpg}
        Libvisio:                 ${with_libvisio}
        Libcdr:                   ${with_libcdr}
        Doing Local Install:      ${with_localinstall}
"
