/** @file
 * Singleton class to access the preferences file - implementation.
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2008,2009 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include <cstring>
#include <sstream>
#include <glibmm/fileutils.h>
#include <glibmm/convert.h>
#include <glibmm/i18n.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include "preferences.h"
#include "preferences-skeleton.h"
#include "inkscape.h"
#include "xml/node-observer.h"
#include "xml/node-iterators.h"
#include "xml/attribute-record.h"
#include "util/units.h"

#define PREFERENCES_FILE_NAME "preferences.xml"

using Inkscape::Util::unit_table;

namespace Inkscape {

static Inkscape::XML::Document *loadImpl( std::string const& prefsFilename, Glib::ustring & errMsg );
static void migrateDetails( Inkscape::XML::Document *from, Inkscape::XML::Document *to );

static Inkscape::XML::Document *migrateFromDoc = 0;

// TODO clean up. Function copied from file.cpp:
// what gets passed here is not actually an URI... it is an UTF-8 encoded filename (!)
static void file_add_recent(gchar const *uri)
{
    if (!uri) {
        g_warning("file_add_recent: uri == NULL");
    } else {
        GtkRecentManager *recent = gtk_recent_manager_get_default();
        gchar *fn = g_filename_from_utf8(uri, -1, NULL, NULL, NULL);
        if (fn) {
            if (g_file_test(fn, G_FILE_TEST_EXISTS)) {
                gchar *uriToAdd = g_filename_to_uri(fn, NULL, NULL);
                if (uriToAdd) {
                    gtk_recent_manager_add_item(recent, uriToAdd);
                    g_free(uriToAdd);
                }
            }
            g_free(fn);
        }
    }
}


// private inner class definition

/**
 * XML - prefs observer bridge.
 *
 * This is an XML node observer that watches for changes in the XML document storing the preferences.
 * It is used to implement preference observers.
 */
class Preferences::PrefNodeObserver : public XML::NodeObserver {
public:
    PrefNodeObserver(Observer &o, Glib::ustring const &filter) :
        _observer(o),
        _filter(filter)
    {}
    virtual ~PrefNodeObserver() {}
    virtual void notifyAttributeChanged(XML::Node &node, GQuark name, Util::ptr_shared<char>, Util::ptr_shared<char>);
private:
    Observer &_observer;
    Glib::ustring const _filter;
};

Preferences::Preferences() :
    _prefs_basename(PREFERENCES_FILE_NAME),
    _prefs_dir(""),
    _prefs_filename(""),
    _prefs_doc(0),
    _errorHandler(0),
    _writable(false),
    _hasError(false)
{
    // profile_path essentailly returns the argument prefixed by the profile directory.
    gchar *path = profile_path(NULL);
    _prefs_dir = path;
    g_free(path);

    path = profile_path(_prefs_basename.c_str());
    _prefs_filename = path;
    g_free(path);

    _loadDefaults();
    _load();
}

Preferences::~Preferences()
{
    // delete all PrefNodeObservers
    for (_ObsMap::iterator i = _observer_map.begin(); i != _observer_map.end(); ) {
        delete (*i++).second; // avoids reference to a deleted key
    }
    // unref XML document
    Inkscape::GC::release(_prefs_doc);
}

/**
 * Load internal defaults.
 *
 * In the future this will try to load the system-wide file before falling
 * back to the internal defaults.
 */
void Preferences::_loadDefaults()
{
    _prefs_doc = sp_repr_read_mem(preferences_skeleton, PREFERENCES_SKELETON_SIZE, NULL);
}

/**
 * Load the user's customized preferences.
 *
 * Tries to load the user's preferences.xml file. If there is none, creates it.
 */
