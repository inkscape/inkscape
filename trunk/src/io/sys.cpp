
/*
 * System abstraction utility routines
 *
 * Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib/gutils.h>
#include <glibmm/fileutils.h>
#if GLIB_CHECK_VERSION(2,6,0)
    #include <glib/gstdio.h>
#endif
#include <glibmm/ustring.h>
#include <gtk/gtkmessagedialog.h>

#include "preferences.h"
#include "sys.h"

#ifdef WIN32

#define BYPASS_GLIB_SPAWN 1

#ifdef BYPASS_GLIB_SPAWN

#include <process.h>	// declares spawn functions
#include <wchar.h>	// declares _wspawn functions
#ifdef __cplusplus
extern "C" {
#endif
_CRTIMP int __cdecl __MINGW_NOTHROW _wspawnl	(int, const wchar_t*, const wchar_t*, ...);
_CRTIMP int __cdecl __MINGW_NOTHROW _wspawnle	(int, const wchar_t*, const wchar_t*, ...);
_CRTIMP int __cdecl __MINGW_NOTHROW _wspawnlp	(int, const wchar_t*, const wchar_t*, ...);
_CRTIMP int __cdecl __MINGW_NOTHROW _wspawnlpe	(int, const wchar_t*, const wchar_t*, ...);
_CRTIMP int __cdecl __MINGW_NOTHROW _wspawnv	(int, const wchar_t*, const wchar_t* const*);
_CRTIMP int __cdecl __MINGW_NOTHROW _wspawnve	(int, const wchar_t*, const wchar_t* const*, const wchar_t* const*);
_CRTIMP int __cdecl __MINGW_NOTHROW _wspawnvp	(int, const wchar_t*, const wchar_t* const*);
_CRTIMP int __cdecl __MINGW_NOTHROW _wspawnvpe	(int, const wchar_t*, const wchar_t* const*, const wchar_t* const*);
#ifdef __cplusplus
}
#endif
#include <unistd.h>
#include <glibmm/i18n.h>
#include <fcntl.h>
#include <io.h>

#endif // BYPASS_GLIB_SPAWN

// For now to get at is_os_wide().
#include "extension/internal/win32.h"
using Inkscape::Extension::Internal::PrintWin32;

#endif // WIN32

//#define INK_DUMP_FILENAME_CONV 1
#undef INK_DUMP_FILENAME_CONV

//#define INK_DUMP_FOPEN 1
#undef INK_DUMP_FOPEN

void dump_str(gchar const *str, gchar const *prefix);
void dump_ustr(Glib::ustring const &ustr);

extern guint update_in_progress;


#define DEBUG_MESSAGE(key, ...) \
{\
    Inkscape::Preferences *prefs = Inkscape::Preferences::get(); \
    gint dump = prefs->getBool("/options/bulia/" #key) ? 1 : 0;\
    gint dumpD = prefs->getBool("/options/bulia/" #key"D") ? 1 : 0;\
    gint dumpD2 = prefs->getBool("/options/bulia/" #key"D2") ? 1 : 0;\
    dumpD &= ( (update_in_progress == 0) || dumpD2 );\
    if ( dump )\
    {\
        g_message( __VA_ARGS__ );\
\
    }\
    if ( dumpD )\
    {\
        GtkWidget *dialog = gtk_message_dialog_new(NULL,\
                                                   GTK_DIALOG_DESTROY_WITH_PARENT, \
                                                   GTK_MESSAGE_INFO,    \
                                                   GTK_BUTTONS_OK,      \
                                                   __VA_ARGS__          \
                                                   );\
        g_signal_connect_swapped(dialog, "response",\
                                 G_CALLBACK(gtk_widget_destroy),        \
                                 dialog);                               \
        gtk_widget_show_all( dialog );\
    }\
}




void Inkscape::IO::dump_fopen_call( char const *utf8name, char const *id )
{
#ifdef INK_DUMP_FOPEN
    Glib::ustring str;
    for ( int i = 0; utf8name[i]; i++ )
    {
        if ( utf8name[i] == '\\' )
        {
            str += "\\\\";
        }
        else if ( (utf8name[i] >= 0x20) && ((0x0ff & utf8name[i]) <= 0x7f) )
        {
            str += utf8name[i];
        }
        else
        {
            gchar tmp[32];
            g_snprintf( tmp, sizeof(tmp), "\\x%02x", (0x0ff & utf8name[i]) );
            str += tmp;
        }
    }
    g_message( "fopen call %s for [%s]", id, str.data() );
#else
    (void)utf8name;
    (void)id;
#endif
}

FILE *Inkscape::IO::fopen_utf8name( char const *utf8name, char const *mode )
{
    static gint counter = 0;
    FILE* fp = NULL;

    DEBUG_MESSAGE( dumpOne, "entering fopen_utf8name( '%s', '%s' )[%d]", utf8name, mode, (counter++) );

#ifndef WIN32
    DEBUG_MESSAGE( dumpOne, "           STEP 0              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
    gchar *filename = g_filename_from_utf8( utf8name, -1, NULL, NULL, NULL );
    if ( filename )
    {
        DEBUG_MESSAGE( dumpOne, "           STEP 1              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
        fp = std::fopen(filename, mode);
        DEBUG_MESSAGE( dumpOne, "           STEP 2              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
        g_free(filename);
        DEBUG_MESSAGE( dumpOne, "           STEP 3              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
        filename = 0;
    }
#else
    Glib::ustring how( mode );
    how.append("b");
    DEBUG_MESSAGE( dumpOne, "   calling is_os_wide()       ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );

    fp = g_fopen(utf8name, how.c_str());
#endif

    DEBUG_MESSAGE( dumpOne, "leaving fopen_utf8name( '%s', '%s' )[%d]", utf8name, mode, (counter++) );

    return fp;
}


int Inkscape::IO::mkdir_utf8name( char const *utf8name )
{
    static gint counter = 0;
    int retval = -1;

    DEBUG_MESSAGE( dumpMk, "entering mkdir_utf8name( '%s' )[%d]", utf8name, (counter++) );

#ifndef WIN32
    DEBUG_MESSAGE( dumpMk, "           STEP 0              ( '%s' )[%d]", utf8name, (counter++) );
    gchar *filename = g_filename_from_utf8( utf8name, -1, NULL, NULL, NULL );
    if ( filename )
    {
        DEBUG_MESSAGE( dumpMk, "           STEP 1              ( '%s' )[%d]", utf8name, (counter++) );
        retval = ::mkdir(filename, S_IRWXU | S_IRGRP | S_IXGRP);
        DEBUG_MESSAGE( dumpMk, "           STEP 2              ( '%s' )[%d]", utf8name, (counter++) );
        g_free(filename);
        DEBUG_MESSAGE( dumpMk, "           STEP 3              ( '%s' )[%d]", utf8name, (counter++) );
        filename = 0;
    }
#else
    DEBUG_MESSAGE( dumpMk, "   calling is_os_wide()       ( '%s' )[%d]", utf8name, (counter++) );

    // Mode should be ingnored inside of glib on the way in
    retval = g_mkdir( utf8name, 0 );
#endif

    DEBUG_MESSAGE( dumpMk, "leaving mkdir_utf8name( '%s' )[%d]", utf8name, (counter++) );

    return retval;
}

/* 
 * Wrapper around Glib::file_open_tmp()
 * Returns a handle to the temp file
 * name_used contains the actual name used
 * 
 * Returns:
 * A file handle (as from open()) to the file opened for reading and writing. 
 * The file is opened in binary mode on platforms where there is a difference. 
 * The file handle should be closed with close().
 * 
 * Note:
 * On Windows Vista Glib::file_open_tmp fails with the current version of glibmm
 * A special case is implemented for WIN32. This can be removed if the issue is fixed
 * in future versions of glibmm 
 * */
