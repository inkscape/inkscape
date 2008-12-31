/** @file
 * @brief Alternate stub implementations for some functions.
 * 
 * This file exists only for the benefit of the linker when building tests,
 * to avoid circular dependencies. If some test causes link errors because of a function
 * it doesn't need, feel free to add a stub here.
 */
/* Authors: Krzysztof Kosiński <twwenk.pl@gmail.com>
 * This file is in the public domain.
 */

#include "preferences.h"
#include <glib.h>
#include <glib/gstdio.h>
#include <cstring>
#include <cstdlib>

int sp_main_gui(int /*argc*/, char const **/*argv*/) { return 0; }
int sp_main_console(int /*argc*/, char const **/*argv*/) { return 0; }

// stubbed out preferences implementation using a simple map
namespace Inkscape {

std::map<Glib::ustring, Preferences::Entry> _prefs;

Preferences::Preferences() :
    _prefs_basename(""),
    _prefs_dir(""),
    _prefs_filename(""),
    _prefs_doc(NULL),
    _use_gui(true),
    _quiet(false),
    _loaded(false),
    _writable(false)
{
}

Preferences::~Preferences()
{
}

void Preferences::load(bool /*use_gui*/, bool /*quiet*/) {}
void Preferences::save() {}

// getter methods

Preferences::Entry const Preferences::getEntry(Glib::ustring const &pref_path)
{
    return _prefs[pref_path];
}
void Preferences::setBool(Glib::ustring const &pref_path, bool value)
{
    _prefs[pref_path] = _create_pref_value(pref_path, (void const*) (value ? "1" : "0"));
}
void Preferences::setInt(Glib::ustring const &pref_path, int value)
{
    gchar *intstr = (gchar*) g_malloc(32);
    g_snprintf(intstr, 32, "%d", value);
    _prefs[pref_path] = _create_pref_value(pref_path, (void const*) intstr);
}
void Preferences::setDouble(Glib::ustring const &pref_path, double value)
{
    gchar *buf = (gchar*) g_malloc(G_ASCII_DTOSTR_BUF_SIZE);
    g_ascii_dtostr(buf, G_ASCII_DTOSTR_BUF_SIZE, value);
    _prefs[pref_path] = _create_pref_value(pref_path, (void const*) buf);
}
void Preferences::setString(Glib::ustring const &pref_path, Glib::ustring const &value)
{
    _prefs[pref_path] = _create_pref_value(pref_path, (void const*) g_strdup(value.data()));
}

bool Preferences::_extractBool(Entry const &v)
{
    gchar const *s = static_cast<gchar const *>(v._value);
    if ( !s[0] || !strcmp(s, "0") || !strcmp(s, "false") ) return false;
    return true;
}
int Preferences::_extractInt(Entry const &v)
{
    gchar const *s = static_cast<gchar const *>(v._value);
    if ( !strcmp(s, "true") ) return true;
    if ( !strcmp(s, "false") ) return false;
    return atoi(s);
}
double Preferences::_extractDouble(Entry const &v)
{
    gchar const *s = static_cast<gchar const *>(v._value);
    return g_ascii_strtod(s, NULL);
}

Glib::ustring Preferences::_extractString(Entry const &v)
{
    return Glib::ustring(static_cast<gchar const *>(v._value));
}
Preferences::Entry const Preferences::_create_pref_value(Glib::ustring const &path, void const *ptr)
{
    return Entry(path, ptr);
}

Preferences *Preferences::_instance = NULL;


} // namespace Inkscape


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
