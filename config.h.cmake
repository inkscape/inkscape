#ifndef _CONFIG_H_
#define _CONFIG_H_

/* Define WIN32 when on windows */
#ifndef WIN32
#cmakedefine WIN32
#endif

/* This is for require-config.h */
#define PACKAGE_TARNAME

/* Use binreloc thread support? */
#cmakedefine BR_PTHREADS

/* Use AutoPackage? */
#cmakedefine ENABLE_BINRELOC

/* Use LittleCms color management */
#cmakedefine ENABLE_LCMS

/* always defined to indicate that i18n is enabled */
#cmakedefine ENABLE_NLS

/* Build with OSX .app data dir paths? */
#cmakedefine ENABLE_OSX_APP_LOCATIONS

/* Translation domain used */
#cmakedefine GETTEXT_PACKAGE

/* Define to 1 if you have the `bind_textdomain_codeset' function. */
#cmakedefine HAVE_BIND_TEXTDOMAIN_CODESET

/* Define to 1 if you have the <boost/concept_check.hpp> header file. */
#cmakedefine HAVE_BOOST_CONCEPT_CHECK_HPP

/* Whether the Cairo PDF backend is available */
#cmakedefine HAVE_CAIRO_PDF

/* define to 1 if Carbon is available */
#cmakedefine HAVE_CARBON

/* Define to 1 if you have the `dcgettext' function. */
#cmakedefine HAVE_DCGETTEXT

/* Define to 1 if you have the `ecvt' function. */
#cmakedefine HAVE_ECVT

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H

/* Define to 1 if you have the `floor' function. */
#cmakedefine HAVE_FLOOR

/* Define to 1 if you have the `fpsetmask' function. */
#cmakedefine HAVE_FPSETMASK

/* Define to 1 if you have the <gc/gc.h> header file. */
#cmakedefine HAVE_GC_GC_H

/* Define to 1 if you have the <gc.h> header file. */
#cmakedefine HAVE_GC_H

/* Define if the GNU gettext() function is already present or preinstalled. */
#cmakedefine HAVE_GETTEXT

/* Define to 1 if you have the `gettimeofday' function. */
#cmakedefine HAVE_GETTIMEOFDAY

/* Define to 1 if you have the `gtk_window_fullscreen' function. */
#cmakedefine HAVE_GTK_WINDOW_FULLSCREEN

/* Define to 1 if you have the `gtk_window_set_default_icon_from_file'
   function. */
#cmakedefine HAVE_GTK_WINDOW_SET_DEFAULT_ICON_FROM_FILE

/* Define to 1 if you have the <ieeefp.h> header file. */
#cmakedefine HAVE_IEEEFP_H

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H

/* Define if your <locale.h> file defines LC_MESSAGES. */
#cmakedefine HAVE_LC_MESSAGES

/* Define to 1 if you have the <libintl.h> header file. */
#cmakedefine HAVE_LIBINTL_H

/* Define to 1 if you have the `m' library (-lm). */
#cmakedefine HAVE_LIBM

/* Define to 1 if you have the <locale.h> header file. */
#cmakedefine HAVE_LOCALE_H

/* Define to 1 if you have the `mallinfo' function. */
#cmakedefine HAVE_MALLINFO

/* Define to 1 if you have the <malloc.h> header file. */
#cmakedefine HAVE_MALLOC_H

/* Define to 1 if you have the `memmove' function. */
#cmakedefine HAVE_MEMMOVE

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H

/* Define to 1 if you have the `memset' function. */
#cmakedefine HAVE_MEMSET

/* Define to 1 if you have the `mkdir' function. */
#cmakedefine HAVE_MKDIR

/* Use libpoppler for direct PDF import */
#cmakedefine HAVE_POPPLER

/* Use libpoppler-cairo for rendering PDF preview */
#cmakedefine HAVE_POPPLER_CAIRO

/* Use libpoppler-glib and Cairo-SVG for PDF import */
#cmakedefine HAVE_POPPLER_GLIB

/* Define to 1 if you have the `pow' function. */
#cmakedefine HAVE_POW