int Inkscape::IO::file_open_tmp(std::string& name_used, const std::string& prefix)
{
#ifndef WIN32
    return Glib::file_open_tmp(name_used, prefix);
#else
    /* Special case for WIN32 due to a bug in glibmm
     * (only needed for Windows Vista, but since there is only one windows build all builds get the workaround)
     * The workaround can be removed if the bug is fixed in glibmm
     * 
     * The code is mostly identical to the implementation in glibmm
     * http://svn.gnome.org/svn/glibmm/branches/glibmm-2-12/glib/src/fileutils.ccg
     * */
    
    std::string basename_template (prefix);
    basename_template += "XXXXXX"; // this sillyness shouldn't be in the interface
    
    GError* error = 0;
    gchar *buf_name_used;
    
    gint fileno = g_file_open_tmp(basename_template.c_str(), &buf_name_used, &error);
    
    if(error)
        Glib::Error::throw_exception(error);
    
    name_used = g_strdup(buf_name_used);
    g_free(buf_name_used);
    return fileno;
#endif
}

bool Inkscape::IO::file_test( char const *utf8name, GFileTest test )
{
    bool exists = false;

    if ( utf8name ) {
        gchar *filename = NULL;
        if (utf8name && !g_utf8_validate(utf8name, -1, NULL)) {
            /* FIXME: Trying to guess whether or not a filename is already in utf8 is unreliable.
               If any callers pass non-utf8 data (e.g. using g_get_home_dir), then change caller to
               use simple g_file_test.  Then add g_return_val_if_fail(g_utf_validate(...), false)
               to beginning of this function. */
            filename = g_strdup(utf8name);
            // Looks like g_get_home_dir isn't safe.
            //g_warning("invalid UTF-8 detected internally. HUNT IT DOWN AND KILL IT!!!");
        } else {
            filename = g_filename_from_utf8 ( utf8name, -1, NULL, NULL, NULL );
        }
        if ( filename ) {
            exists = g_file_test (filename, test);
            g_free(filename);
            filename = NULL;
        } else {
            g_warning( "Unable to convert filename in IO:file_test" );
        }
    }

    return exists;
}