void Preferences::_load()
{
    Glib::ustring const not_saved = _("Inkscape will run with default settings, "
                                      "and new settings will not be saved. ");

    // NOTE: After we upgrade to Glib 2.16, use Glib::ustring::compose

    // 1. Does the file exist?
    if (!g_file_test(_prefs_filename.c_str(), G_FILE_TEST_EXISTS)) {
        // No - we need to create one.
        // Does the profile directory exist?
        if (!g_file_test(_prefs_dir.c_str(), G_FILE_TEST_EXISTS)) {
            // No - create the profile directory
            if (g_mkdir(_prefs_dir.c_str(), 0755)) {
                // the creation failed
                //_reportError(Glib::ustring::compose(_("Cannot create profile directory %1."),
                //    Glib::filename_to_utf8(_prefs_dir)), not_saved);
                gchar *msg = g_strdup_printf(_("Cannot create profile directory %s."),
                    Glib::filename_to_utf8(_prefs_dir).c_str());
                _reportError(msg, not_saved);
                g_free(msg);
                return;
            }
            // create some subdirectories for user stuff
            char const *user_dirs[] = {"keys", "templates", "icons", "extensions", "palettes", NULL};
            for (int i=0; user_dirs[i]; ++i) {
                char *dir = profile_path(user_dirs[i]);
                g_mkdir(dir, 0755);
                g_free(dir);
            }

        } else if (!g_file_test(_prefs_dir.c_str(), G_FILE_TEST_IS_DIR)) {
            // The profile dir is not actually a directory
            //_reportError(Glib::ustring::compose(_("%1 is not a valid directory."),
            //    Glib::filename_to_utf8(_prefs_dir)), not_saved);
            gchar *msg = g_strdup_printf(_("%s is not a valid directory."),
                Glib::filename_to_utf8(_prefs_dir).c_str());
            _reportError(msg, not_saved);
            g_free(msg);
            return;
        }
        // The profile dir exists and is valid.
        if (!g_file_set_contents(_prefs_filename.c_str(), preferences_skeleton, PREFERENCES_SKELETON_SIZE, NULL)) {
            // The write failed.
            //_reportError(Glib::ustring::compose(_("Failed to create the preferences file %1."),
            //    Glib::filename_to_utf8(_prefs_filename)), not_saved);
            gchar *msg = g_strdup_printf(_("Failed to create the preferences file %s."),
                Glib::filename_to_utf8(_prefs_filename).c_str());
            _reportError(msg, not_saved);
            g_free(msg);
            return;
        }

        if ( migrateFromDoc ) {
            migrateDetails( migrateFromDoc, _prefs_doc );
        }

        // The prefs file was just created.
        // We can return now and skip the rest of the load process.
        _writable = true;
        return;
    }

    // Yes, the pref file exists.
    Glib::ustring errMsg;
    Inkscape::XML::Document *prefs_read = loadImpl( _prefs_filename, errMsg );

    if ( prefs_read ) {
        // Merge the loaded prefs with defaults.
        _prefs_doc->root()->mergeFrom(prefs_read->root(), "id");
        Inkscape::GC::release(prefs_read);
        _writable = true;
    } else {
        _reportError(errMsg, not_saved);
    }
}

//_reportError(msg, not_saved);
static Inkscape::XML::Document *loadImpl( std::string const& prefsFilename, Glib::ustring & errMsg )
{
    // 2. Is it a regular file?
    if (!g_file_test(prefsFilename.c_str(), G_FILE_TEST_IS_REGULAR)) {
        gchar *msg = g_strdup_printf(_("The preferences file %s is not a regular file."),
            Glib::filename_to_utf8(prefsFilename).c_str());
        errMsg = msg;
        g_free(msg);
        return 0;
    }

    // 3. Is the file readable?
    gchar *prefs_xml = NULL; gsize len = 0;
    if (!g_file_get_contents(prefsFilename.c_str(), &prefs_xml, &len, NULL)) {
        gchar *msg = g_strdup_printf(_("The preferences file %s could not be read."),
            Glib::filename_to_utf8(prefsFilename).c_str());
        errMsg = msg;
        g_free(msg);
        return 0;
    }

    // 4. Is it valid XML?
    Inkscape::XML::Document *prefs_read = sp_repr_read_mem(prefs_xml, len, NULL);
    g_free(prefs_xml);
    if (!prefs_read) {
        gchar *msg = g_strdup_printf(_("The preferences file %s is not a valid XML document."),
            Glib::filename_to_utf8(prefsFilename).c_str());
        errMsg = msg;
        g_free(msg);
        return 0;
    }

    // 5. Basic sanity check: does the root element have a correct name?
    if (strcmp(prefs_read->root()->name(), "inkscape")) {
        gchar *msg = g_strdup_printf(_("The file %s is not a valid Inkscape preferences file."),
            Glib::filename_to_utf8(prefsFilename).c_str());
        errMsg = msg;
        g_free(msg);
        Inkscape::GC::release(prefs_read);
        return 0;
    }

    return prefs_read;
}

