dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)
AC_INIT(inkscape, 0.43+devel)
dnl N.B. After 0.40, please change to `0.40+cvs' instead of `0.41cvs'.
dnl Rationale: (i) placate simple version comparison software such as
dnl `dpkg --compare-versions'.  (ii) We don't always know what the next
dnl version is going to be called until about the time we release it
dnl (whereas we always know what the previous version was called).
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR(src/main.cpp)
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(config.h)

AC_PROG_INTLTOOL(0.22)

dnl These next few lines are needed only while libcroco is in our source tree.
AC_PROG_CC
AM_PROG_CC_C_O
if test "$GCC" = "yes"; then
  # Enable some warnings from gcc.

  # -Wno-pointer-sign is probably new in gcc 4.0; certainly it isn't accepted
  # by gcc 2.95.
  ink_svd_CFLAGS="$CFLAGS"
  CFLAGS="-Wno-pointer-sign $CFLAGS"
  AC_COMPILE_IFELSE([int dummy;
], , CFLAGS="$ink_svd_CFLAGS",)
  CFLAGS="-Wall -W $CFLAGS"
fi

AC_LANG(C++)
AC_ISC_POSIX
AC_PROG_CXX
AM_PROG_CC_STDC
AM_PROG_AS
AC_HEADER_STDC

dnl Honor aclocal flags
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

dnl RANLIB is outdated now
dnl AC_PROG_RANLIB
AM_PROG_LIBTOOL

dnl Verify our GCC version
if test "x$GXX" = "xyes"; then
	AC_MSG_CHECKING([GNU compiler version])

	cc_version=["`$CXX $CXXFLAGS -v 2>&1 </dev/null |grep 'gcc version' |\
		sed 's/.*gcc version \([-a-z0-9\.]*\).*/\1/'`"]

	AC_MSG_RESULT([$cc_version])

	# Some version numbers
	cc_vers_major=`echo $cc_version | cut -f1 -d.`
	cc_vers_minor=`echo $cc_version | cut -f2 -d.`
	cc_vers_patch=`echo $cc_version | cut -f3 -d.`
	test -n "$cc_vers_major" || cc_vers_major=0
	test -n "$cc_vers_minor" || cc_vers_minor=0
	test -n "$cc_vers_patch" || cc_vers_patch=0
	cc_vers_all=`expr $cc_vers_major '*' 1000000 + $cc_vers_minor '*' 1000 + $cc_vers_patch`

	if test $cc_vers_major -lt 3; then
		AC_MSG_ERROR([gcc >= 3.0 is needed to compile inkscape])
	fi
fi

dnl ******************************
dnl Gettext stuff
dnl ******************************
GETTEXT_PACKAGE="AC_PACKAGE_NAME"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",[Translation domain used])
dnl Add the languages which your application supports here.
ALL_LINGUAS="am az be ca cs da de el es es_MX et eu fr ga gl hu it ja lt mk nb nl nn pa pl pt pt_BR ru sk sl sr sr@Latn sv tr uk zh_CN zh_TW"
AM_GLIB_GNU_GETTEXT

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test "x$PKG_CONFIG" = "xno"; then
	AC_MSG_ERROR(You have to install pkg-config to compile inkscape.)
fi

AC_CHECK_LIB(png, png_read_info, [AC_CHECK_HEADER(png.h, png_ok=yes, png_ok=no)], png_ok=no, -lz -lm)
if test "x$png_ok" != "xyes"; then
	AC_MSG_ERROR([libpng >= 1.2 is needed to compile inkscape])
fi

dnl Handle possible dlopen requirement for libgc
dnl Isn't this internal to something in autoconf?  Couldn't find it...
AC_CHECK_LIB([dld], [shl_load], [lt_cv_dlopen="shl_load" lt_cv_dlopen_libs="-dld"], [AC_CHECK_FUNC([dlopen],
          [lt_cv_dlopen="dlopen"],
      [AC_CHECK_LIB([dl], [dlopen],
            [lt_cv_dlopen="dlopen" lt_cv_dlopen_libs="-ldl"],
        [AC_CHECK_LIB([svld], [dlopen],
              [lt_cv_dlopen="dlopen" lt_cv_dlopen_libs="-lsvld"],
          [AC_CHECK_LIB([dld], [dld_link],
                [lt_cv_dlopen="dld_link" lt_cv_dlopen_libs="-dld"])
          ])
        ])
      ])
    ])

AC_CHECK_HEADERS([gc.h gc/gc.h],
                 [
		    # To test for the different required libs, I have to
		    # overcome autoconf's caching system, so I change the
		    # desired function name.  They're all in libgc.
		    # The "break" will exit from the top level
                    # AC_CHECK_HEADERS.
		    gc_libs=""
		    AC_CHECK_LIB(gc, GC_init,
                                 [gc_ok=yes;
				  LIBS="-lgc $gc_libs $LIBS";
				  break], [gc_ok=no], [$gc_libs])
		    gc_libs="-lpthread"
		    AC_CHECK_LIB(gc, GC_malloc,
                                 [gc_ok=yes;
				  LIBS="-lgc $gc_libs $LIBS";
                                  break], [gc_ok=no], [$gc_libs])
		    gc_libs="$lt_cv_dlopen_libs"
		    AC_CHECK_LIB(gc, GC_realloc,
                                 [gc_ok=yes;
				  LIBS="-lgc $gc_libs $LIBS";
                                  break], [gc_ok=no], [$gc_libs])
		    gc_libs="-lpthread $lt_cv_dlopen_libs"
		    AC_CHECK_LIB(gc, GC_free,
                                 [gc_ok=yes;
				  LIBS="-lgc $gc_libs $LIBS";
				  break], [gc_ok=no], [$gc_libs])
		    break],
                 [gc_ok=no])
if test "x$gc_ok" = "xyes"; then
	AC_MSG_CHECKING([libgc version 6.4+])
	AC_RUN_IFELSE(
		[AC_LANG_SOURCE([[
			#ifdef HAVE_GC_GC_H
			# include <gc/gc.h>
			#else
			# include <gc.h>
			#endif
			#include <stdio.h>
			extern unsigned GC_version;
			int main(void){
				unsigned min = ((6 << 16) | (4 << 8) | 0);
				printf("%d.%d.%d ",GC_version >> 16, (GC_version >> 8) & 0xFF, GC_version & 0xFF);
				if (GC_version>=min) return 0;
				return 1;
			}]])],
		[gc_ok=yes],
		[gc_ok=no]
	)
	AC_MSG_RESULT([$gc_ok])
fi
if test "x$gc_ok" != "xyes"; then
	AC_MSG_ERROR([libgc (the Boehm Conservative Collector) 6.4+, is needed to compile inkscape -- http://www.hpl.hp.com/personal/Hans_Boehm/gc])
fi

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
AC_MSG_CHECKING([for native Win32])
case "$host" in
  *-*-mingw*)
    os_win32=yes
    INKSCAPE_CFLAGS="$INKSCAPE_CFLAGS -mms-bitfields -DLIBXML_STATIC"
    ;;
  *)
    os_win32=no
    ;;
esac
AC_MSG_RESULT([$os_win32])
AM_CONDITIONAL(OS_WIN32, test "$os_win32" = "yes")

dnl ******************************
dnl Xft checking
dnl ******************************

AC_ARG_WITH(xft,
            AC_HELP_STRING([--with-xft], [use xft scalable font database (default is auto)]),
            [with_xft=$withval], [with_xft=auto])

if test "x$with_xft" != "xno" ; then
	dnl Test fontconfig package
	PKG_CHECK_MODULES(XFT, xft, xft_ok=yes, xft_ok=no)
	if test "x$xft_ok" != "xyes"; then
		dnl test xft package
		PKG_CHECK_MODULES(XFT, fontconfig, xft_ok=yes, xft_ok=no)
		if test "x$xft_ok" != "xyes"; then
			dnl Have to test xft presence
			AC_CHECK_HEADER(X11/Xft/Xft.h, xft_ok=yes, xft_ok=no)
			if test "x$xft_ok" != "xyes"; then
				dnl No xft found
				if test "x$with_xft" = "xyes"; then
					dnl Xft was explicitly asked, so stop
					AC_MSG_ERROR([--with-xft was specified, but appropriate development packages could not be found])
				fi
			else
				dnl Working Xft1
				XFT_LIBS="-L/usr/X11R6/lib -lXft "
			fi
		fi
	fi
else
	dnl Asked to ignore xft
	xft_ok=no
fi

AC_SUBST(XFT_CFLAGS)
AC_SUBST(XFT_LIBS)

AM_CONDITIONAL(USE_XFT, test "x$xft_ok" = "xyes")
if test "x$xft_ok" = "xyes"; then
	AC_DEFINE(WITH_XFT, 1, [Use Xft font database])
fi

dnl ******************************
dnl pangoft2 for xft
dnl ******************************

if test "x$xft_ok" = "xyes"; then
	PKG_CHECK_MODULES(PANGOFT2, pangoft2, pango_ok=yes, pango_ok=no)
	if test "x$pango_ok" = "xyes"; then
		XFT_LIBS="$XFT_LIBS $PANGOFT2_LIBS"
	fi
fi

dnl ******************************
dnl GnomePrint checking
dnl ******************************

AC_ARG_WITH(gnome-print,
            AC_HELP_STRING([--with-gnome-print], [use gnome print font database and spooler frontend]),
            [with_gp=$withval], [with_gp=auto])

if test "x$with_gp" = "xyes"; then
	dnl Have to test gnome-print presence
	PKG_CHECK_MODULES(GNOME_PRINT, libgnomeprint-2.2 >= 1.116.0  libgnomeprintui-2.2 >= 1.116.0, gp=yes, gp=no)
	if test "x$gp" != "xyes"; then
		dnl No gnome-print found
		if test "x$with_gp" = "xyes"; then
			dnl Gnome-print was explicitly asked, so stop
			AC_MSG_ERROR([--with-gnome-print was specified, but appropriate libgnomeprint development packages could not be found])
		else 
			# gp is no, tell us for the log file 
			AC_MSG_RESULT($gp)
		fi
	fi
else
	dnl Asked to ignore gnome-print
	gp=no
fi

AC_SUBST(GNOME_PRINT_CFLAGS)
AC_SUBST(GNOME_PRINT_LIBS)

AM_CONDITIONAL(USE_GNOME_PRINT, test "x$gp" = "xyes")
if test "x$gp" = "xyes"; then
	AC_DEFINE(WITH_GNOME_PRINT, 1, [Use gnome print font database and spooler frontend])
fi

dnl ******************************
dnl gnome vfs checking
dnl ******************************

AC_ARG_WITH(gnome-vfs,
	AC_HELP_STRING([--with-gnome-vfs], [use gnome vfs for loading files]),
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
	AC_HELP_STRING([--without-inkjar], [disable openoffice files (SVG jars)]),[with_ij=$withval], [with_ij=yes])

if test "x$with_ij" = "xyes"; then
	AC_DEFINE(WITH_INKJAR, 1, [enable openoffice files (SVG jars)])
	AC_C_BIGENDIAN
	AC_CHECK_HEADERS(zlib.h)
	ij=yes
else
	ij=no
fi
AM_CONDITIONAL(INKJAR, test "$with_ij" = "yes")

ink_spell_pkg=
if pkg-config --exists gtkspell-2.0; then
	ink_spell_pkg=gtkspell-2.0
	AC_DEFINE(WITH_GTKSPELL, 1, [enable gtk spelling widget])
fi

dnl ******************************
dnl PERL checking
dnl ******************************

AC_MSG_CHECKING(for Perl development environment)
AC_ARG_WITH(perl,
            AC_HELP_STRING([--with-perl], [use Perl for embedded scripting]),
            [with_perl=$withval], [with_perl=skipped])

if test "x$with_perl" = "xyes"; then
    checkPERL_CFLAGS=`perl -MExtUtils::Embed -e perl_inc 2>/dev/null`
    if test "$?" -gt "0"; then
        with_perl="no"
    else
        checkPERL_LIBS=`perl -MExtUtils::Embed -e ldopts 2>/dev/null`
        if test "$?" -gt "0"; then
            with_perl="no"
        else
            with_perl="yes"
        fi
    fi
fi
AC_MSG_RESULT([$with_perl])
if test "x$with_perl" = "xyes"; then
    # Test that we actually have the perl libraries installed
    oldCFLAGS="$CFLAGS"
    oldLIBS="$LIBS"
    CFLAGS="$CFLAGS $checkPERL_CFLAGS"
    LIBS="$LIBS $checkPERL_LIBS"
    AC_CHECK_FUNC([perl_parse],[
	PERL_CFLAGS="$checkPERL_CFLAGS"
	PERL_LIBS="$checkPERL_LIBS"
        AC_DEFINE(WITH_PERL, 1, [use Perl for embedded scripting])
        ],[
	with_perl="no"
	])
    CFLAGS="$oldCFLAGS"
    LIBS="$oldLIBS"
fi
AM_CONDITIONAL(WITH_PERL, test "x$with_perl" = "xyes")
AC_SUBST(PERL_CFLAGS)
AC_SUBST(PERL_LIBS)

dnl ******************************
dnl Python checking
dnl ******************************

AC_MSG_CHECKING(for Python development environment)
AC_ARG_WITH(python,
            AC_HELP_STRING([--with-python], [use Python for embedded scripting]),
            [with_python=$withval], [with_python=skipped])

if test "x$with_python" = "xyes"; then
    checkPYTHON_CFLAGS=`python -c "import distutils.sysconfig ; print '-I%s' % distutils.sysconfig.get_config_var('INCLUDEPY')" 2>/dev/null`
    if test "$?" -gt "0"; then
        with_python="no"
    else
        checkPYTHON_LIBS=`python -c "import distutils.sysconfig ; print '%s/%s %s' % (distutils.sysconfig.get_config_var('LIBPL'),distutils.sysconfig.get_config_var('LDLIBRARY'),distutils.sysconfig.get_config_var('LIBS'))" 2>/dev/null`
        if test "$?" -gt "0"; then
            with_python="no"
        else
            with_python="yes"
        fi
    fi
fi
AC_MSG_RESULT([$with_python])
if test "x$with_python" = "xyes"; then
    # Test that we actually have the python libraries installed
    oldCFLAGS="$CFLAGS"
    oldLIBS="$LIBS"
    CFLAGS="$CFLAGS $checkPYTHON_CFLAGS"
    LIBS="$LIBS $checkPYTHON_LIBS"
    AC_CHECK_FUNC([Py_Initialize],[
	PYTHON_CFLAGS="$checkPYTHON_CFLAGS"
	PYTHON_LIBS="$checkPYTHON_LIBS"
        AC_DEFINE(WITH_PYTHON, 1, [use Python for embedded scripting])
        ],[
	with_python="no"
	])
    CFLAGS="$oldCFLAGS"
    LIBS="$oldLIBS"
fi
AM_CONDITIONAL(WITH_PYTHON, test "x$with_python" = "xyes")
AC_SUBST(PYTHON_CFLAGS)
AC_SUBST(PYTHON_LIBS)

dnl ******************************
dnl LittleCms checking
dnl ******************************

AC_ARG_ENABLE(lcms,
	AC_HELP_STRING([--enable-lcms], [enable LittleCms for color management (disabled by default)]),
	[enable_lcms=$enableval], [enable_lcms=no])

if test "x$enable_lcms" = "xno"; then
	dnl Asked to ignore LittleCms
	lcms=no
else
        dnl Have to test LittleCms presence
	PKG_CHECK_MODULES(LCMS, lcms >= 1.14, lcms=yes, lcms=no)
	if test "x$lcms" != "xyes"; then
	        dnl No lcms found
		if test "x$enable_lcms" = "xyes"; then
		        dnl LittleCms was explicitly asked for, so stop
			AC_MSG_ERROR([--enable-lcms was specified, but appropriate LittleCms development packages could not be found])
		else
			# lcms is no, tell us for the log file
			AC_MSG_RESULT($lcms)
		fi
	else
		dnl Working lcms
		LCMS_LIBS="-llcms "
	fi
fi

if test "x$lcms" = "xyes"; then
	LIBS="$LIBS $LCMS_LIBS"
	AC_DEFINE(ENABLE_LCMS, 1, [Use LittleCms color management])
fi
AM_CONDITIONAL(USE_LCMS, test "x$lcms" = "xyes")
AC_SUBST(LCMS_CFLAGS)
AC_SUBST(LCMS_LIBS)

dnl ******************************
dnl Inkboard dependency checking
dnl ******************************

with_inkboard="no"

AC_MSG_CHECKING(for loudmouth-1.0+)

AC_ARG_ENABLE(inkboard,
		AS_HELP_STRING([--enable-inkboard], [enable Inkboard online whiteboard facility (disabled by default)]),
       with_inkboard=$enableval,with_inkboard=no)

if test "x$with_inkboard" = "xyes"; then
	PKG_CHECK_MODULES(LIBLOUDMOUTH, loudmouth-1.0, loudmouth_ok=yes, loudmouth_ok=no)
	if test "x$loudmouth_ok" = "xyes"; then
		with_inkboard="yes"
		AC_DEFINE(WITH_INKBOARD,1,[Build in Inkboard support])
	else
		with_inkboard="no"
	fi
else
	with_inkboard="no"
fi

AC_MSG_RESULT($with_inkboard)
AM_CONDITIONAL(WITH_INKBOARD, test "x$with_inkboard" = "xyes")
AC_SUBST(LIBLOUDMOUTH_CFLAGS)
AC_SUBST(LIBLOUDMOUTH_LIBS)

dnl ******************************
dnl   Unconditional dependencies
dnl ******************************

dnl *** NOTE: when we move to gtk 2.6 or later, we can remove the 
dnl ********* the override for g_ascii_strtod below...
PKG_CHECK_MODULES(INKSCAPE, gdkmm-2.4  glibmm-2.4  gtkmm-2.4  gtk+-2.0 >= 2.4.0  libxml-2.0 >= 2.6.11  libxslt >= 1.0.15  sigc++-2.0 >= 2.0.3  $ink_spell_pkg  gthread-2.0 >= 2.0)

dnl Shouldn't we test for libpng and libz?
INKSCAPE_LIBS="$INKSCAPE_LIBS -lpng -lz"

AC_CHECK_HEADER(popt.h,
		[INKSCAPE_LIBS="$INKSCAPE_LIBS -lpopt"],
		AC_MSG_ERROR([libpopt is required]))

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
       [  --enable-binreloc       compile with binary relocation support],
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
       AC_MSG_RESULT(no (unknown value "$enable_binreloc"))
       enable_binreloc=no
fi
AC_DEFINE(BR_PTHREADS,[0],[Use binreloc thread support?])
if test "$enable_binreloc" = "yes"; then
   AC_DEFINE(ENABLE_BINRELOC,,[Use AutoPackage?])
fi

AC_ARG_ENABLE(osxapp,
       [  --enable-osxapp         compile with OSX .app data dir paths],
       enable_osxapp=$enableval,enable_osxapp=no)
if test "$enable_osxapp" = "yes"; then
   AC_DEFINE(ENABLE_OSX_APP_LOCATIONS,,[Build with OSX .app data dir paths?])
fi

dnl ******************************
dnl   Boost graph library is required for graphlayout functions
dnl ******************************
AC_CHECK_HEADER([boost/graph/adjacency_list.hpp],
                [AC_DEFINE([HAVE_BOOST_GRAPH_LIB],[],[Will enable connector network layout])],
                [AC_MSG_WARN([Boost graph lib not found, can't include connector network layout functionality.])])

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
AC_TYPE_SIGNAL

dnl Work around broken gcc 3.3 (seen on OSX) where "ENABLE_NLS" isn't
dnl set correctly because the gettext function isn't noticed.
if test "$ac_cv_header_libintl_h" = "yes" &&
   test "$ac_cv_func_bind_textdomain_codeset" = "yes"  &&
   test "$gt_cv_func_have_gettext" != "yes"; then
    AC_DEFINE(ENABLE_NLS)
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
  AC_COMPILE_IFELSE([int dummy;
], , CXXFLAGS="-Wno-unused $ink_svd_CXXFLAGS",)
  # Note: At least one bug has been caught from unused parameter warnings,
  # so it might be worth trying not to disable it.
  # One way of selectively disabling the warnings (i.e. only where the
  # programmer deliberately isn't using the parameter, e.g. for a callback)
  # is to remove the parameter name (leaving just its type), as is done
  # in src/seltrans.cpp:sp_seltrans_handle_event; this indicates that the
  # programmer deliberately has an unused parameter (e.g. because it's used
  # as a callback or similar function pointer use).

  CXXFLAGS="-Wall -W -Wpointer-arith -Wcast-align -Wsign-compare -Woverloaded-virtual -Wswitch  $CXXFLAGS"

  dnl Test for arch-specific situations.
  case "$host_cpu" in
    mips|mipsel)
      dnl Symbol tables can get too large: this uses alternate tables
      dnl See http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=283476
      CXXFLAGS="$CXXFLAGS -Wa,-xgot"
      CFLAGS="$CFLAGS -Wa,-xgot"
      ;;
  esac
fi

dnl ******************************
dnl   Plugin Support
dnl ******************************
dnl
dnl UPDATE: This is unconditional now (Lauris)
dnl
dnl AC_ARG_WITH(modules, AC_HELP_STRING([--with-modules], [Compile with plugin support (experimental)]), [mod=$withval], [mod=yes])
dnl
dnl AM_CONDITIONAL(USE_MODULES, test "x$mod" = "xyes")
dnl if test "x$mod" = "xyes"; then
dnl 	AC_DEFINE(WITH_MODULES, 1, [Use experimental module support])
dnl fi

AC_DEFINE(WITH_MODULES, 1, [Use experimental module support])

dnl ******************************
dnl   libinkscape
dnl ******************************
dnl
dnl AC_ARG_ENABLE(libinkscape, AC_HELP_STRING([--enable-libinkscape], [Compile dynamic library (experimental)]), [splib=$enableval], [splib=no])
dnl
dnl AM_CONDITIONAL(ENABLE_LIBINKSCAPE, test "x$splib" != "xno")
dnl

AC_SUBST(INKSCAPE_CFLAGS)
AC_SUBST(INKSCAPE_LIBS)

#
# Checks to see if we should compile in MMX support (there will be
# a runtime test when the code is actually run to see if it should
# be used - this just checks if we can compile it.)
#
# This code is partially taken from Mesa
#
dnl Let people disable the MMX optimization
AC_ARG_ENABLE(mmx, [  --disable-mmx     Don't use MMX optimization [default=auto]], enable_mmx="$enableval", enable_mmx=auto)

AC_MSG_CHECKING(for x86 platform)
case $host_cpu in
  i386|i486|i586|i686|i786|k6|k7)
    use_x86_asm=yes
    ;;
  *)
    use_x86_asm=no
esac
AC_MSG_RESULT($use_x86_asm)

dnl Are we going to use MMX extensions
use_mmx_asm=no

AC_MSG_CHECKING(compiler support for MMX)

if test x$enable_mmx = xauto ; then
  if test $use_x86_asm = yes; then
    save_ac_ext=$ac_ext
    ac_ext=S
    
    cp $srcdir/src/libnr/nr_mmx_R8G8B8A8_P_R8G8B8A8_P_A8_RGBAP.S conftest.S
    if AC_TRY_EVAL(ac_compile); then
        use_mmx_asm=yes
    fi
    dnl rm -f conftest.[oS]

    ac_ext=$save_ac_ext
  fi

dnl Enforce usage of MMX extensions
elif test x$enable_mmx = xyes ; then
    use_mmx_asm=yes
else
    use_mmx_asm=no
fi

if test $use_mmx_asm = yes; then
	AC_DEFINE(WITH_MMX, 1, [Use MMX optimizations, if CPU supports it])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AM_CONDITIONAL(USE_MMX, test x$use_mmx_asm = xyes)

dnl Override the default g_ascii_strtod
AC_DEFINE(g_ascii_strtod, fixed_g_ascii_strtod, [Pre-1.107 gstrfuncs.c version of g_ascii_strtod is broken])

dnl Figure out where the datadir actually is
if test "x${datadir}" = 'x${prefix}/share'; then
  if test "x${prefix}" = "xNONE"; then
    runtime_datadir="${ac_default_prefix}/share"
  else
    runtime_datadir="${prefix}/share"
  fi
else
  runtime_datadir="${datadir}"
fi

inkscape_sharedir="${runtime_datadir}/${PACKAGE_NAME}"

dnl Define our data paths for config.h
AC_SUBST(INKSCAPE_DATADIR)
AC_DEFINE_UNQUOTED([INKSCAPE_DATADIR], "${runtime_datadir}",
					[Base data directory -- only path-prefix.h should use it!])
AC_SUBST(PACKAGE_LOCALE_DIR)
AC_DEFINE_UNQUOTED([PACKAGE_LOCALE_DIR], "${runtime_datadir}/locale",
					[Localization directory])
AC_SUBST(INKSCAPE_LIBDIR)
AC_DEFINE_UNQUOTED([INKSCAPE_LIBDIR], "${prefix}/lib",
					[Base library directory -- only path-prefix.h should use it!])

dnl Have to add module makefiles (lauris)

AC_CONFIG_FILES([
Makefile
src/Makefile
src/check-header-compile
src/algorithms/makefile
src/application/makefile
src/debug/makefile
src/dialogs/makefile
src/display/makefile
src/dom/makefile
src/extension/implementation/makefile
src/extension/internal/makefile
src/extension/makefile
src/extension/plugin/makefile
src/extension/script/makefile
src/helper/makefile
src/inkjar/makefile
src/io/makefile
src/libcroco/makefile
src/libnr/makefile
src/libnrtype/makefile
src/libavoid/makefile
src/livarot/makefile
src/jabber_whiteboard/makefile
src/removeoverlap/makefile
src/svg/makefile
src/trace/makefile
src/traits/makefile
src/ui/dialog/makefile
src/ui/makefile
src/ui/view/makefile
src/ui/widget/makefile
src/utest/makefile
src/util/makefile
src/widgets/makefile
src/xml/makefile
doc/Makefile
po/Makefile.in
share/Makefile
share/clipart/Makefile
share/examples/Makefile
share/extensions/Makefile
share/fonts/Makefile
share/gradients/Makefile
share/icons/Makefile
share/keys/Makefile
share/markers/Makefile
share/palettes/Makefile
share/patterns/Makefile
share/screens/Makefile
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

        Use Xft font database:    ${xft_ok}
        Use gnome-print:          ${gp}
	Use gnome-vfs:            ${gnome_vfs}
        Use openoffice files:     ${ij}
        Use MMX optimizations:    ${use_mmx_asm}
        Use relocation support:   ${enable_binreloc}
        Use Python extensions:    ${with_python}
        Use Perl extensions:      ${with_perl}
	Enable LittleCms:         ${enable_lcms}
	Enable Inkboard:	  ${with_inkboard}
"
