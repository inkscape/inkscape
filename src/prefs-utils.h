/*
 * Utility functions for reading and setting preferences
 *
 * Authors:
 *   bulia byak <bulia@dr.com>
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_PREFS_UTILS_H
#define SEEN_PREFS_UTILS_H

#include <glib/gtypes.h>

bool pref_path_exists(gchar const *path);
void prefs_set_int_attribute(gchar const *path, gchar const *attr, long long int value);
long long int prefs_get_int_attribute(gchar const *path, gchar const *attr, long long int def);
long long int prefs_get_int_attribute_limited(gchar const *path, gchar const *attr, long long int def, long long int min, long long int max);

void prefs_set_double_attribute(gchar const *path, gchar const *attr, double value);
double prefs_get_double_attribute(gchar const *path, gchar const *attr, double def);
double prefs_get_double_attribute_limited(gchar const *path, gchar const *attr, double def, double min, double max);

gchar const *prefs_get_string_attribute(gchar const *path, gchar const *attr);
void prefs_set_string_attribute(gchar const *path, gchar const *attr, gchar const *value);

void prefs_set_recent_file(const gchar * uri, const gchar * name);
const gchar ** prefs_get_recent_files(void);

#endif /* !SEEN_PREFS_UTILS_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
