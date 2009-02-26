/** @file
 * @brief  Singleton class to access the preferences file in a convenient way.
 */
/* Authors:
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
#include <map>
#include <vector>
#include <climits>
#include <cfloat>
#include "xml/xml-forward.h"
#include "xml/repr.h"

class SPCSSAttr;

namespace Inkscape {

/**
 * @brief Preference storage class
 *
 * This is a singleton that allows one to access the user preferences stored in
 * the preferences.xml file. The preferences are stored in a file system-like
 * hierarchy. They are generally typeless - it's up to the programmer to ensure
 * that a given preference is always accessed as the correct type. The backend
 * is not guaranteed to be tolerant to type mismatches.
 *
 * Preferences are identified by paths similar to file system paths. Components
 * of the path are separated by a slash (/). As an additional requirement,
 * the path must start with a slash, and not contain a trailing slash.
 * An example of a correct path would be "/options/some_group/some_option".
 *
 * All preferences are loaded when the first singleton pointer is requested,
 * or when the static load() method is called. Before loading, the static
 * variable @c use_gui should be set accordingly. To save the preferences,
 * the method save() or the static function unload() can be used.
 *
 * In future, this will be a virtual base from which specific backends
 * derive (e.g. GConf, flat XML file...)
 */
class Preferences {
public:
    // #############################
    // ## inner class definitions ##
    // #############################
    
    class Entry;
    class Observer;
    
    /**
     * @brief Base class for preference observers
     * 
     * If you want to watch for changes in the preferences, you'll have to
     * derive a class from this one and override the notify() method.
     */
    class Observer {
        friend class Preferences;
    public:
        /**
         * @brief Constructor.
         *
         * Since each Observer is assigned to a single path, the base
         * constructor takes this path as an argument. This prevents one from
         * adding a single observer to multiple paths, but this is intentional
         * to simplify the implementation of observers and notifications.
         *
         * After you add the object with Preferences::addObserver(), you will
         * receive notifications for everything below the attachment point.
         * You can also specify a single preference as the watch point.
         * For example, watching the directory "/foo" will give you notifications
         * about "/foo/some_pref" as well as "/foo/some_dir/other_pref".
         * Watching the preference "/options/some_group/some_option" will only
         * generate notifications when this single preference changes.
         *
         * @param path Preference path the observer should watch
         */
        Observer(Glib::ustring const &path);
        virtual ~Observer();
        /**
         * @brief Notification about a preference change
         * @param new_val  Entry object containing information about
         *                 the modified preference
         */
        virtual void notify(Preferences::Entry const &new_val) = 0;
        
        Glib::ustring const observed_path; ///< Path which the observer watches
    private:
        void *_data; ///< additional data used by the implementation while the observer is active
    };
    
    
    /**
     * @brief Data type representing a typeless value of a preference
     *
     * This is passed to the observer in the notify() method.
     * To retrieve useful data from it, use its member functions. Setting
     * any preference using the Preferences class invalidates this object,
     * so use its get methods before doing so.
     */
    class Entry {
    friend class Preferences; // Preferences class has to access _value
    public:
        ~Entry() {}
        Entry() : _pref_path(""), _value(NULL) {} // needed to enable use in maps
        Entry(Entry const &other) : _pref_path(other._pref_path), _value(other._value) {}
        /**
         * @brief Check whether the received entry is valid.
         * @return If false, the default value will be returned by the getters.
         */
        bool isValid() const { return _value != NULL; }
        
