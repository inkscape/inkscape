/**
 * @file
 * Connector aux toolbar
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

#include "connector-toolbar.h"
#include "conn-avoid-ref.h"

#include "desktop.h"
#include "document-undo.h"
#include "widgets/ege-adjustment-action.h"
#include "enums.h"
#include "graphlayout.h"
#include "widgets/ink-action.h"
#include "inkscape.h"
#include "preferences.h"
#include "selection.h"
#include "sp-namedview.h"
#include "sp-path.h"
#include "toolbox.h"
#include "ui/icon-names.h"
#include "ui/tools/connector-tool.h"
#include "ui/uxmanager.h"
#include "verbs.h"
#include "widgets/spinbutton-events.h"
#include "xml/node-event-vector.h"
#include "xml/repr.h"

using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;

//#########################
//##      Connector      ##
//#########################

static void sp_connector_path_set_avoid(void)
{
    Inkscape::UI::Tools::cc_selection_set_avoid(true);
}


static void sp_connector_path_set_ignore(void)
{
    Inkscape::UI::Tools::cc_selection_set_avoid(false);
}

static void sp_connector_orthogonal_toggled( GtkToggleAction* act, GObject *tbl )
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));
    SPDocument *doc = desktop->getDocument();

    if (!DocumentUndo::getUndoSensitive(doc)) {
        return;
    }


    // quit if run by the _changed callbacks
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    bool is_orthog = gtk_toggle_action_get_active( act );
    gchar orthog_str[] = "orthogonal";
    gchar polyline_str[] = "polyline";
    gchar *value = is_orthog ? orthog_str : polyline_str ;

    bool modmade = false;
    std::vector<SPItem*> itemlist=desktop->getSelection()->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;

        if (Inkscape::UI::Tools::cc_item_is_connector(item)) {
            item->setAttribute( "inkscape:connector-type",
                    value, NULL);
            item->avoidRef->handleSettingChange();
            modmade = true;
        }
    }

    if (!modmade) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setBool("/tools/connector/orthogonal", is_orthog);
    } else {

        DocumentUndo::done(doc, SP_VERB_CONTEXT_CONNECTOR,
                       is_orthog ? _("Set connector type: orthogonal"): _("Set connector type: polyline"));
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void connector_curvature_changed(GtkAdjustment *adj, GObject* tbl)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));
    SPDocument *doc = desktop->getDocument();

    if (!DocumentUndo::getUndoSensitive(doc)) {
        return;
    }


    // quit if run by the _changed callbacks
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE) );

    gdouble newValue = gtk_adjustment_get_value(adj);
    gchar value[G_ASCII_DTOSTR_BUF_SIZE];
    g_ascii_dtostr(value, G_ASCII_DTOSTR_BUF_SIZE, newValue);

    bool modmade = false;
    std::vector<SPItem*> itemlist=desktop->getSelection()->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;

        if (Inkscape::UI::Tools::cc_item_is_connector(item)) {
            item->setAttribute( "inkscape:connector-curvature",
                    value, NULL);
            item->avoidRef->handleSettingChange();
            modmade = true;
        }
    }

    if (!modmade) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble(Glib::ustring("/tools/connector/curvature"), newValue);
    }
    else {
        DocumentUndo::done(doc, SP_VERB_CONTEXT_CONNECTOR,
                       _("Change connector curvature"));
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}


static void connector_spacing_changed(GtkAdjustment *adj, GObject* tbl)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));
    SPDocument *doc = desktop->getDocument();

    if (!DocumentUndo::getUndoSensitive(doc)) {
        return;
    }

    Inkscape::XML::Node *repr = desktop->namedview->getRepr();

    if ( !repr->attribute("inkscape:connector-spacing") &&
            ( gtk_adjustment_get_value(adj) == defaultConnSpacing )) {
        // Don't need to update the repr if the attribute doesn't
        // exist and it is being set to the default value -- as will
        // happen at startup.
        return;
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE));

    sp_repr_set_css_double(repr, "inkscape:connector-spacing", gtk_adjustment_get_value(adj));
    desktop->namedview->updateRepr();
    bool modmade = false;

    std::vector<SPItem *> items;
    items = get_avoided_items(items, desktop->currentRoot(), desktop);
    for (std::vector<SPItem *>::const_iterator iter = items.begin(); iter != items.end(); ++iter ) {
        SPItem *item = *iter;
        Geom::Affine m = Geom::identity();
        avoid_item_move(&m, item);
        modmade = true;
    }

    if(modmade) {
        DocumentUndo::done(doc, SP_VERB_CONTEXT_CONNECTOR,
                       _("Change connector spacing"));
    }
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_connector_graph_layout(void)
{
    if (!SP_ACTIVE_DESKTOP) {
        return;
    }
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // hack for clones, see comment in align-and-distribute.cpp
    int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
    prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

    graphlayout(SP_ACTIVE_DESKTOP->getSelection()->itemList());

    prefs->setInt("/options/clonecompensation/value", saved_compensation);

    DocumentUndo::done(SP_ACTIVE_DESKTOP->getDocument(), SP_VERB_DIALOG_ALIGN_DISTRIBUTE, _("Arrange connector network"));
}

static void sp_directed_graph_layout_toggled( GtkToggleAction* act, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/connector/directedlayout",
                gtk_toggle_action_get_active( act ));
}

static void sp_nooverlaps_graph_layout_toggled( GtkToggleAction* act, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/tools/connector/avoidoverlaplayout",
                gtk_toggle_action_get_active( act ));
}


static void connector_length_changed(GtkAdjustment *adj, GObject* /*tbl*/)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble("/tools/connector/length", gtk_adjustment_get_value(adj));
}