static void migrateDetails( Inkscape::XML::Document *from, Inkscape::XML::Document *to )
{
    // TODO pull in additional prefs with more granularity
    to->root()->mergeFrom(from->root(), "id");
}

/**
 * Flush all pref changes to the XML file.
 */
void Preferences::save()
{
    // no-op if the prefs file is not writable
    if (_writable) {
        // sp_repr_save_file uses utf-8 instead of the glib filename encoding.
        // I don't know why filenames are kept in utf-8 in Inkscape and then
        // converted to filename encoding when necessary through special functions
        // - wouldn't it be easier to keep things in the encoding they are supposed
        // to be in?

        // No, it would not. There are many reasons, one key reason being that the
        // rest of GTK+ is explicitly UTF-8. From an engineering standpoint, keeping
        // the filesystem encoding would change things from a one-to-many problem to
        // instead be a many-to-many problem. Also filesystem encoding can change
        // from one run of the program to the next, so can not be stored.
        // There are many other factors, so ask if you would like to learn them. - JAC
        Glib::ustring utf8name = Glib::filename_to_utf8(_prefs_filename);
        if (!utf8name.empty()) {
            sp_repr_save_file(_prefs_doc, utf8name.c_str());
        }
    }
}

bool Preferences::getLastError( Glib::ustring& primary, Glib::ustring& secondary )
{
    bool result = _hasError;
    if ( _hasError ) {
        primary = _lastErrPrimary;
        secondary = _lastErrSecondary;
        _hasError = false;
        _lastErrPrimary.clear();
        _lastErrSecondary.clear();
    } else {
        primary.clear();
        secondary.clear();
    }
    return result;
}

void Preferences::migrate( std::string const& legacyDir, std::string const& prefdir )
{
    int mode = S_IRWXU;
#ifdef S_IRGRP
    mode |= S_IRGRP;
#endif
#ifdef S_IXGRP
    mode |= S_IXGRP;
#endif
#ifdef S_IXOTH
    mode |= S_IXOTH;
#endif
    if ( g_mkdir_with_parents(prefdir.c_str(), mode) == -1 ) {
    } else {
    }

    gchar * oldPrefFile = g_build_filename(legacyDir.c_str(), PREFERENCES_FILE_NAME, NULL);
    if (oldPrefFile) {
        if (g_file_test(oldPrefFile, G_FILE_TEST_EXISTS)) {
            Glib::ustring errMsg;
            Inkscape::XML::Document *oldPrefs = loadImpl( oldPrefFile, errMsg );
            if (oldPrefs) {
                Glib::ustring docId("documents");
                Glib::ustring recentId("recent");
                Inkscape::XML::Node *node = oldPrefs->root();
                Inkscape::XML::Node *child = 0;
                Inkscape::XML::Node *recentNode = 0;
                if (node->attribute("version")) {
                    node->setAttribute("version", "1");
                }
                for (child = node->firstChild(); child; child = child->next()) {
                    if (docId == child->attribute("id")) {
                        for (child = child->firstChild(); child; child = child->next()) {
                            if (recentId == child->attribute("id")) {
                                recentNode = child;
                                for (child = child->firstChild(); child; child = child->next()) {
                                    gchar const* uri = child->attribute("uri");
                                    if (uri) {
                                        file_add_recent(uri);
                                    }
                                }
                                break;
                            }
                        }
                        break;
                    }
                }

                if (recentNode) {
                    while (recentNode->firstChild()) {
                        recentNode->removeChild(recentNode->firstChild());
                    }
                }
                migrateFromDoc = oldPrefs;
                //Inkscape::GC::release(oldPrefs);
                oldPrefs = 0;
            } else {
                g_warning( "%s", errMsg.c_str() );
            }
        }
        g_free(oldPrefFile);
        oldPrefFile = 0;
    }
}

