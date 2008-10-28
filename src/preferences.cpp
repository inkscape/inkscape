/** @file
 * @brief  Singleton class to access the preferences file - implementation
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2008 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include "preferences.h"
#include "preferences-skeleton.h"
#include "inkscape.h"
#include "xml/repr.h"
#include "xml/node-observer.h"
#include "xml/node-iterators.h"
#include "xml/attribute-record.h"
#include <cstring>
#include <vector>
#include <glibmm/fileutils.h>
#include <glibmm/i18n.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtkmm/messagedialog.h>

#define PREFERENCES_FILE_NAME "preferences.xml"

namespace Inkscape {

// private inner class definition

/**
 * @brief XML - prefs observer bridge
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
    _writable(false),
    _prefs_doc(NULL)
{
    // profile_path essentailly returns the argument prefixed by the profile directory.
    gchar *path = profile_path(NULL);
    _prefs_dir = path;
    g_free(path);
    
    path = profile_path(_prefs_basename.data());
    _prefs_filename = path;
    g_free(path);
    
    _load();
}

Preferences::~Preferences()
{
    // when the preferences are unloaded, save them
    save();
    
    // delete all PrefNodeObservers
    for (_ObsMap::iterator i = _observer_map.begin(); i != _observer_map.end(); ) {
        delete (*i++).second; // avoids reference to a deleted key
    }
    // unref XML document
    Inkscape::GC::release(_prefs_doc);
}

/**
 * @brief Load internal defaults
 *
 * In the future this will try to load the system-wide file before falling
 * back to the internal defaults.
 */
void Preferences::_loadDefaults()
{
    _prefs_doc = sp_repr_read_mem(preferences_skeleton, PREFERENCES_SKELETON_SIZE, NULL);
}

/**
 * @brief Load the user's customized preferences
 *
 * Tries to load the user's preferences.xml file. If there is none, creates it.
 * Displays dialog boxes on any errors.
 */
