#define __SP_MAINTOOLBOX_C__

/** \file
 * Controls bars for some of Inkscape's tools 
 * (for some tools, they are in their own files)
 */

/*
*
* Authors:
*   MenTaLguY <mental@rydia.net>
*   Lauris Kaplinski <lauris@kaplinski.com>
*   bulia byak <buliabyak@users.sf.net>
*   Frank Felfe <innerspace@iname.com>
*   John Cliff <simarilius@yahoo.com>
*   David Turner <novalis@gnu.org>
*   Josh Andler <scislac@scislac.com>
*
* Copyright (C) 2004 David Turner
* Copyright (C) 2003 MenTaLguY
* Copyright (C) 1999-2005 authors
* Copyright (C) 2001-2002 Ximian, Inc.
*
* Released under GNU GPL, read the file 'COPYING' for more information
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm.h>
#include <gtk/gtk.h>
#include <iostream>
#include <sstream>

#include "widgets/button.h"
#include "widgets/widget-sizes.h"
#include "widgets/spw-utilities.h"
#include "widgets/spinbutton-events.h"
#include "dialogs/text-edit.h"

#include "ui/widget/style-swatch.h"

#include "prefs-utils.h"
#include "verbs.h"
#include "sp-namedview.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "nodepath.h"
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include <glibmm/i18n.h>
#include "helper/unit-menu.h"
#include "helper/units.h"

#include "inkscape.h"
#include "conn-avoid-ref.h"


#include "select-toolbar.h"
#include "gradient-toolbar.h"

#include "connector-context.h"
#include "sp-rect.h"
#include "sp-star.h"
#include "sp-spiral.h"
#include "sp-ellipse.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "style.h"
#include "selection.h"
#include "document-private.h"
#include "desktop-style.h"
#include "../libnrtype/font-lister.h"
#include "../connection-pool.h"
#include "../prefs-utils.h"
#include "../inkscape-stock.h"
#include "icon.h"

#include "mod360.h"

#include "toolbox.h"

typedef void (*SetupFunction)(GtkWidget *toolbox, SPDesktop *desktop);
typedef void (*UpdateFunction)(SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

static GtkWidget *sp_node_toolbox_new(SPDesktop *desktop);
static GtkWidget *sp_zoom_toolbox_new(SPDesktop *desktop);
static GtkWidget *sp_star_toolbox_new(SPDesktop *desktop);
static GtkWidget *sp_arc_toolbox_new(SPDesktop *desktop);
static GtkWidget *sp_rect_toolbox_new(SPDesktop *desktop);
static GtkWidget *sp_spiral_toolbox_new(SPDesktop *desktop);
static GtkWidget *sp_pencil_toolbox_new(SPDesktop *desktop);
static GtkWidget *sp_pen_toolbox_new(SPDesktop *desktop);
static GtkWidget *sp_calligraphy_toolbox_new(SPDesktop *desktop);
static GtkWidget *sp_dropper_toolbox_new(SPDesktop *desktop);
static GtkWidget *sp_empty_toolbox_new(SPDesktop *desktop);
static GtkWidget *sp_connector_toolbox_new(SPDesktop *desktop);

namespace { GtkWidget *sp_text_toolbox_new (SPDesktop *desktop); }


static struct {
    gchar const *type_name;
    gchar const *data_name;
    sp_verb_t verb;
    sp_verb_t doubleclick_verb;
} const tools[] = {
    { "SPSelectContext",   "select_tool",    SP_VERB_CONTEXT_SELECT,  SP_VERB_CONTEXT_SELECT_PREFS},
    { "SPNodeContext",     "node_tool",      SP_VERB_CONTEXT_NODE, SP_VERB_CONTEXT_NODE_PREFS },
    { "SPZoomContext",     "zoom_tool",      SP_VERB_CONTEXT_ZOOM, SP_VERB_CONTEXT_ZOOM_PREFS },
    { "SPRectContext",     "rect_tool",      SP_VERB_CONTEXT_RECT, SP_VERB_CONTEXT_RECT_PREFS },
    { "SPArcContext",      "arc_tool",       SP_VERB_CONTEXT_ARC, SP_VERB_CONTEXT_ARC_PREFS },
    { "SPStarContext",     "star_tool",      SP_VERB_CONTEXT_STAR, SP_VERB_CONTEXT_STAR_PREFS },
    { "SPSpiralContext",   "spiral_tool",    SP_VERB_CONTEXT_SPIRAL, SP_VERB_CONTEXT_SPIRAL_PREFS },
    { "SPPencilContext",   "pencil_tool",    SP_VERB_CONTEXT_PENCIL, SP_VERB_CONTEXT_PENCIL_PREFS },
    { "SPPenContext",      "pen_tool",       SP_VERB_CONTEXT_PEN, SP_VERB_CONTEXT_PEN_PREFS },
    { "SPDynaDrawContext", "dyna_draw_tool", SP_VERB_CONTEXT_CALLIGRAPHIC, SP_VERB_CONTEXT_CALLIGRAPHIC_PREFS },
    { "SPTextContext",     "text_tool",      SP_VERB_CONTEXT_TEXT, SP_VERB_CONTEXT_TEXT_PREFS },
    { "SPConnectorContext","connector_tool", SP_VERB_CONTEXT_CONNECTOR, SP_VERB_CONTEXT_CONNECTOR_PREFS },
    { "SPGradientContext", "gradient_tool",  SP_VERB_CONTEXT_GRADIENT, SP_VERB_CONTEXT_GRADIENT_PREFS },
    { "SPDropperContext",  "dropper_tool",   SP_VERB_CONTEXT_DROPPER, SP_VERB_CONTEXT_DROPPER_PREFS },
    { NULL, NULL, 0, 0 }
};

static struct {
    gchar const *type_name;
    gchar const *data_name;
    GtkWidget *(*create_func)(SPDesktop *desktop);
} const aux_toolboxes[] = {
    { "SPSelectContext", "select_toolbox", sp_select_toolbox_new },
    { "SPNodeContext",   "node_toolbox",   sp_node_toolbox_new },
    { "SPZoomContext",   "zoom_toolbox",   sp_zoom_toolbox_new },
    { "SPStarContext",   "star_toolbox",   sp_star_toolbox_new },
    { "SPRectContext",   "rect_toolbox",   sp_rect_toolbox_new },
    { "SPArcContext",    "arc_toolbox",    sp_arc_toolbox_new },
    { "SPSpiralContext", "spiral_toolbox", sp_spiral_toolbox_new },
    { "SPPencilContext", "pencil_toolbox", sp_pencil_toolbox_new },
    { "SPPenContext", "pen_toolbox", sp_pen_toolbox_new },
    { "SPDynaDrawContext", "calligraphy_toolbox", sp_calligraphy_toolbox_new },
    { "SPTextContext",   "text_toolbox",   sp_text_toolbox_new },
    { "SPDropperContext", "dropper_toolbox", sp_dropper_toolbox_new },
    { "SPGradientContext", "gradient_toolbox", sp_gradient_toolbox_new },
    { "SPConnectorContext", "connector_toolbox", sp_connector_toolbox_new },
    { NULL, NULL, NULL }
};

static void toolbox_set_desktop (GtkWidget *toolbox, SPDesktop *desktop, SetupFunction setup_func, UpdateFunction update_func, sigc::connection*);

static void setup_tool_toolbox (GtkWidget *toolbox, SPDesktop *desktop);
static void update_tool_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

static void setup_aux_toolbox (GtkWidget *toolbox, SPDesktop *desktop);
static void update_aux_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

static void setup_commands_toolbox (GtkWidget *toolbox, SPDesktop *desktop);
static void update_commands_toolbox (SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox);

/* Global text entry widgets necessary for update */
/* GtkWidget *dropper_rgb_entry, 
          *dropper_opacity_entry ; */
// should be made a private member once this is converted to class

static void delete_connection(GObject *obj, sigc::connection *connection) {
    connection->disconnect();
    delete connection;
}

static GtkWidget *
sp_toolbox_button_new(GtkWidget *t, Inkscape::IconSize size, gchar const *pxname, GtkSignalFunc handler,
                      GtkTooltips *tt, gchar const *tip)
{
    GtkWidget *b = sp_button_new_from_data(size, SP_BUTTON_TYPE_NORMAL, NULL, pxname, tip, tt);
    gtk_widget_show(b);
    if (handler) gtk_signal_connect(GTK_OBJECT(b), "clicked", handler, NULL);
    gtk_box_pack_start(GTK_BOX(t), b, FALSE, FALSE, 0);

    return b;
}

GtkWidget *
sp_toolbox_button_new_from_verb_with_doubleclick(GtkWidget *t, Inkscape::IconSize size, SPButtonType type,
                                                 Inkscape::Verb *verb, Inkscape::Verb *doubleclick_verb,
                                                 Inkscape::UI::View::View *view, GtkTooltips *tt)
{
    SPAction *action = verb->get_action(view);
    if (!action) return NULL;

    SPAction *doubleclick_action;
    if (doubleclick_verb)
        doubleclick_action = doubleclick_verb->get_action(view);
    else
        doubleclick_action = NULL;

    /* fixme: Handle sensitive/unsensitive */
    /* fixme: Implement sp_button_new_from_action */
    GtkWidget *b = sp_button_new(size, type, action, doubleclick_action, tt);
    gtk_widget_show(b);
    gtk_box_pack_start(GTK_BOX(t), b, FALSE, FALSE, 0);

    return b;
}

GtkWidget *sp_toolbox_button_new_from_verb(GtkWidget *t, Inkscape::IconSize size, SPButtonType type, Inkscape::Verb *verb,
                                           Inkscape::UI::View::View *view, GtkTooltips *tt)
{
    return sp_toolbox_button_new_from_verb_with_doubleclick(t, size, type, verb, NULL, view, tt);
}

GtkWidget * sp_toolbox_button_normal_new_from_verb(GtkWidget *t, Inkscape::IconSize size, Inkscape::Verb *verb,
                                                   Inkscape::UI::View::View *view, GtkTooltips *tt)
{
    return sp_toolbox_button_new_from_verb(t, size, SP_BUTTON_TYPE_NORMAL, verb, view, tt);
}

GtkWidget *
sp_tool_toolbox_new()
{
    GtkTooltips *tt = gtk_tooltips_new();
    GtkWidget *tb = gtk_vbox_new(FALSE, 0);

    g_object_set_data(G_OBJECT(tb), "desktop", NULL);
    g_object_set_data(G_OBJECT(tb), "tooltips", tt);

    gtk_widget_set_sensitive(tb, FALSE);

    GtkWidget *hb = gtk_handle_box_new();
    gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(hb), GTK_POS_TOP);
    gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(hb), GTK_SHADOW_OUT);
    gtk_handle_box_set_snap_edge(GTK_HANDLE_BOX(hb), GTK_POS_LEFT);

    gtk_container_add(GTK_CONTAINER(hb), tb);
    gtk_widget_show(GTK_WIDGET(tb));

    sigc::connection* conn = new sigc::connection;
    g_object_set_data(G_OBJECT(hb), "event_context_connection", conn);

    return hb;
}

static void
aux_toolbox_attached(GtkHandleBox *toolbox, GtkWidget *child)
{
    g_object_set_data(G_OBJECT(child), "is_detached", GINT_TO_POINTER(FALSE));
    gtk_widget_queue_resize(child);
}

static void
aux_toolbox_detached(GtkHandleBox *toolbox, GtkWidget *child)
{
    g_object_set_data(G_OBJECT(child), "is_detached", GINT_TO_POINTER(TRUE));
    gtk_widget_queue_resize(child);
}

GtkWidget *
sp_aux_toolbox_new()
{
    GtkWidget *tb = gtk_vbox_new(FALSE, 0);

    GtkWidget *tb_s = gtk_vbox_new(FALSE, 0);
    GtkWidget *tb_e = gtk_vbox_new(FALSE, 0);
    gtk_box_set_spacing(GTK_BOX(tb), AUX_SPACING);
    gtk_box_pack_start(GTK_BOX(tb), GTK_WIDGET(tb_s), FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(tb), GTK_WIDGET(tb_e), FALSE, FALSE, 0);

    g_object_set_data(G_OBJECT(tb), "desktop", NULL);
    g_object_set_data(G_OBJECT(tb), "top_spacer", tb_s);

    gtk_widget_set_sensitive(tb, FALSE);

    GtkWidget *hb = gtk_handle_box_new();
    gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(hb), GTK_POS_LEFT);
    gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(hb), GTK_SHADOW_OUT);
    gtk_handle_box_set_snap_edge(GTK_HANDLE_BOX(hb), GTK_POS_LEFT);

    g_signal_connect(G_OBJECT(hb), "child_attached", G_CALLBACK(aux_toolbox_attached), (gpointer)tb);
    g_signal_connect(G_OBJECT(hb), "child_detached", G_CALLBACK(aux_toolbox_detached), (gpointer)tb);

    gtk_container_add(GTK_CONTAINER(hb), tb);
    gtk_widget_show(GTK_WIDGET(tb));

    sigc::connection* conn = new sigc::connection;
    g_object_set_data(G_OBJECT(hb), "event_context_connection", conn);

    return hb;
}

//####################################
//# Commands Bar
//####################################

GtkWidget *
sp_commands_toolbox_new()
{
    GtkWidget *tb = gtk_vbox_new(FALSE, 0);

    GtkWidget *tb_s = gtk_vbox_new(FALSE, 0);
    GtkWidget *tb_e = gtk_vbox_new(FALSE, 0);
    gtk_box_set_spacing(GTK_BOX(tb), AUX_SPACING);
    gtk_box_pack_start(GTK_BOX(tb), GTK_WIDGET(tb_s), FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(tb), GTK_WIDGET(tb_e), FALSE, FALSE, 0);

    g_object_set_data(G_OBJECT(tb), "desktop", NULL);
    gtk_widget_set_sensitive(tb, FALSE);

    GtkWidget *hb = gtk_handle_box_new();
    gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(hb), GTK_POS_LEFT);
    gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(hb), GTK_SHADOW_OUT);
    gtk_handle_box_set_snap_edge(GTK_HANDLE_BOX(hb), GTK_POS_LEFT);

    g_signal_connect(G_OBJECT(hb), "child_attached", G_CALLBACK(aux_toolbox_attached), (gpointer)tb);
    g_signal_connect(G_OBJECT(hb), "child_detached", G_CALLBACK(aux_toolbox_detached), (gpointer)tb);

    gtk_container_add(GTK_CONTAINER(hb), tb);
    gtk_widget_show(GTK_WIDGET(tb));

    sigc::connection* conn = new sigc::connection;
    g_object_set_data(G_OBJECT(hb), "event_context_connection", conn);

    return hb;
}


//####################################
//# node editing callbacks
//####################################

void
sp_node_path_edit_add(void)
{
    sp_node_selected_add_node();
}

void
sp_node_path_edit_delete(void)
{
    sp_node_selected_delete();
}

void
sp_node_path_edit_delete_segment(void)
{
    sp_node_selected_delete_segment();
}

void
sp_node_path_edit_break(void)
{
    sp_node_selected_break();
}

void
sp_node_path_edit_join(void)
{
    sp_node_selected_join();
}

void
sp_node_path_edit_join_segment(void)
{
    sp_node_selected_join_segment();
}

void
sp_node_path_edit_toline(void)
{
    sp_node_selected_set_line_type(NR_LINETO);
}

void
sp_node_path_edit_tocurve(void)
{
    sp_node_selected_set_line_type(NR_CURVETO);
}

void
sp_node_path_edit_cusp(void)
{
    sp_node_selected_set_type(Inkscape::NodePath::NODE_CUSP);
}

void
sp_node_path_edit_smooth(void)
{
    sp_node_selected_set_type(Inkscape::NodePath::NODE_SMOOTH);
}

void
sp_node_path_edit_symmetrical(void)
{
    sp_node_selected_set_type(Inkscape::NodePath::NODE_SYMM);
}

static void toggle_show_handles (GtkWidget *button, gpointer data) {
    bool show = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
    prefs_set_int_attribute ("tools.nodes", "show_handles",  show ? 1 : 0);
    sp_nodepath_show_handles(show);
}

//################################
//##    Node Editing Toolbox    ##
//################################