static void connector_tb_event_attr_changed(Inkscape::XML::Node *repr,
                                            gchar const *name, gchar const * /*old_value*/, gchar const * /*new_value*/,
                                            bool /*is_interactive*/, gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    if ( !g_object_get_data(G_OBJECT(tbl), "freeze")
         && (strcmp(name, "inkscape:connector-spacing") == 0) ) {
        GtkAdjustment *adj = static_cast<GtkAdjustment*>(g_object_get_data(G_OBJECT(tbl), "spacing"));
        gdouble spacing = defaultConnSpacing;
        sp_repr_get_double(repr, "inkscape:connector-spacing", &spacing);

        gtk_adjustment_set_value(adj, spacing);
        gtk_adjustment_value_changed(adj);

        spinbutton_defocus(tbl);
    }
}

static Inkscape::XML::NodeEventVector connector_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    connector_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

static void sp_connector_toolbox_selection_changed(Inkscape::Selection *selection, GObject *tbl)
{
    GtkAdjustment *adj = GTK_ADJUSTMENT( g_object_get_data( tbl, "curvature" ) );
    GtkToggleAction *act = GTK_TOGGLE_ACTION( g_object_get_data( tbl, "orthogonal" ) );
    SPItem *item = selection->singleItem();
    if (SP_IS_PATH(item))
    {
        gdouble curvature = SP_PATH(item)->connEndPair.getCurvature();
        bool is_orthog = SP_PATH(item)->connEndPair.isOrthogonal();
        gtk_toggle_action_set_active(act, is_orthog);
        gtk_adjustment_set_value(adj, curvature);
    }

}

