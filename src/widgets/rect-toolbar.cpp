/**
 * @file
 * Rect aux toolbar
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

#include "rect-toolbar.h"

#include "desktop.h"
#include "document-undo.h"
#include "widgets/ege-adjustment-action.h"
#include "widgets/ege-output-action.h"
#include "widgets/ink-action.h"
#include "inkscape.h"
#include "preferences.h"
#include "selection.h"
#include "sp-namedview.h"
#include "sp-rect.h"
#include "toolbox.h"
#include "ui/icon-names.h"
#include "ui/tools/rect-tool.h"
#include "ui/uxmanager.h"
#include "ui/widget/unit-tracker.h"
#include "util/units.h"
#include "verbs.h"
#include "widgets/widget-sizes.h"
#include "xml/node-event-vector.h"
#include "xml/repr.h"

using Inkscape::UI::Widget::UnitTracker;
using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;
using Inkscape::Util::Unit;
using Inkscape::Util::Quantity;
using Inkscape::Util::unit_table;


//########################
//##       Rect         ##
//########################

static void sp_rtb_sensitivize( GObject *tbl )
{
    GtkAdjustment *adj1 = GTK_ADJUSTMENT( g_object_get_data(tbl, "rx") );
    GtkAdjustment *adj2 = GTK_ADJUSTMENT( g_object_get_data(tbl, "ry") );
    GtkAction* not_rounded = GTK_ACTION( g_object_get_data(tbl, "not_rounded") );

    if (gtk_adjustment_get_value(adj1) == 0 && gtk_adjustment_get_value(adj2) == 0 && g_object_get_data(tbl, "single")) { // only for a single selected rect (for now)
        gtk_action_set_sensitive( not_rounded, FALSE );
    } else {
        gtk_action_set_sensitive( not_rounded, TRUE );
    }
}


static void sp_rtb_value_changed(GtkAdjustment *adj, GObject *tbl, gchar const *value_name,
                                 void (SPRect::*setter)(gdouble))
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));

    UnitTracker* tracker = reinterpret_cast<UnitTracker*>(g_object_get_data( tbl, "tracker" ));
    Unit const *unit = tracker->getActiveUnit();
    g_return_if_fail(unit != NULL);

    if (DocumentUndo::getUndoSensitive(desktop->getDocument())) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble(Glib::ustring("/tools/shapes/rect/") + value_name,
            Quantity::convert(gtk_adjustment_get_value(adj), unit, "px"));
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" ) || tracker->isUpdating()) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE));

    bool modmade = false;
    Inkscape::Selection *selection = desktop->getSelection();
    std::vector<SPItem*> itemlist=selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        if (SP_IS_RECT(*i)) {
            if (gtk_adjustment_get_value(adj) != 0) {
                (SP_RECT(*i)->*setter)(Quantity::convert(gtk_adjustment_get_value(adj), unit, "px"));
            } else {
                (*i)->getRepr()->setAttribute(value_name, NULL);
            }
            modmade = true;
        }
    }

    sp_rtb_sensitivize( tbl );

    if (modmade) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_RECT,
                           _("Change rectangle"));
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_rtb_rx_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_rtb_value_changed(adj, tbl, "rx", &SPRect::setVisibleRx);
}

static void sp_rtb_ry_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_rtb_value_changed(adj, tbl, "ry", &SPRect::setVisibleRy);
}

static void sp_rtb_width_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_rtb_value_changed(adj, tbl, "width", &SPRect::setVisibleWidth);
}

static void sp_rtb_height_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_rtb_value_changed(adj, tbl, "height", &SPRect::setVisibleHeight);
}



static void sp_rtb_defaults( GtkWidget * /*widget*/, GObject *obj)
{
    GtkAdjustment *adj = 0;

    adj = GTK_ADJUSTMENT( g_object_get_data(obj, "rx") );
    gtk_adjustment_set_value(adj, 0.0);
    // this is necessary if the previous value was 0, but we still need to run the callback to change all selected objects
    gtk_adjustment_value_changed(adj);

    adj = GTK_ADJUSTMENT( g_object_get_data(obj, "ry") );
    gtk_adjustment_set_value(adj, 0.0);
    gtk_adjustment_value_changed(adj);

    sp_rtb_sensitivize( obj );
}