/** Wrapper around g_dir_open, but taking a utf8name as first argument. */
GDir *
Inkscape::IO::dir_open(gchar const *const utf8name, guint const flags, GError **const error)
{
    gchar *const opsys_name = g_filename_from_utf8(utf8name, -1, NULL, NULL, error);
    if (opsys_name) {
        GDir *ret = g_dir_open(opsys_name, flags, error);
        g_free(opsys_name);
        return ret;
    } else {
        return NULL;
    }
}

/**
 * Like g_dir_read_name, but returns a utf8name (which must be freed, unlike g_dir_read_name).
 *
 * N.B. Skips over any dir entries that fail to convert to utf8.
 */
gchar *
Inkscape::IO::dir_read_utf8name(GDir *dir)
{
    for (;;) {
        gchar const *const opsys_name = g_dir_read_name(dir);
        if (!opsys_name) {
            return NULL;
        }
        gchar *utf8_name = g_filename_to_utf8(opsys_name, -1, NULL, NULL, NULL);
        if (utf8_name) {
            return utf8_name;
        }
    }
}


gchar* Inkscape::IO::locale_to_utf8_fallback( const gchar *opsysstring,
                                              gssize len,
                                              gsize *bytes_read,
                                              gsize *bytes_written,
                                              GError **error )
{
    gchar *result = NULL;
    if ( opsysstring ) {
        gchar *newFileName = g_locale_to_utf8( opsysstring, len, bytes_read, bytes_written, error );
        if ( newFileName ) {
            if ( !g_utf8_validate(newFileName, -1, NULL) ) {
                g_warning( "input filename did not yield UTF-8" );
                g_free( newFileName );
            } else {
                result = newFileName;
            }
            newFileName = 0;
        } else if ( g_utf8_validate(opsysstring, -1, NULL) ) {
            // This *might* be a case that we want
            // g_warning( "input failed filename->utf8, fell back to original" );
            // TODO handle cases when len >= 0
            result = g_strdup( opsysstring );
        } else {
            gchar const *charset = 0;
            g_get_charset(&charset);
            g_warning( "input filename conversion failed for file with locale charset '%s'", charset );
        }
    }
    return result;
}

#ifdef BYPASS_GLIB_SPAWN
/*
	this code was taken from the original glib sources
*/
#define GSPAWN_HELPER
 
enum
{
  CHILD_NO_ERROR,
  CHILD_CHDIR_FAILED,
  CHILD_SPAWN_FAILED,
};

enum {
  ARG_CHILD_ERR_REPORT = 1,
  ARG_HELPER_SYNC,
  ARG_STDIN,
  ARG_STDOUT,
  ARG_STDERR,
  ARG_WORKING_DIRECTORY,
  ARG_CLOSE_DESCRIPTORS,
  ARG_USE_PATH,
  ARG_WAIT,
  ARG_PROGRAM,
  ARG_COUNT = ARG_PROGRAM
};
static int debug = 0;
#define HELPER_PROCESS "gspawn-win32-helper"


static int
dup_noninherited (int fd,
		  int mode)
{
  HANDLE filehandle;

  DuplicateHandle (GetCurrentProcess (), (LPHANDLE) _get_osfhandle (fd),
		   GetCurrentProcess (), &filehandle,
		   0, FALSE, DUPLICATE_SAME_ACCESS);
  close (fd);
  return _open_osfhandle ((long) filehandle, mode | _O_NOINHERIT);
}


/* The helper process writes a status report back to us, through a
 * pipe, consisting of two ints.
 */
static gboolean
read_helper_report (int      fd,
		    gint     report[2],
		    GError **error)
{
  gint bytes = 0;

  while (bytes < sizeof(gint)*2)
    {
      gint chunk;

      if (debug)
	g_print ("%s:read_helper_report: read %d...\n",
		 __FILE__,
		 sizeof(gint)*2 - bytes);

      chunk = read (fd, ((gchar*)report) + bytes,
		    sizeof(gint)*2 - bytes);

      if (debug)
	g_print ("...got %d bytes\n", chunk);

      if (chunk < 0)
        {
          /* Some weird shit happened, bail out */

          g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
                       _("Failed to read from child pipe (%s)"),
                       g_strerror (errno));

          return FALSE;
        }
      else if (chunk == 0)
	{
	  g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
		       _("Failed to read from child pipe (%s)"),
		       "EOF");
	  break; /* EOF */
	}
      else
	bytes += chunk;
    }

  if (bytes < sizeof(gint)*2)
    return FALSE;

  return TRUE;
}


