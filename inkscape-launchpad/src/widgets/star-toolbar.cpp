/**
 * @file
 * Star aux toolbar
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

#include "star-toolbar.h"

#include "desktop.h"
#include "document-undo.h"
#include "widgets/ege-adjustment-action.h"
#include "widgets/ege-output-action.h"
#include "widgets/ege-select-one-action.h"
#include "widgets/ink-action.h"
#include "selection.h"
#include "sp-star.h"
#include "toolbox.h"
#include "ui/icon-names.h"
#include "ui/tools/star-tool.h"
#include "ui/uxmanager.h"
#include "verbs.h"
#include "widgets/../preferences.h"
#include "xml/node-event-vector.h"
#include "xml/node.h"
#include "xml/repr.h"

using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;


//########################
//##       Star         ##
//########################

static void sp_stb_magnitude_value_changed( GtkAdjustment *adj, GObject *dataKludge )
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( dataKludge, "desktop" ));

    if (DocumentUndo::getUndoSensitive(desktop->getDocument())) {
        // do not remember prefs if this call is initiated by an undo change, because undoing object
        // creation sets bogus values to its attributes before it is deleted
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setInt("/tools/shapes/star/magnitude",
            (gint)gtk_adjustment_get_value(adj));
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( dataKludge, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(TRUE) );

    bool modmade = false;

    Inkscape::Selection *selection = desktop->getSelection();
    std::vector<SPItem*> itemlist=selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;
        if (SP_IS_STAR(item)) {
            Inkscape::XML::Node *repr = item->getRepr();
            sp_repr_set_int(repr,"sodipodi:sides",
                (gint)gtk_adjustment_get_value(adj));
            double arg1 = 0.5;
            sp_repr_get_double(repr, "sodipodi:arg1", &arg1);
            sp_repr_set_svg_double(repr, "sodipodi:arg2",
                                   (arg1 + M_PI / (gint)gtk_adjustment_get_value(adj)));
            item->updateRepr();
            modmade = true;
        }
    }
    if (modmade) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_STAR,
                           _("Star: Change number of corners"));
    }

    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_stb_proportion_value_changed( GtkAdjustment *adj, GObject *dataKludge )
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( dataKludge, "desktop" ));

    if (DocumentUndo::getUndoSensitive(desktop->getDocument())) {
        if (!IS_NAN(gtk_adjustment_get_value(adj))) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            prefs->setDouble("/tools/shapes/star/proportion",
                gtk_adjustment_get_value(adj));
        }
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( dataKludge, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(TRUE) );

    bool modmade = false;
    Inkscape::Selection *selection = desktop->getSelection();
    std::vector<SPItem*> itemlist=selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;
        if (SP_IS_STAR(item)) {
            Inkscape::XML::Node *repr = item->getRepr();

            gdouble r1 = 1.0;
            gdouble r2 = 1.0;
            sp_repr_get_double(repr, "sodipodi:r1", &r1);
            sp_repr_get_double(repr, "sodipodi:r2", &r2);
            if (r2 < r1) {
                sp_repr_set_svg_double(repr, "sodipodi:r2",
                r1*gtk_adjustment_get_value(adj));
            } else {
                sp_repr_set_svg_double(repr, "sodipodi:r1",
                r2*gtk_adjustment_get_value(adj));
            }

            item->updateRepr();
            modmade = true;
        }
    }

    if (modmade) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_STAR,
                           _("Star: Change spoke ratio"));
    }

    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_stb_sides_flat_state_changed( EgeSelectOneAction *act, GObject *dataKludge )
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( dataKludge, "desktop" ));
    bool flat = ege_select_one_action_get_active( act ) == 0;

    if (DocumentUndo::getUndoSensitive(desktop->getDocument())) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setBool( "/tools/shapes/star/isflatsided", flat);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( dataKludge, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(TRUE) );

    Inkscape::Selection *selection = desktop->getSelection();
    GtkAction* prop_action = GTK_ACTION( g_object_get_data( dataKludge, "prop_action" ) );
    bool modmade = false;

    if ( prop_action ) {
        gtk_action_set_visible( prop_action, !flat );
    }

    std::vector<SPItem*> itemlist=selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;
        if (SP_IS_STAR(item)) {
            Inkscape::XML::Node *repr = item->getRepr();
            repr->setAttribute("inkscape:flatsided", flat ? "true" : "false" );
            item->updateRepr();
            modmade = true;
        }
    }

    if (modmade) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_STAR,
                           flat ? _("Make polygon") : _("Make star"));
    }

    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_stb_rounded_value_changed( GtkAdjustment *adj, GObject *dataKludge )
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( dataKludge, "desktop" ));

    if (DocumentUndo::getUndoSensitive(desktop->getDocument())) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble("/tools/shapes/star/rounded", (gdouble) gtk_adjustment_get_value(adj));
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( dataKludge, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(TRUE) );

    bool modmade = false;

    Inkscape::Selection *selection = desktop->getSelection();
    std::vector<SPItem*> itemlist=selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;
        if (SP_IS_STAR(item)) {
            Inkscape::XML::Node *repr = item->getRepr();
            sp_repr_set_svg_double(repr, "inkscape:rounded",
                (gdouble) gtk_adjustment_get_value(adj));
            item->updateRepr();
            modmade = true;
        }
    }
    if (modmade) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_STAR,
                           _("Star: Change rounding"));
    }

    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_stb_randomized_value_changed( GtkAdjustment *adj, GObject *dataKludge )
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( dataKludge, "desktop" ));

    if (DocumentUndo::getUndoSensitive(desktop->getDocument())) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble("/tools/shapes/star/randomized",
            (gdouble) gtk_adjustment_get_value(adj));
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( dataKludge, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(TRUE) );

    bool modmade = false;

    Inkscape::Selection *selection = desktop->getSelection();
    std::vector<SPItem*> itemlist=selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;
        if (SP_IS_STAR(item)) {
            Inkscape::XML::Node *repr = item->getRepr();
            sp_repr_set_svg_double(repr, "inkscape:randomized",
                (gdouble) gtk_adjustment_get_value(adj));
            item->updateRepr();
            modmade = true;
        }
    }
    if (modmade) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_STAR,
                           _("Star: Change randomization"));
    }

    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(FALSE) );
}


static void star_tb_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
                                       gchar const * /*old_value*/, gchar const * /*new_value*/,
                                       bool /*is_interactive*/, gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    GtkAdjustment *adj = 0;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool isFlatSided = prefs->getBool("/tools/shapes/star/isflatsided", true);

    if (!strcmp(name, "inkscape:randomized")) {
        adj = GTK_ADJUSTMENT( g_object_get_data(G_OBJECT(tbl), "randomized") );
        double randomized = 0.0;
        sp_repr_get_double(repr, "inkscape:randomized", &randomized);
        gtk_adjustment_set_value(adj, randomized);
    } else if (!strcmp(name, "inkscape:rounded")) {
        adj = GTK_ADJUSTMENT( g_object_get_data(G_OBJECT(tbl), "rounded") );
        double rounded = 0.0;
        sp_repr_get_double(repr, "inkscape:rounded", &rounded);
        gtk_adjustment_set_value(adj, rounded);
    } else if (!strcmp(name, "inkscape:flatsided")) {
        GtkAction* prop_action = GTK_ACTION( g_object_get_data(G_OBJECT(tbl), "prop_action") );
        char const *flatsides = repr->attribute("inkscape:flatsided");
        EgeSelectOneAction* flat_action = EGE_SELECT_ONE_ACTION( g_object_get_data( G_OBJECT(tbl), "flat_action" ) );
        if ( flatsides && !strcmp(flatsides,"false") ) {
            ege_select_one_action_set_active( flat_action, 1 );
            gtk_action_set_visible( prop_action, TRUE );
        } else {
            ege_select_one_action_set_active( flat_action, 0 );
            gtk_action_set_visible( prop_action, FALSE );
        }
    } else if ((!strcmp(name, "sodipodi:r1") || !strcmp(name, "sodipodi:r2")) && (!isFlatSided) ) {
        adj = GTK_ADJUSTMENT(g_object_get_data(G_OBJECT(tbl), "proportion"));
        gdouble r1 = 1.0;
        gdouble r2 = 1.0;
        sp_repr_get_double(repr, "sodipodi:r1", &r1);
        sp_repr_get_double(repr, "sodipodi:r2", &r2);
        if (r2 < r1) {
            gtk_adjustment_set_value(adj, r2/r1);
        } else {
            gtk_adjustment_set_value(adj, r1/r2);
        }
    } else if (!strcmp(name, "sodipodi:sides")) {
        adj = GTK_ADJUSTMENT(g_object_get_data(G_OBJECT(tbl), "magnitude"));
        int sides = 0;
        sp_repr_get_int(repr, "sodipodi:sides", &sides);
        gtk_adjustment_set_value(adj, sides);
    }

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));
}


