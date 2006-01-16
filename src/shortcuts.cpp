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

#include <gdk/gdkkeys.h>
#include <gdk/gdkkeysyms.h>

#include "helper/action.h"
#include "shortcuts.h"
#include "verbs.h"
#include "xml/node-iterators.h"
#include "xml/repr.h"

using namespace Inkscape;

static void sp_shortcut_set(unsigned int const shortcut, Inkscape::Verb *const verb, bool const is_primary);

static void set_shortcuts_xml(XML::Document const *doc);

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

extern char const shortcuts_default_xml[];

static void
sp_shortcut_init()
{
    verbs = g_hash_table_new(NULL, NULL);
    primary_shortcuts = g_hash_table_new(NULL, NULL);

    XML::Document *shortcuts=sp_repr_read_mem(shortcuts_default_xml, strlen(shortcuts_default_xml), NULL);
    if (shortcuts) {
        set_shortcuts_xml(shortcuts);
        GC::release(shortcuts);
    } else {
        g_error("Unable to parse default shortcuts");
    }
}

static void set_shortcuts_xml(XML::Document const *doc) {
    XML::Node const *root=doc->root();
    g_return_if_fail(!strcmp(root->name(), "keybindings"));
    XML::NodeConstSiblingIterator iter=root->firstChild();
    for ( ; iter ; ++iter ) {
        bool is_primary;

        if (!strcmp(iter->name(), "primary")) {
            is_primary = true;
        } else if (!strcmp(iter->name(), "secondary")) {
            is_primary = false;
        } else {
            g_warning("Unknown key binding type %s", iter->name());
            continue;
        }

        gchar const *verb_name=iter->attribute("verb");
        if (!verb_name) {
            g_warning("Missing verb name for shortcut");
            continue;
        }

        gchar const *keyval_name=iter->attribute("keyval");
        if (!keyval_name) {
            g_warning("Missing keyval for %s", verb_name);
            continue;
        }
        guint keyval=gdk_keyval_from_name(keyval_name);
        if (keyval == GDK_VoidSymbol) {
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
                if (!strcmp(mod, "control")) {
                    modifiers |= SP_SHORTCUT_CONTROL_MASK;
                } else if (!strcmp(mod, "shift")) {
                    modifiers |= SP_SHORTCUT_SHIFT_MASK;
                } else if (!strcmp(mod, "alt")) {
                    modifiers |= SP_SHORTCUT_ALT_MASK;
                } else {
                    g_warning("Unknown modifier %s for %s", mod, verb_name);
                }
                g_free(mod);
                iter += length;
                if (*iter) iter++;
            }
        }

        sp_shortcut_set(keyval | modifiers,
                        Inkscape::Verb::getbyid(verb_name),
                        is_primary);
    }
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
