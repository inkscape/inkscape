/** \file
 * Keyboard shortcut processing.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Peter Moulder <pmoulder@mail.csse.monash.edu.au>
 *
 * Copyright (C) 2005  Monash University
 * Copyright (C) 2005  MenTaLguY <mental@rydia.net>
 *
 * You may redistribute and/or modify this file under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vector>
#include <cstring>
#include <string>
#include <map>

#include "shortcuts.h"
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <glibmm/convert.h>
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>

#include "helper/action.h"
#include "helper/action-context.h"
#include "io/sys.h"
#include "io/resource.h"
#include "verbs.h"
#include "xml/node-iterators.h"
#include "xml/repr.h"
#include "document.h"
#include "preferences.h"
#include "ui/tools/tool-base.h"
#include "inkscape.h"
#include "desktop.h"
#include "path-prefix.h"
#include "ui/dialog/filedialog.h"

using namespace Inkscape;

using Inkscape::IO::Resource::get_path;
using Inkscape::IO::Resource::SYSTEM;
using Inkscape::IO::Resource::USER;
using Inkscape::IO::Resource::KEYS;

static void try_shortcuts_file(char const *filename);
static void read_shortcuts_file(char const *filename, bool const is_user_set=false);

unsigned int sp_shortcut_get_key(unsigned int const shortcut);
GdkModifierType sp_shortcut_get_modifiers(unsigned int const shortcut);

/* Returns true if action was performed */

bool
sp_shortcut_invoke(unsigned int shortcut, Inkscape::UI::View::View *view)
{
    Inkscape::Verb *verb = sp_shortcut_get_verb(shortcut);
    if (verb) {
        SPAction *action = verb->get_action(Inkscape::ActionContext(view));
        if (action) {
            sp_action_perform(action, NULL);
            return true;
        }
    }
    return false;
}

static std::map<unsigned int, Inkscape::Verb * > *verbs = NULL;
static std::map<Inkscape::Verb *, unsigned int> *primary_shortcuts = NULL;
static std::map<Inkscape::Verb *, unsigned int> *user_shortcuts = NULL;

void sp_shortcut_init()
{

    verbs = new std::map<unsigned int, Inkscape::Verb * >();
    primary_shortcuts = new std::map<Inkscape::Verb *, unsigned int>();
    user_shortcuts = new std::map<Inkscape::Verb *, unsigned int>();

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring shortcutfile = prefs->getString("/options/kbshortcuts/shortcutfile");
    if (shortcutfile.empty()) {
        shortcutfile  = Glib::ustring(get_path(SYSTEM, KEYS, "default.xml"));
    }

    read_shortcuts_file(shortcutfile.c_str());
    try_shortcuts_file(get_path(USER, KEYS, "default.xml"));
}

static void try_shortcuts_file(char const *filename) {
    using Inkscape::IO::file_test;

    /* ah, if only we had an exception to catch... (permission, forgiveness) */
    if (file_test(filename, G_FILE_TEST_EXISTS)) {
        read_shortcuts_file(filename, true);
    }
}

/*
 *  Inkscape expects to add the Shift modifier to any accel_keys create with Shift
 *  For exmaple on en_US keyboard <Shift>+6 = "&" - in this case return <Shift>+&
 *  See get_group0_keyval() for explanation on why
 */
unsigned int sp_gdkmodifier_to_shortcut(guint accel_key, Gdk::ModifierType gdkmodifier, guint hardware_keycode) {


    unsigned int shortcut = 0;
    GdkEventKey event;
    event.state = gdkmodifier;
    event.keyval = accel_key;
    event.hardware_keycode = hardware_keycode;
    guint keyval = Inkscape::UI::Tools::get_group0_keyval (&event);

    shortcut = accel_key |
               ( (gdkmodifier & GDK_SHIFT_MASK) || ( accel_key != keyval) ?
                 SP_SHORTCUT_SHIFT_MASK : 0 ) |
               ( gdkmodifier & GDK_CONTROL_MASK ?
                 SP_SHORTCUT_CONTROL_MASK : 0 ) |
               ( gdkmodifier & GDK_MOD1_MASK ?
                 SP_SHORTCUT_ALT_MASK : 0 );

    return shortcut;
}

