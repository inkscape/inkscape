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
 *   Jabiertxo Arraiza <jabier.arraiza@marker.es>
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2003 MenTaLguY
 * Copyright (C) 1999-2015 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm.h>

#include "spray-toolbar.h"
#include "desktop.h"
#include "inkscape.h"
#include "widgets/ege-adjustment-action.h"
#include "widgets/ege-select-one-action.h"
#include "widgets/ink-action.h"
#include "preferences.h"
#include "toolbox.h"
#include "ui/dialog/clonetiler.h"
#include "ui/dialog/dialog-manager.h"
#include "ui/dialog/panel-dialog.h"
#include "ui/icon-names.h"

#include <glibmm/i18n.h>

using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;

// Disabled in 0.91 because of Bug #1274831 (crash, spraying an object 
// with the mode: spray object in single path)
// Please enable again when working on 1.0
#define ENABLE_SPRAY_MODE_SINGLE_PATH

//########################
//##       Spray        ##
//########################

static void sp_stb_update_widgets( GObject *tbl )
{
    GtkAction* offset = GTK_ACTION( g_object_get_data(tbl, "offset") );
    GtkAction* spray_scale = GTK_ACTION( g_object_get_data(tbl, "spray_scale") );
    GtkAdjustment *adj_offset = ege_adjustment_action_get_adjustment( EGE_ADJUSTMENT_ACTION(offset) );
    GtkAdjustment *adj_scale = ege_adjustment_action_get_adjustment( EGE_ADJUSTMENT_ACTION(spray_scale) );
    GtkAction *no_overlap_action = GTK_ACTION( g_object_get_data(tbl, "no_overlap") );
    GtkToggleAction *no_overlap = GTK_TOGGLE_ACTION( g_object_get_data(tbl, "no_overlap") );
    GtkAction *picker_action = GTK_ACTION( g_object_get_data(tbl, "picker") );
    GtkToggleAction *picker = GTK_TOGGLE_ACTION( g_object_get_data(tbl, "picker") );
    GtkToggleAction *usepressurescale = GTK_TOGGLE_ACTION( g_object_get_data(tbl, "usepressurescale") );
    GtkAction *pick_fill = GTK_ACTION( g_object_get_data(tbl, "pick_fill") );
    GtkAction *pick_stroke = GTK_ACTION( g_object_get_data(tbl, "pick_stroke") );
    GtkAction *pick_inverse_value = GTK_ACTION( g_object_get_data(tbl, "pick_inverse_value") );
    GtkAction *pick_center = GTK_ACTION( g_object_get_data(tbl, "pick_center") );
    gtk_adjustment_set_value( adj_offset, 100.0 );
    if (gtk_toggle_action_get_active(no_overlap) && gtk_action_get_visible(no_overlap_action)) {
        gtk_action_set_visible( offset, true );
    } else {
        gtk_action_set_visible( offset, false );
    }
    if (gtk_toggle_action_get_active(usepressurescale)) {
        gtk_adjustment_set_value( adj_scale, 0.0 );
        gtk_action_set_sensitive( spray_scale, false );
    } else {
        gtk_action_set_sensitive( spray_scale, true );
    }
    if(gtk_toggle_action_get_active(picker) && gtk_action_get_visible(picker_action)){
        gtk_action_set_visible( pick_fill, true );
        gtk_action_set_visible( pick_stroke, true );
        gtk_action_set_visible( pick_inverse_value, true );
        gtk_action_set_visible( pick_center, true );
    } else {
        gtk_action_set_visible( pick_fill, false );
        gtk_action_set_visible( pick_stroke, false );
        gtk_action_set_visible( pick_inverse_value, false );
        gtk_action_set_visible( pick_center, false );
    }
}

