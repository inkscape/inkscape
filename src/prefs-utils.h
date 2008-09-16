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
#include <glib/gslist.h>
#include "preferences.h"

inline bool pref_path_exists(gchar const *path){
    Inkscape::Preferences *ps = Inkscape::Preferences::get();
    return ps->exists(path);
}

inline unsigned pref_path_number_of_children(gchar const *path){
    Inkscape::Preferences *ps = Inkscape::Preferences::get();
    return ps->childCount(path);
}

inline void
prefs_set_int_attribute(gchar const *path, gchar const *attr, long long int value)
{
    Inkscape::Preferences *ps = Inkscape::Preferences::get();
    ps->setInt(path, attr, value);
}

inline int
prefs_get_int_attribute(gchar const *path, gchar const *attr, long long int def)
{
    Inkscape::Preferences *ps = Inkscape::Preferences::get();
    return ps->getInt(path, attr, def);
}

inline int
prefs_get_int_attribute_limited(gchar const *path, gchar const *attr, long long int def, long long int min, long long int max)
{
    Inkscape::Preferences *ps = Inkscape::Preferences::get();
    return ps->getIntLimited(path, attr, def, min, max);
}

inline void
prefs_set_double_attribute(gchar const *path, gchar const *attr, double value)
{
    Inkscape::Preferences *ps = Inkscape::Preferences::get();
    ps->setDouble(path, attr, value);
}

inline double
prefs_get_double_attribute(gchar const *path, gchar const *attr, double def)
{
    Inkscape::Preferences *ps = Inkscape::Preferences::get();
    return ps->getDouble(path, attr, def);
}

inline double
prefs_get_double_attribute_limited(gchar const *path, gchar const *attr, double def, double min, double max)
{
    Inkscape::Preferences *ps = Inkscape::Preferences::get();
    return ps->getDoubleLimited(path, attr, def, min, max);
}

inline void
prefs_set_string_attribute(gchar const *path, gchar const *attr, gchar const *value)
{
    Inkscape::Preferences *ps = Inkscape::Preferences::get();
    ps->setString(path, attr, value);
}

/// @todo Reimplement using Gtk::RecentManager
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