Glib::ustring sp_shortcut_to_label(unsigned int const shortcut) {

    Glib::ustring modifiers = "";

    if (shortcut & SP_SHORTCUT_CONTROL_MASK)
        modifiers += "Ctrl,";
    if (shortcut & SP_SHORTCUT_SHIFT_MASK)
        modifiers += "Shift,";
    if (shortcut & SP_SHORTCUT_ALT_MASK)
        modifiers += "Alt,";

    if(modifiers.length() > 0 &&
            modifiers.find(',',modifiers.length()-1)!=modifiers.npos) {
        modifiers.erase(modifiers.length()-1, 1);
    }

    return modifiers;
}

/*
 * REmove all shortucts from the users file
 */

void sp_shortcuts_delete_all_from_file() {


    char const *filename = get_path(USER, KEYS, "default.xml");

    XML::Document *doc=sp_repr_read_file(filename, NULL);
    if (!doc) {
        g_warning("Unable to read keys file %s", filename);
        return;
    }

    XML::Node *root=doc->root();
    g_return_if_fail(!strcmp(root->name(), "keys"));

    XML::Node *iter=root->firstChild();
    while (iter) {

        if (strcmp(iter->name(), "bind")) {
            // some unknown element, do not complain
            iter = iter->next();
            continue;
        }

        // Delete node
        sp_repr_unparent(iter);
        iter=root->firstChild();
    }


    sp_repr_save_file(doc, filename, NULL);

    GC::release(doc);
}

Inkscape::XML::Document *sp_shortcut_create_template_file(char const *filename) {

    gchar const *buffer =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?> "
            "<keys name=\"My custom shortcuts\">"
            "</keys>";

    Inkscape::XML::Document *doc = sp_repr_read_mem(buffer, strlen(buffer), NULL);
    sp_repr_save_file(doc, filename, NULL);

    return sp_repr_read_file(filename, NULL);
}

/*
 * Get a list of keyboard shortcut files names and paths from the system and users paths
 * Dont add the users custom keyboards file
 */
void sp_shortcut_get_file_names(std::vector<Glib::ustring> *names, std::vector<Glib::ustring> *paths) {

    std::list<gchar *> sources;
    sources.push_back( Inkscape::Application::profile_path("keys") );
    sources.push_back( g_strdup(INKSCAPE_KEYSDIR) );

    // loop through possible keyboard shortcut file locations.
    while (!sources.empty()) {
        gchar *dirname = sources.front();
        if ( Inkscape::IO::file_test( dirname, G_FILE_TEST_EXISTS )
            && Inkscape::IO::file_test( dirname, G_FILE_TEST_IS_DIR )) {
            GError *err = 0;
            GDir *directory = g_dir_open(dirname, 0, &err);
            if (!directory) {
                gchar *safeDir = Inkscape::IO::sanitizeString(dirname);
                g_warning(_("Keyboard directory (%s) is unavailable."), safeDir);
                g_free(safeDir);
            } else {
                gchar *filename = 0;
                while ((filename = (gchar *) g_dir_read_name(directory)) != NULL) {
                    gchar* lower = g_ascii_strdown(filename, -1);
                    if (!strcmp(dirname, Inkscape::Application::profile_path("keys")) &&
                            !strcmp(lower, "default.xml")) {
                        // Dont add the users custom keys file
                        continue;
                    }
                    if (!strcmp(dirname, INKSCAPE_KEYSDIR) &&
                            !strcmp(lower, "inkscape.xml")) {
                        // Dont add system inkscape.xml (since its a duplicate? of dfefault.xml)
                        continue;
                    }
                    if (g_str_has_suffix(lower, ".xml")) {
                        gchar* full = g_build_filename(dirname, filename, NULL);
                        if (!Inkscape::IO::file_test(full, G_FILE_TEST_IS_DIR)) {

                        	// Get the "key name" from the root element of each file
                            XML::Document *doc=sp_repr_read_file(full, NULL);
                            if (!doc) {
                                g_warning("Unable to read keyboard shortcut file %s", full);
                                continue;
                            }
                            XML::Node *root=doc->root();
                            if (strcmp(root->name(), "keys")) {
                                g_warning("Not a shortcut keys file %s", full);
                                Inkscape::GC::release(doc);
                                continue;
                            }

                            gchar const *name=root->attribute("name");
                            Glib::ustring label(filename);
                            if (name) {
                            	label = Glib::ustring(name)+ " (" + filename + ")";
                            }

                            if (!strcmp(filename, "default.xml")) {
                                paths->insert(paths->begin(), full);
                                names->insert(names->begin(), label.c_str());
                            } else {
                                paths->push_back(full);
                                names->push_back(label.c_str());
                            }

                            Inkscape::GC::release(doc);
                        }
                        g_free(full);
                    }
                    g_free(lower);
                }
                g_dir_close(directory);
            }
        }

        g_free(dirname);
        sources.pop_front();
    }

}

