/**
 * GNOME Utils - Migration helper
 *
 * Authors:
 *   GNOME Developer
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 *
 * Released under GNU GPL
 */

#ifndef SEEN_GNOME_UTILS_H
#define SEEN_GNOME_UTILS_H

#include <glib.h>

GList *gnome_uri_list_extract_uris(gchar const *uri_list);
GList *gnome_uri_list_extract_filenames(gchar const *uri_list);

#endif // !SEEN_GNOME_UTILS_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