static void sp_spray_init( GObject *tbl){
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int mode = prefs->getInt("/tools/spray/mode", 0);
    bool show = true;
    if(mode == 3 || mode == 2){
        show = false;
    }
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "no_overlap") ), show );
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "over_no_transparent") ), show );
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "over_transparent") ), show );
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "pick_no_overlap") ), show );
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "pick_stroke") ), show );
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "pick_fill") ), show );
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "pick_inverse_value") ), show );
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "pick_center") ), show );
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "picker") ), show );
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "offset") ), show );
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "pick_fill") ), show );
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "pick_stroke") ), show );
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "pick_inverse_value") ), show );
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "pick_center") ), show );
    if(mode == 2){
        show = true;
    }
    gtk_action_set_visible( GTK_ACTION( g_object_get_data(tbl, "spray_rotation") ), show );
    sp_stb_update_widgets( tbl );
}

Inkscape::UI::Dialog::CloneTiler *get_clone_tiler_panel(SPDesktop *desktop)
{
    if (Inkscape::UI::Dialog::PanelDialogBase *panel_dialog =
        dynamic_cast<Inkscape::UI::Dialog::PanelDialogBase *>(desktop->_dlg_mgr->getDialog("CloneTiler"))) {
        try {
            Inkscape::UI::Dialog::CloneTiler &clone_tiler =
                dynamic_cast<Inkscape::UI::Dialog::CloneTiler &>(panel_dialog->getPanel());
            return &clone_tiler;
        } catch (std::exception &e) { }
    }

    return 0;
}

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

static void sp_spray_mode_changed( EgeSelectOneAction *act, GObject * tbl )
{
    int mode = ege_select_one_action_get_active( act );
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/tools/spray/mode", mode);
    sp_spray_init(tbl);
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

static void sp_spray_offset_value_changed( GtkAdjustment *adj, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/spray/offset",
            gtk_adjustment_get_value(adj));
}

static void sp_toggle_no_overlap( GtkToggleAction* act, gpointer data)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/spray/no_overlap", active);
    GObject *tbl = G_OBJECT(data);
    sp_stb_update_widgets(tbl);
}

static void sp_toggle_pressure_scale( GtkToggleAction* act, gpointer data)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/spray/usepressurescale", active);
    if(active == true){
        prefs->setDouble("/tools/spray/scale_variation", 0);
    }
    GObject *tbl = G_OBJECT(data);
    sp_stb_update_widgets( tbl );
}

static void sp_toggle_over_no_transparent( GtkToggleAction* act, gpointer data)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/spray/over_no_transparent", active);
}

static void sp_toggle_over_transparent( GtkToggleAction* act, gpointer data)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/spray/over_transparent", active);
}


static void sp_toggle_picker( GtkToggleAction* act, gpointer data )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/spray/picker", active);
    if(active == true){
        prefs->setBool("/dialogs/clonetiler/dotrace", false);
        SPDesktop *dt = SP_ACTIVE_DESKTOP;
        if (Inkscape::UI::Dialog::CloneTiler *ct = get_clone_tiler_panel(dt)){
            dt->_dlg_mgr->showDialog("CloneTiler");
            ct->show_page_trace();
        }
    }
    GObject *tbl = G_OBJECT(data);
    sp_stb_update_widgets(tbl);
}

static void sp_toggle_pick_center( GtkToggleAction* act, gpointer data )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/spray/pick_center", active);
}

static void sp_toggle_pick_fill( GtkToggleAction* act, gpointer data )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/spray/pick_fill", active);
}

static void sp_toggle_pick_stroke( GtkToggleAction* act, gpointer data )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/spray/pick_stroke", active);
}

static void sp_toggle_pick_no_overlap( GtkToggleAction* act, gpointer data )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/spray/pick_no_overlap", active);
}

