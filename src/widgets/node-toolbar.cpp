/**
 * @file
 * Node aux toolbar
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

#include "ui/tool/multi-path-manipulator.h"
#include <glibmm/i18n.h>
#include "node-toolbar.h"

#include "desktop.h"
#include "document-undo.h"
#include "widgets/ege-adjustment-action.h"
#include "widgets/ink-action.h"
#include "inkscape.h"
#include "preferences.h"
#include "selection-chemistry.h"
#include "selection.h"
#include "sp-namedview.h"
#include "toolbox.h"
#include "ui/icon-names.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tools/node-tool.h"
#include "ui/widget/unit-tracker.h"
#include "util/units.h"
#include "verbs.h"
#include "widgets/widget-sizes.h"

using Inkscape::UI::Widget::UnitTracker;
using Inkscape::Util::Unit;
using Inkscape::Util::Quantity;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;
using Inkscape::Util::unit_table;
using Inkscape::UI::Tools::NodeTool;

//####################################
//# node editing callbacks
//####################################

/** Temporary hack: Returns the node tool in the active desktop.
 * Will go away during tool refactoring. */
static NodeTool *get_node_tool()
{
    NodeTool *tool = 0;
    if (SP_ACTIVE_DESKTOP ) {
        Inkscape::UI::Tools::ToolBase *ec = SP_ACTIVE_DESKTOP->event_context;
        if (INK_IS_NODE_TOOL(ec)) {
            tool = static_cast<NodeTool*>(ec);
        }
    }
    return tool;
}

static void sp_node_path_edit_add(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->insertNodes();
    }
}

static void sp_node_path_edit_add_min_x(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->insertNodesAtExtrema(Inkscape::UI::PointManipulator::EXTR_MIN_X);
    }
}
static void sp_node_path_edit_add_max_x(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->insertNodesAtExtrema(Inkscape::UI::PointManipulator::EXTR_MAX_X);
    }
}
static void sp_node_path_edit_add_min_y(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->insertNodesAtExtrema(Inkscape::UI::PointManipulator::EXTR_MIN_Y);
    }
}
static void sp_node_path_edit_add_max_y(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->insertNodesAtExtrema(Inkscape::UI::PointManipulator::EXTR_MAX_Y);
    }
}

static void sp_node_path_edit_delete(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        nt->_multipath->deleteNodes(prefs->getBool("/tools/nodes/delete_preserves_shape", true));
    }
}

static void sp_node_path_edit_delete_segment(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->deleteSegments();
    }
}

static void sp_node_path_edit_break(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->breakNodes();
    }
}

static void sp_node_path_edit_join(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->joinNodes();
    }
}

static void sp_node_path_edit_join_segment(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->joinSegments();
    }
}

static void sp_node_path_edit_toline(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setSegmentType(Inkscape::UI::SEGMENT_STRAIGHT);
    }
}

static void sp_node_path_edit_tocurve(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setSegmentType(Inkscape::UI::SEGMENT_CUBIC_BEZIER);
    }
}

static void sp_node_path_edit_cusp(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_CUSP);
    }
}

static void sp_node_path_edit_smooth(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_SMOOTH);
    }
}

static void sp_node_path_edit_symmetrical(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_SYMMETRIC);
    }
}

static void sp_node_path_edit_auto(void)
{
    NodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_AUTO);
    }
}

static void sp_node_path_edit_nextLPEparam(GtkAction * /*act*/, gpointer data) {
    sp_selection_next_patheffect_param( reinterpret_cast<SPDesktop*>(data) );
}