static Inkscape::XML::NodeEventVector star_tb_repr_events =
{
    NULL, /* child_added */
    NULL, /* child_removed */
    star_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


/**
 *  \param selection Should not be NULL.
 */
static void
sp_star_toolbox_selection_changed(Inkscape::Selection *selection, GObject *tbl)
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;

    purge_repr_listener( tbl, tbl );

    std::vector<SPItem*> itemlist=selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;
        if (SP_IS_STAR(item)) {
            n_selected++;
            repr = item->getRepr();
        }
    }

    EgeOutputAction* act = EGE_OUTPUT_ACTION( g_object_get_data( tbl, "mode_action" ) );

    if (n_selected == 0) {
        g_object_set( G_OBJECT(act), "label", _("<b>New:</b>"), NULL );
    } else if (n_selected == 1) {
        g_object_set( G_OBJECT(act), "label", _("<b>Change:</b>"), NULL );

        if (repr) {
            g_object_set_data( tbl, "repr", repr );
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &star_tb_repr_events, tbl);
            sp_repr_synthesize_events(repr, &star_tb_repr_events, tbl);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected stars
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Change:</b>"));
    }
}


static void sp_stb_defaults( GtkWidget * /*widget*/, GObject *dataKludge )
{
    // FIXME: in this and all other _default functions, set some flag telling the value_changed
    // callbacks to lump all the changes for all selected objects in one undo step

    GtkAdjustment *adj = 0;

    // fixme: make settable in prefs!
    gint mag = 5;
    gdouble prop = 0.5;
    gboolean flat = FALSE;
    gdouble randomized = 0;
    gdouble rounded = 0;

    EgeSelectOneAction* flat_action = EGE_SELECT_ONE_ACTION( g_object_get_data( dataKludge, "flat_action" ) );
    ege_select_one_action_set_active( flat_action, flat ? 0 : 1 );

    GtkAction* sb2 = GTK_ACTION( g_object_get_data( dataKludge, "prop_action" ) );
    gtk_action_set_visible( sb2, !flat );

    adj = GTK_ADJUSTMENT( g_object_get_data( dataKludge, "magnitude" ) );
    gtk_adjustment_set_value(adj, mag);
    gtk_adjustment_value_changed(adj);

    adj = GTK_ADJUSTMENT( g_object_get_data( dataKludge, "proportion" ) );
    gtk_adjustment_set_value(adj, prop);
    gtk_adjustment_value_changed(adj);

    adj = GTK_ADJUSTMENT( g_object_get_data( dataKludge, "rounded" ) );
    gtk_adjustment_set_value(adj, rounded);
    gtk_adjustment_value_changed(adj);

    adj = GTK_ADJUSTMENT( g_object_get_data( dataKludge, "randomized" ) );
    gtk_adjustment_set_value(adj, randomized);
    gtk_adjustment_value_changed(adj);
}

