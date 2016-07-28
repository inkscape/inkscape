/**
 * @file
 * Tweak aux toolbar
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

#include "ui/widget/spinbutton.h"
#include <glibmm/i18n.h>
#include "tweak-toolbar.h"
#include "desktop.h"
#include "document-undo.h"
#include "widgets/ege-adjustment-action.h"
#include "widgets/ege-output-action.h"
#include "widgets/ege-select-one-action.h"
#include "widgets/ink-action.h"
#include "preferences.h"
#include "toolbox.h"
#include "ui/icon-names.h"
#include "ui/tools/tweak-tool.h"

using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;


//########################
//##       Tweak        ##
//########################

static void sp_tweak_width_value_changed( GtkAdjustment *adj, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/tweak/width",
            gtk_adjustment_get_value(adj) * 0.01 );
}

static void sp_tweak_force_value_changed( GtkAdjustment *adj, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/tweak/force",
            gtk_adjustment_get_value(adj) * 0.01 );
}

static void sp_tweak_pressure_state_changed( GtkToggleAction *act, gpointer /*data*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/tweak/usepressure", gtk_toggle_action_get_active(act));
}

static void sp_tweak_mode_changed( EgeSelectOneAction *act, GObject *tbl )
{
    int mode = ege_select_one_action_get_active( act );
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/tools/tweak/mode", mode);

    static gchar const* names[] = {"tweak_doh", "tweak_dos", "tweak_dol", "tweak_doo", "tweak_channels_label"};
    bool flag = ((mode == Inkscape::UI::Tools::TWEAK_MODE_COLORPAINT) || (mode == Inkscape::UI::Tools::TWEAK_MODE_COLORJITTER));
    for (size_t i = 0; i < G_N_ELEMENTS(names); ++i) {
        GtkAction *act = GTK_ACTION(g_object_get_data( tbl, names[i] ));
        if (act) {
            gtk_action_set_visible(act, flag);
        }
    }
    GtkAction *fid = GTK_ACTION(g_object_get_data( tbl, "tweak_fidelity"));
    if (fid) {
        gtk_action_set_visible(fid, !flag);
    }
}

static void sp_tweak_fidelity_value_changed( GtkAdjustment *adj, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/tweak/fidelity",
            gtk_adjustment_get_value(adj) * 0.01 );
}

static void tweak_toggle_doh(GtkToggleAction *act, gpointer /*data*/) {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/tweak/doh", gtk_toggle_action_get_active(act));
}
static void tweak_toggle_dos(GtkToggleAction *act, gpointer /*data*/) {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/tweak/dos", gtk_toggle_action_get_active(act));
}
static void tweak_toggle_dol(GtkToggleAction *act, gpointer /*data*/) {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/tweak/dol", gtk_toggle_action_get_active(act));
}
static void tweak_toggle_doo(GtkToggleAction *act, gpointer /*data*/) {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/tweak/doo", gtk_toggle_action_get_active(act));
}