static GtkWidget *
sp_node_toolbox_new(SPDesktop *desktop)
{
    Inkscape::UI::View::View *view = desktop;

    GtkTooltips *tt = gtk_tooltips_new();
    GtkWidget *tb = gtk_hbox_new(FALSE, 0);

    gtk_box_pack_start(GTK_BOX(tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, "node_insert",
        GTK_SIGNAL_FUNC(sp_node_path_edit_add), tt, _("Insert new nodes into selected segments"));
    sp_toolbox_button_new(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, "node_delete",
        GTK_SIGNAL_FUNC(sp_node_path_edit_delete), tt, _("Delete selected nodes"));

    gtk_box_pack_start(GTK_BOX(tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, "node_join",
        GTK_SIGNAL_FUNC(sp_node_path_edit_join), tt, _("Join selected endnodes"));
    sp_toolbox_button_new(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, "node_join_segment",
        GTK_SIGNAL_FUNC(sp_node_path_edit_join_segment), tt, _("Join selected endnodes with a new segment"));

    sp_toolbox_button_new(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, "node_delete_segment",
        GTK_SIGNAL_FUNC(sp_node_path_edit_delete_segment), tt, _("Split path between two non-endpoint nodes"));

    sp_toolbox_button_new(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, "node_break",
        GTK_SIGNAL_FUNC(sp_node_path_edit_break), tt, _("Break path at selected nodes"));

    gtk_box_pack_start(GTK_BOX(tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, "node_cusp",
        GTK_SIGNAL_FUNC(sp_node_path_edit_cusp), tt, _("Make selected nodes corner"));

    sp_toolbox_button_new(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, "node_smooth",
        GTK_SIGNAL_FUNC(sp_node_path_edit_smooth), tt, _("Make selected nodes smooth"));

    sp_toolbox_button_new(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, "node_symmetric",
        GTK_SIGNAL_FUNC(sp_node_path_edit_symmetrical), tt, _("Make selected nodes symmetric"));

    gtk_box_pack_start(GTK_BOX(tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, "node_line",
        GTK_SIGNAL_FUNC(sp_node_path_edit_toline), tt, _("Make selected segments lines"));

    sp_toolbox_button_new(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, "node_curve",
        GTK_SIGNAL_FUNC(sp_node_path_edit_tocurve), tt, _("Make selected segments curves"));

    gtk_box_pack_start(GTK_BOX(tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_normal_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, Inkscape::Verb::get(SP_VERB_OBJECT_TO_CURVE), view, tt);

    sp_toolbox_button_normal_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, Inkscape::Verb::get(SP_VERB_SELECTION_OUTLINE), view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    GtkWidget *cvbox = gtk_vbox_new (FALSE, 0);
    GtkWidget *cbox = gtk_hbox_new (FALSE, 0);

    {
    GtkWidget *button = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              "nodes_show_handles",
                                              _("Show the Bezier handles of selected nodes"),
                                              tt);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), prefs_get_int_attribute ("tools.nodes", "show_handles", 1));
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (toggle_show_handles), desktop);
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(cvbox), cbox, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tb), cvbox, FALSE, FALSE, 0);

    gtk_widget_show_all(tb);

    return tb;

} // end of sp_node_toolbox_new()


//########################
//##    Zoom Toolbox    ##
//########################

static GtkWidget *
sp_zoom_toolbox_new(SPDesktop *desktop)
{
    Inkscape::UI::View::View *view=desktop;

    GtkTooltips *tt = gtk_tooltips_new();
    GtkWidget *tb = gtk_hbox_new(FALSE, 0);

    gtk_box_pack_start(GTK_BOX(tb), gtk_hbox_new(FALSE, 0),
                       FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_IN), view, tt);

    sp_toolbox_button_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_OUT), view, tt);

    gtk_box_pack_start(GTK_BOX(tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_SELECTION), view, tt);

    sp_toolbox_button_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_DRAWING), view, tt);

    sp_toolbox_button_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_PAGE), view, tt);

    sp_toolbox_button_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_PAGE_WIDTH), view, tt);

    gtk_box_pack_start(GTK_BOX(tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_PREV), view, tt);

    sp_toolbox_button_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_NEXT), view, tt);

    gtk_box_pack_start(GTK_BOX(tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_1_1), view, tt);

    sp_toolbox_button_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_1_2), view, tt);

    sp_toolbox_button_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_2_1), view, tt);

    gtk_widget_show_all(tb);

    return tb;

} // end of sp_zoom_toolbox_new()

void
sp_tool_toolbox_set_desktop(GtkWidget *toolbox, SPDesktop *desktop)
{
    toolbox_set_desktop(gtk_bin_get_child(GTK_BIN(toolbox)), desktop, setup_tool_toolbox, update_tool_toolbox, static_cast<sigc::connection*>(g_object_get_data(G_OBJECT(toolbox), "event_context_connection")));
}


void
sp_aux_toolbox_set_desktop(GtkWidget *toolbox, SPDesktop *desktop)
{
    toolbox_set_desktop(gtk_bin_get_child(GTK_BIN(toolbox)), desktop, setup_aux_toolbox, update_aux_toolbox, static_cast<sigc::connection*>(g_object_get_data(G_OBJECT(toolbox), "event_context_connection")));
}

void
sp_commands_toolbox_set_desktop(GtkWidget *toolbox, SPDesktop *desktop)
{
    toolbox_set_desktop(gtk_bin_get_child(GTK_BIN(toolbox)), desktop, setup_commands_toolbox, update_commands_toolbox, static_cast<sigc::connection*>(g_object_get_data(G_OBJECT(toolbox), "event_context_connection")));
}

static void
toolbox_set_desktop(GtkWidget *toolbox, SPDesktop *desktop, SetupFunction setup_func, UpdateFunction update_func, sigc::connection *conn)
{
    gpointer ptr = g_object_get_data(G_OBJECT(toolbox), "desktop");
    SPDesktop *old_desktop = static_cast<SPDesktop*>(ptr);

    if (old_desktop) {
        GList *children, *iter;

        children = gtk_container_get_children(GTK_CONTAINER(toolbox));
        for ( iter = children ; iter ; iter = iter->next ) {
            gtk_container_remove( GTK_CONTAINER(toolbox), GTK_WIDGET(iter->data) );
        }
        g_list_free(children);
    }

    g_object_set_data(G_OBJECT(toolbox), "desktop", (gpointer)desktop);

    if (desktop) {
        gtk_widget_set_sensitive(toolbox, TRUE);
        setup_func(toolbox, desktop);
        update_func(desktop, desktop->event_context, toolbox);
        *conn = desktop->connectEventContextChanged 
            (sigc::bind (sigc::ptr_fun(update_func), toolbox));
    } else {
        gtk_widget_set_sensitive(toolbox, FALSE);
    }

} // end of toolbox_set_desktop()


static void
setup_tool_toolbox(GtkWidget *toolbox, SPDesktop *desktop)
{
    GtkTooltips *tooltips=GTK_TOOLTIPS(g_object_get_data(G_OBJECT(toolbox), "tooltips"));
    gint shrinkLeft = prefs_get_int_attribute_limited( "toolbox.left", "small", 0, 0, 1 );
    Inkscape::IconSize toolboxSize = shrinkLeft ? Inkscape::ICON_SIZE_SMALL_TOOLBAR : Inkscape::ICON_SIZE_LARGE_TOOLBAR;

    for (int i = 0 ; tools[i].type_name ; i++ ) {
        GtkWidget *button =
            sp_toolbox_button_new_from_verb_with_doubleclick( toolbox, toolboxSize,
                                                              SP_BUTTON_TYPE_TOGGLE,
                                                              Inkscape::Verb::get(tools[i].verb),
                                                              Inkscape::Verb::get(tools[i].doubleclick_verb),
                                                              desktop,
                                                              tooltips );

        g_object_set_data( G_OBJECT(toolbox), tools[i].data_name,
                           (gpointer)button );
    }
}


static void
update_tool_toolbox( SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox )
{
    gchar const *const tname = ( eventcontext
                                 ? gtk_type_name(GTK_OBJECT_TYPE(eventcontext))
                                 : NULL );
    for (int i = 0 ; tools[i].type_name ; i++ ) {
        SPButton *button = SP_BUTTON(g_object_get_data(G_OBJECT(toolbox), tools[i].data_name));
        sp_button_toggle_set_down(button, tname && !strcmp(tname, tools[i].type_name));
    }
}

static void
setup_aux_toolbox(GtkWidget *toolbox, SPDesktop *desktop)
{
    GtkSizeGroup* grouper = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );

    for (int i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
        GtkWidget *sub_toolbox;
        if (aux_toolboxes[i].create_func == NULL)
            sub_toolbox = sp_empty_toolbox_new(desktop);
        else
            sub_toolbox = aux_toolboxes[i].create_func(desktop);

        gtk_size_group_add_widget( grouper, sub_toolbox );

        gtk_container_add(GTK_CONTAINER(toolbox), sub_toolbox);
        g_object_set_data(G_OBJECT(toolbox), aux_toolboxes[i].data_name, sub_toolbox);
    }
    g_object_unref( G_OBJECT(grouper) );
}

static void
update_aux_toolbox(SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox)
{
    gchar const *tname = ( eventcontext
                           ? gtk_type_name(GTK_OBJECT_TYPE(eventcontext))
                           : NULL );
    for (int i = 0 ; aux_toolboxes[i].type_name ; i++ ) {
        GtkWidget *sub_toolbox = GTK_WIDGET(g_object_get_data(G_OBJECT(toolbox), aux_toolboxes[i].data_name));
        if (tname && !strcmp(tname, aux_toolboxes[i].type_name)) {
            gtk_widget_show_all(sub_toolbox);
            g_object_set_data(G_OBJECT(toolbox), "shows", sub_toolbox);
        } else {
            gtk_widget_hide(sub_toolbox);
        }
    }
}

static void
setup_commands_toolbox(GtkWidget *toolbox, SPDesktop *desktop)
{
    Inkscape::UI::View::View *view = desktop;

    GtkTooltips *tt = gtk_tooltips_new();
    GtkWidget *tb = gtk_hbox_new(FALSE, 0);

    gint shrinkTop = prefs_get_int_attribute_limited( "toolbox", "small", 1, 0, 1 );
    Inkscape::IconSize toolboxSize = shrinkTop ? Inkscape::ICON_SIZE_SMALL_TOOLBAR : Inkscape::ICON_SIZE_LARGE_TOOLBAR;

    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_FILE_NEW), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_FILE_OPEN), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_FILE_SAVE), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_FILE_PRINT), view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_FILE_IMPORT), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_FILE_EXPORT), view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_EDIT_UNDO), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_EDIT_REDO), view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_EDIT_COPY), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_EDIT_CUT), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_EDIT_PASTE), view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_SELECTION), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_DRAWING), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_ZOOM_PAGE), view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_EDIT_DUPLICATE), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_EDIT_CLONE), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_EDIT_UNLINK_CLONE), view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_normal_new_from_verb(tb, toolboxSize, Inkscape::Verb::get(SP_VERB_SELECTION_GROUP), view, tt);
    sp_toolbox_button_normal_new_from_verb(tb, toolboxSize, Inkscape::Verb::get(SP_VERB_SELECTION_UNGROUP), view, tt);

    // disabled until we have icons for them:

    //find

    //sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_EDIT_TILE), view, tt);
    //sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_EDIT_UNTILE), view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_DIALOG_FILL_STROKE), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_DIALOG_TEXT), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_DIALOG_XML_EDITOR), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_DIALOG_ALIGN_DISTRIBUTE), view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_DIALOG_DISPLAY), view, tt);
    sp_toolbox_button_new_from_verb(tb, toolboxSize, SP_BUTTON_TYPE_NORMAL, Inkscape::Verb::get(SP_VERB_DIALOG_NAMEDVIEW), view, tt);

    gtk_widget_show_all(tb);

    gtk_container_add(GTK_CONTAINER(toolbox), tb);
}

static void
update_commands_toolbox(SPDesktop *desktop, SPEventContext *eventcontext, GtkWidget *toolbox)
{
}

void show_aux_toolbox(GtkWidget *toolbox_toplevel)
{
    gtk_widget_show(toolbox_toplevel);
    GtkWidget *toolbox = gtk_bin_get_child(GTK_BIN(toolbox_toplevel));

    GtkWidget *shown_toolbox = GTK_WIDGET(g_object_get_data(G_OBJECT(toolbox), "shows"));
    if (!shown_toolbox) {
        return;
    }
    gtk_widget_show(toolbox);

    // need to show the spacer, or the padding will be off
    GtkWidget *spacer = GTK_WIDGET(g_object_get_data(G_OBJECT(toolbox), "top_spacer"));
    gtk_widget_show(spacer);

    gtk_widget_show_all(shown_toolbox);
}

void
aux_toolbox_space(GtkWidget *tb, gint space)
{
    gtk_box_pack_start(GTK_BOX(tb), gtk_hbox_new(FALSE, 0), FALSE, FALSE, space);
}

static GtkWidget *
sp_empty_toolbox_new(SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new(FALSE, 0);
    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    gtk_widget_show_all(tbl);
    sp_set_font_size_smaller (tbl);

    return tbl;
}

// helper UI functions

GtkWidget *
sp_tb_spinbutton(
    gchar *label, gchar const *tooltip,
    gchar const *path, gchar const *data, gdouble def,
    GtkWidget *us,
    GtkWidget *tbl,
    gboolean altx, gchar const *altx_mark,
    gdouble lower, gdouble upper, gdouble step, gdouble page,
    void (*callback)(GtkAdjustment *, GtkWidget *),
    gdouble climb = 0.1, guint digits = 3, double factor = 1.0)
{
    GtkTooltips *tt = gtk_tooltips_new();

    GtkWidget *hb = gtk_hbox_new(FALSE, 1);

    GtkWidget *l = gtk_label_new(label);
    gtk_widget_show(l);
    gtk_misc_set_alignment(GTK_MISC(l), 1.0, 0.5);
    gtk_container_add(GTK_CONTAINER(hb), l);

    GtkObject *a = gtk_adjustment_new(prefs_get_double_attribute(path, data, def) * factor,
                                      lower, upper, step, page, page);
    gtk_object_set_data(GTK_OBJECT(tbl), data, a);
    if (us)
        sp_unit_selector_add_adjustment(SP_UNIT_SELECTOR(us), GTK_ADJUSTMENT(a));

    GtkWidget *sb = gtk_spin_button_new(GTK_ADJUSTMENT(a), climb, digits);
    gtk_tooltips_set_tip(tt, sb, tooltip, NULL);
    if (altx)
        gtk_object_set_data(GTK_OBJECT(sb), altx_mark, sb);
    gtk_widget_set_size_request(sb, 
                                (upper <= 1.0 || digits == 0)? AUX_SPINBUTTON_WIDTH_SMALL - 10: AUX_SPINBUTTON_WIDTH_SMALL,
                                AUX_SPINBUTTON_HEIGHT);
    gtk_widget_show(sb);
    gtk_signal_connect(GTK_OBJECT(sb), "focus-in-event", GTK_SIGNAL_FUNC(spinbutton_focus_in), tbl);
    gtk_signal_connect(GTK_OBJECT(sb), "key-press-event", GTK_SIGNAL_FUNC(spinbutton_keypress), tbl);
    gtk_container_add(GTK_CONTAINER(hb), sb);
    gtk_signal_connect(GTK_OBJECT(a), "value_changed", GTK_SIGNAL_FUNC(callback), tbl);

    return hb;
}

#define MODE_LABEL_WIDTH 70

//########################
//##       Star         ##
//########################

static void
sp_stb_magnitude_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data(GTK_OBJECT(tbl), "desktop");

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        // do not remember prefs if this call is initiated by an undo change, because undoing object
        // creation sets bogus values to its attributes before it is deleted
        prefs_set_int_attribute("tools.shapes.star", "magnitude", (gint)adj->value);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    bool modmade = false;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_int(repr,"sodipodi:sides",(gint)adj->value);
            sp_repr_set_svg_double(repr, "sodipodi:arg2",
                                   (sp_repr_get_double_attribute(repr, "sodipodi:arg1", 0.5)
                                    + M_PI / (gint)adj->value));
            SP_OBJECT((SPItem *) items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
            modmade = true;
        }
    }
    if (modmade)  sp_document_done(sp_desktop_document(desktop));

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));

    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void
