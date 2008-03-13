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
#include <glib/gtypes.h>
#include <glib/gdir.h>
#include <glib/gfileutils.h>
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

GDir *dir_open(gchar const *utf8name, guint flags, GError **error);

gchar *dir_read_utf8name(GDir *dir);

gchar* locale_to_utf8_fallback( const gchar *opsysstring,
				gssize len,
				gsize *bytes_read,
				gsize *bytes_written,
				GError **error );

gchar* sanitizeString( gchar const * str );

}
}


#endif // SEEN_SYS_H