void sp_tweak_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    Inkscape::IconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    {
        /* Width */
        gchar const* labels[] = {_("(pinch tweak)"), 0, 0, 0, _("(default)"), 0, 0, 0, 0, _("(broad tweak)")};
        gdouble values[] = {1, 3, 5, 10, 15, 20, 30, 50, 75, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "TweakWidthAction",
                                                              _("Width"), _("Width:"), _("The width of the tweak area (relative to the visible canvas area)"),
                                                              "/tools/tweak/width", 15,
                                                              GTK_WIDGET(desktop->canvas), holder, TRUE, "altx-tweak",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_tweak_width_value_changed, NULL /*unit tracker*/, 0.01, 0, 100 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    }


    {
        /* Force */
        gchar const* labels[] = {_("(minimum force)"), 0, 0, _("(default)"), 0, 0, 0, _("(maximum force)")};
        gdouble values[] = {1, 5, 10, 20, 30, 50, 70, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "TweakForceAction",
                                                              _("Force"), _("Force:"), _("The force of the tweak action"),
                                                              "/tools/tweak/force", 20,
                                                              GTK_WIDGET(desktop->canvas), holder, TRUE, "tweak-force",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_tweak_force_value_changed, NULL /*unit tracker*/, 0.01, 0, 100 );
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
                            0, _("Move mode"),
                            1, _("Move objects in any direction"),
                            2, INKSCAPE_ICON("object-tweak-push"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Move in/out mode"),
                            1, _("Move objects towards cursor; with Shift from cursor"),
                            2, INKSCAPE_ICON("object-tweak-attract"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Move jitter mode"),
                            1, _("Move objects in random directions"),
                            2, INKSCAPE_ICON("object-tweak-randomize"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Scale mode"),
                            1, _("Shrink objects, with Shift enlarge"),
                            2, INKSCAPE_ICON("object-tweak-shrink"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Rotate mode"),
                            1, _("Rotate objects, with Shift counterclockwise"),
                            2, INKSCAPE_ICON("object-tweak-rotate"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Duplicate/delete mode"),
                            1, _("Duplicate objects, with Shift delete"),
                            2, INKSCAPE_ICON("object-tweak-duplicate"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Push mode"),
                            1, _("Push parts of paths in any direction"),
                            2, INKSCAPE_ICON("path-tweak-push"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Shrink/grow mode"),
                            1, _("Shrink (inset) parts of paths; with Shift grow (outset)"),
                            2, INKSCAPE_ICON("path-tweak-shrink"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Attract/repel mode"),
                            1, _("Attract parts of paths towards cursor; with Shift from cursor"),
                            2, INKSCAPE_ICON("path-tweak-attract"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Roughen mode"),
                            1, _("Roughen parts of paths"),
                            2, INKSCAPE_ICON("path-tweak-roughen"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Color paint mode"),
                            1, _("Paint the tool's color upon selected objects"),
                            2, INKSCAPE_ICON("object-tweak-paint"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Color jitter mode"),
                            1, _("Jitter the colors of selected objects"),
                            2, INKSCAPE_ICON("object-tweak-jitter-color"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Blur mode"),
                            1, _("Blur selected objects more; with Shift, blur less"),
                            2, INKSCAPE_ICON("object-tweak-blur"),
                            -1 );


        EgeSelectOneAction* act = ege_select_one_action_new( "TweakModeAction", _("Mode"), (""), NULL, GTK_TREE_MODEL(model) );
        g_object_set( act, "short_label", _("Mode:"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        g_object_set_data( holder, "mode_action", act );

        ege_select_one_action_set_appearance( act, "full" );
        ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
        g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
        ege_select_one_action_set_icon_column( act, 2 );
        ege_select_one_action_set_icon_size( act, secondarySize );
        ege_select_one_action_set_tooltip_column( act, 1  );

        gint mode = prefs->getInt("/tools/tweak/mode", 0);
        ege_select_one_action_set_active( act, mode );
        g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(sp_tweak_mode_changed), holder );

        g_object_set_data( G_OBJECT(holder), "tweak_tool_mode", act);
    }

    guint mode = prefs->getInt("/tools/tweak/mode", 0);

    {
        EgeOutputAction* act = ege_output_action_new( "TweakChannelsLabel", _("Channels:"), "", 0 );
        ege_output_action_set_use_markup( act, TRUE );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        if (mode != Inkscape::UI::Tools::TWEAK_MODE_COLORPAINT && mode != Inkscape::UI::Tools::TWEAK_MODE_COLORJITTER) {
            gtk_action_set_visible (GTK_ACTION(act), FALSE);
        }
        g_object_set_data( holder, "tweak_channels_label", act);
    }

    {
        InkToggleAction* act = ink_toggle_action_new( "TweakDoH",
                                                      _("Hue"),
                                                      _("In color mode, act on objects' hue"),
                                                      NULL,
                                                      Inkscape::ICON_SIZE_DECORATION );
        //TRANSLATORS:  "H" here stands for hue
        g_object_set( act, "short_label", C_("Hue", "H"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(tweak_toggle_doh), desktop );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/tweak/doh", true) );
        if (mode != Inkscape::UI::Tools::TWEAK_MODE_COLORPAINT && mode != Inkscape::UI::Tools::TWEAK_MODE_COLORJITTER) {
            gtk_action_set_visible (GTK_ACTION(act), FALSE);
        }
        g_object_set_data( holder, "tweak_doh", act);
    }
    {
        InkToggleAction* act = ink_toggle_action_new( "TweakDoS",
                                                      _("Saturation"),
                                                      _("In color mode, act on objects' saturation"),
                                                      NULL,
                                                      Inkscape::ICON_SIZE_DECORATION );
        //TRANSLATORS: "S" here stands for Saturation
        g_object_set( act, "short_label", C_("Saturation", "S"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(tweak_toggle_dos), desktop );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/tweak/dos", true) );
        if (mode != Inkscape::UI::Tools::TWEAK_MODE_COLORPAINT && mode != Inkscape::UI::Tools::TWEAK_MODE_COLORJITTER) {
            gtk_action_set_visible (GTK_ACTION(act), FALSE);
        }
        g_object_set_data( holder, "tweak_dos", act );
    }
    {
        InkToggleAction* act = ink_toggle_action_new( "TweakDoL",
                                                      _("Lightness"),
                                                      _("In color mode, act on objects' lightness"),
                                                      NULL,
                                                      Inkscape::ICON_SIZE_DECORATION );
        //TRANSLATORS: "L" here stands for Lightness
        g_object_set( act, "short_label", C_("Lightness", "L"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(tweak_toggle_dol), desktop );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/tweak/dol", true) );
        if (mode != Inkscape::UI::Tools::TWEAK_MODE_COLORPAINT && mode != Inkscape::UI::Tools::TWEAK_MODE_COLORJITTER) {
            gtk_action_set_visible (GTK_ACTION(act), FALSE);
        }
        g_object_set_data( holder, "tweak_dol", act );
    }
    {
        InkToggleAction* act = ink_toggle_action_new( "TweakDoO",
                                                      _("Opacity"),
                                                      _("In color mode, act on objects' opacity"),
                                                      NULL,
                                                      Inkscape::ICON_SIZE_DECORATION );
        //TRANSLATORS: "O" here stands for Opacity
        g_object_set( act, "short_label", C_("Opacity", "O"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(tweak_toggle_doo), desktop );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/tweak/doo", true) );
        if (mode != Inkscape::UI::Tools::TWEAK_MODE_COLORPAINT && mode != Inkscape::UI::Tools::TWEAK_MODE_COLORJITTER) {
            gtk_action_set_visible (GTK_ACTION(act), FALSE);
        }
        g_object_set_data( holder, "tweak_doo", act );
    }

    {   /* Fidelity */
        gchar const* labels[] = {_("(rough, simplified)"), 0, 0, _("(default)"), 0, 0, _("(fine, but many nodes)")};
        gdouble values[] = {10, 25, 35, 50, 60, 80, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "TweakFidelityAction",
                                                              _("Fidelity"), _("Fidelity:"),
                                                              _("Low fidelity simplifies paths; high fidelity preserves path features but may generate a lot of new nodes"),
                                                              "/tools/tweak/fidelity", 50,
                                                              GTK_WIDGET(desktop->canvas), holder, TRUE, "tweak-fidelity",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_tweak_fidelity_value_changed, NULL /*unit tracker*/, 0.01, 0, 100 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_visible( GTK_ACTION(eact), TRUE );
        if (mode == Inkscape::UI::Tools::TWEAK_MODE_COLORPAINT || mode == Inkscape::UI::Tools::TWEAK_MODE_COLORJITTER) {
            gtk_action_set_visible (GTK_ACTION(eact), FALSE);
        }
        g_object_set_data( holder, "tweak_fidelity", eact );
    }


    /* Use Pressure button */
    {
        InkToggleAction* act = ink_toggle_action_new( "TweakPressureAction",
                                                      _("Pressure"),
                                                      _("Use the pressure of the input device to alter the force of tweak action"),
                                                      INKSCAPE_ICON("draw-use-pressure"),
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_tweak_pressure_state_changed), NULL);
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/tweak/usepressure", true) );
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