static void sp_toggle_pick_inverse_value( GtkToggleAction* act, gpointer data )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/spray/pick_inverse_value", active);
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
                                                              GTK_WIDGET(desktop->canvas), holder, true, "altx-spray",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_spray_width_value_changed, NULL /*unit tracker*/, 1, 0 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), true );
    }
    
    /* Use Pressure Width button */
    {
        InkToggleAction* act = ink_toggle_action_new( "SprayPressureWidthAction",
                                                      _("Pressure"),
                                                      _("Use the pressure of the input device to alter the width of spray area"),
                                                      INKSCAPE_ICON("draw-use-pressure"),
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/spray/usepressurewidth");
        g_signal_connect(holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);

    }
    
    {
        /* Mean */
        gchar const* labels[] = {_("(default)"), 0, 0, 0, 0, 0, 0, _("(maximum mean)")};
        gdouble values[] = {0, 5, 10, 20, 30, 50, 70, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "SprayMeanAction",
                                                              _("Focus"), _("Focus:"), _("0 to spray a spot; increase to enlarge the ring radius"),
                                                              "/tools/spray/mean", 0,
                                                              GTK_WIDGET(desktop->canvas), holder, true, "spray-mean",
                                                              0, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_spray_mean_value_changed, NULL /*unit tracker*/, 1, 0 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), true );
    }

    {
        /* Standard_deviation */
        gchar const* labels[] = {_("(minimum scatter)"), 0, 0, 0, 0, 0, _("(default)"), _("(maximum scatter)")};
        gdouble values[] = {1, 5, 10, 20, 30, 50, 70, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "SprayStandard_deviationAction",
                                                              C_("Spray tool", "Scatter"), C_("Spray tool", "Scatter:"), _("Increase to scatter sprayed objects"),
                                                              "/tools/spray/standard_deviation", 70,
                                                              GTK_WIDGET(desktop->canvas), holder, true, "spray-standard_deviation",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_spray_standard_deviation_value_changed, NULL /*unit tracker*/, 1, 0 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), true );
    }

    /* Mode */
    {
        GtkListStore* model = gtk_list_store_new( 4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

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
        
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Delete sprayed items"),
                            1, _("Delete sprayed items from selection"),
                            2, INKSCAPE_ICON("draw-eraser"),
                            -1 );
        
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
                                                              GTK_WIDGET(desktop->canvas), holder, true, "spray-population",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_spray_population_value_changed, NULL /*unit tracker*/, 1, 0 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), true );
        g_object_set_data( holder, "spray_population", eact );
    }

    /* Use Pressure Population button */
    {
        InkToggleAction* act = ink_toggle_action_new( "SprayPressurePopulationAction",
                                                      _("Pressure"),
                                                      _("Use the pressure of the input device to alter the amount of sprayed objects"),
                                                      INKSCAPE_ICON("draw-use-pressure"),
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/spray/usepressurepopulation");
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
                                                              GTK_WIDGET(desktop->canvas), holder, true, "spray-rotation",
                                                              0, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_spray_rotation_value_changed, NULL /*unit tracker*/, 1, 0 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), true );
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
                                                              GTK_WIDGET(desktop->canvas), holder, true, "spray-scale",
                                                              0, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_spray_scale_value_changed, NULL /*unit tracker*/, 1, 0 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), true );
        g_object_set_data( holder, "spray_scale", eact );
    }

    /* Use Pressure Scale button */
    {
        InkToggleAction* act = ink_toggle_action_new( "SprayPressureScaleAction",
                                                      _("Pressure"),
                                                      _("Use the pressure of the input device to alter the scale of new items"),
                                                      INKSCAPE_ICON("draw-use-pressure"),
                                                      Inkscape::ICON_SIZE_DECORATION);
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/spray/usepressurescale", false) );
        g_object_set_data( holder, "usepressurescale", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_pressure_scale), holder) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }

    /* Picker */
    {
        InkToggleAction* act = ink_toggle_action_new( "SprayPickColorAction",
                                                      _("Pick color from the drawing. You can use clonetiler trace dialog for advanced effects. In clone mode original fill or stroke colors must be unset."),
                                                      _("Pick color from the drawing. You can use clonetiler trace dialog for advanced effects. In clone mode original fill or stroke colors must be unset."),
                                                      INKSCAPE_ICON("color-picker"),
                                                      secondarySize );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/spray/picker", false) );
        g_object_set_data( holder, "picker", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_picker), holder) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }
    
    /* Pick from center */
    {
        InkToggleAction* act = ink_toggle_action_new( "SprayPickCenterAction",
                                                      _("Pick from center instead average area."),
                                                      _("Pick from center instead average area."),
                                                      INKSCAPE_ICON("snap-bounding-box-center"),
                                                      secondarySize );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/spray/pick_center", true) );
        g_object_set_data( holder, "pick_center", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_pick_center), holder) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }

    /* Inverse Value Size */
    {
        InkToggleAction* act = ink_toggle_action_new( "SprayPickInverseValueAction",
                                                      _("Inverted pick value, retaining color in advanced trace mode"),
                                                      _("Inverted pick value, retaining color in advanced trace mode"),
                                                      INKSCAPE_ICON("object-tweak-shrink"),
                                                      secondarySize );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/spray/pick_inverse_value", false) );
        g_object_set_data( holder, "pick_inverse_value", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_pick_inverse_value), holder) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }

    /* Pick Fill */
    {
        InkToggleAction* act = ink_toggle_action_new( "SprayPickFillAction",
                                                      _("Apply picked color to fill"),
                                                      _("Apply picked color to fill"),
                                                      INKSCAPE_ICON("paint-solid"),
                                                      secondarySize );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/spray/pick_fill", false) );
        g_object_set_data( holder, "pick_fill", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_pick_fill), holder) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }

    /* Pick Stroke */
    {
        InkToggleAction* act = ink_toggle_action_new( "SprayPickStrokeAction",
                                                      _("Apply picked color to stroke"),
                                                      _("Apply picked color to stroke"),
                                                      INKSCAPE_ICON("no-marker"),
                                                      secondarySize );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/spray/pick_stroke", false) );
        g_object_set_data( holder, "pick_stroke", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_pick_stroke), holder) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }

    /* Pick No Overlap */
    {
        InkToggleAction* act = ink_toggle_action_new( "SprayPickNoOverlapAction",
                                                      _("No overlap between colors"),
                                                      _("No overlap between colors"),
                                                      INKSCAPE_ICON("symbol-bigger"),
                                                      secondarySize );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/spray/pick_no_overlap", false) );
        g_object_set_data( holder, "pick_no_overlap", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_pick_no_overlap), holder) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }

    /* Over Transparent */
    {
        InkToggleAction* act = ink_toggle_action_new( "SprayOverTransparentAction",
                                                      _("Apply over transparent areas"),
                                                      _("Apply over transparent areas"),
                                                      INKSCAPE_ICON("object-hidden"),
                                                      secondarySize );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/spray/over_transparent", true) );
        g_object_set_data( holder, "over_transparent", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_over_transparent), holder) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }

    /* Over No Transparent */
    {
        InkToggleAction* act = ink_toggle_action_new( "SprayOverNoTransparentAction",
                                                      _("Apply over no transparent areas"),
                                                      _("Apply over no transparent areas"),
                                                      INKSCAPE_ICON("object-visible"),
                                                      secondarySize );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/spray/over_no_transparent", true) );
        g_object_set_data( holder, "over_no_transparent", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_over_no_transparent), holder) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }

    /* Overlap */
    {
        InkToggleAction* act = ink_toggle_action_new( "SprayNoOverlapAction",
                                                      _("Prevent overlapping objects"),
                                                      _("Prevent overlapping objects"),
                                                      INKSCAPE_ICON("distribute-randomize"),
                                                      secondarySize );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/spray/no_overlap", false) );
        g_object_set_data( holder, "no_overlap", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_no_overlap), holder) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }
    
    /* Offset */
    {
        gchar const* labels[] = {_("(minimum offset)"), 0, 0, 0, _("(default)"), 0, 0, _("(maximum offset)")};
        gdouble values[] = {0, 25, 50, 75, 100, 150, 200, 1000};
        EgeAdjustmentAction *eact = create_adjustment_action( "SprayToolOffsetAction",
                                         _("Offset %"), _("Offset %:"),
                                         _("Increase to segregate objects more (value in percent)"),
                                         "/tools/spray/offset", 100,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         0, 1000, 1, 4,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_spray_offset_value_changed, NULL, 0 , 0);
        g_object_set_data( holder, "offset", eact );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }
    sp_spray_init(holder);
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
