#ifndef _CONFIG_H_
#define _CONFIG_H_

/* Define WIN32 when on windows */
#ifndef WIN32
#cmakedefine WIN32
#endif

/* This is for require-config.h */
#define PACKAGE_TARNAME "${PROJECT_NAME} ${INKSCAPE_VERSION}"

/* Use binreloc thread support? */
#cmakedefine BR_PTHREADS 1

/* Use AutoPackage? */
#cmakedefine ENABLE_BINRELOC 1

/* define to 1 if you have lcms version 1.x */
#cmakedefine HAVE_LIBLCMS1 1

/* define to 1 if you have lcms version 2.x */
#cmakedefine HAVE_LIBLCMS2 1

/* always defined to indicate that i18n is enabled */
#cmakedefine ENABLE_NLS 1

/* Build with OSX .app data dir paths? */
#cmakedefine ENABLE_OSX_APP_LOCATIONS 1

/* Translation domain used */
#define GETTEXT_PACKAGE "${PROJECT_NAME}"

/* Define to 1 if you have the `bind_textdomain_codeset' function. */
#cmakedefine HAVE_BIND_TEXTDOMAIN_CODESET 1

/* Define to 1 if you have the <concept_check.hpp> header file. */
#cmakedefine HAVE_BOOST_CONCEPT_CHECK_HPP 1

/* Whether the Cairo PDF backend is available */
#cmakedefine HAVE_CAIRO_PDF 1
#cmakedefine PANGO_ENABLE_ENGINE 1
#cmakedefine RENDER_WITH_PANGO_CAIRO 1

/* define to 1 if Carbon is available */
#cmakedefine HAVE_CARBON 1

/* Define to 1 if you have the `dcgettext' function. */
#cmakedefine HAVE_DCGETTEXT 1

/* Define to 1 if you have the `ecvt' function. */
#cmakedefine HAVE_ECVT 1

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H 1

/* Define to 1 if you have the `floor' function. */
#cmakedefine HAVE_FLOOR 1

/* Define to 1 if you have the `fpsetmask' function. */
#cmakedefine HAVE_FPSETMASK 1

/* Define to 1 if you have the <gc/gc.h> header file. */
#cmakedefine HAVE_GC_GC_H 1

/* Define to 1 if you have the <gc.h> header file. */
#cmakedefine HAVE_GC_H 1

#cmakedefine HAVE_GETOPT_H 1

/* Define if the GNU gettext() function is already present or preinstalled. */
#cmakedefine HAVE_GETTEXT 1

/* Define to 1 if you have the `gettimeofday' function. */
#cmakedefine HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the `gtk_window_fullscreen' function. */
#cmakedefine HAVE_GTK_WINDOW_FULLSCREEN 1

/* Define to 1 if you have the `gtk_window_set_default_icon_from_file'
   function. */
#cmakedefine HAVE_GTK_WINDOW_SET_DEFAULT_ICON_FROM_FILE 1

/* Define to 1 if you have the <ieeefp.h> header file. */
#cmakedefine HAVE_IEEEFP_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define if your <locale.h> file defines LC_MESSAGES. */
#cmakedefine HAVE_LC_MESSAGES 1

/* Define to 1 if you have the <libintl.h> header file. */
#cmakedefine HAVE_LIBINTL_H 1

/* Define to 1 if you have the `m' library (-lm). */
#cmakedefine HAVE_LIBM 1

/* Define to 1 if you have the <locale.h> header file. */
#cmakedefine HAVE_LOCALE_H 1

/* Define to 1 if you have the `mallinfo' function. */
#cmakedefine HAVE_MALLINFO 1

/* Define to 1 if you have the <malloc.h> header file. */
#cmakedefine HAVE_MALLOC_H 1

/* Define to 1 if you have the `memmove' function. */
#cmakedefine HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#cmakedefine HAVE_MEMSET 1

/* Define to 1 if you have the `mkdir' function. */
#cmakedefine HAVE_MKDIR 1

/* Use OpenMP (via cmake) */
#cmakedefine HAVE_OPENMP 1

/* Use aspell for built-in spellchecker */
#cmakedefine HAVE_ASPELL 1

/* Use libpoppler for direct PDF import */
#cmakedefine HAVE_POPPLER 1

/* Use libpoppler-cairo for rendering PDF preview */
#cmakedefine HAVE_POPPLER_CAIRO 1

/* Use libpoppler-glib and Cairo-SVG for PDF import */
#cmakedefine HAVE_POPPLER_GLIB 1

/* Use GfxFont from Poppler >= 0.8.3 */
#cmakedefine POPPLER_NEW_GFXFONT 1

/* Use color space API from Poppler >= 0.12.2 */
#cmakedefine POPPLER_NEW_COLOR_SPACE_API 1

/* Use color space API from Poppler >= 0.26.0 */
#cmakedefine POPPLER_EVEN_NEWER_COLOR_SPACE_API 1

/* Use color space API from Poppler >= 0.29.0 */
#cmakedefine POPPLER_EVEN_NEWER_NEW_COLOR_SPACE_API 1

/* Use new error API from Poppler >= 0.20.0 */
#cmakedefine POPPLER_NEW_ERRORAPI

/* GfxPatch no longer uses GfxColor in >= 0.15.1 */
#cmakedefine POPPLER_NEW_GFXPATCH 1

/* Define to 1 if you have the `pow' function. */
#cmakedefine HAVE_POW 1

/* Define to 1 if you have the `sqrt' function. */
#cmakedefine HAVE_SQRT 1

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
#cmakedefine HAVE_STAT_EMPTY_STRING_BUG 1