void Preferences::_load()
{
    _loadDefaults();
    
    Glib::ustring const not_saved = _("Inkscape will run with default settings, "
                                "and new settings will not be saved. ");
    
    // NOTE: After we upgrade to Glib 2.16, use Glib::ustring::compose
    
    // 1. Does the file exist?
    if (!g_file_test(_prefs_filename.data(), G_FILE_TEST_EXISTS)) {
        // No - we need to create one.
        // Does the profile directory exist?
        if (!g_file_test(_prefs_dir.data(), G_FILE_TEST_EXISTS)) {
            // No - create the profile directory
            if (g_mkdir(_prefs_dir.data(), 0755)) {
                // the creation failed
                //_errorDialog(Glib::ustring::compose(_("Cannot create profile directory %1."),
                //    Glib::filename_to_utf8(_prefs_dir)), not_saved);
                gchar *msg = g_strdup_printf(_("Cannot create profile directory %s."),
                    Glib::filename_to_utf8(_prefs_dir).data());
                _errorDialog(msg, not_saved);
                g_free(msg);
                return;
            }
            // create some subdirectories for user stuff
            char const *user_dirs[] = {"keys", "templates", "icons", "extensions", "palettes", NULL};
            for(int i=0; user_dirs[i]; ++i) {
                char *dir = profile_path(user_dirs[i]);
                g_mkdir(dir, 0755);
                g_free(dir);
            }
            
        } else if (!g_file_test(_prefs_dir.data(), G_FILE_TEST_IS_DIR)) {
            // The profile dir is not actually a directory
            //_errorDialog(Glib::ustring::compose(_("%1 is not a valid directory."),
            //    Glib::filename_to_utf8(_prefs_dir)), not_saved);
            gchar *msg = g_strdup_printf(_("%s is not a valid directory."),
                Glib::filename_to_utf8(_prefs_dir).data());
            _errorDialog(msg, not_saved);
            g_free(msg);
            return;
        }
        // The profile dir exists and is valid.
        if (!g_file_set_contents(_prefs_filename.data(), preferences_skeleton, PREFERENCES_SKELETON_SIZE, NULL)) {
            // The write failed.
            //_errorDialog(Glib::ustring::compose(_("Failed to create the preferences file %1."),
            //    Glib::filename_to_utf8(_prefs_filename)), not_saved);
            gchar *msg = g_strdup_printf(_("Failed to create the preferences file %s."),
                Glib::filename_to_utf8(_prefs_filename).data());
            _errorDialog(msg, not_saved);
            g_free(msg);
            return;
        }
        
        // The prefs file was just created.
        // We can return now and skip the rest of the load process.
        _writable = true;
        return;
    }
    
    // Yes, the pref file exists.
    // 2. Is it a regular file?
    if (!g_file_test(_prefs_filename.data(), G_FILE_TEST_IS_REGULAR)) {
        //_errorDialog(Glib::ustring::compose(_("The preferences file %1 is not a regular file."),
        //    Glib::filename_to_utf8(_prefs_filename)), not_saved);
        gchar *msg = g_strdup_printf(_("The preferences file %s is not a regular file."),
            Glib::filename_to_utf8(_prefs_filename).data());
        _errorDialog(msg, not_saved);
        g_free(msg);
        return;
    }
    
    // 3. Is the file readable?
    gchar *prefs_xml = NULL; gsize len = 0;
    if (!g_file_get_contents(_prefs_filename.data(), &prefs_xml, &len, NULL)) {
        //_errorDialog(Glib::ustring::compose(_("The preferences file %1 could not be read."),
        //    Glib::filename_to_utf8(_prefs_filename)), not_saved);
        gchar *msg = g_strdup_printf(_("The preferences file %s could not be read."),
            Glib::filename_to_utf8(_prefs_filename).data());
        _errorDialog(msg, not_saved);
        g_free(msg);
        return;
    }
    // 4. Is it valid XML?
    Inkscape::XML::Document *prefs_read = sp_repr_read_mem(prefs_xml, len, NULL);
    g_free(prefs_xml);
    if (!prefs_read) {
        //_errorDialog(Glib::ustring::compose(_("The preferences file %1 is not a valid XML document."),
        //    Glib::filename_to_utf8(_prefs_filename)), not_saved);
        gchar *msg = g_strdup_printf(_("The preferences file %s is not a valid XML document."),
            Glib::filename_to_utf8(_prefs_filename).data());
        _errorDialog(msg, not_saved);
        g_free(msg);
        return;
    }
    // 5. Basic sanity check: does the root element have a correct name?
    if (strcmp(prefs_read->root()->name(), "inkscape")) {
        //_errorDialog(Glib::ustring::compose(_("The file %1 is not a valid Inkscape preferences file."),
        //    Glib::filename_to_utf8(_prefs_filename)), not_saved);
        gchar *msg = g_strdup_printf(_("The file %s is not a valid Inkscape preferences file."),
            Glib::filename_to_utf8(_prefs_filename).data());
        _errorDialog(msg, not_saved);
        g_free(msg);
        Inkscape::GC::release(prefs_read);
        return;
    }
    
    // Merge the loaded prefs with defaults.
    _prefs_doc->root()->mergeFrom(prefs_read->root(), "id");
    Inkscape::GC::release(prefs_read);
    _writable = true;
}

/**
 * @brief Flush all pref changes to the XML file
 */
void Preferences::save()
{
    if (!_writable) return; // no-op if the prefs file is not writable
    
    // sp_repr_save_file uses utf-8 instead of the glib filename encoding.
    // I don't know why filenames are kept in utf-8 in Inkscape and then
    // converted to filename encoding when necessary through special functions
    // - wouldn't it be easier to keep things in the encoding they are supposed
    // to be in?
    Glib::ustring utf8name = Glib::filename_to_utf8(_prefs_filename);
    if (utf8name.empty()) return;
    sp_repr_save_file(_prefs_doc, utf8name.data());
}


// Now for the meat.

/**
 * @brief Get names of all entries in the specified path
 * @param path Preference path to query
 * @return A vector containing all entries in the given directory
 */
std::vector<Preferences::Entry> Preferences::getAllEntries(Glib::ustring const &path)
{
    std::vector<Entry> temp;
    Inkscape::XML::Node *node = _getNode(path, false);
    if (!node) return temp;
    
    // argh - purge this Util::List nonsense from XML classes fast
    Inkscape::Util::List<Inkscape::XML::AttributeRecord const> alist = node->attributeList();
    for (; alist; ++alist)
        temp.push_back( Entry(path + '/' + g_quark_to_string(alist->key), static_cast<void const*>(alist->value.pointer())) );
    return temp;
}