sp_stb_proportion_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data(GTK_OBJECT(tbl), "desktop");

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_double_attribute("tools.shapes.star", "proportion", adj->value);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    bool modmade = false;
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);

            gdouble r1 = sp_repr_get_double_attribute(repr, "sodipodi:r1", 1.0);
            gdouble r2 = sp_repr_get_double_attribute(repr, "sodipodi:r2", 1.0);
            if (r2 < r1) {
                sp_repr_set_svg_double(repr, "sodipodi:r2", r1*adj->value);
            } else {
                sp_repr_set_svg_double(repr, "sodipodi:r1", r2*adj->value);
            }

            SP_OBJECT((SPItem *) items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
            modmade = true;
        }
    }

    if (modmade) sp_document_done(sp_desktop_document(desktop));

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));

    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void
sp_stb_sides_flat_state_changed(GtkWidget *widget, GtkObject *tbl)
{
    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data(GTK_OBJECT(tbl), "desktop");

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
            prefs_set_string_attribute("tools.shapes.star", "isflatsided", "true");
        } else {
            prefs_set_string_attribute("tools.shapes.star", "isflatsided", "false");
        }
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    GtkWidget *prop_widget = (GtkWidget*) g_object_get_data(G_OBJECT(tbl), "prop_widget");
    bool modmade = false;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        gtk_widget_set_sensitive(GTK_WIDGET(prop_widget), FALSE);
        for (; items != NULL; items = items->next) {
            if (SP_IS_STAR((SPItem *) items->data)) {
                Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
                repr->setAttribute("inkscape:flatsided", "true");
                SP_OBJECT((SPItem *) items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
                modmade = true;
            }
        }
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(prop_widget), TRUE);
        for (; items != NULL; items = items->next) {
            if (SP_IS_STAR((SPItem *) items->data)) {
                Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
                repr->setAttribute("inkscape:flatsided", "false");
                SP_OBJECT(items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
                modmade = true;
            }
        }
    }
    if (modmade) sp_document_done(sp_desktop_document(desktop));

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));

    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void
sp_stb_rounded_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data(GTK_OBJECT(tbl), "desktop");

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_double_attribute("tools.shapes.star", "rounded", (gdouble) adj->value);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    bool modmade = false;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_svg_double(repr, "inkscape:rounded", (gdouble) adj->value);
            SP_OBJECT(items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
            modmade = true;
        }
    }
    if (modmade)  sp_document_done(sp_desktop_document(desktop));

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));

    spinbutton_defocus(GTK_OBJECT(tbl));
}


static void
sp_stb_randomized_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data(GTK_OBJECT(tbl), "desktop");

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_double_attribute("tools.shapes.star", "randomized", (gdouble) adj->value);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    bool modmade = false;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_svg_double(repr, "inkscape:randomized", (gdouble) adj->value);
            SP_OBJECT(items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
            modmade = true;
        }
    }
    if (modmade)  sp_document_done(sp_desktop_document(desktop));

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));

    spinbutton_defocus(GTK_OBJECT(tbl));
}


static void star_tb_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
                                       gchar const *old_value, gchar const *new_value,
                                       bool is_interactive, gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    GtkAdjustment *adj;

    if (!strcmp(name, "inkscape:randomized")) {
        adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "randomized");
        gtk_adjustment_set_value(adj, sp_repr_get_double_attribute(repr, "inkscape:randomized", 0.0));
    } else if (!strcmp(name, "inkscape:rounded")) {
        adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "rounded");
        gtk_adjustment_set_value(adj, sp_repr_get_double_attribute(repr, "inkscape:rounded", 0.0));
    } else if (!strcmp(name, "inkscape:flatsided")) {
        GtkWidget *fscb = (GtkWidget*) g_object_get_data(G_OBJECT(tbl), "flat_checkbox");
        GtkWidget *prop_widget = (GtkWidget*) g_object_get_data(G_OBJECT(tbl), "prop_widget");
        char const *flatsides = repr->attribute("inkscape:flatsided");
        if (flatsides && !strcmp(flatsides,"false" )) {
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(fscb),  FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(prop_widget), TRUE);
        } else {
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(fscb),  TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(prop_widget), FALSE);
        }
    } else if (!strcmp(name, "sodipodi:r1") || !strcmp(name, "sodipodi:r2")) {
        adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "proportion");
        gdouble r1 = sp_repr_get_double_attribute(repr, "sodipodi:r1", 1.0);
        gdouble r2 = sp_repr_get_double_attribute(repr, "sodipodi:r2", 1.0);
        if (r2 < r1) {
            gtk_adjustment_set_value(adj, r2/r1);
        } else {
            gtk_adjustment_set_value(adj, r1/r2);
        }
    } else if (!strcmp(name, "sodipodi:sides")) {
        adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "magnitude");
        gtk_adjustment_set_value(adj, sp_repr_get_int_attribute(repr, "sodipodi:sides", 0));
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
sp_star_toolbox_selection_changed(Inkscape::Selection *selection, GtkObject *tbl)
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;
    Inkscape::XML::Node *oldrepr = NULL;

    for (GSList const *items = selection->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_STAR((SPItem *) items->data)) {
            n_selected++;
            repr = SP_OBJECT_REPR((SPItem *) items->data);
        }
    }

    GtkWidget *l = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(tbl), "mode_label"));

    if (n_selected == 0) {
        gtk_label_set_markup(GTK_LABEL(l), _("<b>New:</b>"));
    } else if (n_selected == 1) {
        gtk_label_set_markup(GTK_LABEL(l), _("<b>Change:</b>"));

        oldrepr = (Inkscape::XML::Node *) gtk_object_get_data(GTK_OBJECT(tbl), "repr");
        if (oldrepr) { // remove old listener
            sp_repr_remove_listener_by_data(oldrepr, tbl);
            Inkscape::GC::release(oldrepr);
            oldrepr = 0;
            g_object_set_data(G_OBJECT(tbl), "repr", NULL);
        }

        if (repr) {
            g_object_set_data(G_OBJECT(tbl), "repr", repr);
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &star_tb_repr_events, tbl);
            sp_repr_synthesize_events(repr, &star_tb_repr_events, tbl);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected stars
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        gtk_label_set_markup(GTK_LABEL(l), _("<b>Change:</b>"));
    }
}


static void
sp_stb_defaults(GtkWidget *widget, GtkWidget *tbl)
{
    // FIXME: in this and all other _default functions, set some flag telling the value_changed
    // callbacks to lump all the changes for all selected objects in one undo step

    GtkAdjustment *adj;

    // fixme: make settable in prefs!
    gint mag = 5;
    gdouble prop = 0.5;
    gboolean flat = FALSE;
    gdouble randomized = 0;
    gdouble rounded = 0;

    GtkWidget *fscb = (GtkWidget*) g_object_get_data(G_OBJECT(tbl), "flat_checkbox");
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(fscb),  flat);
    GtkWidget *sb2 = (GtkWidget*) g_object_get_data(G_OBJECT(tbl), "prop_widget");
    gtk_widget_set_sensitive(GTK_WIDGET(sb2), !flat);

    adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "magnitude");
    gtk_adjustment_set_value(adj, mag);
    gtk_adjustment_value_changed(adj);

    adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "proportion");
    gtk_adjustment_set_value(adj, prop);
    gtk_adjustment_value_changed(adj);

    adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "rounded");
    gtk_adjustment_set_value(adj, rounded);
    gtk_adjustment_value_changed(adj);

    adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "randomized");
    gtk_adjustment_set_value(adj, randomized);
    gtk_adjustment_value_changed(adj);

    spinbutton_defocus(GTK_OBJECT(tbl));
}


void
sp_toolbox_add_label(GtkWidget *tbl, gchar const *title, bool wide)
{
    GtkWidget *boxl = gtk_hbox_new(FALSE, 0);
    if (wide) gtk_widget_set_size_request(boxl, MODE_LABEL_WIDTH, -1);
    GtkWidget *l = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(l), title);
    gtk_box_pack_end(GTK_BOX(boxl), l, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tbl), boxl, FALSE, FALSE, 0);
    gtk_object_set_data(GTK_OBJECT(tbl), "mode_label", l);
}


static GtkWidget *
sp_star_toolbox_new(SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new(FALSE, 0);

    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new();

    sp_toolbox_add_label(tbl, _("<b>New:</b>"));

    gchar const *flatsidedstr = NULL;

    /* Flatsided checkbox */
    {
        GtkWidget *hb = gtk_hbox_new(FALSE, 1);
        GtkWidget *fscb = gtk_check_button_new_with_label(_("Polygon"));
        gtk_widget_set_sensitive(GTK_WIDGET(fscb), TRUE);
        flatsidedstr = prefs_get_string_attribute("tools.shapes.star", "isflatsided");
        if (!flatsidedstr || (flatsidedstr && !strcmp(flatsidedstr, "false")))
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(fscb),  FALSE);
        else
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(fscb),  TRUE);
        gtk_tooltips_set_tip(tt, fscb, _("Regular polygon (with one handle) instead of a star"), NULL);
        gtk_widget_show(fscb);
        gtk_object_set_data(GTK_OBJECT(tbl), "flat_checkbox", fscb);
        gtk_container_add(GTK_CONTAINER(hb), fscb);
        g_signal_connect(G_OBJECT(fscb), "toggled", GTK_SIGNAL_FUNC(sp_stb_sides_flat_state_changed ), tbl);
        gtk_box_pack_start(GTK_BOX(tbl),hb, FALSE, FALSE, AUX_SPACING);
    }

    aux_toolbox_space(tbl, AUX_BETWEEN_BUTTON_GROUPS);

    /* Magnitude */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Corners:"), _("Number of corners of a polygon or star"),
                                         "tools.shapes.star", "magnitude", 3,
                                         NULL, tbl, TRUE, "altx-star",
                                         3, 1024, 1, 1,
                                         sp_stb_magnitude_value_changed, 1, 0);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_SPACING);
    }

    /* Spoke ratio */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Spoke ratio:"),
                                         // TRANSLATORS: Tip radius of a star is the distance from the center to the farthest handle.
                                         // Base radius is the same for the closest handle.
                                         _("Base radius to tip radius ratio"),
                                         "tools.shapes.star", "proportion", 0.5,
                                         NULL, tbl, FALSE, NULL,
                                         0.01, 1.0, 0.01, 0.1,
                                         sp_stb_proportion_value_changed);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_SPACING);
        g_object_set_data(G_OBJECT(tbl), "prop_widget", hb);
        if (!flatsidedstr || (flatsidedstr && !strcmp(flatsidedstr, "false")))
            gtk_widget_set_sensitive(GTK_WIDGET(hb), TRUE);
        else
            gtk_widget_set_sensitive(GTK_WIDGET(hb), FALSE);
    }

    /* Roundedness */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Rounded:"), _("How much rounded are the corners (0 for sharp)"),
                                         "tools.shapes.star", "rounded", 0.0,
                                         NULL, tbl, FALSE, NULL,
                                         -100.0, 100.0, 0.01, 0.1,
                                         sp_stb_rounded_value_changed);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_SPACING);
    }

    /* Randomization */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Randomized:"), _("Scatter randomly the corners and angles"),
                                         "tools.shapes.star", "randomized", 0.0,
                                         NULL, tbl, FALSE, NULL,
                                         -10.0, 10.0, 0.001, 0.01,
                                         sp_stb_randomized_value_changed, 0.1, 3);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_SPACING);
    }

    aux_toolbox_space(tbl, AUX_SPACING);

    /* Reset */
    {
        GtkWidget *hb = gtk_hbox_new(FALSE, 1);
        GtkWidget *b = gtk_button_new_with_label(_("Defaults"));
        gtk_tooltips_set_tip(tt, b, _("Reset shape parameters to defaults (use Inkscape Preferences > Tools to change defaults)"), NULL);
        gtk_widget_show(b);
        gtk_container_add(GTK_CONTAINER(hb), b);
        gtk_signal_connect(GTK_OBJECT(b), "clicked", GTK_SIGNAL_FUNC(sp_stb_defaults), tbl);
        gtk_box_pack_start(GTK_BOX(tbl),hb, FALSE, FALSE, AUX_SPACING);
    }

    Inkscape::UI::Widget::StyleSwatch *swatch = new Inkscape::UI::Widget::StyleSwatch(NULL);
    swatch->setWatchedTool ("tools.shapes.star", true);
    GtkWidget *swatch_ = GTK_WIDGET(swatch->gobj());
    gtk_box_pack_end(GTK_BOX(tbl), swatch_, FALSE, FALSE, 0);

    gtk_widget_show_all(tbl);
    sp_set_font_size_smaller (tbl);

    sigc::connection *connection = new sigc::connection(
        sp_desktop_selection(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_star_toolbox_selection_changed), (GtkObject *)tbl))
        );
    g_signal_connect(G_OBJECT(tbl), "destroy", G_CALLBACK(delete_connection), connection);

    return tbl;
}


//########################
//##       Rect         ##
//########################

static void 
sp_rtb_sensitivize (GtkWidget *tbl)
{
    GtkAdjustment *adj1 = GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(tbl), "rx"));
    GtkAdjustment *adj2 = GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(tbl), "ry"));
    GtkWidget *not_rounded = (GtkWidget*) g_object_get_data(G_OBJECT(tbl), "not_rounded");

    if (adj1->value == 0 && adj2->value == 0 && gtk_object_get_data(GTK_OBJECT(tbl), "single")) { // only for a single selected rect (for now)
        gtk_widget_set_sensitive(GTK_WIDGET(not_rounded), FALSE);
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(not_rounded), TRUE);
    }
}


static void
sp_rtb_value_changed(GtkAdjustment *adj, GtkWidget *tbl, gchar const *value_name,
                          void (*setter)(SPRect *, gdouble))
{
    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data(GTK_OBJECT(tbl), "desktop");

    GtkWidget *us = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(tbl), "units");
    SPUnit const *unit = sp_unit_selector_get_unit(SP_UNIT_SELECTOR(us));

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_double_attribute("tools.shapes.rect", value_name, sp_units_get_pixels(adj->value, *unit));
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    bool modmade = false;
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    for (GSList const *items = selection->itemList(); items != NULL; items = items->next) {
        if (SP_IS_RECT(items->data)) {
            if (adj->value != 0) {
                setter(SP_RECT(items->data), sp_units_get_pixels(adj->value, *unit));
            } else {
                SP_OBJECT_REPR(items->data)->setAttribute(value_name, NULL);
            }
            modmade = true;
        }
    }

    sp_rtb_sensitivize (tbl);

    if (modmade) {
        sp_document_done(sp_desktop_document(desktop));
    }

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));

    // defocus spinbuttons by moving focus to the canvas, unless "stay" is on
    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void
sp_rtb_rx_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    sp_rtb_value_changed(adj, tbl, "rx", sp_rect_set_visible_rx);
}

static void
sp_rtb_ry_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    sp_rtb_value_changed(adj, tbl, "ry", sp_rect_set_visible_ry);
}

static void
sp_rtb_width_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    sp_rtb_value_changed(adj, tbl, "width", sp_rect_set_visible_width);
}

static void
sp_rtb_height_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    sp_rtb_value_changed(adj, tbl, "height", sp_rect_set_visible_height);
}



