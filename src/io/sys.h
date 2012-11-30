#ifndef SEEN_SYS_H
#define SEEN_SYS_H

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

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <glib.h>
#include <glibmm/spawn.h>
#include <string>

/*#####################
## U T I L I T Y
#####################*/

namespace Inkscape {
namespace IO {

void dump_fopen_call( char const *utf8name, char const *id );

FILE *fopen_utf8name( char const *utf8name, char const *mode );

int mkdir_utf8name( char const *utf8name );

int file_open_tmp( std::string& name_used, const std::string& prefix );

bool file_test( char const *utf8name, GFileTest test );

bool file_directory_exists( char const *utf8name );

bool file_is_writable( char const *utf8name);

GDir *dir_open(gchar const *utf8name, guint flags, GError **error);

gchar *dir_read_utf8name(GDir *dir);

gchar* locale_to_utf8_fallback( const gchar *opsysstring,
				gssize len,
				gsize *bytes_read,
				gsize *bytes_written,
				GError **error );

gchar* sanitizeString( gchar const * str );

void spawn_async_with_pipes (const std::string& working_directory,
                             const Glib::ArrayHandle<std::string>& argv,
                             Glib::SpawnFlags flags,
                             const sigc::slot<void>& child_setup,
                             Glib::Pid* child_pid,
                             int* standard_input,
                             int* standard_output,
                             int* standard_error);

Glib::ustring get_file_extension(Glib::ustring path);

}
}


#endif // SEEN_SYS_H