/**
 * @brief Get the paths to all subdirectories of the specified path
 * @param path Preference path to query
 * @return A vector containing absolute paths to all subdirectories in the given path
 */
std::vector<Glib::ustring> Preferences::getAllDirs(Glib::ustring const &path)
{
    std::vector<Glib::ustring> temp;
    Inkscape::XML::Node *node = _getNode(path, false);
    if (!node) return temp;
    
    for (Inkscape::XML::NodeSiblingIterator i = node->firstChild(); i; ++i) {
        temp.push_back(path + '/' + i->attribute("id"));
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
 * @brief Set a boolean attribute of a preference
 * @param pref_path Path of the preference to modify
 * @param value The new value of the pref attribute
 */
void Preferences::setBool(Glib::ustring const &pref_path, bool value)
{
    /// @todo Boolean values should be stored as "true" and "false",
    /// but this is not possible due to an interaction with event contexts.
    /// Investigate this in depth.
    _setRawValue(pref_path, ( value ? "1" : "0" ));
}

/**
 * @brief Set an integer attribute of a preference
 * @param pref_path Path of the preference to modify
 * @param value The new value of the pref attribute
 */
void Preferences::setInt(Glib::ustring const &pref_path, int value)
{
    gchar intstr[32];
    g_snprintf(intstr, 32, "%d", value);
    _setRawValue(pref_path, intstr);
}

/**
 * @brief Set a floating point attribute of a preference
 * @param pref_path Path of the preference to modify
 * @param value The new value of the pref attribute
 */
void Preferences::setDouble(Glib::ustring const &pref_path, double value)
{
    gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
    g_ascii_dtostr(buf, G_ASCII_DTOSTR_BUF_SIZE, value);
    _setRawValue(pref_path, buf);
}

/**
 * @brief Set a string attribute of a preference
 * @param pref_path Path of the preference to modify
 * @param value The new value of the pref attribute
 */
void Preferences::setString(Glib::ustring const &pref_path, Glib::ustring const &value)
{
    _setRawValue(pref_path, value.data());
}

void Preferences::setStyle(Glib::ustring const &pref_path, SPCSSAttr *style)
{
    gchar *css_str = sp_repr_css_write_string(style);
    _setRawValue(pref_path, css_str);
    g_free(css_str);
}

void Preferences::mergeStyle(Glib::ustring const &pref_path, SPCSSAttr *style)
{
    SPCSSAttr *current = getStyle(pref_path);
    sp_repr_css_merge(current, style);
    gchar *css_str = sp_repr_css_write_string(current);
    _setRawValue(pref_path, css_str);
    g_free(css_str);
    sp_repr_css_attr_unref(current);
}


// Observer stuff
namespace {

/**
 * @brief Structure that holds additional information for registered Observers
 */
struct _ObserverData {
    Inkscape::XML::Node *_node; ///< Node at which the wrapping PrefNodeObserver is registered
    bool _is_attr; ///< Whether this Observer watches a single attribute
};

} // anonymous namespace

Preferences::Observer::Observer(Glib::ustring const &path) :
    observed_path(path)
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
    if ( !_filter.empty() && _filter != attr_name ) return;
    
    _ObserverData *d = static_cast<_ObserverData*>(Preferences::_get_pref_observer_data(_observer));
    Glib::ustring notify_path = _observer.observed_path;
    
    if (!d->_is_attr) {
        std::vector<gchar const *> path_fragments;
        notify_path.reserve(256); // this will make appending operations faster
        
        // walk the XML tree, saving each of the id attributes in a vector
        // we terminate when we hit the observer's attachment node, because the path to this node
        // is already stored in notify_path
        for (XML::NodeParentIterator n = &node; static_cast<XML::Node*>(n) != d->_node; ++n)
            path_fragments.push_back(n->attribute("id"));
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

/**
 * @brief Find the XML node to observe
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
    if ( _observer_map.find(&o) != _observer_map.end() ) return;
    
    Glib::ustring node_key, attr_key;
    Inkscape::XML::Node *node;
    node = _findObserverNode(o.observed_path, node_key, attr_key, false);
    if (!node) return;
    
    // set additional data
    _ObserverData *priv_data = new _ObserverData;
    priv_data->_node = node;
    priv_data->_is_attr = !attr_key.empty();
    o._data = static_cast<void*>(priv_data);
    
    _observer_map[&o] = new PrefNodeObserver(o, attr_key);
    
    // if we watch a single pref, we want to receive notifications only for a single node
    if (priv_data->_is_attr) {
        node->addObserver( *(_observer_map[&o]) );
    } else {
        node->addSubtreeObserver( *(_observer_map[&o]) );
    }
}

void Preferences::removeObserver(Observer &o)
{
    // prevent removing an observer which was not added
    if ( _observer_map.find(&o) == _observer_map.end() ) return;
    Inkscape::XML::Node *node = static_cast<_ObserverData*>(o._data)->_node;
    _ObserverData *priv_data = static_cast<_ObserverData*>(o._data);
    o._data = NULL;
    
    if (priv_data->_is_attr)
        node->removeObserver( *(_observer_map[&o]) );
    else
        node->removeSubtreeObserver( *(_observer_map[&o]) );

    delete priv_data;
    delete _observer_map[&o];
    _observer_map.erase(&o);
}


/**
 * @brief Get the XML node corresponding to the given pref key
 * @param pref_key Preference key (path) to get
 * @param create Whether to create the corresponding node if it doesn't exist
 * @param separator The character used to separate parts of the pref key
 * @return XML node corresponding to the specified key
 *
 * Derived from former inkscape_get_repr(). Private because it assumes that the backend is
 * a flat XML file, which may not be the case e.g. if we are using GConf (in future).
 */
Inkscape::XML::Node *Preferences::_getNode(Glib::ustring const &pref_key, bool create)
{
    // verify path
    g_assert( pref_key.at(0) == '/' );
    g_assert( pref_key.find('.') == Glib::ustring::npos );

    Inkscape::XML::Node *node = _prefs_doc->root(), *child = NULL;
    gchar **splits = g_strsplit(pref_key.data(), "/", 0);
    
    if ( splits == NULL ) return node;
    
    for (int part_i = 0; splits[part_i]; ++part_i) {
        // skip empty path segments
        if (!splits[part_i][0]) continue;
        
        for (child = node->firstChild(); child; child = child->next())
            if (!strcmp(splits[part_i], child->attribute("id"))) break;
        
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
                return node;
            } else {
                return NULL;
            }
        }

        node = child;
    }
    g_strfreev(splits);
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
        gchar const *attr = node->attribute(attr_key.data());
        if ( attr == NULL ) {
            result = NULL;
        } else {
            result = attr;
        }
    }
}