static void
set_child_error (gint         report[2],
		 const gchar *working_directory,
		 GError     **error)
{
  switch (report[0])
    {
    case CHILD_CHDIR_FAILED:
      g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_CHDIR,
		   _("Failed to change to directory '%s' (%s)"),
		   working_directory,
		   g_strerror (report[1]));
      break;
    case CHILD_SPAWN_FAILED:
      g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
		   _("Failed to execute child process (%s)"),
		   g_strerror (report[1]));
      break;
    default:
      g_assert_not_reached ();
    }
}

static gchar *
protect_argv_string (const gchar *string)
{
  const gchar *p = string;
  gchar *retval, *q;
  gint len = 0;
  gboolean need_dblquotes = FALSE;
  while (*p)
    {
      if (*p == ' ' || *p == '\t')
	need_dblquotes = TRUE;
      else if (*p == '"')
	len++;
      else if (*p == '\\')
	{
	  const gchar *pp = p;
	  while (*pp && *pp == '\\')
	    pp++;
	  if (*pp == '"')
	    len++;
	}
      len++;
      p++;
    }

  q = retval = (gchar *)g_malloc (len + need_dblquotes*2 + 1);
  p = string;

  if (need_dblquotes)
    *q++ = '"';

  while (*p)
    {
      if (*p == '"')
	*q++ = '\\';
      else if (*p == '\\')
	{
	  const gchar *pp = p;
	  while (*pp && *pp == '\\')
	    pp++;
	  if (*pp == '"')
	    *q++ = '\\';
	}
      *q++ = *p;
      p++;
    }

  if (need_dblquotes)
    *q++ = '"';
  *q++ = '\0';

  return retval;
}


static gint
protect_argv (gchar  **argv,
	      gchar ***new_argv)
{
  gint i;
  gint argc = 0;

  while (argv[argc])
    ++argc;
  *new_argv = g_new (gchar *, argc+1);

  /* Quote each argv element if necessary, so that it will get
   * reconstructed correctly in the C runtime startup code.  Note that
   * the unquoting algorithm in the C runtime is really weird, and
   * rather different than what Unix shells do. See stdargv.c in the C
   * runtime sources (in the Platform SDK, in src/crt).
   *
   * Note that an new_argv[0] constructed by this function should
   * *not* be passed as the filename argument to a spawn* or exec*
   * family function. That argument should be the real file name
   * without any quoting.
   */
  for (i = 0; i < argc; i++)
    (*new_argv)[i] = protect_argv_string (argv[i]);

  (*new_argv)[argc] = NULL;

  return argc;
}


static gboolean
utf8_charv_to_wcharv (char     **utf8_charv,
		      wchar_t ***wcharv,
		      int       *error_index,
		      GError   **error)
{
  wchar_t **retval = NULL;

  *wcharv = NULL;
  if (utf8_charv != NULL)
    {
      int n = 0, i;

      while (utf8_charv[n])
	n++;
      retval = g_new (wchar_t *, n + 1);

      for (i = 0; i < n; i++)
	{
	  retval[i] = (wchar_t *)g_utf8_to_utf16 (utf8_charv[i], -1, NULL, NULL, error);
	  if (retval[i] == NULL)
	    {
	      if (error_index)
		*error_index = i;
	      while (i)
		g_free (retval[--i]);
	      g_free (retval);
	      return FALSE;
	    }
	}

      retval[n] = NULL;
    }
  *wcharv = retval;
  return TRUE;
}


/* Avoids a danger in threaded situations (calling close()
 * on a file descriptor twice, and another thread has
 * re-opened it since the first close)
 */
static void
close_and_invalidate (gint *fd)
{
  if (*fd < 0)
    return;

  close (*fd);
  *fd = -1;
}