// Now for the meat.

/**
 * Get names of all entries in the specified path.
 *
 * @param path Preference path to query.
 * @return A vector containing all entries in the given directory.
 */
std::vector<Preferences::Entry> Preferences::getAllEntries(Glib::ustring const &path)
{
    std::vector<Entry> temp;
    Inkscape::XML::Node *node = _getNode(path, false);
    if (node) {
        // argh - purge this Util::List nonsense from XML classes fast
        Inkscape::Util::List<Inkscape::XML::AttributeRecord const> alist = node->attributeList();
        for (; alist; ++alist) {
            temp.push_back( Entry(path + '/' + g_quark_to_string(alist->key), static_cast<void const*>(alist->value.pointer())) );
        }
    }
    return temp;
}

/**
 * Get the paths to all subdirectories of the specified path.
 *
 * @param path Preference path to query.
 * @return A vector containing absolute paths to all subdirectories in the given path.
 */
std::vector<Glib::ustring> Preferences::getAllDirs(Glib::ustring const &path)
{
    std::vector<Glib::ustring> temp;
    Inkscape::XML::Node *node = _getNode(path, false);
    if (node) {
        for (Inkscape::XML::NodeSiblingIterator i = node->firstChild(); i; ++i) {
            temp.push_back(path + '/' + i->attribute("id"));
        }
    }
    return temp;
}

// getter methods

Preferences::Entry const Preferences::getEntry(Glib::ustring const &pref_path)
{
    gchar const *v;
    _getRawValue(pref_path, v);
    return Entry(pref_path, v);
}

// setter methods

/**
 * Set a boolean attribute of a preference.
 *
 * @param pref_path Path of the preference to modify.
 * @param value The new value of the pref attribute.
 */
void Preferences::setBool(Glib::ustring const &pref_path, bool value)
{
    /// @todo Boolean values should be stored as "true" and "false",
    /// but this is not possible due to an interaction with event contexts.
    /// Investigate this in depth.
    _setRawValue(pref_path, ( value ? "1" : "0" ));
}

/**
 * Set an integer attribute of a preference.
 *
 * @param pref_path Path of the preference to modify.
 * @param value The new value of the pref attribute.
 */
void Preferences::setInt(Glib::ustring const &pref_path, int value)
{
    _setRawValue(pref_path, Glib::ustring::compose("%1",value));
}

/**
 * Set a floating point attribute of a preference.
 *
 * @param pref_path Path of the preference to modify.
 * @param value The new value of the pref attribute.
 */
void Preferences::setDouble(Glib::ustring const &pref_path, double value)
{
    _setRawValue(pref_path, Glib::ustring::compose("%1",value));
}

/**
 * Set a floating point attribute of a preference.
 *
 * @param pref_path Path of the preference to modify.
 * @param value The new value of the pref attribute.
 * @param unit_abbr The string of the unit (abbreviated).
 */
void Preferences::setDoubleUnit(Glib::ustring const &pref_path, double value, Glib::ustring const &unit_abbr)
{
    Glib::ustring str = Glib::ustring::compose("%1%2",value,unit_abbr);
    _setRawValue(pref_path, str);
}

void Preferences::setColor(Glib::ustring const &pref_path, guint32 value)
{
    gchar buf[16];
    g_snprintf(buf, 16, "#%08x", value);
    _setRawValue(pref_path, buf);
}

/**
 * Set a string attribute of a preference.
 *
 * @param pref_path Path of the preference to modify.
 * @param value The new value of the pref attribute.
 */
void Preferences::setString(Glib::ustring const &pref_path, Glib::ustring const &value)
{
    _setRawValue(pref_path, value);
}

