/**
 * @file
 * Erasor aux toolbar
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

#include "eraser-toolbar.h"
#include "calligraphy-toolbar.h" // TODO: needed for update_presets_list
#include "desktop-handles.h"
#include "desktop.h"
#include "document-undo.h"
#include "widgets/ege-adjustment-action.h"
#include "widgets/ege-select-one-action.h"
#include "widgets/ink-action.h"
#include "preferences.h"
#include "toolbox.h"
#include "ui/icon-names.h"

using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;

//########################
//##       Eraser       ##
//########################

static void sp_erc_width_value_changed( GtkAdjustment *adj, GObject *tbl )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/eraser/width", gtk_adjustment_get_value(adj) );
    update_presets_list(tbl);
}

static void sp_erasertb_mode_changed( EgeSelectOneAction *act, GObject *tbl )
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));
    bool eraserMode = ege_select_one_action_get_active( act ) != 0;
    if (DocumentUndo::getUndoSensitive(sp_desktop_document(desktop))) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setBool( "/tools/eraser/mode", eraserMode );
    }

    // only take action if run by the attr_changed listener
    if (!g_object_get_data( tbl, "freeze" )) {
        // in turn, prevent listener from responding
        g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

        /*
        if ( eraserMode != 0 ) {
        } else {
        }
        */
        // TODO finish implementation

        g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
    }
}

void sp_eraser_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    {
        GtkListStore* model = gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

        GtkTreeIter iter;
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Delete"),
                            1, _("Delete objects touched by the eraser"),
                            2, INKSCAPE_ICON("draw-eraser-delete-objects"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Cut"),
                            1, _("Cut out from objects"),
                            2, INKSCAPE_ICON("path-difference"),
                            -1 );

        EgeSelectOneAction* act = ege_select_one_action_new( "EraserModeAction", (""), (""), NULL, GTK_TREE_MODEL(model) );
        g_object_set( act, "short_label", _("Mode:"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        g_object_set_data( holder, "eraser_mode_action", act );

        ege_select_one_action_set_appearance( act, "full" );
        ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
        g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
        ege_select_one_action_set_icon_column( act, 2 );
        ege_select_one_action_set_tooltip_column( act, 1  );

        /// @todo Convert to boolean?
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        gint eraserMode = prefs->getBool("/tools/eraser/mode") ? 1 : 0;
        ege_select_one_action_set_active( act, eraserMode );
        g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(sp_erasertb_mode_changed), holder );
    }

    {
        /* Width */
        gchar const* labels[] = {_("(hairline)"), 0, 0, 0, _("(default)"), 0, 0, 0, 0, _("(broad stroke)")};
        gdouble values[] = {1, 3, 5, 10, 15, 20, 30, 50, 75, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "EraserWidthAction",
                                                              _("Pen Width"), _("Width:"),
                                                              _("The width of the eraser pen (relative to the visible canvas area)"),
                                                              "/tools/eraser/width", 15,
                                                              GTK_WIDGET(desktop->canvas), holder, TRUE, "altx-eraser",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_erc_width_value_changed, NULL /*unit tracker*/, 1, 0);
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
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
