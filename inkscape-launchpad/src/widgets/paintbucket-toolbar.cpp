/**
 * @file
 * Paint bucket aux toolbar
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

#include "paintbucket-toolbar.h"
#include "desktop.h"
#include "document-undo.h"
#include "widgets/ege-adjustment-action.h"
#include "widgets/ege-select-one-action.h"
#include "preferences.h"
#include "toolbox.h"
#include "ui/icon-names.h"
#include "ui/tools/flood-tool.h"
#include "ui/uxmanager.h"
#include "ui/widget/unit-tracker.h"
#include "util/units.h"

using Inkscape::UI::Widget::UnitTracker;
using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;
using Inkscape::Util::unit_table;


//#########################
//##     Paintbucket     ##
//#########################

static void paintbucket_channels_changed(EgeSelectOneAction* act, GObject* /*tbl*/)
{
    gint channels = ege_select_one_action_get_active( act );
    //flood_channels_set_channels( channels );
    Inkscape::UI::Tools::FloodTool::set_channels(channels);
}

static void paintbucket_threshold_changed(GtkAdjustment *adj, GObject * /*tbl*/)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/tools/paintbucket/threshold", (gint)gtk_adjustment_get_value(adj));
}

static void paintbucket_autogap_changed(EgeSelectOneAction* act, GObject * /*tbl*/)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/paintbucket/autogap", ege_select_one_action_get_active( act ));
}

static void paintbucket_offset_changed(GtkAdjustment *adj, GObject *tbl)
{
    UnitTracker* tracker = static_cast<UnitTracker*>(g_object_get_data( tbl, "tracker" ));
    Unit const *unit = tracker->getActiveUnit();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // Don't adjust the offset value because we're saving the
    // unit and it'll be correctly handled on load.
    prefs->setDouble("/tools/paintbucket/offset", (gdouble)gtk_adjustment_get_value(adj));

    g_return_if_fail(unit != NULL);
    prefs->setString("/tools/paintbucket/offsetunits", unit->abbr);
}

static void paintbucket_defaults(GtkWidget *, GObject *tbl)
{
    // FIXME: make defaults settable via Inkscape Options
    struct KeyValue {
        char const *key;
        double value;
    } const key_values[] = {
        {"threshold", 15},
        {"offset", 0.0}
    };

    for (unsigned i = 0; i < G_N_ELEMENTS(key_values); ++i) {
        KeyValue const &kv = key_values[i];
        GtkAdjustment* adj = static_cast<GtkAdjustment *>(g_object_get_data(tbl, kv.key));
        if ( adj ) {
            gtk_adjustment_set_value(adj, kv.value);
        }
    }

    EgeSelectOneAction* channels_action = EGE_SELECT_ONE_ACTION( g_object_get_data (tbl, "channels_action" ) );
    ege_select_one_action_set_active( channels_action, Inkscape::UI::Tools::FLOOD_CHANNELS_RGB );
    EgeSelectOneAction* autogap_action = EGE_SELECT_ONE_ACTION( g_object_get_data (tbl, "autogap_action" ) );
    ege_select_one_action_set_active( autogap_action, 0 );
}

void sp_paintbucket_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    EgeAdjustmentAction* eact = 0;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    {
        GtkListStore* model = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_INT );

        GList* items = 0;
        gint count = 0;
        for ( items = Inkscape::UI::Tools::flood_channels_dropdown_items_list(); items ; items = g_list_next(items) )
        {
            GtkTreeIter iter;
            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter, 0, reinterpret_cast<gchar*>(items->data), 1, count, -1 );
            count++;
        }
        g_list_free( items );
        items = 0;
        EgeSelectOneAction* act1 = ege_select_one_action_new( "ChannelsAction", _("Fill by"), (""), NULL, GTK_TREE_MODEL(model) );
        g_object_set( act1, "short_label", _("Fill by:"), NULL );
        ege_select_one_action_set_appearance( act1, "compact" );
        ege_select_one_action_set_active( act1, prefs->getInt("/tools/paintbucket/channels", 0) );
        g_signal_connect( G_OBJECT(act1), "changed", G_CALLBACK(paintbucket_channels_changed), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act1) );
        g_object_set_data( holder, "channels_action", act1 );
    }

    // Spacing spinbox
    {
        eact = create_adjustment_action(
            "ThresholdAction",
            _("Fill Threshold"), _("Threshold:"),
            _("The maximum allowed difference between the clicked pixel and the neighboring pixels to be counted in the fill"),
            "/tools/paintbucket/threshold", 5, GTK_WIDGET(desktop->canvas), holder, TRUE,
            "inkscape:paintbucket-threshold", 0, 100.0, 1.0, 10.0,
            0, 0, 0,
            paintbucket_threshold_changed, NULL /*unit tracker*/, 1, 0 );

        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    // Create the units menu.
    UnitTracker* tracker = new UnitTracker(Inkscape::Util::UNIT_TYPE_LINEAR);
    Glib::ustring stored_unit = prefs->getString("/tools/paintbucket/offsetunits");
    if (!stored_unit.empty()) {
        Unit const *u = unit_table.getUnit(stored_unit);
        tracker->setActiveUnit(u);
    }
    g_object_set_data( holder, "tracker", tracker );
    {
        GtkAction* act = tracker->createAction( "PaintbucketUnitsAction", _("Units"), ("") );
        gtk_action_group_add_action( mainActions, act );
    }

    // Offset spinbox
    {
        eact = create_adjustment_action(
            "OffsetAction",
            _("Grow/shrink by"), _("Grow/shrink by:"),
            _("The amount to grow (positive) or shrink (negative) the created fill path"),
            "/tools/paintbucket/offset", 0, GTK_WIDGET(desktop->canvas), holder, TRUE,
            "inkscape:paintbucket-offset", -1e4, 1e4, 0.1, 0.5,
            0, 0, 0,
            paintbucket_offset_changed, tracker, 1, 2);
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );

        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* Auto Gap */
    {
        GtkListStore* model = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_INT );

        GList* items = 0;
        gint count = 0;
        for ( items = Inkscape::UI::Tools::flood_autogap_dropdown_items_list(); items ; items = g_list_next(items) )
        {
            GtkTreeIter iter;
            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter, 0, reinterpret_cast<gchar*>(items->data), 1, count, -1 );
            count++;
        }
        g_list_free( items );
        items = 0;
        EgeSelectOneAction* act2 = ege_select_one_action_new( "AutoGapAction", _("Close gaps"), (""), NULL, GTK_TREE_MODEL(model) );
        g_object_set( act2, "short_label", _("Close gaps:"), NULL );
        ege_select_one_action_set_appearance( act2, "compact" );
        ege_select_one_action_set_active( act2, prefs->getBool("/tools/paintbucket/autogap") );
        g_signal_connect( G_OBJECT(act2), "changed", G_CALLBACK(paintbucket_autogap_changed), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act2) );
        g_object_set_data( holder, "autogap_action", act2 );
    }

    /* Reset */
    {
        GtkAction* act = gtk_action_new( "PaintbucketResetAction",
                                          _("Defaults"),
                                          _("Reset paint bucket parameters to defaults (use Inkscape Preferences > Tools to change defaults)"),
                                          INKSCAPE_ICON("edit-clear"));
        g_signal_connect_after( G_OBJECT(act), "activate", G_CALLBACK(paintbucket_defaults), holder );
        gtk_action_group_add_action( mainActions, act );
        gtk_action_set_sensitive( act, TRUE );
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