static gboolean
do_spawn_directly (gint                 *exit_status,
		   gboolean		 do_return_handle,
		   GSpawnFlags           flags,
		   gchar               **argv,
		   char                **envp,
		   char                **protected_argv,
		   GSpawnChildSetupFunc  child_setup,
		   gpointer              user_data,
		   GPid                 *child_handle,
		   GError              **error)
{
  const int mode = (exit_status == NULL) ? P_NOWAIT : P_WAIT;
  char **new_argv;
  int rc = -1;
  int saved_errno;
  GError *conv_error = NULL;
  gint conv_error_index;
  wchar_t *wargv0, **wargv, **wenvp;

  new_argv = (flags & G_SPAWN_FILE_AND_ARGV_ZERO) ? protected_argv + 1 : protected_argv;

  wargv0 = (wchar_t *)g_utf8_to_utf16 (argv[0], -1, NULL, NULL, &conv_error);
  if (wargv0 == NULL)
    {
      g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
		   _("Invalid program name: %s"),
		   conv_error->message);
      g_error_free (conv_error);

      return FALSE;
    }

  if (!utf8_charv_to_wcharv (new_argv, &wargv, &conv_error_index, &conv_error))
    {
      g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
		   _("Invalid string in argument vector at %d: %s"),
		   conv_error_index, conv_error->message);
      g_error_free (conv_error);
      g_free (wargv0);

      return FALSE;
    }

  if (!utf8_charv_to_wcharv (envp, &wenvp, NULL, &conv_error))
    {
      g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
		   _("Invalid string in environment: %s"),
		   conv_error->message);
      g_error_free (conv_error);
      g_free (wargv0);
      g_strfreev ((gchar **) wargv);

      return FALSE;
    }

  if (child_setup)
    (* child_setup) (user_data);

  if (flags & G_SPAWN_SEARCH_PATH)
    if (wenvp != NULL)
      rc = _wspawnvpe (mode, wargv0, (const wchar_t **) wargv, (const wchar_t **) wenvp);
    else
      rc = _wspawnvp (mode, wargv0, (const wchar_t **) wargv);
  else
    if (wenvp != NULL)
      rc = _wspawnve (mode, wargv0, (const wchar_t **) wargv, (const wchar_t **) wenvp);
    else
      rc = _wspawnv (mode, wargv0, (const wchar_t **) wargv);

  g_free (wargv0);
  g_strfreev ((gchar **) wargv);
  g_strfreev ((gchar **) wenvp);

  saved_errno = errno;

  if (rc == -1 && saved_errno != 0)
    {
      g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
		   _("Failed to execute child process (%s)"),
		   g_strerror (saved_errno));
      return FALSE;
    }

  if (exit_status == NULL)
    {
      if (child_handle && do_return_handle)
	*child_handle = (GPid) rc;
      else
	{
	  CloseHandle ((HANDLE) rc);
	  if (child_handle)
	    *child_handle = 0;
	}
    }
  else
    *exit_status = rc;

  return TRUE;
}

static gboolean
make_pipe (gint     p[2],
           GError **error)
{
  if (_pipe (p, 4096, _O_BINARY) < 0)
    {
      g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
                   _("Failed to create pipe for communicating with child process (%s)"),
                   g_strerror (errno));
      return FALSE;
    }
  else
    return TRUE;
}


