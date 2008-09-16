/** @file
 * @brief  Singleton class to access the preferences file - implementation
 *
 * Authors:
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
#include <glibmm/fileutils.h>
#include <glibmm/i18n.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtkmm/messagedialog.h>

#define PREFERENCES_FILE_NAME "preferences.xml"

namespace Inkscape {

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
    
    Glib::ustring not_saved = _("Inkscape will run with default settings, "
                                "and new settings will not be saved. ");
    
    // 1. Does the file exist?
    if (!g_file_test(_prefs_filename.data(), G_FILE_TEST_EXISTS)) {
        // No - we need to create one.
        // Does the profile directory exist?
        if (!g_file_test(_prefs_dir.data(), G_FILE_TEST_EXISTS)) {
            // No - create the profile directory
            if (g_mkdir(_prefs_dir.data(), 0755)) {
                // the creation failed
                _errorDialog(Glib::ustring::compose(_("Cannot create profile directory %1."),
                    Glib::filename_to_utf8(_prefs_dir)), not_saved);
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
            _errorDialog(Glib::ustring::compose(_("%1 is not a valid directory."),
                Glib::filename_to_utf8(_prefs_dir)), not_saved);
            return;
        }
        // The profile dir exists and is valid.
        if (!g_file_set_contents(_prefs_filename.data(), preferences_skeleton, PREFERENCES_SKELETON_SIZE, NULL)) {
            // The write failed.
            _errorDialog(Glib::ustring::compose(_("Failed to create the preferences file %1."),
                Glib::filename_to_utf8(_prefs_filename)), not_saved);
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
        _errorDialog(Glib::ustring::compose(_("The preferences file %1 is not a regular file."),
            Glib::filename_to_utf8(_prefs_filename)), not_saved);
        return;
    }
    
    // 3. Is the file readable?
    gchar *prefs_xml = NULL; gsize len = 0;
    if (!g_file_get_contents(_prefs_filename.data(), &prefs_xml, &len, NULL)) {
        _errorDialog(Glib::ustring::compose(_("The preferences file %1 could not be read."),
            Glib::filename_to_utf8(_prefs_filename)), not_saved);
        return;
    }
    // 4. Is it valid XML?
    Inkscape::XML::Document *prefs_read = sp_repr_read_mem(prefs_xml, len, NULL);
    g_free(prefs_xml);
    if (!prefs_read) {
        _errorDialog(Glib::ustring::compose(_("The preferences file %1 is not a valid XML document."),
            Glib::filename_to_utf8(_prefs_filename)), not_saved);
        return;
    }
    // 5. Basic sanity check: does the root element have a correct name?
    if (strcmp(prefs_read->root()->name(), "inkscape")) {
        _errorDialog(Glib::ustring::compose(_("The file %1 is not a valid Inkscape preferences file."),
            Glib::filename_to_utf8(_prefs_filename)), not_saved);
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
    // converted to filename encoding when necessary through sepcial functions
    // - wouldn't it be easier to keep things in the encoding they are supposed
    // to be in?
    Glib::ustring utf8name = Glib::filename_from_utf8(_prefs_filename);
    if (utf8name.empty()) return;
    sp_repr_save_file(_prefs_doc, utf8name.data());
}

void Preferences::addPrefsObserver(Inkscape::XML::NodeObserver *observer)
{
    _prefs_doc->addSubtreeObserver(*observer);
}



// Now for the meat.
// Most of the logic is similar to former prefs-utils.cpp


/**
 * @brief Check for the existence of a given pref key
 * @param pref_key Preference key to check
 * @return True if the key exists, false otherwise
 */
bool Preferences::exists(Glib::ustring const &pref_key)
{
    return _getNode(pref_key) != NULL;
}

/**
 * @brief Get the number of sub-preferences of a given pref
 * @param pref_key Preference key to check
 * @return Number of sub-preferences
 *
 * Note: This does not count attributes, only child preferences.
 */
unsigned int Preferences::childCount(Glib::ustring const &pref_key)
{
    Inkscape::XML::Node *node = _getNode(pref_key);
    return ( node ? node->childCount() : 0 );
}

/**
 * @brief Get the key of the n-th sub-preference of the specified pref
 * @param father_key Parent key
 * @param n The zero-based index of the pref key to retrieve
 * @return The key of the n-th sub-preference
 */
Glib::ustring Preferences::getNthChild(Glib::ustring const &father_key, unsigned int n)
{
    Inkscape::XML::Node *node = _getNode(father_key), *child;
    if (!node) return "";
    child = node->nthChild(n);
    if (!child) return "";
    if (child->attribute("id")) {
        Glib::ustring child_key = father_key;
        child_key += '.';
        child_key += child->attribute("id");
        return child_key;
    }
    return "";
}


/**
 * @brief Create the preference with the specified key
 * @return True if the node was created, false if it already existed
 *
 * This method is redundant, because the setters automatically create prefs
 * if they don't already exist. It is only left to accomodate some legacy code
 * which manipulates the DOM of the preferences file directly.
 */
bool Preferences::create(Glib::ustring const &pref_key)
{
    if (_getNode(pref_key)) return false;
    _getNode(pref_key, true);
    return true;
}

// getter methods

/**
 * @brief Get a boolean attribute of a preference
 * @param pref_key Key of he preference to retrieve
 * @param attr Attribute to retrieve
 * @param def The default value to return if the preference is not set
 * @return The retrieved value
 */