        /**
         * @brief Interpret the preference as a Boolean value.
         * @param def Default value if the preference is not set
         */
        inline bool getBool(bool def=false) const;
        /**
         * @brief Interpret the preference as an integer.
         * @param def Default value if the preference is not set
         */
        inline int getInt(int def=0) const;
        /**
         * @brief Interpret the preference as a limited integer.
         *
         * This method will return the default value if the interpreted value is
         * larger than @c max or smaller than @c min. Do not use to store
         * Boolean values as integers.
         *
         * @param def Default value if the preference is not set
         * @param min Minimum value allowed to return
         * @param max Maximum value allowed to return
         */
        inline int getIntLimited(int def=0, int min=INT_MIN, int max=INT_MAX) const;
        /**
         * @brief Interpret the preference as a floating point value.
         * @param def Default value if the preference is not set
         */
        inline double getDouble(double def=0.0) const;
        /**
         * @brief Interpret the preference as a limited floating point value.
         *
         * This method will return the default value if the interpreted value is
         * larger than @c max or smaller than @c min.
         *
         * @param def Default value if the preference is not set
         * @param min Minimum value allowed to return
         * @param max Maximum value allowed to return
         */
        inline double getDoubleLimited(double def=0.0, double min=DBL_MIN, double max=DBL_MAX) const;
        /**
         * @brief Interpret the preference as an UTF-8 string.
         *
         * To store a filename, convert it using Glib::filename_to_utf8().
         */
        inline Glib::ustring getString() const;
        /**
         * @brief Interpret the preference as a CSS style.
         * @return A CSS style that has to be unrefed when no longer necessary. Never NULL.
         */
        inline SPCSSAttr *getStyle() const;
        /**
         * @brief Interpret the preference as a CSS style with directory-based
         *        inheritance
         *
         * This function will look up the preferences with the same entry name
         * in ancestor directories and return the inherited CSS style.
         *
         * @return Inherited CSS style that has to be unrefed after use. Never NULL.
         */
        inline SPCSSAttr *getInheritedStyle() const;
        
        /**
         * @brief Get the full path of the preference described by this Entry.
         */
        Glib::ustring const &getPath() const { return _pref_path; }
        /**
         * @brief Get the last component of the preference's path
         *
         * E.g. For "/options/some_group/some_option" it will return "some_option".
         */
        Glib::ustring getEntryName() const;
    private:
        Entry(Glib::ustring const &path, void const *v) : _pref_path(path), _value(v) {}
        
        Glib::ustring _pref_path;
        void const *_value;
    };

    // utility methods
    
    /**
     * @name Load stored preferences and save them to the disk.
     * @{
     */
    
    /**
     * @brief Load the preferences from the default location.
     *
     * Loads the stored user preferences and enables saving them. If there's
     * no preferences file in the expected location, it creates it. Any changes
     * made to the preferences before loading will be overridden by the stored
     * prefs. Not calling load() is sometimes useful, e.g. for testing.
     *
     * @param use_gui Whether to use dialogs to notify about errors when
     * loading the preferences. Set to false in console mode.
     * @param quiet Whether to output any messages about preference loading.
     * If this is true, the use_gui parameter is ignored.
     */
    void load(bool use_gui=true, bool quiet=false);
    /**
     * @brief Save all preferences to the hard disk.
     *
     * For some backends, the preferences may be saved as they are modified.
     * Not calling this method doesn't guarantee the preferences are unmodified
     * the next time Inkscape runs.
     */
    void save();
    /**
     * @brief Check whether saving the preferences will have any effect.
     */
    bool isWritable() { return _writable; }
    /*@}*/

    /**
     * @name Iterate over directories and entries.
     * @{
     */

    /**
     * @brief Get all entries from the specified directory
     *
     * This method will return a vector populated with preference entries
     * from the specified directory. Subdirectories will not be represented.
     */
    std::vector<Entry> getAllEntries(Glib::ustring const &path);
    /**
     * @brief Get all subdirectories of the specified directory
     *
     * This will return a vector populated with full paths to the subdirectories
     * present in the specified @c path.
     */
    std::vector<Glib::ustring> getAllDirs(Glib::ustring const &path);
    /*@}*/
    