void Preferences::setStyle(Glib::ustring const &pref_path, SPCSSAttr *style)
{
    Glib::ustring css_str;
    sp_repr_css_write_string(style, css_str);
    _setRawValue(pref_path, css_str);
}

void Preferences::mergeStyle(Glib::ustring const &pref_path, SPCSSAttr *style)
{
    SPCSSAttr *current = getStyle(pref_path);
    sp_repr_css_merge(current, style);
    Glib::ustring css_str;
    sp_repr_css_write_string(current, css_str);
    _setRawValue(pref_path, css_str);
    sp_repr_css_attr_unref(current);
}

/**
 *  Remove an entry
 *  Make sure observers have been removed before calling
 */
void Preferences::remove(Glib::ustring const &pref_path)
{
    Inkscape::XML::Node *node = _getNode(pref_path, false);
    if (node && node->parent()) {
        node->parent()->removeChild(node);
    }
}

/**
 * Class that holds additional information for registered Observers.
 */
class Preferences::_ObserverData
{
public:
    _ObserverData(Inkscape::XML::Node *node, bool isAttr) : _node(node), _is_attr(isAttr) {}

    Inkscape::XML::Node *_node; ///< Node at which the wrapping PrefNodeObserver is registered
    bool _is_attr; ///< Whether this Observer watches a single attribute
};

Preferences::Observer::Observer(Glib::ustring const &path) :
    observed_path(path),
    _data(0)
{
}

Preferences::Observer::~Observer()
{
    // on destruction remove observer to prevent invalid references
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->removeObserver(*this);
}

void Preferences::PrefNodeObserver::notifyAttributeChanged(XML::Node &node, GQuark name, Util::ptr_shared<char>, Util::ptr_shared<char> new_value)
{
    // filter out attributes we don't watch
    gchar const *attr_name = g_quark_to_string(name);
    if ( _filter.empty() || (_filter == attr_name) ) {
        _ObserverData *d = Preferences::_get_pref_observer_data(_observer);
        Glib::ustring notify_path = _observer.observed_path;

        if (!d->_is_attr) {
            std::vector<gchar const *> path_fragments;
            notify_path.reserve(256); // this will make appending operations faster

            // walk the XML tree, saving each of the id attributes in a vector
            // we terminate when we hit the observer's attachment node, because the path to this node
            // is already stored in notify_path
            for (XML::NodeParentIterator n = &node; static_cast<XML::Node*>(n) != d->_node; ++n) {
                path_fragments.push_back(n->attribute("id"));
            }
            // assemble the elements into a path
            for (std::vector<gchar const *>::reverse_iterator i = path_fragments.rbegin(); i != path_fragments.rend(); ++i) {
                notify_path.push_back('/');
                notify_path.append(*i);
            }

            // append attribute name
            notify_path.push_back('/');
            notify_path.append(attr_name);
        }

        Entry const val = Preferences::_create_pref_value(notify_path, static_cast<void const*>(new_value.pointer()));
        _observer.notify(val);
    }
}

/**
 * Find the XML node to observe.
 */
XML::Node *Preferences::_findObserverNode(Glib::ustring const &pref_path, Glib::ustring &node_key, Glib::ustring &attr_key, bool create)
{
    // first assume that the last path element is an entry.
    _keySplit(pref_path, node_key, attr_key);

    // find the node corresponding to the "directory".
    Inkscape::XML::Node *node = _getNode(node_key, create), *child;
    for (child = node->firstChild(); child; child = child->next()) {
        // If there is a node with id corresponding to the attr key,
        // this means that the last part of the path is actually a key (folder).
        // Change values accordingly.
        if (attr_key == child->attribute("id")) {
            node = child;
            attr_key = "";
            node_key = pref_path;
            break;
        }
    }
    return node;
}

