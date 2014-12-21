/*
 * Gradient aux toolbar
 *
 * Authors:
 *   bulia byak <bulia@dr.com>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Abhishek Sharma
 *   Tavmjong Bah <tavjong@free.fr>
 *
 * Copyright (C) 2012 Tavmjong Bah
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2005 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

// REVIEW THESE AT END OF REWRITE
#include "ui/widget/color-preview.h"
#include "toolbox.h"
#include "mesh-toolbar.h"

#include "verbs.h"

#include "macros.h"
#include "widgets/button.h"
#include "widgets/widget-sizes.h"
#include "widgets/spw-utilities.h"
#include "widgets/spinbutton-events.h"
#include "widgets/gradient-vector.h"
#include "widgets/gradient-image.h"
#include "style.h"

#include "preferences.h"
#include "document-private.h"
#include "document-undo.h"
#include "desktop.h"

#include <glibmm/i18n.h>

#include "ui/tools/gradient-tool.h"
#include "ui/tools/mesh-tool.h"
#include "gradient-drag.h"
#include "sp-mesh-gradient.h"
#include "gradient-chemistry.h"
#include "gradient-selector.h"
#include "selection.h"
#include "ui/icon-names.h"

#include "widgets/ege-adjustment-action.h"
#include "widgets/ege-output-action.h"
#include "widgets/ege-select-one-action.h"
#include "widgets/ink-action.h"
#include "widgets/ink-comboboxentry-action.h"

#include "sp-stop.h"
#include "svg/css-ostringstream.h"
#include "svg/svg-color.h"
#include "desktop-style.h"

#include "toolbox.h"

using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;

static bool blocked = false;

//########################
//##        Mesh        ##
//########################

/*
 * Core function, setup all the widgets whenever something changes on the desktop
 */
static void ms_tb_selection_changed(Inkscape::Selection * /*selection*/, gpointer /*data*/)
{
    // DOES NOTHING AT MOMENT

    // std::cout << "ms_tb_selection_changed" << std::endl;

    // if (blocked)
    //     return;

    // GtkWidget *widget = GTK_WIDGET(data);

    // SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data(G_OBJECT(widget), "desktop"));
    // if (!desktop) {
    //     return;
    // }

    // Inkscape::Selection *selection = desktop->getSelection(); // take from desktop, not from args
    // if (selection) {
    //     ToolBase *ev = sp_desktop_event_context(desktop);
    //     GrDrag *drag = NULL;
    //     if (ev) {
    //         drag = ev->get_drag();
    //         // Hide/show handles?
    //     }

    // }
}


static void ms_tb_selection_modified(Inkscape::Selection *selection, guint /*flags*/, gpointer data)
{
    ms_tb_selection_changed(selection, data);
}

static void ms_drag_selection_changed(gpointer /*dragger*/, gpointer data)
{
    ms_tb_selection_changed(NULL, data);

}

static void ms_defs_release(SPObject * /*defs*/, GObject *widget)
{
    ms_tb_selection_changed(NULL, widget);
}

static void ms_defs_modified(SPObject * /*defs*/, guint /*flags*/, GObject *widget)
{
    ms_tb_selection_changed(NULL, widget);
}

/*
 * Callback functions for user actions
 */

static void ms_new_type_changed( EgeSelectOneAction *act, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gint typemode = ege_select_one_action_get_active( act ) == 0 ? SP_GRADIENT_MESH_TYPE_NORMAL : SP_GRADIENT_MESH_TYPE_CONICAL;
    prefs->setInt("/tools/mesh/mesh_type", typemode);
}

static void ms_new_fillstroke_changed( EgeSelectOneAction *act, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Inkscape::PaintTarget fsmode = (ege_select_one_action_get_active( act ) == 0) ? Inkscape::FOR_FILL : Inkscape::FOR_STROKE;
    prefs->setInt("/tools/gradient/newfillorstroke", (fsmode == Inkscape::FOR_FILL) ? 1 : 0);
}

static void ms_row_changed(GtkAdjustment *adj, GObject * /*tbl*/ )
{
    if (blocked) {
        return;
    }

    blocked = TRUE;

    int rows = gtk_adjustment_get_value(adj);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    prefs->setInt("/tools/mesh/mesh_rows", rows);

    blocked = FALSE;
}

static void ms_col_changed(GtkAdjustment *adj, GObject * /*tbl*/ )
{
    if (blocked) {
        return;
    }

    blocked = TRUE;

    int cols = gtk_adjustment_get_value(adj);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    prefs->setInt("/tools/mesh/mesh_cols", cols);

    blocked = FALSE;
}

static void mesh_toolbox_watch_ec(SPDesktop* dt, Inkscape::UI::Tools::ToolBase* ec, GObject* holder);

/**
 * Mesh auxiliary toolbar construction and setup.
 *
 */