void Preferences::_setRawValue(Glib::ustring const &path, gchar const *value)
{
    // create node and attribute keys
    Glib::ustring node_key, attr_key;
    _keySplit(path, node_key, attr_key);
    
    // set the attribute
    Inkscape::XML::Node *node = _getNode(node_key, true);
    node->setAttribute(attr_key.data(), value);
}

// The _extract* methods are where the actual wrok is done - they define how preferences are stored
// in the XML file.

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
    return sp_repr_css_attr_inherited(node, attr_key.data());
}

// XML backend helper: Split the path into a node key and an attribute key.
void Preferences::_keySplit(Glib::ustring const &pref_path, Glib::ustring &node_key, Glib::ustring &attr_key)
{
    // everything after the last slash
    attr_key = pref_path.substr(pref_path.rfind('/') + 1, Glib::ustring::npos);
    // everything before the last slash
    node_key = pref_path.substr(0, pref_path.rfind('/'));
}

void Preferences::_errorDialog(Glib::ustring const &msg, Glib::ustring const &secondary)
{
    if (Preferences::use_gui) {
        Gtk::MessageDialog err(
            msg, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
        err.set_secondary_text(secondary);
        err.run();
    } else {
        g_message("%s", msg.data());
        g_message("%s", secondary.data());
    }
}

Preferences::Entry const Preferences::_create_pref_value(Glib::ustring const &path, void const *ptr)
{
    return Entry(path, ptr);
}

bool Preferences::use_gui = true;
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
