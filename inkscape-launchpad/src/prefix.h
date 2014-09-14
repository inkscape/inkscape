/*
 * BinReloc - a library for creating relocatable executables
 * Written by: Mike Hearn <mike@theoretic.com>
 *             Hongli Lai <h.lai@chello.nl>
 * http://autopackage.org/
 *
 * This source code is public domain. You can relicense this code
 * under whatever license you want.
 *
 * See http://autopackage.org/docs/binreloc/ for
 * more information and how to use this.
 *
 * NOTE: if you're using C++ and are getting "undefined reference
 * to br_*", try renaming prefix.c to prefix.cpp
 */

#ifndef _PREFIX_H_
#define _PREFIX_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* WARNING, BEFORE YOU MODIFY PREFIX.C:
 *
 * If you make changes to any of the functions in prefix.c, you MUST
 * change the BR_NAMESPACE macro.
 * This way you can avoid symbol table conflicts with other libraries
 * that also happen to use BinReloc.
 *
 * Example:
 * #define BR_NAMESPACE(funcName) foobar_ ## funcName
 * --> expands br_locate to foobar_br_locate
 */
#undef BR_NAMESPACE
#define BR_NAMESPACE(funcName) funcName


#ifdef ENABLE_BINRELOC

#define br_thread_local_store BR_NAMESPACE(br_thread_local_store)
#define br_locate BR_NAMESPACE(br_locate)
#define br_locate_prefix BR_NAMESPACE(br_locate_prefix)
#define br_prepend_prefix BR_NAMESPACE(br_prepend_prefix)

#ifndef BR_NO_MACROS
	/* These are convience macros that replace the ones usually used
	   in Autoconf/Automake projects */
	#undef SELFPATH
	#undef PREFIX
	#undef PREFIXDIR
	#undef BINDIR
	#undef SBINDIR
	#undef DATADIR
	#undef LIBDIR
	#undef LIBEXECDIR
	#undef ETCDIR
	#undef SYSCONFDIR
	#undef CONFDIR
	#undef LOCALEDIR

	#define SELFPATH	(br_thread_local_store (br_locate ((void *) "")))
	#define PREFIX		(br_thread_local_store (br_locate_prefix ((void *) "")))
	#define PREFIXDIR	(br_thread_local_store (br_locate_prefix ((void *) "")))
	#define BINDIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/bin")))
	#define SBINDIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/sbin")))
	#define DATADIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/share")))
	#define LIBDIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/lib")))
	#define LIBEXECDIR	(br_thread_local_store (br_prepend_prefix ((void *) "", "/libexec")))
	#define ETCDIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/etc")))
	#define SYSCONFDIR	(br_thread_local_store (br_prepend_prefix ((void *) "", "/etc")))
	#define CONFDIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/etc")))
	#define LOCALEDIR	(br_thread_local_store (br_prepend_prefix ((void *) "", "/share/locale")))
#endif /* BR_NO_MACROS */


/* The following functions are used internally by BinReloc
   and shouldn't be used directly in applications. */

const char *br_thread_local_store (char *str);
char *br_locate		(void *symbol);
char *br_locate_prefix	(void *symbol);
char *br_prepend_prefix	(void *symbol, char const *path);


#endif /* ENABLE_BINRELOC */


/* These macros and functions are not guarded by the ENABLE_BINRELOC
 * macro because they are portable. You can use these functions.
 */

#define br_strcat BR_NAMESPACE(br_strcat)
#define br_extract_dir BR_NAMESPACE(br_extract_dir)
#define br_extract_prefix BR_NAMESPACE(br_extract_prefix)

#ifndef BR_NO_MACROS
	/* Convenience functions for concatenating paths */
	#define BR_SELFPATH(suffix)	(br_thread_local_store (br_strcat (SELFPATH, suffix)))
	#define BR_PREFIX(suffix)	(br_thread_local_store (br_strcat (PREFIX, suffix)))
	#define BR_PREFIXDIR(suffix)	(br_thread_local_store (br_strcat (BR_PREFIX, suffix)))
	#define BR_BINDIR(suffix)	(br_thread_local_store (br_strcat (BINDIR, suffix)))
	#define BR_SBINDIR(suffix)	(br_thread_local_store (br_strcat (SBINDIR, suffix)))
	#define BR_DATADIR(suffix)	(br_thread_local_store (br_strcat (DATADIR, suffix)))
	#define BR_LIBDIR(suffix)	(br_thread_local_store (br_strcat (LIBDIR, suffix)))
	#define BR_LIBEXECDIR(suffix)	(br_thread_local_store (br_strcat (LIBEXECDIR, suffix)))
	#define BR_ETCDIR(suffix)	(br_thread_local_store (br_strcat (ETCDIR, suffix)))
	#define BR_SYSCONFDIR(suffix)	(br_thread_local_store (br_strcat (SYSCONFDIR, suffix)))
	#define BR_CONFDIR(suffix)	(br_thread_local_store (br_strcat (CONFDIR, suffix)))
	#define BR_LOCALEDIR(suffix)	(br_thread_local_store (br_strcat (LOCALEDIR, suffix)))
#endif

char *br_strcat	(const char *str1, const char *str2);
char *br_extract_dir	(const char *path);
char *br_extract_prefix(const char *path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef __WIN32__

#include <glibmm/ustring.h>

Glib::ustring win32_getExePath();
char *win32_relative_path(const char *childPath);

#define WIN32_DATADIR(suffix) (win32_relative_path(suffix))

#endif

#endif /* _PREFIX_H_ */