Glib::ustring sp_shortcut_get_file_path()
{
    //# Get the current directory for finding files
    Glib::ustring open_path;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    Glib::ustring attr = prefs->getString("/dialogs/save_export/path");
    if (!attr.empty()) open_path = attr;

    //# Test if the open_path directory exists
    if (!Inkscape::IO::file_test(open_path.c_str(),
              (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)))
        open_path = "";

    if (open_path.empty()) {
        /* Grab document directory */
        const gchar* docURI = SP_ACTIVE_DOCUMENT->getURI();
        if (docURI) {
            open_path = Glib::path_get_dirname(docURI);
            open_path.append(G_DIR_SEPARATOR_S);
        }
    }

    //# If no open path, default to our home directory
    if (open_path.empty())
    {
        open_path = g_get_home_dir();
        open_path.append(G_DIR_SEPARATOR_S);
    }

    return open_path;
}

//static Inkscape::UI::Dialog::FileSaveDialog * saveDialog = NULL;

void sp_shortcut_file_export()
{
    Glib::ustring open_path = sp_shortcut_get_file_path();
    open_path.append("shortcut_keys.xml");

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    Glib::ustring filename;

    Inkscape::UI::Dialog::FileSaveDialog *saveDialog =
       Inkscape::UI::Dialog::FileSaveDialog::create(
            *(desktop->getToplevel()),
            open_path,
        Inkscape::UI::Dialog::CUSTOM_TYPE,
        _("Select a filename for exporting"),
        "",
        "",
        Inkscape::Extension::FILE_SAVE_METHOD_SAVE_AS
        );
    saveDialog->addFileType("All Files", "*");

    bool success = saveDialog->show();
    if (!success) {
        delete saveDialog;
        return;
    }

    Glib::ustring fileName = saveDialog->getFilename();
    if (fileName.size() > 0) {
        Glib::ustring newFileName = Glib::filename_to_utf8(fileName);
        sp_shortcut_file_export_do(newFileName.c_str());
    }

    delete saveDialog;
}

bool sp_shortcut_file_import() {

    Glib::ustring open_path = sp_shortcut_get_file_path();

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::UI::Dialog::FileOpenDialog *importFileDialog =
              Inkscape::UI::Dialog::FileOpenDialog::create(
                 *desktop->getToplevel(),
                 open_path,
                 Inkscape::UI::Dialog::CUSTOM_TYPE,
                 _("Select a file to import"));
        importFileDialog->addFilterMenu("All Files", "*");

    //# Show the dialog
    bool const success = importFileDialog->show();

    if (!success) {
        delete importFileDialog;
        return false;
    }

    Glib::ustring fileName = importFileDialog->getFilename();
    sp_shortcut_file_import_do(fileName.c_str());

    delete importFileDialog;

    return true;
}

void sp_shortcut_file_import_do(char const *importname) {

    XML::Document *doc=sp_repr_read_file(importname, NULL);
    if (!doc) {
        g_warning("Unable to read keyboard shortcut file %s", importname);
        return;
    }

    char const *filename = get_path(USER, KEYS, "default.xml");
    sp_repr_save_file(doc, filename, NULL);

    GC::release(doc);

    sp_shortcut_init();
}

