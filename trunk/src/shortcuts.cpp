#define __SP_SHORTCUTS_C__

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

#include <gdk/gdkkeys.h>
#include <gdk/gdkkeysyms.h>

#include "helper/action.h"
#include "io/sys.h"
#include "io/resource.h"
#include "shortcuts.h"
#include "verbs.h"
#include "xml/node-iterators.h"
#include "xml/repr.h"

using namespace Inkscape;

static void sp_shortcut_set(unsigned int const shortcut, Inkscape::Verb *const verb, bool const is_primary);
static void try_shortcuts_file(char const *filename);
static void read_shortcuts_file(char const *filename);

/* Returns true if action was performed */

bool
sp_shortcut_invoke(unsigned int shortcut, Inkscape::UI::View::View *view)
{
    Inkscape::Verb *verb = sp_shortcut_get_verb(shortcut);
    if (verb) {
        SPAction *action = verb->get_action(view);
        if (action) {
            sp_action_perform(action, NULL);
            return true;
        }
    }
    return false;
}

static GHashTable *verbs = NULL;
static GHashTable *primary_shortcuts = NULL;

static void
sp_shortcut_init()
{
    using Inkscape::IO::Resource::get_path;
    using Inkscape::IO::Resource::SYSTEM;
    using Inkscape::IO::Resource::USER;
    using Inkscape::IO::Resource::KEYS;

    verbs = g_hash_table_new(NULL, NULL);
    primary_shortcuts = g_hash_table_new(NULL, NULL);

    read_shortcuts_file(get_path(SYSTEM, KEYS, "default.xml"));
    try_shortcuts_file(get_path(USER, KEYS, "default.xml"));
}

static void try_shortcuts_file(char const *filename) {
    using Inkscape::IO::file_test;

    /* ah, if only we had an exception to catch... (permission, forgiveness) */
    if (file_test(filename, G_FILE_TEST_EXISTS)) {
        read_shortcuts_file(filename);
    }
}

static void read_shortcuts_file(char const *filename) {
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
        if (keyval == GDK_VoidSymbol || keyval == 0) {
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

        sp_shortcut_set(keyval | modifiers, verb, is_primary);
    }

    GC::release(doc);
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
static void
sp_shortcut_set(unsigned int const shortcut, Inkscape::Verb *const verb, bool const is_primary)
{
    if (!verbs) sp_shortcut_init();

    Inkscape::Verb *old_verb = (Inkscape::Verb *)(g_hash_table_lookup(verbs, GINT_TO_POINTER(shortcut)));
    g_hash_table_insert(verbs, GINT_TO_POINTER(shortcut), (gpointer)(verb));

    /* Maintain the invariant that sp_shortcut_get_primary(v) returns either 0 or a valid shortcut for v. */
    if (old_verb && old_verb != verb) {
        unsigned int const old_primary = (unsigned int)GPOINTER_TO_INT(g_hash_table_lookup(primary_shortcuts, (gpointer)old_verb));

        if (old_primary == shortcut) {
            g_hash_table_insert(primary_shortcuts, (gpointer)old_verb, GINT_TO_POINTER(0));
        }
    }

    if (is_primary) {
        g_hash_table_insert(primary_shortcuts, (gpointer)(verb), GINT_TO_POINTER(shortcut));
    }
}

Inkscape::Verb *
sp_shortcut_get_verb(unsigned int shortcut)
{
    if (!verbs) sp_shortcut_init();
    return (Inkscape::Verb *)(g_hash_table_lookup(verbs, GINT_TO_POINTER(shortcut)));
}

unsigned int
sp_shortcut_get_primary(Inkscape::Verb *verb)
{
    if (!primary_shortcuts) sp_shortcut_init();
    return (unsigned int)GPOINTER_TO_INT(g_hash_table_lookup(primary_shortcuts,
                                                             (gpointer)(verb)));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
