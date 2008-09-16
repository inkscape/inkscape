/** @file
 * @brief  Singleton class to access the preferences file in a convenient way.
 *
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2008 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_PREFSTORE_H
#define INKSCAPE_PREFSTORE_H

#include <glibmm/ustring.h>
#include <string>
#include <climits>
#include <cfloat>

namespace Inkscape {
namespace XML {
    class Node;
    class Document;
    class NodeObserver;
} // namespace XML

/**
 * @brief Preference storage class
 *
 * This is a singleton that allows one to access the user preferences stored in
 * the preferences.xml file. The preferences are stored in a tree hierarchy.
 * Each preference is identified by its key, and sections of the key
 * corresponding to the levels of the hierarchy are delimited with dots.
 * Preferences are generally typeless - it's up to the programmer to ensure
 * that a given preference is always accessed as the correct type.
 *
 * All preferences are loaded when the first singleton pointer is requested,
 * or when the static load() method is called. Before loading, the static
 * variable @c use_gui should be set accordingly. To save the preferences,
 * the method save() or the static function unload() can be used.
 *
 * In future, this could be a virtual base from which specific backends
 * derive (e.g. GConf, Windows registry, flat XML file...)
 */
class Preferences {
public:
    // utility methods
    void save();
    bool isWritable() { return _writable; }
    
    // some helpers
    bool exists(Glib::ustring const &pref_key);
    unsigned int childCount(Glib::ustring const &pref_key);
    Glib::ustring getNthChild(Glib::ustring const &father_path, unsigned int n);
    bool create(Glib::ustring const &pref_key);
    
    // getter methods
    // Note that default values supplied as arguments are last-chance,
    // and are overridden by those in preferences-defaults.h
    bool getBool(Glib::ustring const &pref_path, Glib::ustring const &attr, bool def=false);
    int getInt(Glib::ustring const &pref_path, Glib::ustring const &attr, int def=0);
    int getIntLimited(Glib::ustring const &pref_path, Glib::ustring const &attr, int def=0, int min=INT_MIN, int max=INT_MAX);
    double getDouble(Glib::ustring const &pref_path, Glib::ustring const &attr, double def=0.0);
    double getDoubleLimited(Glib::ustring const &pref_path, Glib::ustring const &attr, double def=0.0, double min=DBL_MIN, double max=DBL_MAX);
    Glib::ustring getString(Glib::ustring const &pref_path, Glib::ustring const &attr);
    
    // setter methods
    void setBool(Glib::ustring const &pref_path, Glib::ustring const &attr, bool value);
    void setInt(Glib::ustring const &pref_path, Glib::ustring const &attr, int value);
    void setDouble(Glib::ustring const &pref_path, Glib::ustring const &attr, double value);
    void setString(Glib::ustring const &pref_path, Glib::ustring const &attr, Glib::ustring const &value);
    
    // do not use this - it is only temporarily public to ease porting old code to this class
    XML::Node *_getNode(Glib::ustring const &pref_path, bool create=false);
    
    // used for some obscure purpose in sp_desktop_widget_init
    void addPrefsObserver(XML::NodeObserver *observer);

    // singleton accessor
    static Preferences *get() {
        if (!_instance) _instance = new Preferences();
        return _instance;
    }
    static void load() {
        if (!_instance) _instance = new Preferences();
    }
    static void unload() {
        if(_instance)
        {
            delete _instance;
            _instance = NULL;
        }
    }
    // this is a static member to reduce dependency bloat for this class
    static bool use_gui; ///< Whether to use GUI error notifications
    
private:
    Preferences();
    ~Preferences();
    void _load();
    void _loadDefaults();
    void _errorDialog(Glib::ustring const &, Glib::ustring const &);
    
    // disable copying
    Preferences(Preferences const &);
    Preferences operator=(Preferences const &);
    
    std::string _prefs_basename; ///< Basename of the prefs file
    std::string _prefs_dir; ///< Directory in which to look for the prefs file
    std::string _prefs_filename; ///< Full filename (with directory) of the prefs file
    bool _writable; ///< Will the preferences be saved at exit?
    XML::Document *_prefs_doc; ///< XML document storing all the preferences
    
    static Preferences *_instance;
};

} // namespace Inkscape

#endif // INKSCAPE_PREFSTORE_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:75
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
