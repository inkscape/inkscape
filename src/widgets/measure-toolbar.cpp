/**
 * @file
 * Measure aux toolbar
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

#include "measure-toolbar.h"

#include "desktop.h"
#include "inkscape.h"
#include "message-stack.h"
#include "document-undo.h"
#include "widgets/ege-adjustment-action.h"
#include "widgets/ege-output-action.h"
#include "preferences.h"
#include "toolbox.h"
#include "widgets/ink-action.h"
#include "ui/icon-names.h"
#include "ui/tools/measure-tool.h"
#include "ui/widget/unit-tracker.h"

using Inkscape::UI::Widget::UnitTracker;
using Inkscape::Util::Unit;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;
using Inkscape::UI::Tools::MeasureTool;

//########################
//##  Measure Toolbox   ##
//########################

/** Temporary hack: Returns the node tool in the active desktop.
 * Will go away during tool refactoring. */
static MeasureTool *get_measure_tool()
{
    MeasureTool *tool = 0;
    if (SP_ACTIVE_DESKTOP ) {
        Inkscape::UI::Tools::ToolBase *ec = SP_ACTIVE_DESKTOP->event_context;
        if (SP_IS_MEASURE_CONTEXT(ec)) {
            tool = static_cast<MeasureTool*>(ec);
        }
    }
    return tool;
}

static void
sp_measure_fontsize_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));

    if (DocumentUndo::getUndoSensitive(desktop->getDocument())) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble(Glib::ustring("/tools/measure/fontsize"),
            gtk_adjustment_get_value(adj));
        MeasureTool *mt = get_measure_tool();
        if (mt) {
            mt->showCanvasItems();
        }
    }
}

static void
sp_measure_offset_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));

    if (DocumentUndo::getUndoSensitive(desktop->getDocument())) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble(Glib::ustring("/tools/measure/offset"),
            gtk_adjustment_get_value(adj));
        MeasureTool *mt = get_measure_tool();
        if (mt) {
            mt->showCanvasItems();
        }
    }
}

static void sp_measure_scale_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));

    if (DocumentUndo::getUndoSensitive(desktop->getDocument())) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble(Glib::ustring("/tools/measure/scale"),
            gtk_adjustment_get_value(adj));
        MeasureTool *mt = get_measure_tool();
        if (mt) {
            mt->showCanvasItems();
        }
    }
}

static void
sp_measure_precision_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));

    if (DocumentUndo::getUndoSensitive(desktop->getDocument())) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setInt(Glib::ustring("/tools/measure/precision"),
            gtk_adjustment_get_value(adj));
        MeasureTool *mt = get_measure_tool();
        if (mt) {
            mt->showCanvasItems();
        }
    }
}

static void 
sp_measure_unit_changed(GtkAction* /*act*/, GObject* tbl)
{
    UnitTracker* tracker = reinterpret_cast<UnitTracker*>(g_object_get_data(tbl, "tracker"));
    Glib::ustring const unit = tracker->getActiveUnit()->abbr;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setString("/tools/measure/unit", unit);
    MeasureTool *mt = get_measure_tool();
    if (mt) {
        mt->showCanvasItems();
    }
}

static void 
sp_toggle_ignore_1st_and_last( GtkToggleAction* act, gpointer data )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/measure/ignore_1st_and_last", active);
    SPDesktop *desktop = static_cast<SPDesktop *>(data);
    if ( active ) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Start and end measures inactive."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Start and end measures active."));
    }
    MeasureTool *mt = get_measure_tool();
    if (mt) {
        mt->showCanvasItems();
    }
}

static void 
sp_toggle_only_visible( GtkToggleAction* act, gpointer data )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/measure/only_visible", active);
    SPDesktop *desktop = static_cast<SPDesktop *>(data);
    if ( active ) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Show only visible crossings."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Show all crossings."));
    }
    MeasureTool *mt = get_measure_tool();
    if (mt) {
        mt->showCanvasItems();
    }
}

static void 
sp_toggle_all_layers( GtkToggleAction* act, gpointer data )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/measure/all_layers", active);
    SPDesktop *desktop = static_cast<SPDesktop *>(data);
    if ( active ) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Use all layers in the measure."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Use current layer in the measure."));
    }
    MeasureTool *mt = get_measure_tool();
    if (mt) {
        mt->showCanvasItems();
    }
}

static void 
sp_toggle_show_in_between( GtkToggleAction* act, gpointer data )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/tools/measure/show_in_between", active);
    SPDesktop *desktop = static_cast<SPDesktop *>(data);
    if ( active ) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Compute all elements."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Compute max length."));
    }
    MeasureTool *mt = get_measure_tool();
    if (mt) {
        mt->showCanvasItems();
    }
}
static void 
sp_reverse_knots(void){
    MeasureTool *mt = get_measure_tool();
    if (mt) {
        mt->reverseKnots();
    }
}

static void 
sp_to_mark_dimension(void){
    MeasureTool *mt = get_measure_tool();
    if (mt) {
        mt->toMarkDimension();
    }
}

static void 
sp_to_guides(void){
    MeasureTool *mt = get_measure_tool();
    if (mt) {
        mt->toGuides();
    }
}

static void 
sp_to_item(void){
    MeasureTool *mt = get_measure_tool();
    if (mt) {
        mt->toItem();
    }
}

