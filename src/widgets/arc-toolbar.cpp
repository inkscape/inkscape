/**
 * @file
 * Arc aux toolbar
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

#include "arc-toolbar.h"
#include "desktop-handles.h"
#include "desktop.h"
#include "document-undo.h"
#include "ege-adjustment-action.h"
#include "ege-output-action.h"
#include "ege-select-one-action.h"
#include "ink-action.h"
#include "mod360.h"
#include "preferences.h"
#include "selection.h"
#include "sp-ellipse.h"
#include "toolbox.h"
#include "ui/icon-names.h"
#include "ui/uxmanager.h"
#include "verbs.h"
#include "widgets/spinbutton-events.h"
#include "xml/node-event-vector.h"
#include "xml/repr.h"

using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;

//########################
//##    Circle / Arc    ##
//########################

static void sp_arctb_sensitivize( GObject *tbl, double v1, double v2 )
{
    GtkAction *ocb = GTK_ACTION( g_object_get_data( tbl, "open_action" ) );
    GtkAction *make_whole = GTK_ACTION( g_object_get_data( tbl, "make_whole" ) );

    if (v1 == 0 && v2 == 0) {
        if (g_object_get_data( tbl, "single" )) { // only for a single selected ellipse (for now)
            gtk_action_set_sensitive( ocb, FALSE );
            gtk_action_set_sensitive( make_whole, FALSE );
        }
    } else {
        gtk_action_set_sensitive( ocb, TRUE );
        gtk_action_set_sensitive( make_whole, TRUE );
    }
}

static void
sp_arctb_startend_value_changed(GtkAdjustment *adj, GObject *tbl, gchar const *value_name, gchar const *other_name)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));

    if (DocumentUndo::getUndoSensitive(sp_desktop_document(desktop))) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble(Glib::ustring("/tools/shapes/arc/") + value_name, gtk_adjustment_get_value(adj));
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    gchar* namespaced_name = g_strconcat("sodipodi:", value_name, NULL);

    bool modmade = false;
    for (GSList const *items = sp_desktop_selection(desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        SPItem *item = SP_ITEM(items->data);

        if (SP_IS_GENERICELLIPSE(item)) {

            SPGenericEllipse *ge = SP_GENERICELLIPSE(item);

            if (!strcmp(value_name, "start")) {
                ge->start = (gtk_adjustment_get_value(adj) * M_PI)/ 180;
            } else {
                ge->end = (gtk_adjustment_get_value(adj) * M_PI)/ 180;
            }

            ge->normalize();
            (SP_OBJECT(ge))->updateRepr();
            (SP_OBJECT(ge))->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);

            modmade = true;
        }
    }

    g_free(namespaced_name);

    GtkAdjustment *other = GTK_ADJUSTMENT( g_object_get_data( tbl, other_name ) );

    sp_arctb_sensitivize( tbl, gtk_adjustment_get_value(adj), gtk_adjustment_get_value(other) );

    if (modmade) {
        DocumentUndo::maybeDone(sp_desktop_document(desktop), value_name, SP_VERB_CONTEXT_ARC,
                                _("Arc: Change start/end"));
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}


static void sp_arctb_start_value_changed(GtkAdjustment *adj,  GObject *tbl)
{
    sp_arctb_startend_value_changed(adj,  tbl, "start", "end");
}

static void sp_arctb_end_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_arctb_startend_value_changed(adj,  tbl, "end", "start");
}


static void sp_arctb_open_state_changed( EgeSelectOneAction *act, GObject *tbl )
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));
    if (DocumentUndo::getUndoSensitive(sp_desktop_document(desktop))) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setBool("/tools/shapes/arc/open", ege_select_one_action_get_active(act) != 0);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    bool modmade = false;

    if ( ege_select_one_action_get_active(act) != 0 ) {
        for (GSList const *items = sp_desktop_selection(desktop)->itemList();
             items != NULL;
             items = items->next)
        {
            SPItem *item = reinterpret_cast<SPItem*>(items->data);
            if (SP_IS_GENERICELLIPSE(item)) {
                Inkscape::XML::Node *repr = item->getRepr();
                repr->setAttribute("sodipodi:open", "true");
                item->updateRepr();
                modmade = true;
            }
        }
    } else {
        for (GSList const *items = sp_desktop_selection(desktop)->itemList();
             items != NULL;
             items = items->next)
        {
            SPItem *item = reinterpret_cast<SPItem *>(items->data);
            if (SP_IS_GENERICELLIPSE(item)) {
                Inkscape::XML::Node *repr = item->getRepr();
                repr->setAttribute("sodipodi:open", NULL);
                item->updateRepr();
                modmade = true;
            }
        }
    }

    if (modmade) {
        DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_ARC,
                           _("Arc: Change open/closed"));
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_arctb_defaults(GtkWidget *, GObject *obj)
{
    GtkAdjustment *adj;
    adj = GTK_ADJUSTMENT( g_object_get_data(obj, "start") );
    gtk_adjustment_set_value(adj, 0.0);
    gtk_adjustment_value_changed(adj);

    adj = GTK_ADJUSTMENT( g_object_get_data(obj, "end") );
    gtk_adjustment_set_value(adj, 0.0);
    gtk_adjustment_value_changed(adj);

    spinbutton_defocus(GTK_WIDGET(obj));
}

static void arc_tb_event_attr_changed(Inkscape::XML::Node *repr, gchar const * /*name*/,
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

    gdouble start = 0.;
    gdouble end = 0.;
    sp_repr_get_double(repr, "sodipodi:start", &start);
    sp_repr_get_double(repr, "sodipodi:end", &end);

    GtkAdjustment *adj1,*adj2;
    adj1 = GTK_ADJUSTMENT( g_object_get_data( tbl, "start" ) );
    gtk_adjustment_set_value(adj1, mod360((start * 180)/M_PI));
    adj2 = GTK_ADJUSTMENT( g_object_get_data( tbl, "end" ) );
    gtk_adjustment_set_value(adj2, mod360((end * 180)/M_PI));

    sp_arctb_sensitivize( tbl, gtk_adjustment_get_value(adj1), gtk_adjustment_get_value(adj2) );

    char const *openstr = NULL;
    openstr = repr->attribute("sodipodi:open");
    EgeSelectOneAction *ocb = EGE_SELECT_ONE_ACTION( g_object_get_data( tbl, "open_action" ) );

    if (openstr) {
        ege_select_one_action_set_active( ocb, 1 );
    } else {
        ege_select_one_action_set_active( ocb, 0 );
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static Inkscape::XML::NodeEventVector arc_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    arc_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


static void sp_arc_toolbox_selection_changed(Inkscape::Selection *selection, GObject *tbl)
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;

    purge_repr_listener( tbl, tbl );

    for (GSList const *items = selection->itemList();
         items != NULL;
         items = items->next)
    {
        SPItem *item = reinterpret_cast<SPItem *>(items->data);
        if (SP_IS_GENERICELLIPSE(item)) {
            n_selected++;
            repr = item->getRepr();
        }
    }

    EgeOutputAction* act = EGE_OUTPUT_ACTION( g_object_get_data( tbl, "mode_action" ) );

    g_object_set_data( tbl, "single", GINT_TO_POINTER(FALSE) );
    if (n_selected == 0) {
        g_object_set( G_OBJECT(act), "label", _("<b>New:</b>"), NULL );
    } else if (n_selected == 1) {
        g_object_set_data( tbl, "single", GINT_TO_POINTER(TRUE) );
        g_object_set( G_OBJECT(act), "label", _("<b>Change:</b>"), NULL );

        if (repr) {
            g_object_set_data( tbl, "repr", repr );
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &arc_tb_repr_events, tbl);
            sp_repr_synthesize_events(repr, &arc_tb_repr_events, tbl);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        g_object_set( G_OBJECT(act), "label", _("<b>Change:</b>"), NULL );
        sp_arctb_sensitivize( tbl, 1, 0 );
    }
}