static void
sp_rtb_defaults( GtkWidget *widget, GtkObject *obj)
{
    GtkWidget *tbl = GTK_WIDGET(obj);

    GtkAdjustment *adj;

    adj = (GtkAdjustment*)gtk_object_get_data(obj, "rx");
    gtk_adjustment_set_value(adj, 0.0);
    // this is necessary if the previous value was 0, but we still need to run the callback to change all selected objects
    gtk_adjustment_value_changed(adj);

    adj = (GtkAdjustment*)gtk_object_get_data(obj, "ry");
    gtk_adjustment_set_value(adj, 0.0);
    gtk_adjustment_value_changed(adj);

    sp_rtb_sensitivize (tbl);

    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void rect_tb_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
                                       gchar const *old_value, gchar const *new_value,
                                       bool is_interactive, gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    GtkWidget *us = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(tbl), "units");
    SPUnit const *unit = sp_unit_selector_get_unit(SP_UNIT_SELECTOR(us));

    SPItem *item = SP_ITEM(g_object_get_data(G_OBJECT(tbl), "item"));
    if (SP_IS_RECT(item)) {
        {
            GtkAdjustment *adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "rx");
            gdouble rx = sp_rect_get_visible_rx(SP_RECT(item));
            gtk_adjustment_set_value(adj, sp_pixels_get_units(rx, *unit));
        }

        {
            GtkAdjustment *adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "ry");
            gdouble ry = sp_rect_get_visible_ry(SP_RECT(item));
            gtk_adjustment_set_value(adj, sp_pixels_get_units(ry, *unit));
        }

        {
            GtkAdjustment *adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "width");
            gdouble width = sp_rect_get_visible_width (SP_RECT(item));
            gtk_adjustment_set_value(adj, sp_pixels_get_units(width, *unit));
        }

        {
            GtkAdjustment *adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "height");
            gdouble height = sp_rect_get_visible_height (SP_RECT(item));
            gtk_adjustment_set_value(adj, sp_pixels_get_units(height, *unit));
        }
    }

    sp_rtb_sensitivize (tbl);

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));
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
static void
sp_rect_toolbox_selection_changed(Inkscape::Selection *selection, GtkObject *tbl)
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;
    SPItem *item = NULL;
    Inkscape::XML::Node *oldrepr = NULL;

    for (GSList const *items = selection->itemList();
         items != NULL;
         items = items->next) {
        if (SP_IS_RECT((SPItem *) items->data)) {
            n_selected++;
            item = (SPItem *) items->data;
            repr = SP_OBJECT_REPR(item);
        }
    }

    GtkWidget *l = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(tbl), "mode_label"));

    g_object_set_data(G_OBJECT(tbl), "single", GINT_TO_POINTER(FALSE));

    if (n_selected == 0) {
        gtk_label_set_markup(GTK_LABEL(l), _("<b>New:</b>"));

        GtkWidget *w = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(tbl), "width_sb");
        gtk_widget_set_sensitive(w, FALSE);
        GtkWidget *h = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(tbl), "height_sb");
        gtk_widget_set_sensitive(h, FALSE);

    } else if (n_selected == 1) {
        gtk_label_set_markup(GTK_LABEL(l), _("<b>Change:</b>"));
        g_object_set_data(G_OBJECT(tbl), "single", GINT_TO_POINTER(TRUE));

        GtkWidget *w = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(tbl), "width_sb");
        gtk_widget_set_sensitive(w, TRUE);
        GtkWidget *h = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(tbl), "height_sb");
        gtk_widget_set_sensitive(h, TRUE);

        oldrepr = (Inkscape::XML::Node *) gtk_object_get_data(GTK_OBJECT(tbl), "repr");
        if (oldrepr) { // remove old listener
            sp_repr_remove_listener_by_data(oldrepr, tbl);
            Inkscape::GC::release(oldrepr);
            oldrepr = 0;
            g_object_set_data(G_OBJECT(tbl), "repr", NULL);
        }
        if (repr) {
            g_object_set_data(G_OBJECT(tbl), "repr", repr);
            g_object_set_data(G_OBJECT(tbl), "item", item);
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &rect_tb_repr_events, tbl);
            sp_repr_synthesize_events(repr, &rect_tb_repr_events, tbl);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        gtk_label_set_markup(GTK_LABEL(l), _("<b>Change:</b>"));
        sp_rtb_sensitivize (GTK_WIDGET(tbl));
    }
}


static GtkWidget *
sp_rect_toolbox_new(SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new(FALSE, 0);

    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new();

    sp_toolbox_add_label(tbl, _("<b>New:</b>"));

    // rx/ry units menu: create
    GtkWidget *us = sp_unit_selector_new(SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
    sp_unit_selector_setsize(us, AUX_OPTION_MENU_WIDTH, AUX_OPTION_MENU_HEIGHT);
    sp_unit_selector_set_unit(SP_UNIT_SELECTOR(us), desktop->namedview->doc_units);
    // fixme: add % meaning per cent of the width/height

    /* W */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("W:"), _("Width of rectangle"),
                                         "tools.shapes.rect", "width", 0,
                                         us, tbl, TRUE, "altx-rect",
                                         0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         sp_rtb_width_value_changed);
        gtk_object_set_data(GTK_OBJECT(tbl), "width_sb", hb);
        gtk_widget_set_sensitive(hb, FALSE);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* H */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("H:"), _("Height of rectangle"),
                                         "tools.shapes.rect", "height", 0,
                                         us, tbl, FALSE, NULL,
                                         0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         sp_rtb_height_value_changed);
        gtk_object_set_data(GTK_OBJECT(tbl), "height_sb", hb);
        gtk_widget_set_sensitive(hb, FALSE);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* rx */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Rx:"), _("Horizontal radius of rounded corners"),
                                         "tools.shapes.rect", "rx", 0,
                                         us, tbl, FALSE, NULL,
                                         0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         sp_rtb_rx_value_changed);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* ry */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Ry:"), _("Vertical radius of rounded corners"),
                                         "tools.shapes.rect", "ry", 0,
                                         us, tbl, FALSE, NULL,
                                         0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,
                                         sp_rtb_ry_value_changed);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    // add the units menu
    gtk_widget_show(us);
    gtk_box_pack_start(GTK_BOX(tbl), us, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    gtk_object_set_data(GTK_OBJECT(tbl), "units", us);

    /* Reset */
    {
        GtkWidget *hb = gtk_hbox_new(FALSE, 1);
        GtkWidget *b = gtk_button_new_with_label(_("Not rounded"));
        gtk_object_set_data(GTK_OBJECT(tbl), "not_rounded", b);
        gtk_tooltips_set_tip(tt, b, _("Make corners sharp"), NULL);
        gtk_widget_show(b);
        gtk_container_add(GTK_CONTAINER(hb), b);
        gtk_signal_connect(GTK_OBJECT(b), "clicked", GTK_SIGNAL_FUNC(sp_rtb_defaults), tbl);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    Inkscape::UI::Widget::StyleSwatch *swatch = new Inkscape::UI::Widget::StyleSwatch(NULL);
    swatch->setWatchedTool ("tools.shapes.rect", true);
    GtkWidget *swatch_ = GTK_WIDGET(swatch->gobj());
    gtk_box_pack_end(GTK_BOX(tbl), swatch_, FALSE, FALSE, 0);

    g_object_set_data(G_OBJECT(tbl), "single", GINT_TO_POINTER(TRUE));
    sp_rtb_sensitivize (tbl);

    gtk_widget_show_all(tbl);
    sp_set_font_size_smaller (tbl);

    sigc::connection *connection = new sigc::connection(
        sp_desktop_selection(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_rect_toolbox_selection_changed), (GtkObject *)tbl))
        );
    g_signal_connect(G_OBJECT(tbl), "destroy", G_CALLBACK(delete_connection), connection);

    return tbl;
}

//########################
//##       Spiral       ##
//########################

static void
sp_spl_tb_value_changed(GtkAdjustment *adj, GtkWidget *tbl, gchar const *value_name)
{
    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data(GTK_OBJECT(tbl), "desktop");

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_double_attribute("tools.shapes.spiral", value_name, adj->value);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    gchar* namespaced_name = g_strconcat("sodipodi:", value_name, NULL);

    bool modmade = false;
    for (GSList const *items = sp_desktop_selection(desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_SPIRAL((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_svg_double( repr, namespaced_name, adj->value );
            SP_OBJECT((SPItem *) items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
            modmade = true;
        }
    }

    g_free(namespaced_name);

    if (modmade) {
        sp_document_done(sp_desktop_document(desktop));
    }

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));

    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void
sp_spl_tb_revolution_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    sp_spl_tb_value_changed(adj, tbl, "revolution");
}

static void
sp_spl_tb_expansion_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    sp_spl_tb_value_changed(adj, tbl, "expansion");
}

static void
sp_spl_tb_t0_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    sp_spl_tb_value_changed(adj, tbl, "t0");
}

static void
sp_spl_tb_defaults(GtkWidget *widget, GtkObject *obj)
{
    GtkWidget *tbl = GTK_WIDGET(obj);

    GtkAdjustment *adj;

    // fixme: make settable
    gdouble rev = 5;
    gdouble exp = 1.0;
    gdouble t0 = 0.0;

    adj = (GtkAdjustment*)gtk_object_get_data(obj, "revolution");
    gtk_adjustment_set_value(adj, rev);
    gtk_adjustment_value_changed(adj);

    adj = (GtkAdjustment*)gtk_object_get_data(obj, "expansion");
    gtk_adjustment_set_value(adj, exp);
    gtk_adjustment_value_changed(adj);

    adj = (GtkAdjustment*)gtk_object_get_data(obj, "t0");
    gtk_adjustment_set_value(adj, t0);
    gtk_adjustment_value_changed(adj);

    spinbutton_defocus(GTK_OBJECT(tbl));
}


static void spiral_tb_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
                                         gchar const *old_value, gchar const *new_value,
                                         bool is_interactive, gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    GtkAdjustment *adj;
    adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "revolution");
    gtk_adjustment_set_value(adj, (sp_repr_get_double_attribute(repr, "sodipodi:revolution", 3.0)));

    adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "expansion");
    gtk_adjustment_set_value(adj, (sp_repr_get_double_attribute(repr, "sodipodi:expansion", 1.0)));

    adj = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "t0");
    gtk_adjustment_set_value(adj, (sp_repr_get_double_attribute(repr, "sodipodi:t0", 0.0)));

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));
}


static Inkscape::XML::NodeEventVector spiral_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    spiral_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

static void
sp_spiral_toolbox_selection_changed(Inkscape::Selection *selection, GtkObject *tbl)
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;
    Inkscape::XML::Node *oldrepr = NULL;

    for (GSList const *items = selection->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_SPIRAL((SPItem *) items->data)) {
            n_selected++;
            repr = SP_OBJECT_REPR((SPItem *) items->data);
        }
    }

    GtkWidget *l = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(tbl), "mode_label"));

    if (n_selected == 0) {
        gtk_label_set_markup(GTK_LABEL(l), _("<b>New:</b>"));
    } else if (n_selected == 1) {
        gtk_label_set_markup(GTK_LABEL(l), _("<b>Change:</b>"));

        oldrepr = (Inkscape::XML::Node *) gtk_object_get_data(GTK_OBJECT(tbl), "repr");
        if (oldrepr) { // remove old listener
            sp_repr_remove_listener_by_data(oldrepr, tbl);
            Inkscape::GC::release(oldrepr);
            oldrepr = 0;
            g_object_set_data(G_OBJECT(tbl), "repr", NULL);
        }

        if (repr) {
            g_object_set_data(G_OBJECT(tbl), "repr", repr);
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &spiral_tb_repr_events, tbl);
            sp_repr_synthesize_events(repr, &spiral_tb_repr_events, tbl);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        gtk_label_set_markup(GTK_LABEL(l), _("<b>Change:</b>"));
    }
}


static GtkWidget *
sp_spiral_toolbox_new(SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new(FALSE, 0);
    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new();

    sp_toolbox_add_label(tbl, _("<b>New:</b>"));

    /* Revolution */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Turns:"), _("Number of revolutions"),
                                         "tools.shapes.spiral", "revolution", 3.0,
                                         NULL, tbl, TRUE, "altx-spiral",
                                         0.01, 1024.0, 0.1, 1.0,
                                         sp_spl_tb_revolution_value_changed, 1, 2);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_SPACING);
    }

    /* Expansion */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Divergence:"), _("How much denser/sparser are outer revolutions; 1 = uniform"),
                                         "tools.shapes.spiral", "expansion", 1.0,
                                         NULL, tbl, FALSE, NULL,
                                         0.0, 1000.0, 0.01, 1.0,
                                         sp_spl_tb_expansion_value_changed);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_SPACING);
    }

    /* T0 */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Inner radius:"), _("Radius of the innermost revolution (relative to the spiral size)"),
                                         "tools.shapes.spiral", "t0", 0.0,
                                         NULL, tbl, FALSE, NULL,
                                         0.0, 0.999, 0.01, 1.0,
                                         sp_spl_tb_t0_value_changed);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_SPACING);
    }

    aux_toolbox_space(tbl, AUX_SPACING);

    /* Reset */
    {
        GtkWidget *hb = gtk_hbox_new(FALSE, 1);
        GtkWidget *b = gtk_button_new_with_label(_("Defaults"));
        gtk_tooltips_set_tip(tt, b, _("Reset shape parameters to defaults (use Inkscape Preferences > Tools to change defaults)"), NULL);
        gtk_widget_show(b);
        gtk_container_add(GTK_CONTAINER(hb), b);
        gtk_signal_connect(GTK_OBJECT(b), "clicked", GTK_SIGNAL_FUNC(sp_spl_tb_defaults), tbl);
        gtk_box_pack_start(GTK_BOX(tbl),hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    Inkscape::UI::Widget::StyleSwatch *swatch = new Inkscape::UI::Widget::StyleSwatch(NULL);
    swatch->setWatchedTool ("tools.shapes.spiral", true);
    GtkWidget *swatch_ = GTK_WIDGET(swatch->gobj());
    gtk_box_pack_end(GTK_BOX(tbl), swatch_, FALSE, FALSE, 0);

    gtk_widget_show_all(tbl);
    sp_set_font_size_smaller (tbl);

    sigc::connection *connection = new sigc::connection(
        sp_desktop_selection(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_spiral_toolbox_selection_changed), (GtkObject *)tbl))
        );
    g_signal_connect(G_OBJECT(tbl), "destroy", G_CALLBACK(delete_connection), connection);

    return tbl;
}

//########################
//##     Pen/Pencil    ##
//########################


static GtkWidget *
sp_pen_toolbox_new(SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new(FALSE, 0);
    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    Inkscape::UI::Widget::StyleSwatch *swatch = new Inkscape::UI::Widget::StyleSwatch(NULL);
    swatch->setWatchedTool ("tools.freehand.pen", true);
    GtkWidget *swatch_ = GTK_WIDGET(swatch->gobj());
    gtk_box_pack_end(GTK_BOX(tbl), swatch_, FALSE, FALSE, 0);

    gtk_widget_show_all(tbl);
    sp_set_font_size_smaller (tbl);

    return tbl;
}

static GtkWidget *
sp_pencil_toolbox_new(SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new(FALSE, 0);
    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    Inkscape::UI::Widget::StyleSwatch *swatch = new Inkscape::UI::Widget::StyleSwatch(NULL);
    swatch->setWatchedTool ("tools.freehand.pencil", true);
    GtkWidget *swatch_ = GTK_WIDGET(swatch->gobj());
    gtk_box_pack_end(GTK_BOX(tbl), swatch_, FALSE, FALSE, 0);

    gtk_widget_show_all(tbl);
    sp_set_font_size_smaller (tbl);

    return tbl;
}


//########################
//##     Calligraphy    ##
//########################

static void
sp_ddc_mass_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    prefs_set_double_attribute("tools.calligraphic", "mass", adj->value);
    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void
sp_ddc_drag_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    prefs_set_double_attribute("tools.calligraphic", "drag", adj->value);
    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void
sp_ddc_angle_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    prefs_set_double_attribute("tools.calligraphic", "angle", adj->value);
    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void
sp_ddc_width_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    prefs_set_double_attribute("tools.calligraphic", "width", adj->value * 0.01);
    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void
sp_ddc_velthin_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    prefs_set_double_attribute("tools.calligraphic", "thinning", adj->value);
    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void
sp_ddc_flatness_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    prefs_set_double_attribute("tools.calligraphic", "flatness", adj->value);
    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void
sp_ddc_tremor_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    prefs_set_double_attribute("tools.calligraphic", "tremor", adj->value);
    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void
sp_ddc_pressure_state_changed(GtkWidget *button, gpointer data)
{
    prefs_set_int_attribute ("tools.calligraphic", "usepressure", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)) ? 1 : 0);
}

static void
sp_ddc_tilt_state_changed(GtkWidget *button, GtkWidget *calligraphy_angle)
{
    prefs_set_int_attribute ("tools.calligraphic", "usetilt", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)) ? 1 : 0);

    gtk_widget_set_sensitive(GTK_WIDGET(calligraphy_angle), !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)));
}

static void sp_ddc_defaults(GtkWidget *, GtkWidget *tbl)
{
    // FIXME: make defaults settable via Inkscape Options
    struct KeyValue {
        char const *key;
        double value;
    } const key_values[] = {
        {"mass", 0.02},
        {"drag", 1.0},
        {"angle", 30.0},
        {"width", 15},
        {"thinning", 0.1},
        {"tremor", 0.0},
        {"flatness", 0.9}
    };

    for (unsigned i = 0; i < G_N_ELEMENTS(key_values); ++i) {
        KeyValue const &kv = key_values[i];
        GtkAdjustment &adj = *static_cast<GtkAdjustment *>(gtk_object_get_data(GTK_OBJECT(tbl), kv.key));
        gtk_adjustment_set_value(&adj, kv.value);
    }

    spinbutton_defocus(GTK_OBJECT(tbl));
}