static void rect_tb_event_attr_changed(Inkscape::XML::Node * /*repr*/, gchar const * /*name*/,
                                       gchar const * /*old_value*/, gchar const * /*new_value*/,
                                       bool /*is_interactive*/, gpointer data)
{
    GObject *tbl = G_OBJECT(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    UnitTracker* tracker = reinterpret_cast<UnitTracker*>( g_object_get_data( tbl, "tracker" ) );
    Unit const *unit = tracker->getActiveUnit();
    g_return_if_fail(unit != NULL);

    gpointer item = g_object_get_data( tbl, "item" );
    if (item && SP_IS_RECT(item)) {
        {
            GtkAdjustment *adj = GTK_ADJUSTMENT( g_object_get_data( tbl, "rx" ) );

            gdouble rx = SP_RECT(item)->getVisibleRx();
            gtk_adjustment_set_value(adj, Quantity::convert(rx, "px", unit));
        }

        {
            GtkAdjustment *adj = GTK_ADJUSTMENT( g_object_get_data( tbl, "ry" ) );

            gdouble ry = SP_RECT(item)->getVisibleRy();
            gtk_adjustment_set_value(adj, Quantity::convert(ry, "px", unit));
        }

        {
            GtkAdjustment *adj = GTK_ADJUSTMENT( g_object_get_data( tbl, "width" ) );

            gdouble width = SP_RECT(item)->getVisibleWidth();
            gtk_adjustment_set_value(adj, Quantity::convert(width, "px", unit));
        }

        {
            GtkAdjustment *adj = GTK_ADJUSTMENT( g_object_get_data( tbl, "height" ) );

            gdouble height = SP_RECT(item)->getVisibleHeight();
            gtk_adjustment_set_value(adj, Quantity::convert(height, "px", unit));
        }
    }

    sp_rtb_sensitivize( tbl );

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}


static Inkscape::XML::NodeEventVector rect_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    rect_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

/**
 *  \param selection should not be NULL.
 */
static void sp_rect_toolbox_selection_changed(Inkscape::Selection *selection, GObject *tbl)
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;
    SPItem *item = NULL;

    if ( g_object_get_data( tbl, "repr" ) ) {
        g_object_set_data( tbl, "item", NULL );
    }
    purge_repr_listener( tbl, tbl );

    std::vector<SPItem*> itemlist=selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        if (SP_IS_RECT(*i)) {
            n_selected++;
            item = *i;
            repr = item->getRepr();
        }
    }

    EgeOutputAction* act = EGE_OUTPUT_ACTION( g_object_get_data( tbl, "mode_action" ) );

    g_object_set_data( tbl, "single", GINT_TO_POINTER(FALSE) );

    if (n_selected == 0) {
        g_object_set( G_OBJECT(act), "label", _("<b>New:</b>"), NULL );

        GtkAction* w = GTK_ACTION( g_object_get_data( tbl, "width_action" ) );
        gtk_action_set_sensitive(w, FALSE);
        GtkAction* h = GTK_ACTION( g_object_get_data( tbl, "height_action" ) );
        gtk_action_set_sensitive(h, FALSE);

    } else if (n_selected == 1) {
        g_object_set( G_OBJECT(act), "label", _("<b>Change:</b>"), NULL );
        g_object_set_data( tbl, "single", GINT_TO_POINTER(TRUE) );

        GtkAction* w = GTK_ACTION( g_object_get_data( tbl, "width_action" ) );
        gtk_action_set_sensitive(w, TRUE);
        GtkAction* h = GTK_ACTION( g_object_get_data( tbl, "height_action" ) );
        gtk_action_set_sensitive(h, TRUE);

        if (repr) {
            g_object_set_data( tbl, "repr", repr );
            g_object_set_data( tbl, "item", item );
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &rect_tb_repr_events, tbl);
            sp_repr_synthesize_events(repr, &rect_tb_repr_events, tbl);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        g_object_set( G_OBJECT(act), "label", _("<b>Change:</b>"), NULL );
        sp_rtb_sensitivize( tbl );
    }
}