void sp_connector_toolbox_prep( SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Inkscape::IconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);

    {
        InkAction* inky = ink_action_new( "ConnectorAvoidAction",
                                          _("Avoid"),
                                          _("Make connectors avoid selected objects"),
                                          INKSCAPE_ICON("connector-avoid"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_connector_path_set_avoid), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "ConnectorIgnoreAction",
                                          _("Ignore"),
                                          _("Make connectors ignore selected objects"),
                                          INKSCAPE_ICON("connector-ignore"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_connector_path_set_ignore), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    // Orthogonal connectors toggle button
    {
        InkToggleAction* act = ink_toggle_action_new( "ConnectorOrthogonalAction",
                                                      _("Orthogonal"),
                                                      _("Make connector orthogonal or polyline"),
                                                      INKSCAPE_ICON("connector-orthogonal"),
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );

        bool tbuttonstate = prefs->getBool("/tools/connector/orthogonal");
        gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act), ( tbuttonstate ? TRUE : FALSE ));
        g_object_set_data( holder, "orthogonal", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_connector_orthogonal_toggled), holder );
    }

    EgeAdjustmentAction* eact = 0;
    // Curvature spinbox
    eact = create_adjustment_action( "ConnectorCurvatureAction",
                                    _("Connector Curvature"), _("Curvature:"),
                                    _("The amount of connectors curvature"),
                                    "/tools/connector/curvature", defaultConnCurvature,
                                    GTK_WIDGET(desktop->canvas), holder, TRUE, "inkscape:connector-curvature",
                                    0, 100, 1.0, 10.0,
                                    0, 0, 0,
                                    connector_curvature_changed, NULL /*unit tracker*/, 1, 0 );
    gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );

    // Spacing spinbox
    eact = create_adjustment_action( "ConnectorSpacingAction",
                                    _("Connector Spacing"), _("Spacing:"),
                                    _("The amount of space left around objects by auto-routing connectors"),
                                    "/tools/connector/spacing", defaultConnSpacing,
                                    GTK_WIDGET(desktop->canvas), holder, TRUE, "inkscape:connector-spacing",
                                    0, 100, 1.0, 10.0,
                                    0, 0, 0,
                                    connector_spacing_changed, NULL /*unit tracker*/, 1, 0 );
    gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );

    // Graph (connector network) layout
    {
        InkAction* inky = ink_action_new( "ConnectorGraphAction",
                                          _("Graph"),
                                          _("Nicely arrange selected connector network"),
                                          INKSCAPE_ICON("distribute-graph"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_connector_graph_layout), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    // Default connector length spinbox
    eact = create_adjustment_action( "ConnectorLengthAction",
                                     _("Connector Length"), _("Length:"),
                                     _("Ideal length for connectors when layout is applied"),
                                     "/tools/connector/length", 100,
                                     GTK_WIDGET(desktop->canvas), holder, TRUE, "inkscape:connector-length",
                                     10, 1000, 10.0, 100.0,
                                     0, 0, 0,
                                     connector_length_changed, NULL /*unit tracker*/, 1, 0 );
    gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );


    // Directed edges toggle button
    {
        InkToggleAction* act = ink_toggle_action_new( "ConnectorDirectedAction",
                                                      _("Downwards"),
                                                      _("Make connectors with end-markers (arrows) point downwards"),
                                                      INKSCAPE_ICON("distribute-graph-directed"),
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );

        bool tbuttonstate = prefs->getBool("/tools/connector/directedlayout");
        gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act), ( tbuttonstate ? TRUE : FALSE ));

        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_directed_graph_layout_toggled), holder );
        desktop->getSelection()->connectChanged(sigc::bind(sigc::ptr_fun(sp_connector_toolbox_selection_changed), holder));
    }

    // Avoid overlaps toggle button
    {
        InkToggleAction* act = ink_toggle_action_new( "ConnectorOverlapAction",
                                                      _("Remove overlaps"),
                                                      _("Do not allow overlapping shapes"),
                                                      INKSCAPE_ICON("distribute-remove-overlaps"),
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );

        bool tbuttonstate = prefs->getBool("/tools/connector/avoidoverlaplayout");
        gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act), (tbuttonstate ? TRUE : FALSE ));

        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_nooverlaps_graph_layout_toggled), holder );
    }


    // Code to watch for changes to the connector-spacing attribute in
    // the XML.
    Inkscape::XML::Node *repr = desktop->namedview->getRepr();
    g_assert(repr != NULL);

    purge_repr_listener( holder, holder );

    if (repr) {
        g_object_set_data( holder, "repr", repr );
        Inkscape::GC::anchor(repr);
        sp_repr_add_listener( repr, &connector_tb_repr_events, holder );
        sp_repr_synthesize_events( repr, &connector_tb_repr_events, holder );
    }
} // end of sp_connector_toolbox_prep()


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
