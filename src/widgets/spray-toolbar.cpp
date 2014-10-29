/**
 * @file
 * Spray aux toolbar
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

#include "spray-toolbar.h"
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

// Disabled in 0.91 because of Bug #1274831 (crash, spraying an object 
// with the mode: spray object in single path)
// Please enable again when working on 1.0
#define ENABLE_SPRAY_MODE_SINGLE_PATH

//########################
//##       Spray        ##
//########################

static void sp_spray_width_value_changed( GtkAdjustment *adj, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/width",
            gtk_adjustment_get_value(adj));
}

static void sp_spray_mean_value_changed( GtkAdjustment *adj, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/mean",
            gtk_adjustment_get_value(adj));
}

static void sp_spray_standard_deviation_value_changed( GtkAdjustment *adj, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/standard_deviation",
            gtk_adjustment_get_value(adj));
}

static void sp_spray_mode_changed( EgeSelectOneAction *act, GObject * /*tbl*/ )
{
    int mode = ege_select_one_action_get_active( act );
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/tools/spray/mode", mode);
}

static void sp_spray_population_value_changed( GtkAdjustment *adj, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/population",
            gtk_adjustment_get_value(adj));
}

static void sp_spray_rotation_value_changed( GtkAdjustment *adj, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/rotation_variation",
            gtk_adjustment_get_value(adj));
}

static void sp_spray_scale_value_changed( GtkAdjustment *adj, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/scale_variation",
            gtk_adjustment_get_value(adj));
}


void sp_spray_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    Inkscape::IconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    {
        /* Width */
        gchar const* labels[] = {_("(narrow spray)"), 0, 0, 0, _("(default)"), 0, 0, 0, 0, _("(broad spray)")};
        gdouble values[] = {1, 3, 5, 10, 15, 20, 30, 50, 75, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "SprayWidthAction",
                                                              _("Width"), _("Width:"), _("The width of the spray area (relative to the visible canvas area)"),
                                                              "/tools/spray/width", 15,
                                                              GTK_WIDGET(desktop->canvas), holder, TRUE, "altx-spray",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_spray_width_value_changed, NULL /*unit tracker*/, 1, 0 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    }

    {
        /* Mean */
        gchar const* labels[] = {_("(default)"), 0, 0, 0, 0, 0, 0, _("(maximum mean)")};
        gdouble values[] = {0, 5, 10, 20, 30, 50, 70, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "SprayMeanAction",
                                                              _("Focus"), _("Focus:"), _("0 to spray a spot; increase to enlarge the ring radius"),
                                                              "/tools/spray/mean", 0,
                                                              GTK_WIDGET(desktop->canvas), holder, TRUE, "spray-mean",
                                                              0, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_spray_mean_value_changed, NULL /*unit tracker*/, 1, 0 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    }

    {
        /* Standard_deviation */
        gchar const* labels[] = {_("(minimum scatter)"), 0, 0, 0, 0, 0, _("(default)"), _("(maximum scatter)")};
        gdouble values[] = {1, 5, 10, 20, 30, 50, 70, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "SprayStandard_deviationAction",
                                                              C_("Spray tool", "Scatter"), C_("Spray tool", "Scatter:"), _("Increase to scatter sprayed objects"),
                                                              "/tools/spray/standard_deviation", 70,
                                                              GTK_WIDGET(desktop->canvas), holder, TRUE, "spray-standard_deviation",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_spray_standard_deviation_value_changed, NULL /*unit tracker*/, 1, 0 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    }

    /* Mode */
    {
        GtkListStore* model = gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

        GtkTreeIter iter;
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Spray with copies"),
                            1, _("Spray copies of the initial selection"),
                            2, INKSCAPE_ICON("spray-mode-copy"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Spray with clones"),
                            1, _("Spray clones of the initial selection"),
                            2, INKSCAPE_ICON("spray-mode-clone"),
                            -1 );
#ifdef ENABLE_SPRAY_MODE_SINGLE_PATH
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Spray single path"),
                            1, _("Spray objects in a single path"),
                            2, INKSCAPE_ICON("spray-mode-union"),
                            -1 );
#endif
        EgeSelectOneAction* act = ege_select_one_action_new( "SprayModeAction", _("Mode"), (""), NULL, GTK_TREE_MODEL(model) );
        g_object_set( act, "short_label", _("Mode:"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        g_object_set_data( holder, "mode_action", act );

        ege_select_one_action_set_appearance( act, "full" );
        ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
        g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
        ege_select_one_action_set_icon_column( act, 2 );
        ege_select_one_action_set_icon_size( act, secondarySize );
        ege_select_one_action_set_tooltip_column( act, 1  );

        gint mode = prefs->getInt("/tools/spray/mode", 1);
        ege_select_one_action_set_active( act, mode );
        g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(sp_spray_mode_changed), holder );

        g_object_set_data( G_OBJECT(holder), "spray_tool_mode", act);
    }

    {   /* Population */
        gchar const* labels[] = {_("(low population)"), 0, 0, 0, _("(default)"), 0, _("(high population)")};
        gdouble values[] = {5, 20, 35, 50, 70, 85, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "SprayPopulationAction",
                                                              _("Amount"), _("Amount:"),
                                                              _("Adjusts the number of items sprayed per click"),
                                                              "/tools/spray/population", 70,
                                                              GTK_WIDGET(desktop->canvas), holder, TRUE, "spray-population",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_spray_population_value_changed, NULL /*unit tracker*/, 1, 0 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        g_object_set_data( holder, "spray_population", eact );
    }

    /* Use Pressure button */
    {
        InkToggleAction* act = ink_toggle_action_new( "SprayPressureAction",
                                                      _("Pressure"),
                                                      _("Use the pressure of the input device to alter the amount of sprayed objects"),
                                                      INKSCAPE_ICON("draw-use-pressure"),
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/spray/usepressure");
        g_signal_connect(holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);

    }

    {   /* Rotation */
        gchar const* labels[] = {_("(default)"), 0, 0, 0, 0, 0, 0, _("(high rotation variation)")};
        gdouble values[] = {0, 10, 25, 35, 50, 60, 80, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "SprayRotationAction",
                                                              _("Rotation"), _("Rotation:"),
                                                              // xgettext:no-c-format
                                                              _("Variation of the rotation of the sprayed objects; 0% for the same rotation than the original object"),
                                                              "/tools/spray/rotation_variation", 0,
                                                              GTK_WIDGET(desktop->canvas), holder, TRUE, "spray-rotation",
                                                              0, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_spray_rotation_value_changed, NULL /*unit tracker*/, 1, 0 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        g_object_set_data( holder, "spray_rotation", eact );
    }

    {   /* Scale */
        gchar const* labels[] = {_("(default)"), 0, 0, 0, 0, 0, 0, _("(high scale variation)")};
        gdouble values[] = {0, 10, 25, 35, 50, 60, 80, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "SprayScaleAction",
                                                              C_("Spray tool", "Scale"), C_("Spray tool", "Scale:"),
                                                              // xgettext:no-c-format
                                                              _("Variation in the scale of the sprayed objects; 0% for the same scale than the original object"),
                                                              "/tools/spray/scale_variation", 0,
                                                              GTK_WIDGET(desktop->canvas), holder, TRUE, "spray-scale",
                                                              0, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_spray_scale_value_changed, NULL /*unit tracker*/, 1, 0 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        g_object_set_data( holder, "spray_scale", eact );
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