/* is called when the node selection is modified */
static void sp_node_toolbox_coord_changed(gpointer /*shape_editor*/, GObject *tbl)
{
    GtkAction* xact = GTK_ACTION( g_object_get_data( tbl, "nodes_x_action" ) );
    GtkAction* yact = GTK_ACTION( g_object_get_data( tbl, "nodes_y_action" ) );
    GtkAdjustment *xadj = ege_adjustment_action_get_adjustment(EGE_ADJUSTMENT_ACTION(xact));
    GtkAdjustment *yadj = ege_adjustment_action_get_adjustment(EGE_ADJUSTMENT_ACTION(yact));

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE));

    UnitTracker* tracker = reinterpret_cast<UnitTracker*>( g_object_get_data( tbl, "tracker" ) );
    if (!tracker) {
        return;
    }
    Unit const *unit = tracker->getActiveUnit();
    g_return_if_fail(unit != NULL);

    NodeTool *nt = get_node_tool();
    if (!nt || !(nt->_selected_nodes) ||nt->_selected_nodes->empty()) {
        // no path selected
        gtk_action_set_sensitive(xact, FALSE);
        gtk_action_set_sensitive(yact, FALSE);
    } else {
        gtk_action_set_sensitive(xact, TRUE);
        gtk_action_set_sensitive(yact, TRUE);
        Geom::Coord oldx = Quantity::convert(gtk_adjustment_get_value(xadj), unit, "px");
        Geom::Coord oldy = Quantity::convert(gtk_adjustment_get_value(yadj), unit, "px");
        Geom::Point mid = nt->_selected_nodes->pointwiseBounds()->midpoint();

        if (oldx != mid[Geom::X]) {
            gtk_adjustment_set_value(xadj, Quantity::convert(mid[Geom::X], "px", unit));
        }
        if (oldy != mid[Geom::Y]) {
            gtk_adjustment_set_value(yadj, Quantity::convert(mid[Geom::Y], "px", unit));
        }
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_node_path_value_changed(GtkAdjustment *adj, GObject *tbl, Geom::Dim2 d)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( tbl, "desktop" ));
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    UnitTracker* tracker = reinterpret_cast<UnitTracker*>(g_object_get_data( tbl, "tracker" ));
    if (!tracker) {
        return;
    }
    Unit const *unit = tracker->getActiveUnit();

    if (DocumentUndo::getUndoSensitive(desktop->getDocument())) {
        prefs->setDouble(Glib::ustring("/tools/nodes/") + (d == Geom::X ? "x" : "y"),
            Quantity::convert(gtk_adjustment_get_value(adj), unit, "px"));
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data( tbl, "freeze" ) || tracker->isUpdating()) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(TRUE));

    NodeTool *nt = get_node_tool();
    if (nt && !nt->_selected_nodes->empty()) {
        double val = Quantity::convert(gtk_adjustment_get_value(adj), unit, "px");
        double oldval = nt->_selected_nodes->pointwiseBounds()->midpoint()[d];
        Geom::Point delta(0,0);
        delta[d] = val - oldval;
        nt->_multipath->move(delta);
    }

    g_object_set_data( tbl, "freeze", GINT_TO_POINTER(FALSE) );
}

static void sp_node_path_x_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_node_path_value_changed(adj, tbl, Geom::X);
}

static void sp_node_path_y_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    sp_node_path_value_changed(adj, tbl, Geom::Y);
}

static void sp_node_toolbox_sel_changed(Inkscape::Selection *selection, GObject *tbl)
{
    {
    GtkAction* w = GTK_ACTION( g_object_get_data( tbl, "nodes_lpeedit" ) );
    SPItem *item = selection->singleItem();
    if (item && SP_IS_LPE_ITEM(item)) {
       if (SP_LPE_ITEM(item)->hasPathEffect()) {
           gtk_action_set_sensitive(w, TRUE);
       } else {
           gtk_action_set_sensitive(w, FALSE);
       }
    } else {
       gtk_action_set_sensitive(w, FALSE);
    }
    }
}

static void sp_node_toolbox_sel_modified(Inkscape::Selection *selection, guint /*flags*/, GObject *tbl)
{
    sp_node_toolbox_sel_changed (selection, tbl);
}

static void node_toolbox_watch_ec(SPDesktop* dt, Inkscape::UI::Tools::ToolBase* ec, GObject* holder);

//################################
//##    Node Editing Toolbox    ##
//################################