static GtkWidget *
sp_calligraphy_toolbox_new(SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new(FALSE, 0);
    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new();
    GtkWidget *calligraphy_angle;

    //  interval
    gtk_box_pack_start(GTK_BOX(tbl), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    /* Width */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Width:"), _("The width of the calligraphic pen (relative to the visible canvas area)"),
                                         "tools.calligraphic", "width", 15,
                                         NULL, tbl, TRUE, "altx-calligraphy",
                                         1, 100, 1.0, 10.0,
                                         sp_ddc_width_value_changed,  0.01, 0, 100);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_SPACING);
    }

    /* Thinning */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Thinning:"), _("How much velocity thins the stroke (> 0 makes fast strokes thinner, < 0 makes them broader, 0 makes width independent of velocity)"),
                                         "tools.calligraphic", "thinning", 0.1,
                                         NULL, tbl, FALSE, NULL,
                                         -1.0, 1.0, 0.01, 0.1,
                                         sp_ddc_velthin_value_changed, 0.01, 2);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_SPACING);
    }

    //  interval
    gtk_box_pack_start(GTK_BOX(tbl), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    /* Angle */
    {
        calligraphy_angle = sp_tb_spinbutton(_("Angle:"), _("The angle of the pen's nib (in degrees; 0 = horizontal; has no effect if fixation = 0)"),
                                             "tools.calligraphic", "angle", 30,
                                             NULL, tbl, TRUE, "calligraphy-angle",
                                             -90.0, 90.0, 1.0, 10.0,
                                             sp_ddc_angle_value_changed, 1, 0);
        gtk_box_pack_start(GTK_BOX(tbl), calligraphy_angle, FALSE, FALSE, AUX_SPACING);
    }

    /* Fixation */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Fixation:"), _("How fixed is the pen angle (0 = always perpendicular to stroke direction, 1 = fixed)"),
                                         "tools.calligraphic", "flatness", 0.9,
                                         NULL, tbl, FALSE, NULL,
                                         0.0, 1.0, 0.01, 0.1,
                                         sp_ddc_flatness_value_changed, 0.01, 2);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_SPACING);
    }

    //  interval
    gtk_box_pack_start(GTK_BOX(tbl), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    /* Tremor */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Tremor:"), _("How uneven or trembling is the pen stroke"),
                                         "tools.calligraphic", "tremor", 0.0,
                                         NULL, tbl, FALSE, NULL,
                                         0.0, 1.0, 0.01, 0.1,
                                         sp_ddc_tremor_value_changed, 0.01, 2);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_SPACING);
    }
    /* Mass */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Mass:"), _("How much inertia affects the movement of the pen"),
                                         "tools.calligraphic", "mass", 0.02,
                                         NULL, tbl, FALSE, NULL,
                                         0.0, 1.0, 0.01, 0.1,
                                         sp_ddc_mass_value_changed, 0.01, 2);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_SPACING);
    }

    /* Drag */
    {
        // TRANSLATORS: "drag" means "resistance" here
        GtkWidget *hb = sp_tb_spinbutton(_("Drag:"), _("How much resistance affects the movement of the pen"),
                                         "tools.calligraphic", "drag", 1,
                                         NULL, tbl, FALSE, NULL,
                                         0.0, 1.0, 0.01, 0.1,
                                         sp_ddc_drag_value_changed, 0.01, 2);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_SPACING);
    }

    //  interval
    gtk_box_pack_start(GTK_BOX(tbl), gtk_hbox_new(FALSE, 0), FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    GtkWidget *cvbox = gtk_vbox_new (FALSE, 0);
    GtkWidget *cbox = gtk_hbox_new (FALSE, 0);

    /* Use Pressure button */
    {
    GtkWidget *button = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                                 SP_BUTTON_TYPE_TOGGLE,
                                                 NULL,
                                                 "use_pressure",
                                                 _("Use the pressure of the input device to alter the width of the pen"),
                                                 tt);
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (sp_ddc_pressure_state_changed), NULL);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), prefs_get_int_attribute ("tools.calligraphic", "usepressure", 1));
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    /* Use Tilt button */
    {
    GtkWidget *button = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                                 SP_BUTTON_TYPE_TOGGLE,
                                                 NULL,
                                                 "use_tilt",
                                                 _("Use the tilt of the input device to alter the angle of the pen's nib"),
                                                 tt);
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (sp_ddc_tilt_state_changed), calligraphy_angle);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), prefs_get_int_attribute ("tools.calligraphic", "usetilt", 1));
    gtk_widget_set_sensitive(GTK_WIDGET(calligraphy_angle), !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)));
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(cvbox), cbox, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tbl), cvbox, FALSE, FALSE, 0);

    /* Reset */
    {
        GtkWidget *hb = gtk_hbox_new(FALSE, 1);
        GtkWidget *b = gtk_button_new_with_label(_("Defaults"));
        gtk_tooltips_set_tip(tt, b, _("Reset shape parameters to defaults (use Inkscape Preferences > Tools to change defaults)"), NULL);
        gtk_widget_show(b);
        gtk_container_add(GTK_CONTAINER(hb), b);
        gtk_signal_connect(GTK_OBJECT(b), "clicked", GTK_SIGNAL_FUNC(sp_ddc_defaults), tbl);
        gtk_box_pack_start(GTK_BOX(tbl),hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }


    Inkscape::UI::Widget::StyleSwatch *swatch = new Inkscape::UI::Widget::StyleSwatch(NULL);
    swatch->setWatchedTool ("tools.calligraphic", true);
    GtkWidget *swatch_ = GTK_WIDGET(swatch->gobj());
    gtk_box_pack_end(GTK_BOX(tbl), swatch_, FALSE, FALSE, 0);

    gtk_widget_show_all(tbl);
    sp_set_font_size_smaller (tbl);

    return tbl;
}


//########################
//##    Circle / Arc    ##
//########################

static void 
sp_arctb_sensitivize (GtkWidget *tbl, double v1, double v2)
{
    GtkWidget *ocb = (GtkWidget*) g_object_get_data(G_OBJECT(tbl), "open_checkbox");
    GtkWidget *make_whole = (GtkWidget*) g_object_get_data(G_OBJECT(tbl), "make_whole");

    if (v1 == 0 && v2 == 0) {
        if (gtk_object_get_data(GTK_OBJECT(tbl), "single")) { // only for a single selected ellipse (for now)
            gtk_widget_set_sensitive(GTK_WIDGET(ocb), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(make_whole), FALSE);
        }
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(ocb), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(make_whole), TRUE);
    }
}

static void
sp_arctb_startend_value_changed(GtkAdjustment *adj, GtkWidget *tbl, gchar const *value_name, gchar const *other_name)
{
    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data(GTK_OBJECT(tbl), "desktop");

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs_set_double_attribute("tools.shapes.arc", value_name, (adj->value * M_PI)/ 180);
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    gchar* namespaced_name = g_strconcat("sodipodi:", value_name, NULL);

    bool modmade = false;
    for (GSList const *items = sp_desktop_selection(desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        SPItem *item = SP_ITEM(items->data);

        if (SP_IS_ARC(item) && SP_IS_GENERICELLIPSE(item)) {

            SPGenericEllipse *ge = SP_GENERICELLIPSE(item);
            SPArc *arc = SP_ARC(item);

            if (!strcmp(value_name, "start"))
                ge->start = (adj->value * M_PI)/ 180;
            else
                ge->end = (adj->value * M_PI)/ 180;

            sp_genericellipse_normalize(ge);
            ((SPObject *)arc)->updateRepr();
            ((SPObject *)arc)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);

            modmade = true;
        }
    }

    g_free(namespaced_name);

    GtkAdjustment *other = (GtkAdjustment *)gtk_object_get_data(GTK_OBJECT(tbl), other_name);

    sp_arctb_sensitivize (tbl, adj->value, other->value);

    if (modmade) {
        sp_document_maybe_done(sp_desktop_document(desktop), value_name);
    }

    // defocus spinbuttons by moving focus to the canvas, unless "stay" is on
    spinbutton_defocus(GTK_OBJECT(tbl));

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));
}


static void
sp_arctb_start_value_changed(GtkAdjustment *adj,  GtkWidget *tbl)
{
    sp_arctb_startend_value_changed(adj,  tbl, "start", "end");
}

static void
sp_arctb_end_value_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    sp_arctb_startend_value_changed(adj,  tbl, "end", "start");
}

static void
sp_arctb_open_state_changed(GtkWidget *widget, GtkObject *tbl)
{
    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data(GTK_OBJECT(tbl), "desktop");

    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
            prefs_set_string_attribute("tools.shapes.arc", "open", "true");
        } else {
            prefs_set_string_attribute("tools.shapes.arc", "open", NULL);
        }
    }

    // quit if run by the attr_changed listener
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    bool modmade = false;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        for (GSList const *items = sp_desktop_selection(desktop)->itemList();
             items != NULL;
             items = items->next)
        {
            if (SP_IS_ARC((SPItem *) items->data)) {
                Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
                repr->setAttribute("sodipodi:open", "true");
                SP_OBJECT((SPItem *) items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
                modmade = true;
            }
        }
    } else {
        for (GSList const *items = sp_desktop_selection(desktop)->itemList();
             items != NULL;
             items = items->next)
        {
            if (SP_IS_ARC((SPItem *) items->data))    {
                Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
                repr->setAttribute("sodipodi:open", NULL);
                SP_OBJECT((SPItem *) items->data)->updateRepr(repr, SP_OBJECT_WRITE_EXT);
                modmade = true;
            }
        }
    }

    if (modmade) {
        sp_document_done(sp_desktop_document(desktop));
    }

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));

    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void sp_arctb_defaults(GtkWidget *, GtkObject *obj)
{
    GtkWidget *tbl = GTK_WIDGET(obj);

    GtkAdjustment *adj;
    adj = (GtkAdjustment*)gtk_object_get_data(obj, "start");
    gtk_adjustment_set_value(adj, 0.0);
    gtk_adjustment_value_changed(adj);

    adj = (GtkAdjustment*)gtk_object_get_data(obj, "end");
    gtk_adjustment_set_value(adj, 0.0);
    gtk_adjustment_value_changed(adj);

    spinbutton_defocus(GTK_OBJECT(tbl));
}

static void arc_tb_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
                                      gchar const *old_value, gchar const *new_value,
                                      bool is_interactive, gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    // quit if run by the _changed callbacks
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    gdouble start = sp_repr_get_double_attribute(repr, "sodipodi:start", 0.0);
    gdouble end = sp_repr_get_double_attribute(repr, "sodipodi:end", 0.0);

    GtkAdjustment *adj1,*adj2;
    adj1 = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "start");
    gtk_adjustment_set_value(adj1, mod360((start * 180)/M_PI));
    adj2 = (GtkAdjustment*)gtk_object_get_data(GTK_OBJECT(tbl), "end");
    gtk_adjustment_set_value(adj2, mod360((end * 180)/M_PI));

    sp_arctb_sensitivize (tbl, adj1->value, adj2->value);

    char const *openstr = NULL;
    openstr = repr->attribute("sodipodi:open");
    GtkWidget *ocb = (GtkWidget*) g_object_get_data(G_OBJECT(tbl), "open_checkbox");

    if (openstr) {
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(ocb),  TRUE);
    } else {
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(ocb),  FALSE);
    }

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));
}

static Inkscape::XML::NodeEventVector arc_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    arc_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


static void
sp_arc_toolbox_selection_changed(Inkscape::Selection *selection, GtkObject *tbl)
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;
    Inkscape::XML::Node *oldrepr = NULL;

    for (GSList const *items = selection->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_ARC((SPItem *) items->data)) {
            n_selected++;
            repr = SP_OBJECT_REPR((SPItem *) items->data);
        }
    }

    GtkWidget *l = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(tbl), "mode_label"));

    g_object_set_data(G_OBJECT(tbl), "single", GINT_TO_POINTER(FALSE));
    if (n_selected == 0) {
        gtk_label_set_markup(GTK_LABEL(l), _("<b>New:</b>"));
    } else if (n_selected == 1) {
        g_object_set_data(G_OBJECT(tbl), "single", GINT_TO_POINTER(TRUE));
        gtk_label_set_markup(GTK_LABEL(l), _("<b>Change:</b>"));

        oldrepr = (Inkscape::XML::Node *) gtk_object_get_data(GTK_OBJECT(tbl), "repr");

        if (oldrepr) { // remove old listener
            sp_repr_remove_listener_by_data(oldrepr, tbl);
            Inkscape::GC::release(oldrepr);
            oldrepr = 0;
            g_object_set_data(G_OBJECT(tbl), "repr", NULL);
        }

        if (repr) {
            g_object_set_data(G_OBJECT(tbl), "repr", repr);
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &arc_tb_repr_events, tbl);
            sp_repr_synthesize_events(repr, &arc_tb_repr_events, tbl);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        gtk_label_set_markup(GTK_LABEL(l), _("<b>Change:</b>"));
        sp_arctb_sensitivize (GTK_WIDGET(tbl), 1, 0);
    }
}


static GtkWidget *
sp_arc_toolbox_new(SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new(FALSE, 0);

    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new();

    sp_toolbox_add_label(tbl, _("<b>New:</b>"));

    /* Start */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("Start:"), _("The angle (in degrees) from the horizontal to the arc's start point"),
                                         "tools.shapes.arc", "start", 0.0,
                                         NULL, tbl, TRUE, "altx-arc",
                                         -360.0, 360.0, 1.0, 10.0,
                                         sp_arctb_start_value_changed);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* End */
    {
        GtkWidget *hb = sp_tb_spinbutton(_("End:"), _("The angle (in degrees) from the horizontal to the arc's end point"),
                                         "tools.shapes.arc", "end", 0.0,
                                         NULL, tbl, FALSE, NULL,
                                         -360.0, 360.0, 1.0, 10.0,
                                         sp_arctb_end_value_changed);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* Segments / Pie checkbox */
    {
        GtkWidget *hb = gtk_hbox_new(FALSE, 1);
        GtkWidget *fscb = gtk_check_button_new_with_label(_("Open arc"));
        gtk_tooltips_set_tip(tt, fscb, _("Switch between arc (unclosed shape) and segment (closed shape with two radii)"), NULL);

        gchar const *openstr = NULL;
        openstr = prefs_get_string_attribute("tools.shapes.arc", "open");
        if (!openstr || (openstr && !strcmp(openstr, "false")))
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(fscb),  FALSE);
        else
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(fscb),  TRUE);

        gtk_widget_show(fscb);
        gtk_object_set_data(GTK_OBJECT(tbl), "open_checkbox", fscb);
        gtk_container_add(GTK_CONTAINER(hb), fscb);
        g_signal_connect(G_OBJECT(fscb), "toggled", GTK_SIGNAL_FUNC(sp_arctb_open_state_changed ), tbl);
        gtk_box_pack_start(GTK_BOX(tbl),hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    /* Make Whole */
    {
        GtkWidget *hb = gtk_hbox_new(FALSE, 1);
        GtkWidget *b = gtk_button_new_with_label(_("Make whole"));
        gtk_object_set_data(GTK_OBJECT(tbl), "make_whole", b);
        gtk_tooltips_set_tip(tt, b, _("Make the shape a whole ellipse, not arc or segment"), NULL);
        gtk_widget_show(b);
        gtk_container_add(GTK_CONTAINER(hb), b);
        gtk_signal_connect(GTK_OBJECT(b), "clicked", GTK_SIGNAL_FUNC(sp_arctb_defaults), tbl);
        gtk_box_pack_start(GTK_BOX(tbl),hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }

    g_object_set_data(G_OBJECT(tbl), "single", GINT_TO_POINTER(TRUE));
    // sensitivize make whole and open checkbox
    {
        GtkAdjustment *adj1 = GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(tbl), "start"));
        GtkAdjustment *adj2 = GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(tbl), "end"));
        sp_arctb_sensitivize (tbl, adj1->value, adj2->value);
    }

    sigc::connection *connection = new sigc::connection(
        sp_desktop_selection(desktop)->connectChanged(sigc::bind(sigc::ptr_fun(sp_arc_toolbox_selection_changed), (GtkObject *)tbl))
        );
    g_signal_connect(G_OBJECT(tbl), "destroy", G_CALLBACK(delete_connection), connection);

    Inkscape::UI::Widget::StyleSwatch *swatch = new Inkscape::UI::Widget::StyleSwatch(NULL);
    swatch->setWatchedTool ("tools.shapes.arc", true);
    GtkWidget *swatch_ = GTK_WIDGET(swatch->gobj());
    gtk_box_pack_end(GTK_BOX(tbl), swatch_, FALSE, FALSE, 0);

    gtk_widget_show_all(tbl);
    sp_set_font_size_smaller (tbl);

    return tbl;
}