    /**
     * @name Retrieve data from the preference storage.
     * @{
     */
    /**
     * @brief Retrieve a Boolean value
     * @param pref_path Path to the retrieved preference
     * @param def The default value to return if the preference is not set
     */
    bool getBool(Glib::ustring const &pref_path, bool def=false) {
        return getEntry(pref_path).getBool(def);
    }
    /**
     * @brief Retrieve an integer
     * @param pref_path Path to the retrieved preference
     * @param def The default value to return if the preference is not set
     */
    int getInt(Glib::ustring const &pref_path, int def=0) {
        return getEntry(pref_path).getInt(def);
    }
    /**
     * @brief Retrieve a limited integer
     *
     * The default value is returned if the actual value is larger than @c max
     * or smaller than @c min. Do not use to store Boolean values.
     *
     * @param pref_path Path to the retrieved preference
     * @param def The default value to return if the preference is not set
     * @param min Minimum value to return
     * @param max Maximum value to return
     */
    int getIntLimited(Glib::ustring const &pref_path, int def=0, int min=INT_MIN, int max=INT_MAX) {
        return getEntry(pref_path).getIntLimited(def, min, max);
    }
    double getDouble(Glib::ustring const &pref_path, double def=0.0) {
        return getEntry(pref_path).getDouble(def);
    }
    /**
     * @brief Retrieve a limited floating point value
     *
     * The default value is returned if the actual value is larger than @c max
     * or smaller than @c min.
     *
     * @param pref_path Path to the retrieved preference
     * @param def The default value to return if the preference is not set
     * @param min Minimum value to return
     * @param max Maximum value to return
     */
    double getDoubleLimited(Glib::ustring const &pref_path, double def=0.0, double min=DBL_MIN, double max=DBL_MAX) {
        return getEntry(pref_path).getDoubleLimited(def, min, max);
    }
    /**
     * @brief Retrieve an UTF-8 string
     * @param pref_path Path to the retrieved preference
     */
    Glib::ustring getString(Glib::ustring const &pref_path) {
        return getEntry(pref_path).getString();
    }
    /**
     * @brief Retrieve a CSS style
     * @param pref_path Path to the retrieved preference
     * @return A CSS style that has to be unrefed after use.
     */
    SPCSSAttr *getStyle(Glib::ustring const &pref_path) {
        return getEntry(pref_path).getStyle();
    }
    /**
     * @brief Retrieve an inherited CSS style
     *
     * This method will look up preferences with the same entry name in ancestor
     * directories and return a style obtained by inheriting properties from
     * ancestor styles.
     *
     * @param pref_path Path to the retrieved preference
     * @return An inherited CSS style that has to be unrefed after use.
     */
    SPCSSAttr *getInheritedStyle(Glib::ustring const &pref_path) {
        return getEntry(pref_path).getInheritedStyle();
    }
    /**
     * @brief Retrieve a preference entry without specifying its type
     */
    Entry const getEntry(Glib::ustring const &pref_path);
    /*@}*/
    
    /**
     * @name Update preference values.
     * @{
     */

    /**
     * @brief Set a Boolean value
     */
    void setBool(Glib::ustring const &pref_path, bool value);
    /**
     * @brief Set an integer value
     */
    void setInt(Glib::ustring const &pref_path, int value);
    /**
     * @brief Set a floating point value
     */
    void setDouble(Glib::ustring const &pref_path, double value);
    /**
     * @brief Set an UTF-8 string value
     */
    void setString(Glib::ustring const &pref_path, Glib::ustring const &value);
    /**
     * @brief Set a CSS style
     */
    void setStyle(Glib::ustring const &pref_path, SPCSSAttr *style);
    /**
     * @brief Merge a CSS style with the current preference value
     *
     * This method is similar to setStyle(), except that it merges the style
     * rather than replacing it. This means that if @c style doesn't have
     * a property set, it is left unchanged in the style stored in
     * the preferences.
     */
    void mergeStyle(Glib::ustring const &pref_path, SPCSSAttr *style);
    /*@}*/

    /**
     * @name Receive notifications about preference changes.
     * @{
     */
    /**
     * @brief Register a preference observer
     */
    void addObserver(Observer &);
    /**
     * @brief Remove an observer an prevent further notifications to it.
     */
    void removeObserver(Observer &);
    /*@}*/

    /**
     * @name Access and manipulate the Preferences object.
     * @{
     */
     