void sp_measure_toolbox_prep(SPDesktop * desktop, GtkActionGroup* mainActions, GObject* holder)
{
    UnitTracker* tracker = new UnitTracker(Inkscape::Util::UNIT_TYPE_LINEAR);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    tracker->setActiveUnitByAbbr(prefs->getString("/tools/measure/unit").c_str());
    
    g_object_set_data( holder, "tracker", tracker );

    EgeAdjustmentAction *eact = 0;
    Inkscape::IconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);

    /* Font Size */
    {
        eact = create_adjustment_action( "MeasureFontSizeAction",
                                         _("Font Size"), _("Font Size:"),
                                         _("The font size to be used in the measurement labels"),
                                         "/tools/measure/fontsize", 10.0,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         1.0, 36.0, 1.0, 4.0,
                                         0, 0, 0,
                                         sp_measure_fontsize_value_changed, NULL, 0 , 2);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact));
    }

    /* units label */
    {
        EgeOutputAction* act = ege_output_action_new( "measure_units_label", _("Units:"), _("The units to be used for the measurements"), 0 );
        ege_output_action_set_use_markup( act, TRUE );
        g_object_set( act, "visible-overflown", FALSE, NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
    }

    /* units menu */
    {
        GtkAction* act = tracker->createAction( "MeasureUnitsAction", _("Units:"), _("The units to be used for the measurements") );
        g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(sp_measure_unit_changed), holder );
        gtk_action_group_add_action( mainActions, act );
    }

    /* Precission */
    {
        eact = create_adjustment_action( "MeasurePrecisionAction",
                                         _("Precision"), _("Precision:"),
                                         _("Decimal precision of measure"),
                                         "/tools/measure/precision", 2,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         0, 10, 1, 0,
                                         0, 0, 0,
                                         sp_measure_precision_value_changed, NULL, 0 ,0);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact));
    }

    /* Scale */
    {
        eact = create_adjustment_action( "MeasureScaleAction",
                                         _("Scale %"), _("Scale %:"),
                                         _("Scale the results"),
                                         "/tools/measure/scale", 100.0,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         0.0, 90000.0, 1.0, 4.0,
                                         0, 0, 0,
                                         sp_measure_scale_value_changed, NULL, 0 , 3);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* Offset */
    {
        eact = create_adjustment_action( "MeasureOffsetAction",
                                         _("Offset"), _("Offset:"),
                                         _("The offset size"),
                                         "/tools/measure/offset", 5.0,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         0.0, 90000.0, 1.0, 4.0,
                                         0, 0, 0,
                                         sp_measure_offset_value_changed, NULL, 0 , 2);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* ignore_1st_and_last */
    {
        InkToggleAction* act = ink_toggle_action_new( "MeasureIgnore1stAndLast",
                                                      _("Ignore first and last"),
                                                      _("Ignore first and last"),
                                                      INKSCAPE_ICON("draw-geometry-line-segment"),
                                                      secondarySize );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/measure/ignore_1st_and_last", true) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_ignore_1st_and_last), desktop) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }
    /* only visible */
    {
        InkToggleAction* act = ink_toggle_action_new( "MeasureOnlyVisible",
                                                      _("Only visible intersections"),
                                                      _("Only visible intersections"),
                                                      INKSCAPE_ICON("zoom"),
                                                      secondarySize );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/measure/only_visible", true) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_only_visible), desktop) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }
        /* measure imbetweens */
    {
        InkToggleAction* act = ink_toggle_action_new( "MeasureInBettween",
                                                      _("Show measures between items"),
                                                      _("Show measures between items"),
                                                      INKSCAPE_ICON("distribute-randomize"),
                                                      secondarySize );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/measure/show_in_between", true) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_show_in_between), desktop) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }
    /* measure only current layer */
    {
        InkToggleAction* act = ink_toggle_action_new( "MeasureAllLayers",
                                                      _("Measure all layers"),
                                                      _("Measure all layers"),
                                                      INKSCAPE_ICON("dialog-layers"),
                                                      secondarySize );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/measure/all_layers", true) );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_toggle_all_layers), desktop) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }
    /* toogle start end */
    {
        InkAction* act = ink_action_new( "MeasureReverse",
                                          _("Reverse measure"),
                                          _("Reverse measure"),
                                          INKSCAPE_ICON("draw-geometry-mirror"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(act), "activate", G_CALLBACK(sp_reverse_knots), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }
    /* to guides */
    {
        InkAction* act = ink_action_new( "MeasureToGuides",
                                          _("To guides"),
                                          _("To guides"),
                                          INKSCAPE_ICON("guides"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(act), "activate", G_CALLBACK(sp_to_guides), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }
    /* to mark dimensions */
    {
        InkAction* act = ink_action_new( "MeasureMarkDimension",
                                          _("Mark Dimension"),
                                          _("Mark Dimension"),
                                          INKSCAPE_ICON("tool-pointer"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(act), "activate", G_CALLBACK(sp_to_mark_dimension), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }
    /* to item */
    {
        InkAction* act = ink_action_new( "MeasureToItem",
                                          _("Convert to item"),
                                          _("Convert to item"),
                                          INKSCAPE_ICON("path-reverse"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(act), "activate", G_CALLBACK(sp_to_item), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
    }
} // end of sp_measure_toolbox_prep()


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