static void rect_toolbox_watch_ec(SPDesktop* dt, Inkscape::UI::Tools::ToolBase* ec, GObject* holder);

void sp_rect_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    EgeAdjustmentAction* eact = 0;
    Inkscape::IconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);

    {
        EgeOutputAction* act = ege_output_action_new( "RectStateAction", _("<b>New:</b>"), "", 0 );
        ege_output_action_set_use_markup( act, TRUE );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "mode_action", act );
    }

    // rx/ry units menu: create
    UnitTracker* tracker = new UnitTracker(Inkscape::Util::UNIT_TYPE_LINEAR);
    //tracker->addUnit( SP_UNIT_PERCENT, 0 );
    // fixme: add % meaning per cent of the width/height
    tracker->setActiveUnit(unit_table.getUnit("px"));
    g_object_set_data( holder, "tracker", tracker );

    /* W */
    {
        gchar const* labels[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {1, 2, 3, 5, 10, 20, 50, 100, 200, 500};
        eact = create_adjustment_action( "RectWidthAction",
                                         _("Width"), _("W:"), _("Width of rectangle"),
                                         "/tools/shapes/rect/width", 0,
                                         GTK_WIDGET(desktop->canvas), holder, TRUE, "altx-rect",
                                         0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_rtb_width_value_changed, tracker);
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );
        g_object_set_data( holder, "width_action", eact );
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* H */
    {
        gchar const* labels[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {1, 2, 3, 5, 10, 20, 50, 100, 200, 500};
        eact = create_adjustment_action( "RectHeightAction",
                                         _("Height"), _("H:"), _("Height of rectangle"),
                                         "/tools/shapes/rect/height", 0,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_rtb_height_value_changed, tracker);
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );
        g_object_set_data( holder, "height_action", eact );
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* rx */
    {
        gchar const* labels[] = {_("not rounded"), 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {0.5, 1, 2, 3, 5, 10, 20, 50, 100};
        eact = create_adjustment_action( "RadiusXAction",
                                         _("Horizontal radius"), _("Rx:"), _("Horizontal radius of rounded corners"),
                                         "/tools/shapes/rect/rx", 0,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_rtb_rx_value_changed, tracker);
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* ry */
    {
        gchar const* labels[] = {_("not rounded"), 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {0.5, 1, 2, 3, 5, 10, 20, 50, 100};
        eact = create_adjustment_action( "RadiusYAction",
                                         _("Vertical radius"), _("Ry:"), _("Vertical radius of rounded corners"),
                                         "/tools/shapes/rect/ry", 0,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_rtb_ry_value_changed, tracker);
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    // add the units menu
    {
        GtkAction* act = tracker->createAction( "RectUnitsAction", _("Units"), ("") );
        gtk_action_group_add_action( mainActions, act );
    }

    /* Reset */
    {
        InkAction* inky = ink_action_new( "RectResetAction",
                                          _("Not rounded"),
                                          _("Make corners sharp"),
                                          INKSCAPE_ICON("rectangle-make-corners-sharp"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_rtb_defaults), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        gtk_action_set_sensitive( GTK_ACTION(inky), TRUE );
        g_object_set_data( holder, "not_rounded", inky );
    }

    g_object_set_data( holder, "single", GINT_TO_POINTER(TRUE) );
    sp_rtb_sensitivize( holder );

    desktop->connectEventContextChanged(sigc::bind(sigc::ptr_fun(rect_toolbox_watch_ec), holder));
    g_signal_connect( holder, "destroy", G_CALLBACK(purge_repr_listener), holder );
}

static void rect_toolbox_watch_ec(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* ec, GObject* holder)
{
    static sigc::connection changed;

    // use of dynamic_cast<> seems wrong here -- we just need to check the current tool

    if (dynamic_cast<Inkscape::UI::Tools::RectTool *>(ec)) {
        Inkscape::Selection *sel = desktop->getSelection();

        changed = sel->connectChanged(sigc::bind(sigc::ptr_fun(sp_rect_toolbox_selection_changed), holder));

        // Synthesize an emission to trigger the update
        sp_rect_toolbox_selection_changed(sel, holder);
    } else {
        if (changed) {
            changed.disconnect();
            purge_repr_listener(NULL, holder);
        }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