    /**
     * @brief Access the singleton Preferences object.
     */
    static Preferences *get() {
        if (!_instance) _instance = new Preferences();
        return _instance;
    }
    /**
     * @brief Unload all preferences
     * @param save Whether to save the preferences; defaults to true
     *
     * This deletes the singleton object. Calling get() after this function
     * will reinstate it, so you shouldn't. Pass false as the parameter
     * to suppress automatic saving.
     */
    static void unload(bool save=true);
    /*@}*/
    
protected:
    /* helper methods used by Entry
     * This will enable using the same Entry class with different backends.
     * For now, however, those methods are not virtual. These methods assume
     * that v._value is not NULL
     */
    bool _extractBool(Entry const &v);
    int _extractInt(Entry const &v);
    double _extractDouble(Entry const &v);
    Glib::ustring _extractString(Entry const &v);
    SPCSSAttr *_extractStyle(Entry const &v);
    SPCSSAttr *_extractInheritedStyle(Entry const &v);
    
private:
    Preferences();
    ~Preferences();
    void _loadDefaults();
    void _getRawValue(Glib::ustring const &path, gchar const *&result);
    void _setRawValue(Glib::ustring const &path, gchar const *value);
    void _errorDialog(Glib::ustring const &, Glib::ustring const &);
    void _keySplit(Glib::ustring const &pref_path, Glib::ustring &node_key, Glib::ustring &attr_key);
    XML::Node *_getNode(Glib::ustring const &pref_path, bool create=false);
    XML::Node *_findObserverNode(Glib::ustring const &pref_path, Glib::ustring &node_key, Glib::ustring &attr_key, bool create);
    
    // disable copying
    Preferences(Preferences const &);
    Preferences operator=(Preferences const &);
    
    std::string _prefs_basename; ///< Basename of the prefs file
    std::string _prefs_dir; ///< Directory in which to look for the prefs file
    std::string _prefs_filename; ///< Full filename (with directory) of the prefs file
    XML::Document *_prefs_doc; ///< XML document storing all the preferences
    bool _use_gui; ///< Use GUI error notifications?
    bool _quiet; ///< Display any messages about loading?
    bool _loaded; ///< Was a load attempt made?
    bool _writable; ///< Will the preferences be saved at exit?
    
    /// Wrapper class for XML node observers
    class PrefNodeObserver;
    
    typedef std::map<Observer *, PrefNodeObserver *> _ObsMap;
    /// Map that keeps track of wrappers assigned to PrefObservers
    _ObsMap _observer_map;
    
    // privilege escalation methods for PrefNodeObserver
    static Entry const _create_pref_value(Glib::ustring const &, void const *ptr);
    static void *_get_pref_observer_data(Observer &o) { return o._data; }
    
    static Preferences *_instance;
    
friend class PrefNodeObserver;
friend class Entry;
};

/* Trivial inline Preferences::Entry functions.
 * In fact only the _extract* methods do something, the rest is delegation
 * to avoid duplication of code. There should be no performance hit if
 * compiled with -finline-functions.
 */

inline bool Preferences::Entry::getBool(bool def) const
{
    if (!this->isValid()) return def;
    return Inkscape::Preferences::get()->_extractBool(*this);
}

inline int Preferences::Entry::getInt(int def) const
{
    if (!this->isValid()) return def;
    return Inkscape::Preferences::get()->_extractInt(*this);
}

inline int Preferences::Entry::getIntLimited(int def, int min, int max) const
{
    if (!this->isValid()) return def;
    int val = Inkscape::Preferences::get()->_extractInt(*this);
    return ( val >= min && val <= max ? val : def );
}

inline double Preferences::Entry::getDouble(double def) const
{
    if (!this->isValid()) return def;
    return Inkscape::Preferences::get()->_extractDouble(*this);
}

inline double Preferences::Entry::getDoubleLimited(double def, double min, double max) const
{
    if (!this->isValid()) return def;
    double val = Inkscape::Preferences::get()->_extractDouble(*this);
    return ( val >= min && val <= max ? val : def );
}

inline Glib::ustring Preferences::Entry::getString() const
{
    if (!this->isValid()) return "";
    return Inkscape::Preferences::get()->_extractString(*this);
}

inline SPCSSAttr *Preferences::Entry::getStyle() const
{
    if (!this->isValid()) return sp_repr_css_attr_new();
    return Inkscape::Preferences::get()->_extractStyle(*this);
}

inline SPCSSAttr *Preferences::Entry::getInheritedStyle() const
{
    if (!this->isValid()) return sp_repr_css_attr_new();
    return Inkscape::Preferences::get()->_extractInheritedStyle(*this);
}

inline Glib::ustring Preferences::Entry::getEntryName() const
{
    Glib::ustring path_base = _pref_path;
    path_base.erase(0, path_base.rfind('/') + 1);
    return path_base;
}

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