void sp_arc_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    EgeAdjustmentAction* eact = 0;
    Inkscape::IconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);


    {
        EgeOutputAction* act = ege_output_action_new( "ArcStateAction", _("<b>New:</b>"), "", 0 );
        ege_output_action_set_use_markup( act, TRUE );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "mode_action", act );
    }

    /* Start */
    {
        eact = create_adjustment_action( "ArcStartAction",
                                         _("Start"), _("Start:"),
                                         _("The angle (in degrees) from the horizontal to the arc's start point"),
                                         "/tools/shapes/arc/start", 0.0,
                                         GTK_WIDGET(desktop->canvas), holder, TRUE, "altx-arc",
                                         -360.0, 360.0, 1.0, 10.0,
                                         0, 0, 0,
                                         sp_arctb_start_value_changed);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* End */
    {
        eact = create_adjustment_action( "ArcEndAction",
                                         _("End"), _("End:"),
                                         _("The angle (in degrees) from the horizontal to the arc's end point"),
                                         "/tools/shapes/arc/end", 0.0,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         -360.0, 360.0, 1.0, 10.0,
                                         0, 0, 0,
                                         sp_arctb_end_value_changed);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* Segments / Pie checkbox */
    {
        GtkListStore* model = gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

        GtkTreeIter iter;
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Closed arc"),
                            1, _("Switch to segment (closed shape with two radii)"),
                            2, INKSCAPE_ICON("draw-ellipse-segment"),
                            -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("Open Arc"),
                            1, _("Switch to arc (unclosed shape)"),
                            2, INKSCAPE_ICON("draw-ellipse-arc"),
                            -1 );

        EgeSelectOneAction* act = ege_select_one_action_new( "ArcOpenAction", (""), (""), NULL, GTK_TREE_MODEL(model) );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        g_object_set_data( holder, "open_action", act );

        ege_select_one_action_set_appearance( act, "full" );
        ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
        g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
        ege_select_one_action_set_icon_column( act, 2 );
        ege_select_one_action_set_icon_size( act, secondarySize );
        ege_select_one_action_set_tooltip_column( act, 1  );

        bool isClosed = !prefs->getBool("/tools/shapes/arc/open", false);
        ege_select_one_action_set_active( act, isClosed ? 0 : 1 );
        g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(sp_arctb_open_state_changed), holder );
    }

    /* Make Whole */
    {
        InkAction* inky = ink_action_new( "ArcResetAction",
                                          _("Make whole"),
                                          _("Make the shape a whole ellipse, not arc or segment"),
                                          INKSCAPE_ICON("draw-ellipse-whole"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_arctb_defaults), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        gtk_action_set_sensitive( GTK_ACTION(inky), TRUE );
        g_object_set_data( holder, "make_whole", inky );
    }

    g_object_set_data( G_OBJECT(holder), "single", GINT_TO_POINTER(TRUE) );
    // sensitivize make whole and open checkbox
    {
        GtkAdjustment *adj1 = GTK_ADJUSTMENT( g_object_get_data( holder, "start" ) );
        GtkAdjustment *adj2 = GTK_ADJUSTMENT( g_object_get_data( holder, "end" ) );
        sp_arctb_sensitivize( holder, gtk_adjustment_get_value(adj1), gtk_adjustment_get_value(adj2) );
    }


    sigc::connection *connection = new sigc::connection(
        sp_desktop_selection(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_arc_toolbox_selection_changed), G_OBJECT(holder)))
        );
    g_signal_connect( holder, "destroy", G_CALLBACK(delete_connection), connection );
    g_signal_connect( holder, "destroy", G_CALLBACK(purge_repr_listener), holder );
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