/* Define to 1 if you have the <stddef.h> header file. */
#cmakedefine HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the `strftime' function. */
#cmakedefine HAVE_STRFTIME 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if you have the `strncasecmp' function. */
#cmakedefine HAVE_STRNCASECMP 1

/* Define to 1 if you have the `strpbrk' function. */
#cmakedefine HAVE_STRPBRK 1

/* Define to 1 if you have the `strrchr' function. */
#cmakedefine HAVE_STRRCHR 1

/* Define to 1 if you have the `strspn' function. */
#cmakedefine HAVE_STRSPN 1

/* Define to 1 if you have the `strstr' function. */
#cmakedefine HAVE_STRSTR 1

/* Define to 1 if you have the `strtoul' function. */
#cmakedefine HAVE_STRTOUL 1

/* Define to 1 if `fordblks' is member of `struct mallinfo'. */
#cmakedefine HAVE_STRUCT_MALLINFO_FORDBLKS 1

/* Define to 1 if `fsmblks' is member of `struct mallinfo'. */
#cmakedefine HAVE_STRUCT_MALLINFO_FSMBLKS 1

/* Define to 1 if `hblkhd' is member of `struct mallinfo'. */
#cmakedefine HAVE_STRUCT_MALLINFO_HBLKHD 1

/* Define to 1 if `uordblks' is member of `struct mallinfo'. */
#cmakedefine HAVE_STRUCT_MALLINFO_UORDBLKS

/* Define to 1 if `usmblks' is member of `struct mallinfo'. */
#cmakedefine HAVE_STRUCT_MALLINFO_USMBLKS 1

/* Define to 1 if you have the <sys/filio.h> header file. */
#cmakedefine HAVE_SYS_FILIO_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if you have the <zlib.h> header file. */
#cmakedefine HAVE_ZLIB_H 1

/* Base data directory -- only path-prefix.h should use it! */
#define INKSCAPE_DATADIR "${CMAKE_INSTALL_PREFIX}/share"

/* Base library directory -- only path-prefix.h should use it! */
#define INKSCAPE_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib"

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
#cmakedefine LSTAT_FOLLOWS_SLASHED_SYMLINK 1

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
#cmakedefine NO_MINUS_C_MINUS_O 1

/* Name of package */
#define PACKAGE "${PROJECT_NAME}"

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT ""

/* Localization directory */
#define PACKAGE_LOCALE_DIR "${PACKAGE_LOCALE_DIR}"

/* Define to the full name of this package. */
#define PACKAGE_NAME "${PROJECT_NAME}"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "${PROJECT_NAME} ${INKSCAPE_VERSION}"

/* Define to the version of this package. */
#define PACKAGE_VERSION "${INKSCAPE_VERSION}"

/* Define the version as a string. */
#define VERSION "${INKSCAPE_VERSION}"

/* Build in dbus */
#cmakedefine WITH_DBUS 1

/* Define as the return type of signal handlers (`int' or `void'). */
#cmakedefine RETSIGTYPE

/* Define to 1 if the `S_IS*' macros in <sys/stat.h> do not work properly. */
#cmakedefine STAT_MACROS_BROKEN 1

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#cmakedefine TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#cmakedefine TM_IN_SYS_TIME 1

/* Use gnome vfs file load functionality */
#cmakedefine WITH_GNOME_VFS 1

/* enable gtk spelling widget */
#cmakedefine WITH_GTKSPELL 1

/* Image Magick++ support for bitmap effects */
#cmakedefine WITH_IMAGE_MAGICK 1

/* Build in Inkboard support */
#cmakedefine WITH_INKBOARD 1

/* Build in SSL support for Inkboard */
#cmakedefine WITH_INKBOARD_SSL 1

/* enable openoffice files (SVG jars) */
#cmakedefine WITH_INKJAR 1

/* Build in libcdr */
#cmakedefine WITH_LIBCDR 1

/* Build using libcdr 0.0.x */
#cmakedefine WITH_LIBCDR00 1

/* Build using libcdr 0.1.x */
#cmakedefine WITH_LIBCDR01 1

/* Build in libvisio */
#cmakedefine WITH_LIBVISIO 1

/* Build using libvisio 0.0.x */
#cmakedefine WITH_LIBVISIO00 1

/* Build using libvisio 0.1.x */
#cmakedefine WITH_LIBVISIO01 1

/* Build in libwpg */
#cmakedefine WITH_LIBWPG 1

/* Build in libwpg-0.1 */
#cmakedefine WITH_LIBWPG01 1

/* Build in libwpg-0.2 */
#cmakedefine WITH_LIBWPG02 1

/* Build in libwpg-0.3 */
#cmakedefine WITH_LIBWPG03 1

/* Use MMX optimizations, if CPU supports it */
#cmakedefine WITH_MMX 1

/* Use experimental module support */
#cmakedefine WITH_MODULES 1

/* use Perl for embedded scripting */
#cmakedefine WITH_PERL 1

/* use Python for embedded scripting */
#cmakedefine WITH_PYTHON 1

/* Use Xft font database */
#cmakedefine WITH_XFT 1

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#cmakedefine WORDS_BIGENDIAN 1

/* Do we want experimental, unsupported, unguaranteed, etc., LivePathEffects enabled? */
#cmakedefine LPE_ENABLE_TEST_EFFECTS 1

/* Define to `int' if <sys/types.h> does not define. */
#cmakedefine mode_t

#endif /* _CONFIG_H_ */