// toggle button callbacks and updaters

//########################
//##      Dropper       ##
//########################

static void toggle_dropper_color_pick (GtkWidget *button, gpointer data) {
    prefs_set_int_attribute ("tools.dropper", "pick", 
        // 0 and 1 are backwards here because of pref
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)) ? 0 : 1);
}


/**
 * Copy the current saved desktop color to the clipboard as full hex + alpha
 * color representation. This is useful for passing values between various 
 * input boxes, or directly to xml.
 */
/* static void
sp_dropper_copy( GtkWidget *widget, GtkObject *obj)
{
    GtkWidget *tbl = GTK_WIDGET(obj);
    
    SPDesktop *desktop = 
        (SPDesktop *) gtk_object_get_data(GTK_OBJECT(tbl), "desktop");

   
    sp_dropper_c32_color_copy( sp_desktop_get_color(desktop, true) );
}*/


/**
 * Copies currently saved desktop color to the clipboard as a hex value. This 
 * is useful for editing webpages and needing a value quickly for web
 * colors.
 * 
 * TODO: When the toggle of the dropper is set to not mix color against 
 *       page background, this still just gets the color of the page and 
 *       doesn't get the actual mixed against background which is needed 
 *       for the hex value ppl. want for web pages, etc.
 */

/* static void
sp_dropper_copy_as_hex ( GtkWidget *widget, GtkObject *obj)
{
    GtkWidget *tbl = GTK_WIDGET(obj);
    
    SPDesktop *desktop = 
        (SPDesktop *) gtk_object_get_data(GTK_OBJECT(tbl), "desktop");
    
    sp_dropper_c32_color_copy_hex( sp_desktop_get_color(desktop, true) );
}*/


/**
 * Sets the input boxes with the changed color and opacity. This is used as a 
 * callback for style changing.
 */
/* static bool
sp_style_changed (const SPCSSAttr *css, gpointer data)
{
    // GrDrag *drag = (GrDrag *) data;
    
    // set fill of text entry box
    if (css->attribute("fill"))
        gtk_entry_set_text((GtkEntry *)dropper_rgb_entry, 
            css->attribute("fill")); 

    // set opacity of text entry box
    if (css->attribute("fill-opacity"))
        gtk_entry_set_text((GtkEntry *)dropper_opacity_entry, 
            css->attribute("fill-opacity")); 
    
    // set fill of text entry box
    if (css->attribute("stroke"))
        gtk_entry_set_text((GtkEntry *)dropper_rgb_entry, 
            css->attribute("stroke")); 

    // set opacity of text entry box
    if (css->attribute("stroke-opacity"))
        gtk_entry_set_text((GtkEntry *)dropper_opacity_entry, 
            css->attribute("stroke-opacity"));
    return false;

}
*/


/**
 * Dropper auxiliary toolbar construction and setup.
 *
 * TODO: Would like to add swatch of current color.
 * TODO: Add queue of last 5 or so colors selected with new swatches so that
 *       can drag and drop places. Will provide a nice mixing palette.
 */
static GtkWidget *
sp_dropper_toolbox_new(SPDesktop *desktop)
{
    GtkWidget *tbl = gtk_hbox_new(FALSE, 0);

    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new();

    
    gtk_box_pack_start(GTK_BOX(tbl), gtk_hbox_new(FALSE, 0), FALSE, FALSE, 
                       AUX_BETWEEN_BUTTON_GROUPS);
    // sp_toolbox_add_label(tbl, _("<b>New:</b>"));


    
    /* RGB Input Field */
 /*   {
        GtkWidget *hb = gtk_hbox_new(FALSE, 1);
        GtkWidget *dropper_rgba_label = gtk_label_new ("Color:");
        gtk_widget_show (dropper_rgba_label);
        gtk_container_add(GTK_CONTAINER(hb), dropper_rgba_label);
        
      	dropper_rgb_entry = gtk_entry_new ();
	sp_dialog_defocus_on_enter (dropper_rgb_entry);
	gtk_entry_set_max_length (GTK_ENTRY (dropper_rgb_entry), 7);
	gtk_entry_set_width_chars (GTK_ENTRY (dropper_rgb_entry), 7);
        gtk_tooltips_set_tip(tt, dropper_rgb_entry, 
                         _("Hexidecimal representation of last selected "
                           "color"), 
                         NULL);
	gtk_widget_show (dropper_rgb_entry);
        gtk_container_add(GTK_CONTAINER(hb), dropper_rgb_entry);
        
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, 
                           AUX_BETWEEN_BUTTON_GROUPS);
    } */
    
    /* Opacity Input Field */
/*    {
        GtkWidget *hb = gtk_hbox_new(FALSE, 1);
        GtkWidget *dropper_opacity_label = gtk_label_new ( _("Opacity:") );
        gtk_widget_show (dropper_opacity_label);
        gtk_container_add(GTK_CONTAINER(hb), dropper_opacity_label);
        
      	dropper_opacity_entry = gtk_entry_new ();
	sp_dialog_defocus_on_enter (dropper_opacity_entry);
	gtk_entry_set_max_length (GTK_ENTRY (dropper_opacity_entry), 11);
	gtk_entry_set_width_chars (GTK_ENTRY (dropper_opacity_entry), 11);
        gtk_tooltips_set_tip(tt, dropper_opacity_entry, 
                         _("Opacity of last selected color"), 
                         NULL);
	gtk_widget_show (dropper_opacity_entry);
        gtk_container_add(GTK_CONTAINER(hb), dropper_opacity_entry);
        
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, 
                           AUX_BETWEEN_BUTTON_GROUPS);
    } */
    
    
    /* Copy to Clipboard */
/*    {
        GtkWidget *hb = gtk_hbox_new(FALSE, 1);
        GtkWidget *b = gtk_button_new_with_label(_("Copy as RGBA"));
        gtk_tooltips_set_tip(tt, b, _("Copy last saved color as hexidecimal "
				      "RGB + Alpha (RGBA) to "
                                      "clipboard"), 
                             NULL);
        gtk_widget_show(b);
        gtk_container_add(GTK_CONTAINER(hb), b);
        gtk_signal_connect(GTK_OBJECT(b), "clicked", 
            GTK_SIGNAL_FUNC(sp_dropper_copy), tbl);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, 
                           AUX_BETWEEN_BUTTON_GROUPS);
    } */


    /* Copy to Clipboard as HEX */
/*    {
        GtkWidget *hb = gtk_hbox_new(FALSE, 1);
        GtkWidget *b = gtk_button_new_with_label(_("Copy as HEX"));
        gtk_tooltips_set_tip(tt, b, _("Copy last saved color as "
				      "hexidecimal RGB without alpha " 
				      "to clipboard"), NULL);
        gtk_widget_show(b);
        gtk_container_add(GTK_CONTAINER(hb), b);
        gtk_signal_connect(GTK_OBJECT(b), "clicked", 
            GTK_SIGNAL_FUNC(sp_dropper_copy_as_hex), tbl);
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, 
                           AUX_BETWEEN_BUTTON_GROUPS);
    } */
    
    // aux_toolbox_space(tbl, AUX_BETWEEN_BUTTON_GROUPS);
    
    {
        GtkWidget *hb = gtk_hbox_new(FALSE, 1);
        
        GtkWidget *button = 
            sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                     SP_BUTTON_TYPE_TOGGLE,
                                     NULL,
                                     "pick_color",
                                     _("When pressed, picks visible color "
				       "without alpha and when not pressed, "
				       "picks color including its "
				       "alpha"),
                                     tt);

        gtk_widget_show(button);
        gtk_container_add (GTK_CONTAINER (hb), button);

        g_signal_connect_after (G_OBJECT (button), "clicked", 
                                G_CALLBACK (toggle_dropper_color_pick), NULL);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), 
                                      !prefs_get_int_attribute ("tools.dropper", 
                                                               "pick", 0));
        gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, 
                   AUX_BETWEEN_BUTTON_GROUPS);
    }
   
    aux_toolbox_space(tbl, AUX_BETWEEN_BUTTON_GROUPS);
    

    // where new gtkmm stuff should go
    
    gtk_widget_show_all(tbl);
    sp_set_font_size_smaller (tbl);

    /*
    sigc::connection *connection = new sigc::connection(
        desktop->connectSetStyle(
            sigc::bind(sigc::ptr_fun(sp_style_changed), 
                       desktop)) ); 
    
    g_signal_connect(G_OBJECT(tbl), "destroy", G_CALLBACK(delete_connection), 
                     connection); */
    
    return tbl;
}


//########################
//##    Text Toolbox    ##
//########################
/*
static void
sp_text_letter_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    //Call back for letter sizing spinbutton
}

static void
sp_text_line_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    //Call back for line height spinbutton
}

static void
sp_text_horiz_kern_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    //Call back for horizontal kerning spinbutton
}

static void
sp_text_vert_kern_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    //Call back for vertical kerning spinbutton
}

static void
sp_text_letter_rotation_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    //Call back for letter rotation spinbutton
}*/

namespace {

bool visible = false;

void
sp_text_toolbox_selection_changed (Inkscape::Selection *selection, GObject *tbl)
{

    SPStyle *query =
        sp_style_new ();

    int result_family =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTFAMILY); 

    int result_style =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTSTYLE); 

    int result_numbers =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS); 

    gtk_widget_hide (GTK_WIDGET (g_object_get_data (G_OBJECT(tbl), "warning-image")));        

    // If querying returned nothing, read the style from the text tool prefs (default style for new texts)
    if (result_family == QUERY_STYLE_NOTHING || result_style == QUERY_STYLE_NOTHING || result_numbers == QUERY_STYLE_NOTHING)
    {
        Inkscape::XML::Node *repr = inkscape_get_repr (INKSCAPE, "tools.text");

        if (repr)
        {
            sp_style_read_from_repr (query, repr);
        }
        else
        {
            return;
        }
    }

    if (query->text)
    {
        if (result_family == QUERY_STYLE_MULTIPLE_DIFFERENT) {
            GtkWidget *entry = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "family-entry"));
            gtk_entry_set_text (GTK_ENTRY (entry), "");

        } else if (query->text->font_family.value) {

            GtkWidget *entry = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "family-entry"));
            gtk_entry_set_text (GTK_ENTRY (entry), query->text->font_family.value);

            Gtk::TreePath path;
            try {
                path = Inkscape::FontLister::get_instance()->get_row_for_font (query->text->font_family.value);
            } catch (...) {
                return;
            }

            GtkTreeSelection *tselection = GTK_TREE_SELECTION (g_object_get_data (G_OBJECT(tbl), "family-tree-selection"));
            GtkTreeView *treeview = GTK_TREE_VIEW (g_object_get_data (G_OBJECT(tbl), "family-tree-view"));

            g_object_set_data (G_OBJECT (tselection), "block", gpointer(1)); 

            gtk_tree_selection_select_path (tselection, path.gobj());
            gtk_tree_view_scroll_to_cell (treeview, path.gobj(), NULL, TRUE, 0.5, 0.0);

            g_object_set_data (G_OBJECT (tselection), "block", gpointer(0)); 
        }

        //Size
        GtkWidget *cbox = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "combo-box-size"));
        char *str = g_strdup_printf ("%.5g", query->font_size.computed);
        g_object_set_data (tbl, "size-block", gpointer(1)); 
        gtk_entry_set_text (GTK_ENTRY(GTK_BIN (cbox)->child), str);
        g_object_set_data (tbl, "size-block", gpointer(0)); 
        free (str);

        //Anchor
        if (query->text_align.computed == SP_CSS_TEXT_ALIGN_JUSTIFY)
        {
            GtkWidget *button = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "text-fill"));
            g_object_set_data (G_OBJECT (button), "block", gpointer(1));
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
            g_object_set_data (G_OBJECT (button), "block", gpointer(0));
        }
        else
        {
            if (query->text_anchor.computed == SP_CSS_TEXT_ANCHOR_START)
            {
                GtkWidget *button = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "text-start"));
                g_object_set_data (G_OBJECT (button), "block", gpointer(1));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
                g_object_set_data (G_OBJECT (button), "block", gpointer(0));
            }
            else if (query->text_anchor.computed == SP_CSS_TEXT_ANCHOR_MIDDLE)
            {
                GtkWidget *button = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "text-middle"));
                g_object_set_data (G_OBJECT (button), "block", gpointer(1));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
                g_object_set_data (G_OBJECT (button), "block", gpointer(0));
            }
            else if (query->text_anchor.computed == SP_CSS_TEXT_ANCHOR_END)
            {
                GtkWidget *button = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "text-end"));
                g_object_set_data (G_OBJECT (button), "block", gpointer(1));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
                g_object_set_data (G_OBJECT (button), "block", gpointer(0));
            }
        }

        //Style
        {
            GtkToggleButton *button = GTK_TOGGLE_BUTTON (g_object_get_data (G_OBJECT (tbl), "style-bold"));

            gboolean active = gtk_toggle_button_get_active (button);
            gboolean check  = (query->font_weight.computed >= SP_CSS_FONT_WEIGHT_700); 

            if (active != check) 
            {
                g_object_set_data (G_OBJECT (button), "block", gpointer(1));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), check); 
                g_object_set_data (G_OBJECT (button), "block", gpointer(0));
            }
        }

        {
            GtkToggleButton *button = GTK_TOGGLE_BUTTON (g_object_get_data (G_OBJECT (tbl), "style-italic"));

            gboolean active = gtk_toggle_button_get_active (button);
            gboolean check  = (query->font_style.computed != SP_CSS_FONT_STYLE_NORMAL); 

            if (active != check)
            {
                g_object_set_data (G_OBJECT (button), "block", gpointer(1));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), check); 
                g_object_set_data (G_OBJECT (button), "block", gpointer(0));
            }
        }

        //Orientation
        //locking both buttons, changing one affect all group (both)
        GtkWidget *button = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "orientation-horizontal"));
        g_object_set_data (G_OBJECT (button), "block", gpointer(1));
        
        GtkWidget *button1 = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "orientation-vertical"));
        g_object_set_data (G_OBJECT (button1), "block", gpointer(1));
        
        if (query->writing_mode.computed == SP_CSS_WRITING_MODE_LR_TB)
        {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
        }
        else
        {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button1), TRUE);
        }
        g_object_set_data (G_OBJECT (button), "block", gpointer(0));
        g_object_set_data (G_OBJECT (button1), "block", gpointer(0));
    }
}

void
sp_text_toolbox_selection_modified (Inkscape::Selection *selection, guint flags, GObject *tbl) 
{
    sp_text_toolbox_selection_changed (selection, tbl); 
}

void
sp_text_toolbox_subselection_changed (gpointer dragger, GObject *tbl)
{
    sp_text_toolbox_selection_changed (NULL, tbl); 
}

void
sp_text_toolbox_family_changed (GtkTreeSelection    *selection,
                                GObject             *tbl) 
{
    SPDesktop    *desktop = SP_ACTIVE_DESKTOP;
    GtkTreeModel *model = 0;
    GtkWidget    *popdown = GTK_WIDGET (g_object_get_data (tbl, "family-popdown-window"));
    GtkWidget    *entry = GTK_WIDGET (g_object_get_data (tbl, "family-entry"));
    GtkTreeIter   iter;
    char         *family = 0;

    gdk_pointer_ungrab (GDK_CURRENT_TIME);
    gdk_keyboard_ungrab (GDK_CURRENT_TIME);

    if ( !gtk_tree_selection_get_selected( selection, &model, &iter ) ) {
        return;
    }

    gtk_tree_model_get (model, &iter, 0, &family, -1);

    if (g_object_get_data (G_OBJECT (selection), "block"))
    {
        gtk_entry_set_text (GTK_ENTRY (entry), family);
        return;
    }
        
    gtk_widget_hide (popdown);
    visible = false;

    gtk_entry_set_text (GTK_ENTRY (entry), family);

    SPStyle *query =
        sp_style_new ();

    int result_numbers =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS); 

    SPCSSAttr *css = sp_repr_css_attr_new (); 
    sp_repr_css_set_property (css, "font-family", family); 

    // If querying returned nothing, read the style from the text tool prefs (default style for new texts)
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        sp_repr_css_change (inkscape_get_repr (INKSCAPE, "tools.text"), css, "style");
        sp_text_edit_dialog_default_set_insensitive (); //FIXME: Replace trough a verb
    }
    else
    {
        sp_desktop_set_style (desktop, css, true, true);
    }

    sp_document_done (sp_desktop_document (SP_ACTIVE_DESKTOP));
    sp_repr_css_attr_unref (css);
    free (family);
    gtk_widget_hide (GTK_WIDGET (g_object_get_data (G_OBJECT(tbl), "warning-image")));        

    gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
}