void sp_shortcut_file_export_do(char const *exportname) {

    char const *filename = get_path(USER, KEYS, "default.xml");

    XML::Document *doc=sp_repr_read_file(filename, NULL);
    if (!doc) {
        g_warning("Unable to read keyboard shortcut file %s", filename);
        return;
    }

    sp_repr_save_file(doc, exportname, NULL);

    GC::release(doc);
}
/*
 * Add or delete a shortcut to the users default.xml keys file
 * @param addremove - when true add/override a shortcut, when false remove shortcut
 * @param addshift - when true addthe Shifg modifier to the non-display element
 *
 * Shortcut file consists of pairs of bind elements :
 * Element (a) is used for shortcut display in menus (display="True") and contains the gdk_keyval_name of the shortcut key
 * Element (b) is used in shortcut lookup and contains an uppercase version of the gdk_keyval_name,
 *   or a gdk_keyval_name name and the "Shift" modifier for Shift altered hardware code keys (see get_group0_keyval() for explanation)
 */
void sp_shortcut_delete_from_file(char const * /*action*/, unsigned int const shortcut) {

    char const *filename = get_path(USER, KEYS, "default.xml");

    XML::Document *doc=sp_repr_read_file(filename, NULL);
    if (!doc) {
        g_warning("Unable to read keyboard shortcut file %s", filename);
        return;
    }

    gchar *key = gdk_keyval_name (sp_shortcut_get_key(shortcut));
    std::string modifiers = sp_shortcut_to_label(shortcut & (SP_SHORTCUT_MODIFIER_MASK));

    if (!key) {
        g_warning("Unknown key for shortcut %u", shortcut);
        return;
    }

    //g_message("Removing key %s, mods %s action %s", key, modifiers.c_str(), action);

    XML::Node *root=doc->root();
    g_return_if_fail(!strcmp(root->name(), "keys"));
    XML::Node *iter=root->firstChild();
    while (iter) {

        if (strcmp(iter->name(), "bind")) {
            // some unknown element, do not complain
            iter = iter->next();
            continue;
        }

        gchar const *verb_name=iter->attribute("action");
        if (!verb_name) {
            iter = iter->next();
            continue;
        }

        gchar const *keyval_name = iter->attribute("key");
        if (!keyval_name || !*keyval_name) {
            // that's ok, it's just listed for reference without assignment, skip it
            iter = iter->next();
            continue;
        }

        if (Glib::ustring(key).lowercase() != Glib::ustring(keyval_name).lowercase()) {
            // If deleting, then delete both the upper and lower case versions
            iter = iter->next();
            continue;
        }

        gchar const *modifiers_string = iter->attribute("modifiers");
        if ((modifiers_string && !strcmp(modifiers.c_str(), modifiers_string)) ||
                (!modifiers_string && modifiers.empty())) {
            //Looks like a match
            // Delete node
            sp_repr_unparent(iter);
            iter = root->firstChild();
            continue;
        }
        iter = iter->next();
    }

    sp_repr_save_file(doc, filename, NULL);

    GC::release(doc);

}