static gboolean
do_spawn_with_pipes (gint                 *exit_status,
		     gboolean		   do_return_handle,
		     const gchar          *working_directory,
		     gchar               **argv,
		     char                **envp,
		     GSpawnFlags           flags,
		     GSpawnChildSetupFunc  child_setup,
		     gpointer              user_data,
		     GPid                 *child_handle,
		     gint                 *standard_input,
		     gint                 *standard_output,
		     gint                 *standard_error,
		     gint		  *err_report,
		     GError              **error)
{
  char **protected_argv;
  char args[ARG_COUNT][10];
  char **new_argv;
  int i;
  int rc = -1;
  int saved_errno;
  int argc;
  int stdin_pipe[2] = { -1, -1 };
  int stdout_pipe[2] = { -1, -1 };
  int stderr_pipe[2] = { -1, -1 };
  int child_err_report_pipe[2] = { -1, -1 };
  int helper_sync_pipe[2] = { -1, -1 };
  int helper_report[2];
  static gboolean warned_about_child_setup = FALSE;
  GError *conv_error = NULL;
  gint conv_error_index;
  gchar *helper_process;
  CONSOLE_CURSOR_INFO cursor_info;
  wchar_t *whelper, **wargv, **wenvp;
  //extern gchar *_glib_get_installation_directory (void);
  gchar *glib_top;

  if (child_setup && !warned_about_child_setup)
    {
      warned_about_child_setup = TRUE;
      g_warning ("passing a child setup function to the g_spawn functions is pointless and dangerous on Win32");
    }

  argc = protect_argv (argv, &protected_argv);

  if (!standard_input && !standard_output && !standard_error &&
      (flags & G_SPAWN_CHILD_INHERITS_STDIN) &&
      !(flags & G_SPAWN_STDOUT_TO_DEV_NULL) &&
      !(flags & G_SPAWN_STDERR_TO_DEV_NULL) &&
      (working_directory == NULL || !*working_directory) &&
      (flags & G_SPAWN_LEAVE_DESCRIPTORS_OPEN))
    {
      /* We can do without the helper process */
      gboolean retval =
	do_spawn_directly (exit_status, do_return_handle, flags,
			   argv, envp, protected_argv,
			   child_setup, user_data, child_handle,
			   error);
      g_strfreev (protected_argv);
      return retval;
    }

  if (standard_input && !make_pipe (stdin_pipe, error))
    goto cleanup_and_fail;

  if (standard_output && !make_pipe (stdout_pipe, error))
    goto cleanup_and_fail;

  if (standard_error && !make_pipe (stderr_pipe, error))
    goto cleanup_and_fail;

  if (!make_pipe (child_err_report_pipe, error))
    goto cleanup_and_fail;

  if (!make_pipe (helper_sync_pipe, error))
    goto cleanup_and_fail;

  new_argv = g_new (char *, argc + 1 + ARG_COUNT);
  if (GetConsoleCursorInfo (GetStdHandle (STD_OUTPUT_HANDLE), &cursor_info))
    helper_process = HELPER_PROCESS "-console.exe";
  else
    helper_process = HELPER_PROCESS ".exe";

  glib_top = NULL;
  if (glib_top != NULL)
    {
      helper_process = g_build_filename (glib_top, "bin", helper_process, NULL);
      g_free (glib_top);
    }
  else
    helper_process = g_strdup (helper_process);

  new_argv[0] = protect_argv_string (helper_process);

  sprintf (args[ARG_CHILD_ERR_REPORT], "%d", child_err_report_pipe[1]);
  new_argv[ARG_CHILD_ERR_REPORT] = args[ARG_CHILD_ERR_REPORT];

  /* Make the read end of the child error report pipe
   * noninherited. Otherwise it will needlessly be inherited by the
   * helper process, and the started actual user process. As such that
   * shouldn't harm, but it is unnecessary.
   */
  child_err_report_pipe[0] = dup_noninherited (child_err_report_pipe[0], _O_RDONLY);

  if (flags & G_SPAWN_FILE_AND_ARGV_ZERO)
    {
      /* Overload ARG_CHILD_ERR_REPORT to also encode the
       * G_SPAWN_FILE_AND_ARGV_ZERO functionality.
       */
      strcat (args[ARG_CHILD_ERR_REPORT], "#");
    }

  sprintf (args[ARG_HELPER_SYNC], "%d", helper_sync_pipe[0]);
  new_argv[ARG_HELPER_SYNC] = args[ARG_HELPER_SYNC];

  /* Make the write end of the sync pipe noninherited. Otherwise the
   * helper process will inherit it, and thus if this process happens
   * to crash before writing the sync byte to the pipe, the helper
   * process won't read but won't get any EOF either, as it has the
   * write end open itself.
   */
  helper_sync_pipe[1] = dup_noninherited (helper_sync_pipe[1], _O_WRONLY);

  if (standard_input)
    {
      sprintf (args[ARG_STDIN], "%d", stdin_pipe[0]);
      new_argv[ARG_STDIN] = args[ARG_STDIN];
    }
  else if (flags & G_SPAWN_CHILD_INHERITS_STDIN)
    {
      /* Let stdin be alone */
      new_argv[ARG_STDIN] = "-";
    }
  else
    {
      /* Keep process from blocking on a read of stdin */
      new_argv[ARG_STDIN] = "z";
    }

  if (standard_output)
    {
      sprintf (args[ARG_STDOUT], "%d", stdout_pipe[1]);
      new_argv[ARG_STDOUT] = args[ARG_STDOUT];
    }
  else if (flags & G_SPAWN_STDOUT_TO_DEV_NULL)
    {
      new_argv[ARG_STDOUT] = "z";
    }
  else
    {
      new_argv[ARG_STDOUT] = "-";
    }

  if (standard_error)
    {
      sprintf (args[ARG_STDERR], "%d", stderr_pipe[1]);
     new_argv[ARG_STDERR] = args[ARG_STDERR];
    }
  else if (flags & G_SPAWN_STDERR_TO_DEV_NULL)
    {
      new_argv[ARG_STDERR] = "z";
    }
  else
    {
      new_argv[ARG_STDERR] = "-";
    }

  if (working_directory && *working_directory)
    new_argv[ARG_WORKING_DIRECTORY] = protect_argv_string (working_directory);
  else
    new_argv[ARG_WORKING_DIRECTORY] = g_strdup ("-");

  if (!(flags & G_SPAWN_LEAVE_DESCRIPTORS_OPEN))
    new_argv[ARG_CLOSE_DESCRIPTORS] = "y";
  else
    new_argv[ARG_CLOSE_DESCRIPTORS] = "-";

  if (flags & G_SPAWN_SEARCH_PATH)
    new_argv[ARG_USE_PATH] = "y";
  else
    new_argv[ARG_USE_PATH] = "-";

  if (exit_status == NULL)
    new_argv[ARG_WAIT] = "-";
  else
    new_argv[ARG_WAIT] = "w";

  for (i = 0; i <= argc; i++)
    new_argv[ARG_PROGRAM + i] = protected_argv[i];

  //SETUP_DEBUG();

  if (debug)
    {
      g_print ("calling %s with argv:\n", helper_process);
      for (i = 0; i < argc + 1 + ARG_COUNT; i++)
	g_print ("argv[%d]: %s\n", i, (new_argv[i] ? new_argv[i] : "NULL"));
    }

  if (!utf8_charv_to_wcharv (new_argv, &wargv, &conv_error_index, &conv_error))
    {
      if (conv_error_index == ARG_WORKING_DIRECTORY)
	g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_CHDIR,
		     _("Invalid working directory: %s"),
		     conv_error->message);
      else
	g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
		     _("Invalid string in argument vector at %d: %s"),
		     conv_error_index - ARG_PROGRAM, conv_error->message);
      g_error_free (conv_error);
      g_strfreev (protected_argv);
      g_free (new_argv[0]);
      g_free (new_argv[ARG_WORKING_DIRECTORY]);
      g_free (new_argv);
      g_free (helper_process);

      goto cleanup_and_fail;
    }

  if (!utf8_charv_to_wcharv (envp, &wenvp, NULL, &conv_error))
    {
      g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
		   _("Invalid string in environment: %s"),
		   conv_error->message);
      g_error_free (conv_error);
      g_strfreev (protected_argv);
      g_free (new_argv[0]);
      g_free (new_argv[ARG_WORKING_DIRECTORY]);
      g_free (new_argv);
      g_free (helper_process);
      g_strfreev ((gchar **) wargv);

      goto cleanup_and_fail;
    }

  if (child_setup)
    (* child_setup) (user_data);

  whelper = (wchar_t *)g_utf8_to_utf16 (helper_process, -1, NULL, NULL, NULL);
  g_free (helper_process);

  if (wenvp != NULL)
    rc = _wspawnvpe (P_NOWAIT, whelper, (const wchar_t **) wargv, (const wchar_t **) wenvp);
  else
    rc = _wspawnvp (P_NOWAIT, whelper, (const wchar_t **) wargv);

  saved_errno = errno;

  g_free (whelper);
  g_strfreev ((gchar **) wargv);
  g_strfreev ((gchar **) wenvp);

  /* Close the other process's ends of the pipes in this process,
   * otherwise the reader will never get EOF.
   */
  close_and_invalidate (&child_err_report_pipe[1]);
  close_and_invalidate (&helper_sync_pipe[0]);
  close_and_invalidate (&stdin_pipe[0]);
  close_and_invalidate (&stdout_pipe[1]);
  close_and_invalidate (&stderr_pipe[1]);

  g_strfreev (protected_argv);

  g_free (new_argv[0]);
  g_free (new_argv[ARG_WORKING_DIRECTORY]);
  g_free (new_argv);

  /* Check if gspawn-win32-helper couldn't be run */
  if (rc == -1 && saved_errno != 0)
    {
      g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
		   _("Failed to execute helper program (%s)"),
		   g_strerror (saved_errno));
      goto cleanup_and_fail;
    }

  if (exit_status != NULL)
    {
      /* Synchronous case. Pass helper's report pipe back to caller,
       * which takes care of reading it after the grandchild has
       * finished.
       */
      g_assert (err_report != NULL);
      *err_report = child_err_report_pipe[0];
      write (helper_sync_pipe[1], " ", 1);
      close_and_invalidate (&helper_sync_pipe[1]);
    }
  else
    {
      /* Asynchronous case. We read the helper's report right away. */
      if (!read_helper_report (child_err_report_pipe[0], helper_report, error))
	goto cleanup_and_fail;

      close_and_invalidate (&child_err_report_pipe[0]);

      switch (helper_report[0])
	{
	case CHILD_NO_ERROR:
	  if (child_handle && do_return_handle)
	    {
	      /* rc is our HANDLE for gspawn-win32-helper. It has
	       * told us the HANDLE of its child. Duplicate that into
	       * a HANDLE valid in this process.
	       */
	      if (!DuplicateHandle ((HANDLE) rc, (HANDLE) helper_report[1],
				    GetCurrentProcess (), (LPHANDLE) child_handle,
				    0, TRUE, DUPLICATE_SAME_ACCESS))
		{
		  char *emsg = g_win32_error_message (GetLastError ());
		  g_print("%s\n", emsg);
		  *child_handle = 0;
		}
	    }
	  else if (child_handle)
	    *child_handle = 0;
	  write (helper_sync_pipe[1], " ", 1);
	  close_and_invalidate (&helper_sync_pipe[1]);
	  break;

	default:
	  write (helper_sync_pipe[1], " ", 1);
	  close_and_invalidate (&helper_sync_pipe[1]);
	  set_child_error (helper_report, working_directory, error);
	  goto cleanup_and_fail;
	}
    }

  /* Success against all odds! return the information */

  if (standard_input)
    *standard_input = stdin_pipe[1];
  if (standard_output)
    *standard_output = stdout_pipe[0];
  if (standard_error)
    *standard_error = stderr_pipe[0];
  if (rc != -1)
    CloseHandle ((HANDLE) rc);

  return TRUE;

  cleanup_and_fail:

  if (rc != -1)
    CloseHandle ((HANDLE) rc);
  if (child_err_report_pipe[0] != -1)
    close (child_err_report_pipe[0]);
  if (child_err_report_pipe[1] != -1)
    close (child_err_report_pipe[1]);
  if (helper_sync_pipe[0] != -1)
    close (helper_sync_pipe[0]);
  if (helper_sync_pipe[1] != -1)
    close (helper_sync_pipe[1]);
  if (stdin_pipe[0] != -1)
    close (stdin_pipe[0]);
  if (stdin_pipe[1] != -1)
    close (stdin_pipe[1]);
  if (stdout_pipe[0] != -1)
    close (stdout_pipe[0]);
  if (stdout_pipe[1] != -1)
    close (stdout_pipe[1]);
  if (stderr_pipe[0] != -1)
    close (stderr_pipe[0]);
  if (stderr_pipe[1] != -1)
    close (stderr_pipe[1]);

  return FALSE;
}