void
sp_text_toolbox_family_entry_activate (GtkEntry     *entry,
                                       GObject      *tbl) 
{
    const char *family = gtk_entry_get_text (entry);

    try {    
        Gtk::TreePath path = Inkscape::FontLister::get_instance()->get_row_for_font (family);
        GtkTreeSelection *selection = GTK_TREE_SELECTION (g_object_get_data (G_OBJECT(tbl), "family-tree-selection"));
        GtkTreeView *treeview = GTK_TREE_VIEW (g_object_get_data (G_OBJECT(tbl), "family-tree-view"));
        gtk_tree_selection_select_path (selection, path.gobj());
        gtk_tree_view_scroll_to_cell (treeview, path.gobj(), NULL, TRUE, 0.5, 0.0);
        gtk_widget_hide (GTK_WIDGET (g_object_get_data (G_OBJECT(tbl), "warning-image")));        
    } catch (...) {
        if (family && strlen (family))
        {
            gtk_widget_show_all (GTK_WIDGET (g_object_get_data (G_OBJECT(tbl), "warning-image")));
        }
    }
}

void
sp_text_toolbox_anchoring_toggled (GtkRadioButton   *button,
                                   gpointer          data)
{
    if (g_object_get_data (G_OBJECT (button), "block")) return;
    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))) return;
    int prop = GPOINTER_TO_INT(data); 

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    SPCSSAttr *css = sp_repr_css_attr_new (); 

    switch (prop)
    {
        case 0:
        {
            sp_repr_css_set_property (css, "text-anchor", "start"); 
            sp_repr_css_set_property (css, "text-align", "start"); 
            break;
        }
        case 1:
        {
            sp_repr_css_set_property (css, "text-anchor", "middle"); 
            sp_repr_css_set_property (css, "text-align", "center"); 
            break;
        }

        case 2:
        {
            sp_repr_css_set_property (css, "text-anchor", "end"); 
            sp_repr_css_set_property (css, "text-align", "end"); 
            break;
        }

        case 3:
        {
            sp_repr_css_set_property (css, "text-anchor", "start"); 
            sp_repr_css_set_property (css, "text-align", "justify"); 
            break;
        }
    }

    SPStyle *query =
        sp_style_new ();
    int result_numbers =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS); 

    // If querying returned nothing, read the style from the text tool prefs (default style for new texts)
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        sp_repr_css_change (inkscape_get_repr (INKSCAPE, "tools.text"), css, "style");
    }

    sp_desktop_set_style (desktop, css, true, true);
    sp_document_done (sp_desktop_document (SP_ACTIVE_DESKTOP));
    sp_repr_css_attr_unref (css);

    gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
}

void
sp_text_toolbox_style_toggled (GtkToggleButton  *button,
                               gpointer          data)
{
    if (g_object_get_data (G_OBJECT (button), "block")) return;

    SPDesktop   *desktop    = SP_ACTIVE_DESKTOP;
    SPCSSAttr   *css        = sp_repr_css_attr_new (); 
    int          prop       = GPOINTER_TO_INT(data); 
    bool         active     = gtk_toggle_button_get_active (button);


    switch (prop)
    {
        case 0:
        {
            sp_repr_css_set_property (css, "font-weight", active ? "bold" : "normal" ); 
            break;
        }

        case 1:
        {
            sp_repr_css_set_property (css, "font-style", active ? "italic" : "normal"); 
            break;
        }
    }

    SPStyle *query =
        sp_style_new ();
    int result_numbers =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS); 

    // If querying returned nothing, read the style from the text tool prefs (default style for new texts)
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        sp_repr_css_change (inkscape_get_repr (INKSCAPE, "tools.text"), css, "style");
    }

    sp_desktop_set_style (desktop, css, true, true);
    sp_document_done (sp_desktop_document (SP_ACTIVE_DESKTOP));
    sp_repr_css_attr_unref (css);

    gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
}

void
sp_text_toolbox_orientation_toggled (GtkRadioButton  *button,
                                     gpointer         data)
{
    if (g_object_get_data (G_OBJECT (button), "block")) {
        g_object_set_data (G_OBJECT (button), "block", gpointer(0));
        return;
    }
    
    SPDesktop   *desktop    = SP_ACTIVE_DESKTOP;
    SPCSSAttr   *css        = sp_repr_css_attr_new (); 
    int          prop       = GPOINTER_TO_INT(data); 

    switch (prop)
    {
        case 0:
        {
            sp_repr_css_set_property (css, "writing-mode", "lr");
            break;
        }

        case 1:
        {
            sp_repr_css_set_property (css, "writing-mode", "tb"); 
            break;
        }
    }

    SPStyle *query =
        sp_style_new ();
    int result_numbers =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS); 

    // If querying returned nothing, read the style from the text tool prefs (default style for new texts)
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        sp_repr_css_change (inkscape_get_repr (INKSCAPE, "tools.text"), css, "style");
    }

    sp_desktop_set_style (desktop, css, true, true);
    sp_document_done (sp_desktop_document (SP_ACTIVE_DESKTOP));
    sp_repr_css_attr_unref (css);

    gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
}

gboolean
sp_text_toolbox_size_keypress (GtkWidget *w, GdkEventKey *event, gpointer data)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) return FALSE;

    switch (get_group0_keyval (event)) {
        case GDK_Escape: // defocus
            gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
            return TRUE; // I consumed the event
            break;
        case GDK_Return: // defocus
        case GDK_KP_Enter:
            gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
            return TRUE; // I consumed the event
            break;
    }
    return FALSE;
}

gboolean
sp_text_toolbox_family_keypress (GtkWidget *w, GdkEventKey *event, GObject *tbl)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) return FALSE;

    switch (get_group0_keyval (event)) {
        case GDK_Escape: // defocus
            gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
            sp_text_toolbox_selection_changed (NULL, tbl); // update
            return TRUE; // I consumed the event
            break;
    }
    return FALSE;
}

gboolean
sp_text_toolbox_family_list_keypress (GtkWidget *w, GdkEventKey *event, GObject *tbl)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) return FALSE;

    switch (get_group0_keyval (event)) {
        case GDK_Escape: // defocus
            gtk_widget_hide (w);
            visible = false;
            gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
            return TRUE; // I consumed the event
            break;
    }
    return FALSE;
}


void
sp_text_toolbox_size_changed  (GtkComboBox *cbox,
                               GObject     *tbl) 
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    if (g_object_get_data (tbl, "size-block")) return;

    char *text = gtk_combo_box_get_active_text (cbox);

    SPCSSAttr *css = sp_repr_css_attr_new (); 
    sp_repr_css_set_property (css, "font-size", text); 
    free (text);

    SPStyle *query =
        sp_style_new ();
    int result_numbers =
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS); 

    // If querying returned nothing, read the style from the text tool prefs (default style for new texts)
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        sp_repr_css_change (inkscape_get_repr (INKSCAPE, "tools.text"), css, "style");
    }

    sp_desktop_set_style (desktop, css, true, true);
    sp_document_maybe_done (sp_desktop_document (SP_ACTIVE_DESKTOP), "ttb:size");
    sp_repr_css_attr_unref (css);


    if (gtk_combo_box_get_active (cbox) > 0) // if this was from drop-down (as opposed to type-in), defocus
        gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
}

void
sp_text_toolbox_text_popdown_clicked    (GtkButton          *button,
                                         GObject            *tbl)
{
    GtkWidget *popdown = GTK_WIDGET (g_object_get_data (tbl, "family-popdown-window"));
    GtkWidget *widget = GTK_WIDGET (g_object_get_data (tbl, "family-entry"));
    int x, y;

    if (!visible)
    {
        gdk_window_get_origin (widget->window, &x, &y);
        gtk_window_move (GTK_WINDOW (popdown), x, y + widget->allocation.height + 2); //2px of grace space 
        gtk_widget_show_all (popdown);

        gdk_pointer_grab (widget->window, TRUE,
                          GdkEventMask (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                                        GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
                                        GDK_POINTER_MOTION_MASK),
                          NULL, NULL, GDK_CURRENT_TIME);

        gdk_keyboard_grab (widget->window, TRUE, GDK_CURRENT_TIME);

        visible = true;
    }
    else
    {
        gdk_pointer_ungrab (GDK_CURRENT_TIME);
        gdk_keyboard_ungrab (GDK_CURRENT_TIME);
        gtk_widget_hide (popdown);
        visible = false;
    }
}

gboolean
sp_text_toolbox_entry_focus_in  (GtkWidget        *entry,
                                 GdkEventFocus    *event,
                                 GObject          *tbl)
{
    gtk_entry_select_region (GTK_ENTRY (entry), 0, -1);
    return FALSE;
}

gboolean
sp_text_toolbox_popdown_focus_out (GtkWidget        *popdown,
                                   GdkEventFocus    *event,
                                   GObject          *tbl)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    gtk_widget_hide (popdown);
    visible = false;
    gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas));
    return TRUE;
}

void
cell_data_func  (GtkTreeViewColumn *column,
                 GtkCellRenderer   *cell,
                 GtkTreeModel      *tree_model,
                 GtkTreeIter       *iter,
                 gpointer           data)
{
    char        *family,
        *family_escaped,
        *sample_escaped;

    static const char *sample = _("AaBbCcIiPpQq12369$\342\202\254\302\242?.;/()");

    gtk_tree_model_get (tree_model, iter, 0, &family, -1);

    family_escaped = g_markup_escape_text (family, -1);
    sample_escaped = g_markup_escape_text (sample, -1);
    
    std::stringstream markup; 
    markup << family_escaped << "  <span foreground='darkgray' font_family='" << family_escaped << "'>" << sample_escaped << "</span>";
    g_object_set (G_OBJECT (cell), "markup", markup.str().c_str(), NULL);

    free (family);
    free (family_escaped);
    free (sample_escaped);
}

GtkWidget*
sp_text_toolbox_new (SPDesktop *desktop)
{
    GtkWidget   *tbl = gtk_hbox_new (FALSE, 0);

    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    GtkTooltips *tt = gtk_tooltips_new();
    Glib::RefPtr<Gtk::ListStore> store = Inkscape::FontLister::get_instance()->get_font_list();

    ////////////Family
    //Window
    GtkWidget   *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated (GTK_WINDOW (window), FALSE);

    //Entry
    GtkWidget           *entry = gtk_entry_new ();
    gtk_object_set_data(GTK_OBJECT(entry), "altx-text", entry);
    GtkEntryCompletion  *completion = gtk_entry_completion_new ();
    gtk_entry_completion_set_model (completion, GTK_TREE_MODEL (Glib::unwrap(store)));
    gtk_entry_completion_set_text_column (completion, 0);
    gtk_entry_completion_set_minimum_key_length (completion, 1); 
    g_object_set (G_OBJECT(completion), "inline-completion", TRUE, "popup-completion", TRUE, NULL);
    gtk_entry_set_completion (GTK_ENTRY(entry), completion);
    aux_toolbox_space (tbl, 1);
    gtk_box_pack_start (GTK_BOX (tbl), entry, FALSE, FALSE, 0);
        
    //Button
    GtkWidget   *button = gtk_button_new ();
    gtk_container_add       (GTK_CONTAINER (button), gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE));
    gtk_box_pack_start      (GTK_BOX (tbl), button, FALSE, FALSE, 0);

    //Popdown
    GtkWidget           *sw = gtk_scrolled_window_new (NULL, NULL);
    GtkWidget           *treeview = gtk_tree_view_new ();

    GtkCellRenderer     *cell = gtk_cell_renderer_text_new ();
    GtkTreeViewColumn   *column = gtk_tree_view_column_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_cell_data_func (column, cell, GtkTreeCellDataFunc (cell_data_func), NULL, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (Glib::unwrap(store)));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
    gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (treeview), TRUE);

    //gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), TRUE);

    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS); 
    gtk_container_add (GTK_CONTAINER (sw), treeview);

    gtk_container_add (GTK_CONTAINER (window), sw);
    gtk_widget_set_size_request (window, 300, 450);

    g_signal_connect (G_OBJECT (entry),  "activate", G_CALLBACK (sp_text_toolbox_family_entry_activate), tbl);
    g_signal_connect (G_OBJECT (entry),  "focus-in-event", G_CALLBACK (sp_text_toolbox_entry_focus_in), tbl);
    g_signal_connect (G_OBJECT (entry), "key-press-event", G_CALLBACK(sp_text_toolbox_family_keypress), tbl);

    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (sp_text_toolbox_text_popdown_clicked), tbl);

    g_signal_connect (G_OBJECT (window), "focus-out-event", G_CALLBACK (sp_text_toolbox_popdown_focus_out), tbl);
    g_signal_connect (G_OBJECT (window), "key-press-event", G_CALLBACK(sp_text_toolbox_family_list_keypress), tbl);

    GtkTreeSelection *tselection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    g_signal_connect (G_OBJECT (tselection), "changed", G_CALLBACK (sp_text_toolbox_family_changed), tbl);

    g_object_set_data (G_OBJECT (tbl), "family-entry", entry);
    g_object_set_data (G_OBJECT (tbl), "family-popdown-button", button);
    g_object_set_data (G_OBJECT (tbl), "family-popdown-window", window);
    g_object_set_data (G_OBJECT (tbl), "family-tree-selection", tselection);
    g_object_set_data (G_OBJECT (tbl), "family-tree-view", treeview);

    GtkWidget *image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_SMALL_TOOLBAR);
    aux_toolbox_space (tbl, 1);
    GtkWidget *box = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (box), image);
    gtk_box_pack_start (GTK_BOX (tbl), box, FALSE, FALSE, 4);
    g_object_set_data (G_OBJECT (tbl), "warning-image", box);
    GtkTooltips *tooltips = gtk_tooltips_new ();
    gtk_tooltips_set_tip (tooltips, box, _("This font is currently not installed on your system. Inkscape will use the default font instead."), "");
    gtk_widget_hide (GTK_WIDGET (box));
    g_signal_connect_swapped (G_OBJECT (tbl), "show", G_CALLBACK (gtk_widget_hide), box);

    ////////////Size
    const char *sizes[] = {
        "4", "6", "8", "9", "10", "11", "12", "13", "14",
        "16", "18", "20", "22", "24", "28",
        "32", "36", "40", "48", "56", "64", "72", "144"
    };

    GtkWidget *cbox = gtk_combo_box_entry_new_text ();
    for (unsigned int n = 0; n < G_N_ELEMENTS (sizes); gtk_combo_box_append_text (GTK_COMBO_BOX(cbox), sizes[n++]));
    gtk_widget_set_size_request (cbox, 80, -1);
    aux_toolbox_space (tbl, 1);
    gtk_box_pack_start (GTK_BOX (tbl), cbox, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT (tbl), "combo-box-size", cbox);
    g_signal_connect (G_OBJECT (cbox), "changed", G_CALLBACK (sp_text_toolbox_size_changed), tbl);
    gtk_signal_connect(GTK_OBJECT(cbox), "key-press-event", GTK_SIGNAL_FUNC(sp_text_toolbox_size_keypress), NULL);

    //spacer
    aux_toolbox_space (tbl, 4);
    gtk_box_pack_start (GTK_BOX (tbl), gtk_vseparator_new (), FALSE, FALSE, 4);

    ////////////Text anchor
    GtkWidget *group   = gtk_radio_button_new (NULL);
    GtkWidget *row     = gtk_hbox_new (FALSE, 4);
    g_object_set_data (G_OBJECT (tbl), "anchor-group", group);

    // left
    GtkWidget *rbutton = group;
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_LEFT, GTK_ICON_SIZE_SMALL_TOOLBAR));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "text-start", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_anchoring_toggled), gpointer(0));
    gtk_tooltips_set_tip(tt, rbutton, _("Align left"), NULL);

    // center
    rbutton = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (group)));
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_CENTER, GTK_ICON_SIZE_SMALL_TOOLBAR));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "text-middle", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_anchoring_toggled), gpointer (1)); 
    gtk_tooltips_set_tip(tt, rbutton, _("Center"), NULL);

    // right
    rbutton = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (group)));
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_RIGHT, GTK_ICON_SIZE_SMALL_TOOLBAR));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "text-end", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_anchoring_toggled), gpointer(2));
    gtk_tooltips_set_tip(tt, rbutton, _("Align right"), NULL);

    // fill
    rbutton = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (group)));
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_FILL, GTK_ICON_SIZE_SMALL_TOOLBAR));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "text-fill", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_anchoring_toggled), gpointer(3));
    gtk_tooltips_set_tip(tt, rbutton, _("Justify"), NULL);

    aux_toolbox_space (tbl, 1);
    gtk_box_pack_start (GTK_BOX (tbl), row, FALSE, FALSE, 4);

    //spacer
    gtk_box_pack_start (GTK_BOX (tbl), gtk_vseparator_new (), FALSE, FALSE, 4);

    ////////////Text style
    row = gtk_hbox_new (FALSE, 4);

    // bold 
    rbutton = gtk_toggle_button_new (); 
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), gtk_image_new_from_stock (GTK_STOCK_BOLD, GTK_ICON_SIZE_SMALL_TOOLBAR));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);
    gtk_tooltips_set_tip(tt, rbutton, _("Bold"), NULL);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "style-bold", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_style_toggled), gpointer(0));

    // italic
    rbutton = gtk_toggle_button_new (); 
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), gtk_image_new_from_stock (GTK_STOCK_ITALIC, GTK_ICON_SIZE_SMALL_TOOLBAR));
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);
    gtk_tooltips_set_tip(tt, rbutton, _("Italic"), NULL);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "style-italic", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_style_toggled), gpointer (1)); 

    aux_toolbox_space (tbl, 1);
    gtk_box_pack_start (GTK_BOX (tbl), row, FALSE, FALSE, 4);

    //spacer
    gtk_box_pack_start (GTK_BOX (tbl), gtk_vseparator_new (), FALSE, FALSE, 4);

    ////////////Text orientation
    group   = gtk_radio_button_new (NULL);
    row     = gtk_hbox_new (FALSE, 4);
    g_object_set_data (G_OBJECT (tbl), "orientation-group", group);

    // horizontal
    rbutton = group;
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_STOCK_WRITING_MODE_LR)); 
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);
    gtk_tooltips_set_tip(tt, rbutton, _("Horizontal text"), NULL);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "orientation-horizontal", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_orientation_toggled), gpointer(0));

    // vertical
    rbutton = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (group)));
    gtk_button_set_relief       (GTK_BUTTON (rbutton), GTK_RELIEF_NONE);
    gtk_container_add           (GTK_CONTAINER (rbutton), sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_STOCK_WRITING_MODE_TB)); 
    gtk_toggle_button_set_mode  (GTK_TOGGLE_BUTTON (rbutton), FALSE);
    gtk_tooltips_set_tip(tt, rbutton, _("Vertical text"), NULL);

    gtk_box_pack_start  (GTK_BOX  (row), rbutton, FALSE, FALSE, 0);
    g_object_set_data   (G_OBJECT (tbl), "orientation-vertical", rbutton);
    g_signal_connect    (G_OBJECT (rbutton), "toggled", G_CALLBACK (sp_text_toolbox_orientation_toggled), gpointer (1)); 
    gtk_box_pack_start (GTK_BOX (tbl), row, FALSE, FALSE, 4);


    //watch selection
    Inkscape::ConnectionPool* pool = Inkscape::ConnectionPool::new_connection_pool ("ISTextToolbox");

    sigc::connection *c_selection_changed =
        new sigc::connection (sp_desktop_selection (desktop)->connectChanged 
                              (sigc::bind (sigc::ptr_fun (sp_text_toolbox_selection_changed), (GObject*)tbl)));
    pool->add_connection ("selection-changed", c_selection_changed);

    sigc::connection *c_selection_modified =
        new sigc::connection (sp_desktop_selection (desktop)->connectModified 
                              (sigc::bind (sigc::ptr_fun (sp_text_toolbox_selection_modified), (GObject*)tbl)));
    pool->add_connection ("selection-modified", c_selection_modified);

    sigc::connection *c_subselection_changed =
        new sigc::connection (desktop->connectToolSubselectionChanged
                              (sigc::bind (sigc::ptr_fun (sp_text_toolbox_subselection_changed), (GObject*)tbl)));
    pool->add_connection ("tool-subselection-changed", c_subselection_changed);

    Inkscape::ConnectionPool::connect_destroy (G_OBJECT (tbl), pool);