bool Preferences::getBool(Glib::ustring const &pref_key, Glib::ustring const &attr, bool def)
{
    Inkscape::XML::Node *node = _getNode(pref_key);
    if (!node) return def;
    gchar const *rawstr = node->attribute(attr.data());
    if(!rawstr || !rawstr[0]) return def;
    Glib::ustring str = rawstr;
    
    // This is to handle legacy preferences using ints as booleans
    if (str == "true" || str == "1") return true;
    return false;
}


/**
 * @brief Get an integer attribute of a preference
 * @param pref_key Key of he preference to retrieve
 * @param attr Attribute to retrieve
 * @param def The default value to return if the preference is not set
 * @return The retrieved value
 */
int Preferences::getInt(Glib::ustring const &pref_key, Glib::ustring const &attr, int def)
{
    Inkscape::XML::Node *node = _getNode(pref_key);
    if (!node) return def;
    gchar const *rawstr = node->attribute(attr.data());
    if (!rawstr || !rawstr[0]) return def;
    Glib::ustring str = rawstr;
    // Protection against leftover getInt calls when the value is in fact a boolean
    if (str == "true") return 1;
    if (str == "false") return 0;
    return atoi(str.data());
}

int Preferences::getIntLimited(Glib::ustring const &pref_key, Glib::ustring const &attr, int def, int min, int max)
{
    int value = getInt(pref_key, attr, def);
    return ( value >= min && value <= max ? value : def);
}

/**
 * @brief Get a floating point attribute of a preference
 * @param pref_key Key of he preference to retrieve
 * @param attr Attribute to retrieve
 * @param def The default value to return if the preference is not set
 * @return The retrieved value
 */
double Preferences::getDouble(Glib::ustring const &pref_key, Glib::ustring const &attr, double def)
{
    Inkscape::XML::Node *node = _getNode(pref_key);
    if (!node) return def;
    gchar const *str = node->attribute(attr.data());
    if (!str) return def;
    return g_ascii_strtod(str, NULL);
}

double Preferences::getDoubleLimited(Glib::ustring const &pref_key, Glib::ustring const &attr, double def, double min, double max)
{
    double value = getDouble(pref_key, attr, def);
    return ( value >= min && value <= max ? value : def);
}

/**
 * @brief Get a string attribute of a preference
 * @param pref_key Key of he preference to retrieve
 * @param attr Attribute to retrieve
 * @param def The default value to return if the preference is not set
 * @return The retrieved value
 */
Glib::ustring Preferences::getString(Glib::ustring const &pref_key, Glib::ustring const &attr)
{
    Inkscape::XML::Node *node = _getNode(pref_key);
    if (!node) return "";
    gchar const *str = node->attribute(attr.data());
    if (!str) return "";
    return Glib::ustring(str);
}


// setter methods

/**
 * @brief Set a boolean attribute of a preference
 * @param pref_key Key of the preference to modify
 * @param attr Attribute to set
 * @param value The new value of the pref attribute
 */
void Preferences::setBool(Glib::ustring const &pref_key, Glib::ustring const &attr, bool value)
{
    Inkscape::XML::Node *node = _getNode(pref_key, true);
    node->setAttribute(attr.data(), ( value ? "true" : "false" ));
}

/**
 * @brief Set an integer attribute of a preference
 * @param pref_key Key of the preference to modify
 * @param attr Attribute to set
 * @param value The new value of the pref attribute
 */
void Preferences::setInt(Glib::ustring const &pref_key, Glib::ustring const &attr, int value)
{
    Inkscape::XML::Node *node = _getNode(pref_key, true);
    gchar intstr[32];
    g_snprintf(intstr, 32, "%d", value);
    node->setAttribute(attr.data(), intstr);
}

/**
 * @brief Set a floating point attribute of a preference
 * @param pref_key Key of the preference to modify
 * @param attr Attribute to set
 * @param value The new value of the pref attribute
 */
void Preferences::setDouble(Glib::ustring const &pref_key, Glib::ustring const &attr, double value)
{
    Inkscape::XML::Node *node = _getNode(pref_key, true);
    sp_repr_set_svg_double(node, attr.data(), value);
    /*
    gchar dblstr[32];
    g_snprintf(dblstr, 32, "%g", value);
    node->setAttribute(attr, dblstr);
    */
}

/**
 * @brief Set a string attribute of a preference
 * @param pref_key Key of the preference to modify
 * @param attr Attribute to set
 * @param value The new value of the pref attribute
 */
void Preferences::setString(Glib::ustring const &pref_key, Glib::ustring const &attr, Glib::ustring const &value)
{
    Inkscape::XML::Node *node = _getNode(pref_key, true);
    node->setAttribute(attr.data(), value.data());
}

/**
 * @brief Get the XML node corresponding to the given pref key
 * @param pref_key Preference key (path) to get
 * @param create Whether to create the corresponding node if it doesn't exist
 * @return XML node corresponding to the specified key
 *
 * The separator for key components is '.' (a dot). Derived from former
 * inkscape_get_repr().
 */
Inkscape::XML::Node *Preferences::_getNode(Glib::ustring const &pref_key, bool create)
{
    Inkscape::XML::Node *node = _prefs_doc->root(), *child = NULL;
    gchar **splits = g_strsplit(pref_key.data(), ".", 0);
    int part_i = 0;
    
    while(splits[part_i]) {
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
        
        ++part_i;
        node = child;
    }
    g_strfreev(splits);
    return node;
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

bool Preferences::use_gui = true;
Preferences *Preferences::_instance = NULL;


} // namespace Inkscape
 
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