static void star_toolbox_watch_ec(SPDesktop* dt, Inkscape::UI::Tools::ToolBase* ec, GObject* holder);

void sp_star_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    Inkscape::IconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);

    {
        EgeOutputAction* act = ege_output_action_new( "StarStateAction", _("<b>New:</b>"), "", 0 );
        ege_output_action_set_use_markup( act, TRUE );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "mode_action", act );
    }

    {
        EgeAdjustmentAction* eact = 0;
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        bool isFlatSided = prefs->getBool("/tools/shapes/star/isflatsided", true);

        /* Flatsided checkbox */
        {
            GtkListStore* model = gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

            GtkTreeIter iter;
            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter,
                                0, _("Polygon"),
                                1, _("Regular polygon (with one handle) instead of a star"),
                                2, INKSCAPE_ICON("draw-polygon"),
                                -1 );

            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter,
                                0, _("Star"),
                                1, _("Star instead of a regular polygon (with one handle)"),
                                2, INKSCAPE_ICON("draw-star"),
                                -1 );

            EgeSelectOneAction* act = ege_select_one_action_new( "FlatAction", (""), (""), NULL, GTK_TREE_MODEL(model) );
            gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
            g_object_set_data( holder, "flat_action", act );

            ege_select_one_action_set_appearance( act, "full" );
            ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
            g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
            ege_select_one_action_set_icon_column( act, 2 );
            ege_select_one_action_set_icon_size( act, secondarySize );
            ege_select_one_action_set_tooltip_column( act, 1  );

            ege_select_one_action_set_active( act, isFlatSided ? 0 : 1 );
            g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(sp_stb_sides_flat_state_changed), holder);
        }

        /* Magnitude */
        {
        gchar const* labels[] = {_("triangle/tri-star"), _("square/quad-star"), _("pentagon/five-pointed star"), _("hexagon/six-pointed star"), 0, 0, 0, 0, 0};
        gdouble values[] = {3, 4, 5, 6, 7, 8, 10, 12, 20};
        eact = create_adjustment_action( "MagnitudeAction",
                                         _("Corners"), _("Corners:"), _("Number of corners of a polygon or star"),
                                         "/tools/shapes/star/magnitude", 3,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         3, 1024, 1, 5,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_stb_magnitude_value_changed, NULL /*unit tracker*/,
                                         1.0, 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        /* Spoke ratio */
        {
        gchar const* labels[] = {_("thin-ray star"), 0, _("pentagram"), _("hexagram"), _("heptagram"), _("octagram"), _("regular polygon")};
        gdouble values[] = {0.01, 0.2, 0.382, 0.577, 0.692, 0.765, 1};
        eact = create_adjustment_action( "SpokeAction",
                                         _("Spoke ratio"), _("Spoke ratio:"),
                                         // TRANSLATORS: Tip radius of a star is the distance from the center to the farthest handle.
                                         // Base radius is the same for the closest handle.
                                         _("Base radius to tip radius ratio"),
                                         "/tools/shapes/star/proportion", 0.5,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         0.01, 1.0, 0.01, 0.1,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_stb_proportion_value_changed );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        g_object_set_data( holder, "prop_action", eact );
        }

        if ( !isFlatSided ) {
            gtk_action_set_visible( GTK_ACTION(eact), TRUE );
        } else {
            gtk_action_set_visible( GTK_ACTION(eact), FALSE );
        }

        /* Roundedness */
        {
        gchar const* labels[] = {_("stretched"), _("twisted"), _("slightly pinched"), _("NOT rounded"), _("slightly rounded"), _("visibly rounded"), _("well rounded"), _("amply rounded"), 0, _("stretched"), _("blown up")};
        gdouble values[] = {-1, -0.2, -0.03, 0, 0.05, 0.1, 0.2, 0.3, 0.5, 1, 10};
        eact = create_adjustment_action( "RoundednessAction",
                                         _("Rounded"), _("Rounded:"), _("How much rounded are the corners (0 for sharp)"),
                                         "/tools/shapes/star/rounded", 0.0,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         -10.0, 10.0, 0.01, 0.1,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_stb_rounded_value_changed );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        /* Randomization */
        {
        gchar const* labels[] = {_("NOT randomized"), _("slightly irregular"), _("visibly randomized"), _("strongly randomized"), _("blown up")};
        gdouble values[] = {0, 0.01, 0.1, 0.5, 10};
        eact = create_adjustment_action( "RandomizationAction",
                                         _("Randomized"), _("Randomized:"), _("Scatter randomly the corners and angles"),
                                         "/tools/shapes/star/randomized", 0.0,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         -10.0, 10.0, 0.001, 0.01,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_stb_randomized_value_changed, NULL /*unit tracker*/, 0.1, 3 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }
    }

    {
        /* Reset */
        {
            InkAction* inky = ink_action_new( "StarResetAction",
                                             _("Defaults"),
                                             _("Reset shape parameters to defaults (use Inkscape Preferences > Tools to change defaults)"),
                                             INKSCAPE_ICON("edit-clear"),
                                             Inkscape::ICON_SIZE_SMALL_TOOLBAR);
            g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_stb_defaults), holder );
            gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
            gtk_action_set_sensitive( GTK_ACTION(inky), TRUE );
        }
    }

    desktop->connectEventContextChanged(sigc::bind(sigc::ptr_fun(star_toolbox_watch_ec), holder));
    g_signal_connect(holder, "destroy", G_CALLBACK(purge_repr_listener), holder);
}

static void star_toolbox_watch_ec(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* ec, GObject* holder)
{
    static sigc::connection changed;

    if (dynamic_cast<Inkscape::UI::Tools::StarTool const*>(ec) != NULL) {
        changed = desktop->getSelection()->connectChanged(sigc::bind(sigc::ptr_fun(sp_star_toolbox_selection_changed), holder));
        sp_star_toolbox_selection_changed(desktop->getSelection(), holder);
    } else {
        if (changed)
            changed.disconnect();
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