/* Define to 1 if you have the `sqrt' function. */
#cmakedefine HAVE_SQRT

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
#cmakedefine HAVE_STAT_EMPTY_STRING_BUG

/* Define to 1 if you have the <stddef.h> header file. */
#cmakedefine HAVE_STDDEF_H

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H

/* Define to 1 if you have the `strftime' function. */
#cmakedefine HAVE_STRFTIME

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H

/* Define to 1 if you have the `strncasecmp' function. */
#cmakedefine HAVE_STRNCASECMP

/* Define to 1 if you have the `strpbrk' function. */
#cmakedefine HAVE_STRPBRK

/* Define to 1 if you have the `strrchr' function. */
#cmakedefine HAVE_STRRCHR

/* Define to 1 if you have the `strspn' function. */
#cmakedefine HAVE_STRSPN

/* Define to 1 if you have the `strstr' function. */
#cmakedefine HAVE_STRSTR

/* Define to 1 if you have the `strtoul' function. */
#cmakedefine HAVE_STRTOUL

/* Define to 1 if `fordblks' is member of `struct mallinfo'. */
#cmakedefine HAVE_STRUCT_MALLINFO_FORDBLKS

/* Define to 1 if `fsmblks' is member of `struct mallinfo'. */
#cmakedefine HAVE_STRUCT_MALLINFO_FSMBLKS

/* Define to 1 if `hblkhd' is member of `struct mallinfo'. */
#cmakedefine HAVE_STRUCT_MALLINFO_HBLKHD

/* Define to 1 if `uordblks' is member of `struct mallinfo'. */
#cmakedefine HAVE_STRUCT_MALLINFO_UORDBLKS

/* Define to 1 if `usmblks' is member of `struct mallinfo'. */
#cmakedefine HAVE_STRUCT_MALLINFO_USMBLKS

/* Define to 1 if you have the <sys/filio.h> header file. */
#cmakedefine HAVE_SYS_FILIO_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H

/* Define to 1 if you have the <zlib.h> header file. */
#cmakedefine HAVE_ZLIB_H

/* Base data directory -- only path-prefix.h should use it! */
#cmakedefine INKSCAPE_DATADIR

/* Base library directory -- only path-prefix.h should use it! */
#cmakedefine INKSCAPE_LIBDIR

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
#cmakedefine LSTAT_FOLLOWS_SLASHED_SYMLINK

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
#cmakedefine NO_MINUS_C_MINUS_O

/* Name of package */
#cmakedefine PACKAGE

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT

/* Localization directory */
#cmakedefine PACKAGE_LOCALE_DIR

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION

/* Define as the return type of signal handlers (`int' or `void'). */
#cmakedefine RETSIGTYPE

/* Define to 1 if the `S_IS*' macros in <sys/stat.h> do not work properly. */
#cmakedefine STAT_MACROS_BROKEN

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#cmakedefine TIME_WITH_SYS_TIME

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#cmakedefine TM_IN_SYS_TIME

/* Version number of package */
#cmakedefine VERSION

/* Use gnome vfs file load functionality */
#cmakedefine WITH_GNOME_VFS

/* enable gtk spelling widget */
#cmakedefine WITH_GTKSPELL

/* Image Magick++ support for bitmap effects */
#cmakedefine WITH_IMAGE_MAGICK

/* Build in Inkboard support */
#cmakedefine WITH_INKBOARD

/* Build in SSL support for Inkboard */
#cmakedefine WITH_INKBOARD_SSL

/* enable openoffice files (SVG jars) */
#cmakedefine WITH_INKJAR

/* Build in libwpg */
#cmakedefine WITH_LIBWPG

/* Use MMX optimizations, if CPU supports it */
#cmakedefine WITH_MMX

/* Use experimental module support */
#cmakedefine WITH_MODULES

/* use Perl for embedded scripting */
#cmakedefine WITH_PERL

/* use Python for embedded scripting */
#cmakedefine WITH_PYTHON

/* Use Xft font database */
#cmakedefine WITH_XFT

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#cmakedefine WORDS_BIGENDIAN

/* Define to `int' if <sys/types.h> does not define. */
#cmakedefine mode_t


#endif /* _CONFIG_H_ */


