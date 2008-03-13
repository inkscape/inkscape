
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

#include "prefs-utils.h"
#include "sys.h"

#ifdef WIN32

// For now to get at is_os_wide().
#include "extension/internal/win32.h"
using Inkscape::Extension::Internal::PrintWin32;

#endif

//#define INK_DUMP_FILENAME_CONV 1
#undef INK_DUMP_FILENAME_CONV

//#define INK_DUMP_FOPEN 1
#undef INK_DUMP_FOPEN

void dump_str(gchar const *str, gchar const *prefix);
void dump_ustr(Glib::ustring const &ustr);

extern guint update_in_progress;


#define DEBUG_MESSAGE(key, ...) \
{\
    gint dump = prefs_get_int_attribute_limited("options.bulia", #key, 0, 0, 1);\
    gint dumpD = prefs_get_int_attribute_limited("options.bulia", #key"D", 0, 0, 1);\
    gint dumpD2 = prefs_get_int_attribute_limited("options.bulia", #key"D2", 0, 0, 1);\
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