void Preferences::addObserver(Observer &o)
{
    // prevent adding the same observer twice
    if ( _observer_map.find(&o) == _observer_map.end() ) {
        Glib::ustring node_key, attr_key;
        Inkscape::XML::Node *node;
        node = _findObserverNode(o.observed_path, node_key, attr_key, false);
        if (node) {
            // set additional data
            if (o._data) {
                delete o._data;
            }
            o._data = new _ObserverData(node, !attr_key.empty());

            _observer_map[&o] = new PrefNodeObserver(o, attr_key);

            // if we watch a single pref, we want to receive notifications only for a single node
            if (o._data->_is_attr) {
                node->addObserver( *(_observer_map[&o]) );
            } else {
                node->addSubtreeObserver( *(_observer_map[&o]) );
            }
        }
    }
}

void Preferences::removeObserver(Observer &o)
{
    // prevent removing an observer which was not added
    if ( _observer_map.find(&o) != _observer_map.end() ) {
        Inkscape::XML::Node *node = o._data->_node;
        _ObserverData *priv_data = o._data;
        o._data = 0;

        if (priv_data->_is_attr) {
            node->removeObserver( *(_observer_map[&o]) );
        } else {
            node->removeSubtreeObserver( *(_observer_map[&o]) );
        }

        delete priv_data;
        priv_data = 0;
        delete _observer_map[&o];
        _observer_map.erase(&o);
    }
}


/**
 * Get the XML node corresponding to the given pref key.
 *
 * @param pref_key Preference key (path) to get.
 * @param create Whether to create the corresponding node if it doesn't exist.
 * @param separator The character used to separate parts of the pref key.
 * @return XML node corresponding to the specified key.
 *
 * Derived from former inkscape_get_repr(). Private because it assumes that the backend is
 * a flat XML file, which may not be the case e.g. if we are using GConf (in future).
 */
Inkscape::XML::Node *Preferences::_getNode(Glib::ustring const &pref_key, bool create)
{
    // verify path
    g_assert( pref_key.at(0) == '/' );
    // No longer necessary, can cause problems with input devices which have a dot in the name
    // g_assert( pref_key.find('.') == Glib::ustring::npos );

    if (_prefs_doc == NULL){
        return NULL;
    }
    Inkscape::XML::Node *node = _prefs_doc->root();
    Inkscape::XML::Node *child = NULL;
    gchar **splits = g_strsplit(pref_key.c_str(), "/", 0);

    if ( splits ) {
        for (int part_i = 0; splits[part_i]; ++part_i) {
            // skip empty path segments
            if (!splits[part_i][0]) {
                continue;
            }

            for (child = node->firstChild(); child; child = child->next()) {
                if (!strcmp(splits[part_i], child->attribute("id"))) {
                    break;
                }
            }

            // If the previous loop found a matching key, child now contains the node
            // matching the processed key part. If no node was found then it is NULL.
            if (!child) {
                if (create) {
                    // create the rest of the key
                    while(splits[part_i]) {
                        child = node->document()->createElement("group");
                        child->setAttribute("id", splits[part_i]);
                        node->appendChild(child);

                        ++part_i;
                        node = child;
                    }
                    g_strfreev(splits);
                    splits = 0;
                    return node;
                } else {
                    g_strfreev(splits);
                    splits = 0;
                    return NULL;
                }
            }

            node = child;
        }
        g_strfreev(splits);
    }
    return node;
}

void Preferences::_getRawValue(Glib::ustring const &path, gchar const *&result)
{
    // create node and attribute keys
    Glib::ustring node_key, attr_key;
    _keySplit(path, node_key, attr_key);

    // retrieve the attribute
    Inkscape::XML::Node *node = _getNode(node_key, false);
    if ( node == NULL ) {
        result = NULL;
    } else {
        gchar const *attr = node->attribute(attr_key.c_str());
        if ( attr == NULL ) {
            result = NULL;
        } else {
            result = attr;
        }
    }
}

void Preferences::_setRawValue(Glib::ustring const &path, Glib::ustring const &value)
{
    // create node and attribute keys
    Glib::ustring node_key, attr_key;
    _keySplit(path, node_key, attr_key);

    // set the attribute
    Inkscape::XML::Node *node = _getNode(node_key, true);
    node->setAttribute(attr_key.c_str(), value.c_str());
}

// The _extract* methods are where the actual wrok is done - they define how preferences are stored
// in the XML file.