gboolean
my_spawn_async_with_pipes_utf8 (const gchar          *working_directory,
			       gchar               **argv,
			       gchar               **envp,
			       GSpawnFlags           flags,
			       GSpawnChildSetupFunc  child_setup,
			       gpointer              user_data,
			       GPid                 *child_handle,
			       gint                 *standard_input,
			       gint                 *standard_output,
			       gint                 *standard_error,
			       GError              **error)
{
  g_return_val_if_fail (argv != NULL, FALSE);
  g_return_val_if_fail (standard_output == NULL ||
                        !(flags & G_SPAWN_STDOUT_TO_DEV_NULL), FALSE);
  g_return_val_if_fail (standard_error == NULL ||
                        !(flags & G_SPAWN_STDERR_TO_DEV_NULL), FALSE);
  /* can't inherit stdin if we have an input pipe. */
  g_return_val_if_fail (standard_input == NULL ||
                        !(flags & G_SPAWN_CHILD_INHERITS_STDIN), FALSE);

  return do_spawn_with_pipes (NULL,
			      (flags & G_SPAWN_DO_NOT_REAP_CHILD),
			      working_directory,
			      argv,
			      envp,
			      flags,
			      child_setup,
			      user_data,
			      child_handle,
			      standard_input,
			      standard_output,
			      standard_error,
			      NULL,
			      error);
}