void sp_shortcut_add_to_file(char const *action, unsigned int const shortcut) {

    char const *filename = get_path(USER, KEYS, "default.xml");

    XML::Document *doc=sp_repr_read_file(filename, NULL);
    if (!doc) {
        g_warning("Unable to read keyboard shortcut file %s, creating ....", filename);
        doc = sp_shortcut_create_template_file(filename);
        if (!doc) {
            g_warning("Unable to create keyboard shortcut file %s", filename);
            return;
        }
    }

    gchar *key = gdk_keyval_name (sp_shortcut_get_key(shortcut));
    std::string modifiers = sp_shortcut_to_label(shortcut & (SP_SHORTCUT_MODIFIER_MASK));

    if (!key) {
        g_warning("Unknown key for shortcut %u", shortcut);
        return;
    }

    //g_message("Adding key %s, mods %s action %s", key, modifiers.c_str(), action);

    // Add node
    Inkscape::XML::Node *newnode;
    newnode = doc->createElement("bind");
    newnode->setAttribute("key", key);
    if (!modifiers.empty()) {
        newnode->setAttribute("modifiers", modifiers.c_str());
    }
    newnode->setAttribute("action", action);
    newnode->setAttribute("display", "true");

    doc->root()->appendChild(newnode);

    if (strlen(key) == 1) {
        // Add another uppercase version if a character
        Inkscape::XML::Node *newnode;
        newnode = doc->createElement("bind");
        newnode->setAttribute("key", Glib::ustring(key).uppercase().c_str());
        if (!modifiers.empty()) {
            newnode->setAttribute("modifiers", modifiers.c_str());
        }

        newnode->setAttribute("action", action);
        doc->root()->appendChild(newnode);
    }

    sp_repr_save_file(doc, filename, NULL);

    GC::release(doc);

}
static void read_shortcuts_file(char const *filename, bool const is_user_set) {
    XML::Document *doc=sp_repr_read_file(filename, NULL);
    if (!doc) {
        g_warning("Unable to read keys file %s", filename);
        return;
    }

    XML::Node const *root=doc->root();
    g_return_if_fail(!strcmp(root->name(), "keys"));
    XML::NodeConstSiblingIterator iter=root->firstChild();
    for ( ; iter ; ++iter ) {
        bool is_primary;

        if (!strcmp(iter->name(), "bind")) {
            is_primary = iter->attribute("display") && strcmp(iter->attribute("display"), "false") && strcmp(iter->attribute("display"), "0");
        } else {
            // some unknown element, do not complain
            continue;
        }

        gchar const *verb_name=iter->attribute("action");
        if (!verb_name) {
            g_warning("Missing verb name (action= attribute) for shortcut");
            continue;
        }

        Inkscape::Verb *verb=Inkscape::Verb::getbyid(verb_name);
        if (!verb) {
            g_warning("Unknown verb name: %s", verb_name);
            continue;
        }

        gchar const *keyval_name=iter->attribute("key");
        if (!keyval_name || !*keyval_name) {
            // that's ok, it's just listed for reference without assignment, skip it
            continue;
        }

        guint keyval=gdk_keyval_from_name(keyval_name);
        if (keyval == GDK_KEY_VoidSymbol || keyval == 0) {
            g_warning("Unknown keyval %s for %s", keyval_name, verb_name);
            continue;
        }

        guint modifiers=0;

        gchar const *modifiers_string=iter->attribute("modifiers");
        if (modifiers_string) {
            gchar const *iter=modifiers_string;
            while (*iter) {
                size_t length=strcspn(iter, ",");
                gchar *mod=g_strndup(iter, length);
                if (!strcmp(mod, "Control") || !strcmp(mod, "Ctrl")) {
                    modifiers |= SP_SHORTCUT_CONTROL_MASK;
                } else if (!strcmp(mod, "Shift")) {
                    modifiers |= SP_SHORTCUT_SHIFT_MASK;
                } else if (!strcmp(mod, "Alt")) {
                    modifiers |= SP_SHORTCUT_ALT_MASK;
                } else {
                    g_warning("Unknown modifier %s for %s", mod, verb_name);
                }
                g_free(mod);
                iter += length;
                if (*iter) iter++;
            }
        }

        sp_shortcut_set(keyval | modifiers, verb, is_primary, is_user_set);
    }

    GC::release(doc);
}

/**
 * Removes a keyboard shortcut for the given verb.
 * (Removes any existing binding for the given shortcut, including appropriately
 * adjusting sp_shortcut_get_primary if necessary.)*
 */
void
sp_shortcut_unset(unsigned int const shortcut)
{
    if (!verbs) sp_shortcut_init();

    Inkscape::Verb *verb = (*verbs)[shortcut];

    /* Maintain the invariant that sp_shortcut_get_primary(v) returns either 0 or a valid shortcut for v. */
    if (verb) {

        (*verbs)[shortcut] = 0;

        unsigned int const old_primary = (*primary_shortcuts)[verb];
        if (old_primary == shortcut) {
            (*primary_shortcuts)[verb] = 0;
        }

    }
}

GtkAccelGroup *
sp_shortcut_get_accel_group()
{
    static GtkAccelGroup *accel_group = NULL;

    if (!accel_group) {
        accel_group = gtk_accel_group_new ();
    }

    return accel_group;
}

/**
 * Adds a gtk accelerator to a widget
 * Used to display the keyboard shortcuts in the main menu items
 */
