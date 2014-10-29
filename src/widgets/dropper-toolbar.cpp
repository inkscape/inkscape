/**
 * @file
 * Dropper aux toolbar
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Frank Felfe <innerspace@iname.com>
 *   John Cliff <simarilius@yahoo.com>
 *   David Turner <novalis@gnu.org>
 *   Josh Andler <scislac@scislac.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2003 MenTaLguY
 * Copyright (C) 1999-2011 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>

#include "dropper-toolbar.h"
#include "document-undo.h"
#include "widgets/ege-output-action.h"
#include "widgets/ink-action.h"
#include "preferences.h"
#include "widgets/spinbutton-events.h"

using Inkscape::DocumentUndo;

//########################
//##      Dropper       ##
//########################

static void toggle_dropper_pick_alpha( GtkToggleAction* act, gpointer tbl )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt( "/tools/dropper/pick", gtk_toggle_action_get_active( act ) );
    GtkAction* set_action = GTK_ACTION( g_object_get_data(G_OBJECT(tbl), "set_action") );
    if ( set_action ) {
        if ( gtk_toggle_action_get_active( act ) ) {
            gtk_action_set_sensitive( set_action, TRUE );
        } else {
            gtk_action_set_sensitive( set_action, FALSE );
        }
    }

    spinbutton_defocus(GTK_WIDGET(tbl));
}

static void toggle_dropper_set_alpha( GtkToggleAction* act, gpointer tbl )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool( "/tools/dropper/setalpha", gtk_toggle_action_get_active( act ) );
    spinbutton_defocus(GTK_WIDGET(tbl));
}


/**
 * Dropper auxiliary toolbar construction and setup.
 *
 * TODO: Would like to add swatch of current color.
 * TODO: Add queue of last 5 or so colors selected with new swatches so that
 *       can drag and drop places. Will provide a nice mixing palette.
 */
void sp_dropper_toolbox_prep(SPDesktop * /*desktop*/, GtkActionGroup* mainActions, GObject* holder)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gint pickAlpha = prefs->getInt( "/tools/dropper/pick", 1 );

    {
        EgeOutputAction* act = ege_output_action_new( "DropperOpacityAction", _("Opacity:"), "", 0 );
        ege_output_action_set_use_markup( act, TRUE );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
    }

    {
        InkToggleAction* act = ink_toggle_action_new( "DropperPickAlphaAction",
                                                      _("Pick opacity"),
                                                      _("Pick both the color and the alpha (transparency) under cursor; otherwise, pick only the visible color premultiplied by alpha"),
                                                      NULL,
                                                      Inkscape::ICON_SIZE_DECORATION );
        g_object_set( act, "short_label", _("Pick"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "pick_action", act );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), pickAlpha );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_dropper_pick_alpha), holder );
    }

    {
        InkToggleAction* act = ink_toggle_action_new( "DropperSetAlphaAction",
                                                      _("Assign opacity"),
                                                      _("If alpha was picked, assign it to selection as fill or stroke transparency"),
                                                      NULL,
                                                      Inkscape::ICON_SIZE_DECORATION );
        g_object_set( act, "short_label", _("Assign"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "set_action", act );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool( "/tools/dropper/setalpha", true) );
        // make sure it's disabled if we're not picking alpha
        gtk_action_set_sensitive( GTK_ACTION(act), pickAlpha );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(toggle_dropper_set_alpha), holder );
    }
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