typedef GPid Pid;

// _WRAP_ENUM(SpawnFlags, GSpawnFlags, NO_GTYPE)

/* Helper callback to invoke the actual sigc++ slot.
 * We don't need to worry about (un)referencing, since the
 * child process gets its own copy of the parent's memory anyway.
 */
static void child_setup_callback(void* user_data)
{
  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
  #endif //GLIBMM_EXCEPTIONS_ENABLED
    (*reinterpret_cast<sigc::slot<void>*>(user_data))();
  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  }
  catch(...)
  {
    Glib::exception_handlers_invoke();
  }
  #endif //GLIBMM_EXCEPTIONS_ENABLED
}


void my_spawn_async_with_pipes(const std::string& working_directory,
                            const Glib::ArrayHandle<std::string>& argv,
                            GSpawnFlags flags,
                            const sigc::slot<void>& child_setup,
                            Pid* child_pid,
                            int* standard_input,
                            int* standard_output,
                            int* standard_error)
{
  const bool setup_slot = !child_setup.empty();
  sigc::slot<void> child_setup_ = child_setup;
  GError* error = 0;

  my_spawn_async_with_pipes_utf8(
      working_directory.c_str(),
      const_cast<char**>(argv.data()), 0,
      static_cast<GSpawnFlags>(unsigned(flags)),
      (setup_slot) ? &child_setup_callback : 0,
      (setup_slot) ? &child_setup_         : 0,
      child_pid,
      standard_input, standard_output, standard_error,
      &error);

  if(error)
    Glib::Error::throw_exception(error);
}

#endif

void
Inkscape::IO::spawn_async_with_pipes( const std::string& working_directory,
                                      const Glib::ArrayHandle<std::string>& argv,
                                      Glib::SpawnFlags flags,
                                      const sigc::slot<void>& child_setup,
                                      Glib::Pid* child_pid,
                                      int* standard_input,
                                      int* standard_output,
                                      int* standard_error)
{
#ifndef BYPASS_GLIB_SPAWN
    Glib::spawn_async_with_pipes(working_directory,
                                 argv,
                                 flags,
                                 child_setup,
                                 child_pid,
                                 standard_input,
                                 standard_output,
                                 standard_error);
#else
    my_spawn_async_with_pipes(working_directory,
                              argv,
                              static_cast<GSpawnFlags>(flags),
                              child_setup,
                              child_pid,
                              standard_input,
                              standard_output,
                              standard_error);
#endif
}


gchar* Inkscape::IO::sanitizeString( gchar const * str )
{
    gchar *result = NULL;
    if ( str ) {
        if ( g_utf8_validate(str, -1, NULL) ) {
            result = g_strdup(str);
        } else {
            guchar scratch[8];
            Glib::ustring buf;
            guchar const *ptr = (guchar const*)str;
            while ( *ptr )
            {
                if ( *ptr == '\\' )
                {
                    buf.append("\\\\");
                } else if ( *ptr < 0x80 ) {
                    buf += (char)(*ptr);
                } else {
                    g_snprintf((gchar*)scratch, sizeof(scratch), "\\x%02x", *ptr);
                    buf.append((const char*)scratch);
                }
                ptr++;
            }
            result = g_strdup(buf.c_str());
        }
    }
    return result;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