void
sp_shortcut_add_accelerator(GtkWidget *item, unsigned int const shortcut)
{
    if (shortcut == GDK_KEY_VoidSymbol) {
        return;
    }

    unsigned int accel_key = sp_shortcut_get_key(shortcut);
    if (accel_key > 0) {
        gtk_widget_add_accelerator (item,
                "activate",
                sp_shortcut_get_accel_group(),
                accel_key,
                sp_shortcut_get_modifiers(shortcut),
                GTK_ACCEL_VISIBLE);
    }
}


unsigned int
sp_shortcut_get_key(unsigned int const shortcut)
{
    return (shortcut & (~SP_SHORTCUT_MODIFIER_MASK));
}

GdkModifierType
sp_shortcut_get_modifiers(unsigned int const shortcut)
{
    return static_cast<GdkModifierType>(
            ((shortcut & SP_SHORTCUT_SHIFT_MASK) ? GDK_SHIFT_MASK : 0) |
            ((shortcut & SP_SHORTCUT_CONTROL_MASK) ? GDK_CONTROL_MASK : 0) |
            ((shortcut & SP_SHORTCUT_ALT_MASK) ? GDK_MOD1_MASK : 0)
            );
}

/**
 * Adds a keyboard shortcut for the given verb.
 * (Removes any existing binding for the given shortcut, including appropriately
 * adjusting sp_shortcut_get_primary if necessary.)
 *
 * \param is_primary True iff this is the shortcut to be written in menu items or buttons.
 *
 * \post sp_shortcut_get_verb(shortcut) == verb.
 * \post !is_primary or sp_shortcut_get_primary(verb) == shortcut.
 */
void
sp_shortcut_set(unsigned int const shortcut, Inkscape::Verb *const verb, bool const is_primary, bool const is_user_set)
{
    if (!verbs) sp_shortcut_init();

    Inkscape::Verb *old_verb = (*verbs)[shortcut];
    (*verbs)[shortcut] = verb;

    /* Maintain the invariant that sp_shortcut_get_primary(v) returns either 0 or a valid shortcut for v. */
    if (old_verb && old_verb != verb) {
        unsigned int const old_primary = (*primary_shortcuts)[old_verb];

        if (old_primary == shortcut) {
            (*primary_shortcuts)[old_verb] = 0;
            (*user_shortcuts)[old_verb] = 0;
        }
    }

    if (is_primary) {
        (*primary_shortcuts)[verb] = shortcut;
        (*user_shortcuts)[verb] = is_user_set;
    }
}

Inkscape::Verb *
sp_shortcut_get_verb(unsigned int shortcut)
{
    if (!verbs) sp_shortcut_init();
    return (*verbs)[shortcut];
}

unsigned int sp_shortcut_get_primary(Inkscape::Verb *verb)
{
    unsigned int result = GDK_KEY_VoidSymbol;
    if (!primary_shortcuts) {
        sp_shortcut_init();
    }
    
    if (primary_shortcuts->count(verb)) {
        result = (*primary_shortcuts)[verb];
    }
    return result;
}

bool sp_shortcut_is_user_set(Inkscape::Verb *verb)
{
    unsigned int result = false;
    if (!primary_shortcuts) {
        sp_shortcut_init();
    }

    if (primary_shortcuts->count(verb)) {
        result = (*user_shortcuts)[verb];
    }
    return result;
}

gchar *sp_shortcut_get_label(unsigned int shortcut)
{
    // The comment below was copied from the function sp_ui_shortcut_string in interface.cpp (which was subsequently removed)
    /* TODO: This function shouldn't exist.  Our callers should use GtkAccelLabel instead of
     * a generic GtkLabel containing this string, and should call gtk_widget_add_accelerator.
     * Will probably need to change sp_shortcut_invoke callers.
     *
     * The existing gtk_label_new_with_mnemonic call can be replaced with
     * g_object_new(GTK_TYPE_ACCEL_LABEL, NULL) followed by
     * gtk_label_set_text_with_mnemonic(lbl, str).
     */
    gchar *result = 0;
    if (shortcut != GDK_KEY_VoidSymbol) {
        result = gtk_accelerator_get_label(
                sp_shortcut_get_key(shortcut),
                sp_shortcut_get_modifiers(shortcut));
    }
    return result;
}

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