void sp_mesh_toolbox_prep(SPDesktop * desktop, GtkActionGroup* mainActions, GObject* holder)
{
    Inkscape::IconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);

    EgeAdjustmentAction* eact = 0;

    /* New mesh: normal or conical */
    {
        GtkListStore* model = gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

        GtkTreeIter iter;
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                0, _("normal"), 1, _("Create mesh gradient"), 2, INKSCAPE_ICON("paint-gradient-mesh"), -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                0, _("conical"), 1, _("Create conical gradient"), 2, INKSCAPE_ICON("paint-gradient-conical"), -1 );

        EgeSelectOneAction* act = ege_select_one_action_new( "MeshNewTypeAction", (""), (""), NULL, GTK_TREE_MODEL(model) );
        g_object_set( act, "short_label", _("New:"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        g_object_set_data( holder, "mesh_new_type_action", act );

        ege_select_one_action_set_appearance( act, "full" );
        ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
        g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
        ege_select_one_action_set_icon_column( act, 2 );
        ege_select_one_action_set_tooltip_column( act, 1  );

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        gint mode = prefs->getInt("/tools/mesh/mesh_type", SP_GRADIENT_MESH_TYPE_NORMAL) != SP_GRADIENT_MESH_TYPE_NORMAL;
        ege_select_one_action_set_active( act, mode );
        g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(ms_new_type_changed), holder );
    }

    /* New gradient on fill or stroke*/
    {
        GtkListStore* model = gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

        GtkTreeIter iter;
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("fill"), 1, _("Create gradient in the fill"), 2, INKSCAPE_ICON("object-fill"), -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                            0, _("stroke"), 1, _("Create gradient in the stroke"), 2, INKSCAPE_ICON("object-stroke"), -1 );

        EgeSelectOneAction* act = ege_select_one_action_new( "MeshNewFillStrokeAction", (""), (""), NULL, GTK_TREE_MODEL(model) );
        g_object_set( act, "short_label", _("on:"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        g_object_set_data( holder, "mesh_new_fillstroke_action", act );

        ege_select_one_action_set_appearance( act, "full" );
        ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
        g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
        ege_select_one_action_set_icon_column( act, 2 );
        ege_select_one_action_set_tooltip_column( act, 1  );

        /// @todo Convert to boolean?
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        bool fillstrokemode = !prefs->getBool("/tools/gradient/newfillorstroke", true);
        ege_select_one_action_set_active( act, fillstrokemode );
        g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(ms_new_fillstroke_changed), holder );
    }

    /* Number of mesh rows */
    {
        gchar const* labels[] = {};
        gdouble values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        eact = create_adjustment_action( "MeshRowAction",
                                         _("Rows"), _("Rows:"), _("Number of rows in new mesh"),
                                         "/tools/mesh/mesh_rows", 1,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         1, 20, 1, 1,
                                         labels, values, G_N_ELEMENTS(labels),
                                         ms_row_changed, NULL /*unit tracker*/,
                                         1.0, 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    }

    /* Number of mesh columns */
    {
        gchar const* labels[] = {};
        gdouble values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        eact = create_adjustment_action( "MeshColumnAction",
                                         _("Columns"), _("Columns:"), _("Number of columns in new mesh"),
                                         "/tools/mesh/mesh_cols", 1,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         1, 20, 1, 1,
                                         labels, values, G_N_ELEMENTS(labels),
                                         ms_col_changed, NULL /*unit tracker*/,
                                         1.0, 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    }

    /* Edit fill mesh */
    {
        InkToggleAction* act = ink_toggle_action_new( "MeshEditFillAction",
                                                      _("Edit Fill"),
                                                      _("Edit fill mesh"),
                                                      INKSCAPE_ICON("object-fill"),
                                                      secondarySize );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/mesh/edit_fill");
        g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
    }

    /* Edit stroke mesh */
    {
        InkToggleAction* act = ink_toggle_action_new( "MeshEditStrokeAction",
                                                      _("Edit Stroke"),
                                                      _("Edit stroke mesh"),
                                                      INKSCAPE_ICON("object-stroke"),
                                                      secondarySize );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/mesh/edit_stroke");
        g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
    }

    /* Show/hide side and tensor handles */
    {
        InkToggleAction* act = ink_toggle_action_new( "MeshShowHandlesAction",
                                                      _("Show Handles"),
                                                      _("Show side and tensor handles"),
                                                      INKSCAPE_ICON("show-node-handles"),
                                                      secondarySize );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/mesh/show_handles");
        g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
    }

    g_object_set_data(holder, "desktop", desktop);

    desktop->connectEventContextChanged(sigc::bind(sigc::ptr_fun(mesh_toolbox_watch_ec), holder));
}

static void mesh_toolbox_watch_ec(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* ec, GObject* holder)
{
    static sigc::connection c_selection_changed;
    static sigc::connection c_selection_modified;
    static sigc::connection c_subselection_changed;
    static sigc::connection c_defs_release;
    static sigc::connection c_defs_modified;

    if (SP_IS_MESH_CONTEXT(ec)) {
        // connect to selection modified and changed signals
        Inkscape::Selection *selection = desktop->getSelection();
        SPDocument *document = desktop->getDocument();

        c_selection_changed = selection->connectChanged(sigc::bind(sigc::ptr_fun(&ms_tb_selection_changed), holder));
        c_selection_modified = selection->connectModified(sigc::bind(sigc::ptr_fun(&ms_tb_selection_modified), holder));
        c_subselection_changed = desktop->connectToolSubselectionChanged(sigc::bind(sigc::ptr_fun(&ms_drag_selection_changed), holder));

        c_defs_release = document->getDefs()->connectRelease(sigc::bind<1>(sigc::ptr_fun(&ms_defs_release), holder));
        c_defs_modified = document->getDefs()->connectModified(sigc::bind<2>(sigc::ptr_fun(&ms_defs_modified), holder));
        ms_tb_selection_changed(selection, holder);
    } else {
        if (c_selection_changed)
            c_selection_changed.disconnect();
        if (c_selection_modified)
            c_selection_modified.disconnect();
        if (c_subselection_changed)
            c_subselection_changed.disconnect();
        if (c_defs_release)
            c_defs_release.disconnect();
        if (c_defs_modified)
            c_defs_modified.disconnect();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