#if 0
    // horizontal
    {
        GtkWidget *px= sp_icon_new(Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_STOCK_WRITING_MODE_LR);
        GtkWidget *b = group = gtk_radio_button_new (NULL);
        gtk_container_add (GTK_CONTAINER (b), px);
        gtk_tooltips_set_tip (tt, b, _("Horizontal text"), NULL);
        gtk_button_set_relief (GTK_BUTTON (b), GTK_RELIEF_NONE);
        gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE );
        gtk_box_pack_start (GTK_BOX (tbl), b, FALSE, FALSE, 0);
    }

    // vertical
    {
        GtkWidget *px = sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_STOCK_WRITING_MODE_TB);
        GtkWidget *b = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (group)));
        gtk_container_add (GTK_CONTAINER (b), px);
        gtk_tooltips_set_tip (tt, b, _("Vertical text"), NULL);
        gtk_button_set_relief (GTK_BUTTON (b), GTK_RELIEF_NONE);
        gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE );
        gtk_box_pack_start (GTK_BOX (tbl), b, FALSE, FALSE, 0);
    }

    aux_toolbox_space(tbl, AUX_BETWEEN_BUTTON_GROUPS);

    // letter spacing
    {
        {
            GtkWidget *image = sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_STOCK_TEXT_LETTER_SPACING);
            GtkWidget *hb = gtk_hbox_new(FALSE, 1);
            gtk_container_add (GTK_CONTAINER (hb), image);
            gtk_widget_show(image);
            gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, 0);
        }
    
        {
            GtkWidget *hb = sp_tb_spinbutton(_(""), _("Spacing between letters"),
                                             "tools.text", "letter_spacing", 0.0,
                                             us, tbl, FALSE, NULL,
                                             -1000.0, 1000.0, 0.1, 0.1,
                                             sp_text_letter_changed, 0.1, 1);
            gtk_widget_set_size_request (hb, 45, 6);
            gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, 6);
        }
    }

    // line spacing
    {
        {
            GtkWidget *image = sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_STOCK_TEXT_LINE_SPACING);
            GtkWidget *hb = gtk_hbox_new(FALSE, 1);
            gtk_container_add (GTK_CONTAINER (hb), image);
            gtk_widget_show(image);
            gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, 0);
        }
    
        {
            GtkWidget *hb = sp_tb_spinbutton(_(""), _("Spacing between lines"),
                                             "tools.text", "line_spacing", 0,
                                             us, tbl, FALSE, NULL,
                                             -1000.0, 1000.0, 0.1, 0.1,
                                             sp_text_line_changed, 0.1, 1);
            gtk_widget_set_size_request (hb, 45, 0);
            gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, 3);
        }
    }

    {
        // horizontal kerning/vertical kerning units menu: create
        GtkWidget *us = sp_unit_selector_new(SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
        sp_unit_selector_setsize(us, AUX_OPTION_MENU_WIDTH, AUX_OPTION_MENU_HEIGHT);
        sp_unit_selector_set_unit(SP_UNIT_SELECTOR(us), desktop->namedview->doc_units);

        aux_toolbox_space(tbl, AUX_BETWEEN_BUTTON_GROUPS);

        // horizontal kerning
        {
            {
                GtkWidget *image = sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_STOCK_TEXT_HORZ_KERN);
                GtkWidget *hb = gtk_hbox_new(FALSE, 1);
                gtk_container_add (GTK_CONTAINER (hb), image);
                gtk_widget_show(image);
                gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, 0);
            }

            {
                GtkWidget *hb = sp_tb_spinbutton(_(""), _("Horizontal kerning"),
                                                 "tools.text", "horizontal_kerning", 0,
                                                 us, tbl, FALSE, NULL,
                                                 -100.00, 100.00, 0.01, 0.1,
                                                 sp_text_horiz_kern_changed);
                gtk_widget_set_size_request (hb, 45, 0);
                gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, 6);
            }
        }

        // vertical kerning
        {
            {
                GtkWidget *image = sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_STOCK_TEXT_VERT_KERN);
                GtkWidget *hb = gtk_hbox_new(FALSE, 1);
                gtk_container_add (GTK_CONTAINER (hb), image);
                gtk_widget_show(image);
                gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, 0);
            }
    
            {
                GtkWidget *hb = sp_tb_spinbutton(_(""), _("Vertical kerning"),
                                                 "tools.text", "vertical_kerning", 0,
                                                 us, tbl, FALSE, NULL,
                                                 -100.00, 100.00, 0.01, 0.1,
                                                 sp_text_vert_kern_changed);
                gtk_widget_set_size_request (hb, 45, 0);
                gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, 5);
            }
        }

        // add the units menu
        gtk_widget_show(us);
        gtk_box_pack_start(GTK_BOX(tbl), us, FALSE, FALSE, 1);
        gtk_object_set_data(GTK_OBJECT(tbl), "units", us);
    }

    // letter rotation
    aux_toolbox_space(tbl, AUX_BETWEEN_BUTTON_GROUPS);
    {
        {
            GtkWidget *image = sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_STOCK_TEXT_ROTATION);
            GtkWidget *hb = gtk_hbox_new(FALSE, 1);
            gtk_container_add (GTK_CONTAINER (hb), image);
            gtk_widget_show(image);
            gtk_box_pack_start (GTK_BOX (tbl), hb, FALSE, FALSE, 0);
        }
        {
            GtkWidget *hb = sp_tb_spinbutton(_(""), _("Letter rotation"),
                                             "tools.text", "letter_rotation", 0,
                                             us, tbl, FALSE, NULL,
                                             -180.0, 180.0, 0.1, 0.1,
                                             sp_text_letter_rotation_changed, 0.1, 1);
            gtk_widget_set_size_request (hb, 45, 0);
            gtk_box_pack_start(GTK_BOX(tbl), hb, FALSE, FALSE, 6);
        }
        // rotation degree label
        {
            GtkWidget *label = gtk_widget_new (GTK_TYPE_LABEL, "label", "\302\260", "xalign", 0.0, NULL);
            gtk_box_pack_start(GTK_BOX(tbl), label, FALSE, FALSE, 0);
        }
    }
	
    // Remove Manual Kerns
    {
        GtkWidget *px = sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_STOCK_TEXT_REMOVE_KERNS);
        GtkWidget *button = gtk_button_new ();
        gtk_container_add (GTK_CONTAINER (button), px);
        gtk_widget_show(button);
        gtk_tooltips_set_tip (tt, button, _("Remove manual kerns"), NULL);
        gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
        gtk_widget_set_sensitive(button, TRUE);
        gtk_box_pack_start (GTK_BOX (tbl), button, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    }
#endif

    gtk_widget_show_all (tbl);
    return tbl;

} // end of sp_text_toolbox_new()

}//<unnamed> namespace


//#########################
//##  Connector Toolbox  ##
//#########################

static void sp_connector_path_set_avoid(void)
{
    cc_selection_set_avoid(true);
}


static void sp_connector_path_set_ignore(void)
{
    cc_selection_set_avoid(false);
}


static void connector_spacing_changed(GtkAdjustment *adj, GtkWidget *tbl)
{
    // quit if run by the _changed callbacks
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }
        
    SPDesktop *desktop = (SPDesktop *) gtk_object_get_data(GTK_OBJECT(tbl),
            "desktop");
    SPDocument *doc = sp_desktop_document(desktop);

    if (!sp_document_get_undo_sensitive(doc))
    {
        return;
    }

    // in turn, prevent callbacks from responding
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));
    
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(desktop->namedview);
    
    sp_repr_set_css_double(repr, "inkscape:connector-spacing", adj->value);
    SP_OBJECT(desktop->namedview)->updateRepr();
    
    GSList *items = get_avoided_items(NULL, desktop->currentRoot(), desktop);
    for ( GSList const *iter = items ; iter != NULL ; iter = iter->next ) {
        SPItem *item = reinterpret_cast<SPItem *>(iter->data);
        NR::Matrix m = NR::identity();
        avoid_item_move(&m, item);
    }

    if (items) {
        g_slist_free(items);
    }
    
    sp_document_done(doc);

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));
    
    spinbutton_defocus(GTK_OBJECT(tbl));
}


static void connector_tb_event_attr_changed(Inkscape::XML::Node *repr,
        gchar const *name, gchar const *old_value, gchar const *new_value,
        bool is_interactive, gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }
    if (strcmp(name, "inkscape:connector-spacing") != 0) {
        return;
    }

    GtkAdjustment *adj = (GtkAdjustment*)
            gtk_object_get_data(GTK_OBJECT(tbl), "spacing");
    gdouble spacing = defaultConnSpacing;
    sp_repr_get_double(repr, "inkscape:connector-spacing", &spacing);
    
    gtk_adjustment_set_value(adj, spacing);
}


static Inkscape::XML::NodeEventVector connector_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    connector_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


static GtkWidget *
sp_connector_toolbox_new(SPDesktop *desktop)
{
    GtkTooltips *tt = gtk_tooltips_new();
    GtkWidget *tbl = gtk_hbox_new(FALSE, 0);
    
    gtk_object_set_data(GTK_OBJECT(tbl), "dtw", desktop->canvas);
    gtk_object_set_data(GTK_OBJECT(tbl), "desktop", desktop);

    gtk_box_pack_start(GTK_BOX(tbl), gtk_hbox_new(FALSE, 0), FALSE, FALSE,
            AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_new(tbl, Inkscape::ICON_SIZE_SMALL_TOOLBAR,
            "connector_avoid", GTK_SIGNAL_FUNC(sp_connector_path_set_avoid),
            tt, _("Make connectors avoid selected objects"));

    sp_toolbox_button_new(tbl, Inkscape::ICON_SIZE_SMALL_TOOLBAR,
            "connector_ignore", GTK_SIGNAL_FUNC(sp_connector_path_set_ignore),
            tt, _("Make connectors ignore selected objects"));

    //  interval
    gtk_box_pack_start(GTK_BOX(tbl), gtk_hbox_new(FALSE, 0), FALSE, FALSE,
            AUX_BETWEEN_BUTTON_GROUPS);

    // Spacing spinbox
    {
        GtkWidget *object_spacing = sp_tb_spinbutton(_("Spacing:"),
                _("The amount of space left around objects by auto-routing connectors"),
                "tools.connector", "spacing", 10, NULL, tbl, TRUE,
                "inkscape:connector-spacing", 0, 100, 1.0, 10.0,
                connector_spacing_changed, 1, 0);

        gtk_box_pack_start(GTK_BOX(tbl), object_spacing, FALSE, FALSE,
                AUX_SPACING);
    }

    gtk_widget_show_all(tbl);
    sp_set_font_size_smaller (tbl);
    
    // Code to watch for changes to the connector-spacing attribute in
    // the XML.
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(desktop->namedview);
    g_assert(repr != NULL);
        
    Inkscape::XML::Node *oldrepr = (Inkscape::XML::Node *)
            gtk_object_get_data(GTK_OBJECT(tbl), "repr");
    
    if (oldrepr) { // remove old listener
        sp_repr_remove_listener_by_data(oldrepr, tbl);
        Inkscape::GC::release(oldrepr);
        oldrepr = NULL;
        g_object_set_data(G_OBJECT(tbl), "repr", NULL);
    }

    if (repr) {
        g_object_set_data(G_OBJECT(tbl), "repr", repr);
        Inkscape::GC::anchor(repr);
        sp_repr_add_listener(repr, &connector_tb_repr_events, tbl);
        sp_repr_synthesize_events(repr, &connector_tb_repr_events, tbl);
    }
    
    return tbl;

} // end of sp_connector_toolbox_new()


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