bool Preferences::_extractBool(Entry const &v)
{
    gchar const *s = static_cast<gchar const *>(v._value);
    if ( !s[0] || !strcmp(s, "0") || !strcmp(s, "false") ) {
        return false;
    } else {
        return true;
    }
}

int Preferences::_extractInt(Entry const &v)
{
    gchar const *s = static_cast<gchar const *>(v._value);
    if ( !strcmp(s, "true") ) {
        return true;
    } else if ( !strcmp(s, "false") ) {
        return false;
    } else {
        return atoi(s);
    }
}

double Preferences::_extractDouble(Entry const &v)
{
    gchar const *s = static_cast<gchar const *>(v._value);
    return g_ascii_strtod(s, NULL);
}

double Preferences::_extractDouble(Entry const &v, Glib::ustring const &requested_unit)
{
    double val = _extractDouble(v);
    Glib::ustring unit = _extractUnit(v);

    if (unit.length() == 0) {
        // no unit specified, don't do conversion
        return val;
    }
    return val * (unit_table.getUnit(unit)->factor / unit_table.getUnit(requested_unit)->factor); /// \todo rewrite using Quantity class, so the standard code handles unit conversion
}

Glib::ustring Preferences::_extractString(Entry const &v)
{
    return Glib::ustring(static_cast<gchar const *>(v._value));
}

Glib::ustring Preferences::_extractUnit(Entry const &v)
{
    gchar const *str = static_cast<gchar const *>(v._value);
    gchar const *e;
    g_ascii_strtod(str, (char **) &e);
    if (e == str) {
        return "";
    }

    if (e[0] == 0) {
        /* Unitless */
        return "";
    } else {
        return Glib::ustring(e);
    }
}

guint32 Preferences::_extractColor(Entry const &v)
{
    gchar const *s = static_cast<gchar const *>(v._value);
    std::istringstream hr(s);
    guint32 color;
    if (s[0] == '#') {
        hr.ignore(1);
        hr >> std::hex >> color;
    } else {
        hr >> color;
    }
    return color;
}

SPCSSAttr *Preferences::_extractStyle(Entry const &v)
{
    SPCSSAttr *style = sp_repr_css_attr_new();
    sp_repr_css_attr_add_from_string(style, static_cast<gchar const*>(v._value));
    return style;
}

SPCSSAttr *Preferences::_extractInheritedStyle(Entry const &v)
{
    // This is the dirtiest extraction method. Generally we ignore whatever was in v._value
    // and just get the style using sp_repr_css_attr_inherited. To implement this in GConf,
    // we'll have to walk up the tree and call sp_repr_css_attr_add_from_string
    Glib::ustring node_key, attr_key;
    _keySplit(v._pref_path, node_key, attr_key);

    Inkscape::XML::Node *node = _getNode(node_key, false);
    return sp_repr_css_attr_inherited(node, attr_key.c_str());
}

// XML backend helper: Split the path into a node key and an attribute key.
void Preferences::_keySplit(Glib::ustring const &pref_path, Glib::ustring &node_key, Glib::ustring &attr_key)
{
    // everything after the last slash
    attr_key = pref_path.substr(pref_path.rfind('/') + 1, Glib::ustring::npos);
    // everything before the last slash
    node_key = pref_path.substr(0, pref_path.rfind('/'));
}

void Preferences::_reportError(Glib::ustring const &msg, Glib::ustring const &secondary)
{
    _hasError = true;
    _lastErrPrimary = msg;
    _lastErrSecondary = secondary;
    if (_errorHandler) {
        _errorHandler->handleError(msg, secondary);
    }
}

Preferences::Entry const Preferences::_create_pref_value(Glib::ustring const &path, void const *ptr)
{
    return Entry(path, ptr);
}

void Preferences::setErrorHandler(ErrorReporter* handler)
{
    _errorHandler = handler;
}

void Preferences::unload(bool save)
{
    if (_instance)
    {
        if (save) {
            _instance->save();
        }
        delete _instance;
        _instance = NULL;
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