void sp_node_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    UnitTracker* tracker = new UnitTracker(Inkscape::Util::UNIT_TYPE_LINEAR);
    Unit doc_units = *desktop->getNamedView()->display_units;
    tracker->setActiveUnit(&doc_units);
    g_object_set_data( holder, "tracker", tracker );

    Inkscape::IconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);

    {
        InkToolMenuAction* inky = ink_tool_menu_action_new( "NodeInsertAction",
                                                            _("Insert node"),
                                                            _("Insert new nodes into selected segments"),
                                                            INKSCAPE_ICON("node-add"),
                                                            secondarySize );
        g_object_set( INK_ACTION(inky), "short_label", _("Insert"), NULL );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_add), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        GtkToolItem *menu_tool_button = gtk_menu_tool_button_new (NULL, NULL);
        gtk_activatable_set_related_action (GTK_ACTIVATABLE (menu_tool_button), GTK_ACTION(inky));
        // also create dummy menu action:
        gtk_action_group_add_action( mainActions, gtk_action_new("NodeInsertActionMenu", NULL, NULL, NULL) );
    }

    {
        InkAction* inky = ink_action_new( "NodeInsertActionMinX",
                                          _("Insert node at min X"),
                                          _("Insert new nodes at min X into selected segments"),
                                          INKSCAPE_ICON("node_insert_min_x"),
                                          secondarySize );
        g_object_set( inky, "short_label", _("Insert min X"), NULL );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_add_min_x), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }
    {
        InkAction* inky = ink_action_new( "NodeInsertActionMaxX",
                                          _("Insert node at max X"),
                                          _("Insert new nodes at max X into selected segments"),
                                          INKSCAPE_ICON("node_insert_max_x"),
                                          secondarySize );
        g_object_set( inky, "short_label", _("Insert max X"), NULL );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_add_max_x), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }
    {
        InkAction* inky = ink_action_new( "NodeInsertActionMinY",
                                          _("Insert node at min Y"),
                                          _("Insert new nodes at min Y into selected segments"),
                                          INKSCAPE_ICON("node_insert_min_y"),
                                          secondarySize );
        g_object_set( inky, "short_label", _("Insert min Y"), NULL );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_add_min_y), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }
    {
        InkAction* inky = ink_action_new( "NodeInsertActionMaxY",
                                          _("Insert node at max Y"),
                                          _("Insert new nodes at max Y into selected segments"),
                                          INKSCAPE_ICON("node_insert_max_y"),
                                          secondarySize );
        g_object_set( inky, "short_label", _("Insert max Y"), NULL );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_add_max_y), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeDeleteAction",
                                          _("Delete node"),
                                          _("Delete selected nodes"),
                                          INKSCAPE_ICON("node-delete"),
                                          secondarySize );
        g_object_set( inky, "short_label", _("Delete"), NULL );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_delete), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeJoinAction",
                                          _("Join nodes"),
                                          _("Join selected nodes"),
                                          INKSCAPE_ICON("node-join"),
                                          secondarySize );
        g_object_set( inky, "short_label", _("Join"), NULL );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_join), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeBreakAction",
                                          _("Break nodes"),
                                          _("Break path at selected nodes"),
                                          INKSCAPE_ICON("node-break"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_break), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }


    {
        InkAction* inky = ink_action_new( "NodeJoinSegmentAction",
                                          _("Join with segment"),
                                          _("Join selected endnodes with a new segment"),
                                          INKSCAPE_ICON("node-join-segment"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_join_segment), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeDeleteSegmentAction",
                                          _("Delete segment"),
                                          _("Delete segment between two non-endpoint nodes"),
                                          INKSCAPE_ICON("node-delete-segment"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_delete_segment), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeCuspAction",
                                          _("Node Cusp"),
                                          _("Make selected nodes corner"),
                                          INKSCAPE_ICON("node-type-cusp"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_cusp), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeSmoothAction",
                                          _("Node Smooth"),
                                          _("Make selected nodes smooth"),
                                          INKSCAPE_ICON("node-type-smooth"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_smooth), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeSymmetricAction",
                                          _("Node Symmetric"),
                                          _("Make selected nodes symmetric"),
                                          INKSCAPE_ICON("node-type-symmetric"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_symmetrical), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeAutoAction",
                                          _("Node Auto"),
                                          _("Make selected nodes auto-smooth"),
                                          INKSCAPE_ICON("node-type-auto-smooth"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_auto), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeLineAction",
                                          _("Node Line"),
                                          _("Make selected segments lines"),
                                          INKSCAPE_ICON("node-segment-line"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_toline), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkAction* inky = ink_action_new( "NodeCurveAction",
                                          _("Node Curve"),
                                          _("Make selected segments curves"),
                                          INKSCAPE_ICON("node-segment-curve"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_tocurve), 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
    }

    {
        InkToggleAction* act = ink_toggle_action_new( "NodesShowTransformHandlesAction",
                                                      _("Show Transform Handles"),
                                                      _("Show transformation handles for selected nodes"),
                                                      "node-transform",
                                                      secondarySize );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/nodes/show_transform_handles");
        g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
    }

    {
        InkToggleAction* act = ink_toggle_action_new( "NodesShowHandlesAction",
                                                      _("Show Handles"),
                                                      _("Show Bezier handles of selected nodes"),
                                                      INKSCAPE_ICON("show-node-handles"),
                                                      secondarySize );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/nodes/show_handles");
        g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
    }

    {
        InkToggleAction* act = ink_toggle_action_new( "NodesShowHelperpath",
                                                      _("Show Outline"),
                                                      _("Show path outline (without path effects)"),
                                                      INKSCAPE_ICON("show-path-outline"),
                                                      secondarySize );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/nodes/show_outline");
        g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
    }

    {
        Inkscape::Verb* verb = Inkscape::Verb::get(SP_VERB_EDIT_NEXT_PATHEFFECT_PARAMETER);
        InkAction* inky = ink_action_new( verb->get_id(),
                                          verb->get_name(),
                                          verb->get_tip(),
                                          INKSCAPE_ICON("path-effect-parameter-next"),
                                          secondarySize );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_node_path_edit_nextLPEparam), desktop );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        g_object_set_data( holder, "nodes_lpeedit", inky);
    }

    {
        InkToggleAction* inky = ink_toggle_action_new( "ObjectEditClipPathAction",
                                          _("Edit clipping paths"),
                                          _("Show clipping path(s) of selected object(s)"),
                                          INKSCAPE_ICON("path-clip-edit"),
                                          secondarySize );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(inky), "/tools/nodes/edit_clipping_paths");
        g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
    }

    {
        InkToggleAction* inky = ink_toggle_action_new( "ObjectEditMaskPathAction",
                                          _("Edit masks"),
                                          _("Show mask(s) of selected object(s)"),
                                          INKSCAPE_ICON("path-mask-edit"),
                                          secondarySize );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(inky), "/tools/nodes/edit_masks");
        g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
    }

    /* X coord of selected node(s) */
    {
        EgeAdjustmentAction* eact = 0;
        gchar const* labels[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {1, 2, 3, 5, 10, 20, 50, 100, 200, 500};
        eact = create_adjustment_action( "NodeXAction",
                                         _("X coordinate:"), _("X:"), _("X coordinate of selected node(s)"),
                                         "/tools/nodes/Xcoord", 0,
                                         GTK_WIDGET(desktop->canvas), holder, TRUE, "altx-nodes",
                                         -1e6, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_node_path_x_value_changed, tracker );
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );
        g_object_set_data( holder, "nodes_x_action", eact );
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    /* Y coord of selected node(s) */
    {
        EgeAdjustmentAction* eact = 0;
        gchar const* labels[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        gdouble values[] = {1, 2, 3, 5, 10, 20, 50, 100, 200, 500};
        eact = create_adjustment_action( "NodeYAction",
                                         _("Y coordinate:"), _("Y:"), _("Y coordinate of selected node(s)"),
                                         "/tools/nodes/Ycoord", 0,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         -1e6, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         labels, values, G_N_ELEMENTS(labels),
                                         sp_node_path_y_value_changed, tracker );
        tracker->addAdjustment( ege_adjustment_action_get_adjustment(eact) );
        g_object_set_data( holder, "nodes_y_action", eact );
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }

    // add the units menu
    {
        GtkAction* act = tracker->createAction( "NodeUnitsAction", _("Units"), ("") );
        gtk_action_group_add_action( mainActions, act );
    }

    sp_node_toolbox_sel_changed(desktop->getSelection(), holder);
    desktop->connectEventContextChanged(sigc::bind(sigc::ptr_fun(node_toolbox_watch_ec), holder));

} // end of sp_node_toolbox_prep()

static void node_toolbox_watch_ec(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* ec, GObject* holder)
{
    static sigc::connection c_selection_changed;
    static sigc::connection c_selection_modified;
    static sigc::connection c_subselection_changed;

    if (INK_IS_NODE_TOOL(ec)) {
        // watch selection
        c_selection_changed = desktop->getSelection()->connectChanged(sigc::bind(sigc::ptr_fun(sp_node_toolbox_sel_changed), holder));
        c_selection_modified = desktop->getSelection()->connectModified(sigc::bind(sigc::ptr_fun(sp_node_toolbox_sel_modified), holder));
        c_subselection_changed = desktop->connectToolSubselectionChanged(sigc::bind(sigc::ptr_fun(sp_node_toolbox_coord_changed), holder));

        sp_node_toolbox_sel_changed(desktop->getSelection(), holder);
    } else {
        if (c_selection_changed)
            c_selection_changed.disconnect();
        if (c_selection_modified)
            c_selection_modified.disconnect();
        if (c_subselection_changed)
            c_subselection_changed.disconnect();
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
