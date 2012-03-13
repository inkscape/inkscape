/*
 * Gradient aux toolbar
 *
 * Authors:
 *   bulia byak <bulia@dr.com>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2005 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "verbs.h"
#include <gtk/gtk.h>

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
#include "desktop-handles.h"
#include <glibmm/i18n.h>

#include "gradient-context.h"
#include "gradient-drag.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "gradient-chemistry.h"
#include "selection.h"
#include "ui/icon-names.h"

#include "toolbox.h"

using Inkscape::DocumentUndo;

//########################
//##       Gradient     ##
//########################

static void gr_toggle_type (GtkWidget *button, gpointer data) {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    GtkWidget *linear = (GtkWidget *) g_object_get_data (G_OBJECT(data), "linear");
    GtkWidget *radial = (GtkWidget *) g_object_get_data (G_OBJECT(data), "radial");
    if (button == linear && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (linear))) {
        prefs->setInt("/tools/gradient/newgradient", SP_GRADIENT_TYPE_LINEAR);
        if (radial) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radial), FALSE);
    } else if (button == radial && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radial))) {
        prefs->setInt("/tools/gradient/newgradient", SP_GRADIENT_TYPE_RADIAL);
        if (linear) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (linear), FALSE);
    }
}

static void gr_toggle_fillstroke (GtkWidget *button, gpointer data) {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    GtkWidget *fill = (GtkWidget *) g_object_get_data (G_OBJECT(data), "fill");
    GtkWidget *stroke = (GtkWidget *) g_object_get_data (G_OBJECT(data), "stroke");
    if (button == fill && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fill))) {
        prefs->setBool("/tools/gradient/newfillorstroke", true);
        if (stroke) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (stroke), FALSE);
    } else if (button == stroke && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (stroke))) {
        prefs->setBool("/tools/gradient/newfillorstroke", false);
        if (fill) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fill), FALSE);
    }
}

void gr_apply_gradient_to_item( SPItem *item, SPGradient *gr, SPGradientType new_type, guint new_fill, bool do_fill, bool do_stroke )
{
    SPStyle *style = item->style;

    if (do_fill) {
        if (style && (style->fill.isPaintserver()) &&
            SP_IS_GRADIENT( item->style->getFillPaintServer() )) {
            SPPaintServer *server = item->style->getFillPaintServer();
            if ( SP_IS_LINEARGRADIENT(server) ) {
                sp_item_set_gradient(item, gr, SP_GRADIENT_TYPE_LINEAR, true);
            } else if ( SP_IS_RADIALGRADIENT(server) ) {
                sp_item_set_gradient(item, gr, SP_GRADIENT_TYPE_RADIAL, true);
            }
        } else if (new_fill) {
            sp_item_set_gradient(item, gr, new_type, true);
        }
    }

    if (do_stroke) {
        if (style && (style->stroke.isPaintserver()) &&
            SP_IS_GRADIENT( item->style->getStrokePaintServer() )) {
            SPPaintServer *server = item->style->getStrokePaintServer();
            if ( SP_IS_LINEARGRADIENT(server) ) {
                sp_item_set_gradient(item, gr, SP_GRADIENT_TYPE_LINEAR, false);
            } else if ( SP_IS_RADIALGRADIENT(server) ) {
                sp_item_set_gradient(item, gr, SP_GRADIENT_TYPE_RADIAL, false);
            }
        } else if (!new_fill) {
            sp_item_set_gradient(item, gr, new_type, false);
        }
    }
}

/**
Applies gradient vector gr to the gradients attached to the selected dragger of drag, or if none,
to all objects in selection. If there was no previous gradient on an item, uses gradient type and
fill/stroke setting from preferences to create new default (linear: left/right; radial: centered)
gradient.
*/
void gr_apply_gradient (Inkscape::Selection *selection, GrDrag *drag, SPGradient *gr)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    SPGradientType new_type = static_cast<SPGradientType>(prefs->getInt("/tools/gradient/newgradient", SP_GRADIENT_TYPE_LINEAR));
    guint new_fill = prefs->getBool("/tools/gradient/newfillorstroke", true);


    // GRADIENTFIXME: make this work for multiple selected draggers.

    // First try selected dragger
    if (drag && drag->selected) {
        GrDragger *dragger = static_cast<GrDragger*>(drag->selected->data);
        for (GSList const* i = dragger->draggables; i != NULL; i = i->next) { // for all draggables of dragger
            GrDraggable *draggable = (GrDraggable *) i->data;
            gr_apply_gradient_to_item (draggable->item, gr, new_type, new_fill, draggable->fill_or_stroke, !draggable->fill_or_stroke);
        }
        return;
    }

   // If no drag or no dragger selected, act on selection
   for (GSList const* i = selection->itemList(); i != NULL; i = i->next) {
       gr_apply_gradient_to_item (SP_ITEM(i->data), gr, new_type, new_fill, new_fill, !new_fill);
   }
}

void gr_combo_box_changed (GtkComboBox *widget, gpointer data)
{
    GtkTreeIter  iter;
    if (!gtk_combo_box_get_active_iter (widget, &iter)) {
        return;
    }

    GtkTreeModel *model = gtk_combo_box_get_model (widget);
    SPGradient *gr = NULL;
    gtk_tree_model_get (model, &iter, 2, &gr, -1);

    if (gr) {
        gr = sp_gradient_ensure_vector_normalized(gr);

        SPDesktop *desktop = static_cast<SPDesktop *>(data);
        Inkscape::Selection *selection = sp_desktop_selection (desktop);
        SPEventContext *ev = sp_desktop_event_context (desktop);

        gr_apply_gradient (selection, ev? ev->get_drag() : NULL, gr);

        DocumentUndo::done(sp_desktop_document (desktop), SP_VERB_CONTEXT_GRADIENT,
                   _("Assign gradient to object"));
    }

}

gchar *gr_prepare_label (SPObject *obj)
{
    const gchar *id = obj->defaultLabel();
    if (strlen(id) > 15 && (!strncmp (id, "#linearGradient", 15) || !strncmp (id, "#radialGradient", 15)))
        return g_strdup_printf ("#%s", id+15);
    return g_strdup_printf ("%s", id);
}

GtkWidget *gr_vector_list(SPDesktop *desktop, bool selection_empty, SPGradient *gr_selected, bool gr_multi)
{
    SPDocument *document = sp_desktop_document (desktop);

    GtkListStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkWidget *combo_box;

    store = gtk_list_store_new (3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_POINTER);
    combo_box = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));

    renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), renderer, FALSE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), renderer, "pixbuf", 0,  NULL);
    gtk_cell_renderer_set_padding(renderer, 5, 0);

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), renderer, "text", 1, NULL);

    GSList *gl = NULL;
    const GSList *gradients = document->getResourceList("gradient");
    for (const GSList *i = gradients; i != NULL; i = i->next) {
        SPGradient *grad = SP_GRADIENT(i->data);
        if ( grad->hasStops() && !grad->isSolid() ) {
            gl = g_slist_prepend (gl, i->data);
        }
    }
    gl = g_slist_reverse (gl);

    guint pos = 0;
    guint idx = 0;

    if (!gl) {
        // The document has no gradients
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, NULL, 1, _("No gradients"), 2, NULL, -1);
        gtk_widget_set_sensitive (combo_box, FALSE);

    } else if (selection_empty) {
        // Document has gradients, but nothing is currently selected.
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, NULL, 1, _("Nothing selected"), 2, NULL, -1);
        gtk_widget_set_sensitive (combo_box, FALSE);

    } else {

        if (gr_selected == NULL) {

            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter, 0, NULL, 1, _("No gradient"), 2, NULL, -1);
            gtk_widget_set_sensitive (combo_box, FALSE);

        }

        if (gr_multi) {

            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter, 0, NULL, 1, _("Multiple gradients"), 2, NULL, -1);
            gtk_widget_set_sensitive (combo_box, FALSE);

        }

        while (gl) {
            SPGradient *gradient = SP_GRADIENT (gl->data);
            gl = g_slist_remove (gl, gradient);

            gchar *label = gr_prepare_label(gradient);
            GdkPixbuf *pixb = sp_gradient_to_pixbuf (gradient, 60, 22);
            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter, 0, pixb, 1, label, 2, gradient, -1);
            g_free (label);

            if (gradient == gr_selected) {
                pos = idx;
            }
            idx ++;
        }
        gtk_widget_set_sensitive (combo_box, TRUE);
    }

    /* Select the current gradient, or the Multi/Nothing line */
    if (gr_multi || gr_selected == NULL) {
        gtk_combo_box_set_active (GTK_COMBO_BOX(combo_box) , 0);
    }
    else {
        gtk_combo_box_set_active (GTK_COMBO_BOX(combo_box) , pos);
    }

    sp_set_font_size_smaller(combo_box);

    g_signal_connect (G_OBJECT (combo_box), "changed", G_CALLBACK (gr_combo_box_changed), desktop);

    return combo_box;
}


void gr_read_selection( Inkscape::Selection *selection,
                        GrDrag *drag,
                        SPGradient *&gr_selected,
                        bool &gr_multi,
                        SPGradientSpread &spr_selected,
                        bool &spr_multi )
{
    if (drag && drag->selected) {
        // GRADIENTFIXME: make this work for more than one selected dragger?
        GrDragger *dragger = static_cast<GrDragger*>(drag->selected->data);
        for (GSList const* i = dragger->draggables; i; i = i->next) { // for all draggables of dragger
            GrDraggable *draggable = static_cast<GrDraggable *>(i->data);
            SPGradient *gradient = sp_item_gradient_get_vector(draggable->item, draggable->fill_or_stroke);
            SPGradientSpread spread = sp_item_gradient_get_spread(draggable->item, draggable->fill_or_stroke);

            if (gradient && gradient->isSolid()) {
                gradient = 0;
            }

            if (gradient && (gradient != gr_selected)) {
                if (gr_selected) {
                    gr_multi = true;
                } else {
                    gr_selected = gradient;
                }
            }
            if (spread != spr_selected) {
                if (spr_selected != INT_MAX) {
                    spr_multi = true;
                } else {
                    spr_selected = spread;
                }
            }
         }
        return;
    }

   // If no selected dragger, read desktop selection
   for (GSList const* i = selection->itemList(); i; i = i->next) {
        SPItem *item = SP_ITEM(i->data);
        SPStyle *style = item->style;

        if (style && (style->fill.isPaintserver())) {
            SPPaintServer *server = item->style->getFillPaintServer();
            if ( SP_IS_GRADIENT(server) ) {
                SPGradient *gradient = SP_GRADIENT(server)->getVector();
                SPGradientSpread spread = SP_GRADIENT(server)->fetchSpread();

                if (gradient && gradient->isSolid()) {
                    gradient = 0;
                }

                if (gradient && (gradient != gr_selected)) {
                    if (gr_selected) {
                        gr_multi = true;
                    } else {
                        gr_selected = gradient;
                    }
                }
                if (spread != spr_selected) {
                    if (spr_selected != INT_MAX) {
                        spr_multi = true;
                    } else {
                        spr_selected = spread;
                    }
                }
            }
        }
        if (style && (style->stroke.isPaintserver())) {
            SPPaintServer *server = item->style->getStrokePaintServer();
            if ( SP_IS_GRADIENT(server) ) {
                SPGradient *gradient = SP_GRADIENT(server)->getVector();
                SPGradientSpread spread = SP_GRADIENT(server)->fetchSpread();

                if (gradient && gradient->isSolid()) {
                    gradient = 0;
                }

                if (gradient && (gradient != gr_selected)) {
                    if (gr_selected) {
                        gr_multi = true;
                    } else {
                        gr_selected = gradient;
                    }
                }
                if (spread != spr_selected) {
                    if (spr_selected != INT_MAX) {
                        spr_multi = true;
                    } else {
                        spr_selected = spread;
                    }
                }
            }
        }
    }
 }

static void gr_tb_selection_changed(Inkscape::Selection * /*selection*/, gpointer data)
{
    GtkWidget *widget = GTK_WIDGET(data);

    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data(G_OBJECT(widget), "desktop"));
    if (desktop) {
        Inkscape::Selection *selection = sp_desktop_selection(desktop); // take from desktop, not from args
        if (selection) {
            SPEventContext *ev = sp_desktop_event_context(desktop);

            GtkWidget *combo_box = (GtkWidget *) g_object_get_data(G_OBJECT(widget), "combobox");
            if (combo_box) {
                gtk_widget_destroy(combo_box);
                combo_box = 0;
            }

            SPGradient *gr_selected = 0;
            bool gr_multi = false;

            SPGradientSpread spr_selected = static_cast<SPGradientSpread>(INT_MAX); // meaning undefined
            bool spr_multi = false;

            gr_read_selection(selection, ev ? ev->get_drag() : 0, gr_selected, gr_multi, spr_selected, spr_multi);

            combo_box = gr_vector_list(desktop, selection->isEmpty(), gr_selected, gr_multi);
            g_object_set_data(G_OBJECT(widget), "combobox", combo_box);

            GtkWidget *buttons = (GtkWidget *) g_object_get_data(G_OBJECT(widget), "buttons");
            gtk_widget_set_sensitive(buttons, (gr_selected && !gr_multi));

            gtk_box_pack_start(GTK_BOX(widget), combo_box, TRUE, TRUE, 0);

            gtk_widget_show_all(widget);
        }
    }
}

static void
gr_tb_selection_modified (Inkscape::Selection *selection, guint /*flags*/, gpointer data)
{
    gr_tb_selection_changed (selection, data);
}

static void
gr_drag_selection_changed (gpointer /*dragger*/, gpointer data)
{
    gr_tb_selection_changed (NULL, data);
}

static void
gr_defs_release (SPObject */*defs*/, GtkWidget *widget)
{
    gr_tb_selection_changed (NULL, (gpointer) widget);
}

static void
gr_defs_modified (SPObject */*defs*/, guint /*flags*/, GtkWidget *widget)
{
    gr_tb_selection_changed (NULL, (gpointer) widget);
}

static void gr_disconnect_sigc (GObject */*obj*/, sigc::connection *connection) {
    connection->disconnect();
    delete connection;
}

static void
gr_edit (GtkWidget */*button*/, GtkWidget *widget)
{
    GtkWidget *combo_box = (GtkWidget *) g_object_get_data (G_OBJECT(widget), "combobox");

    spinbutton_defocus(GTK_OBJECT(widget));

    if (combo_box) {
        GtkTreeIter  iter;
        if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX(combo_box), &iter)) {
            return;
        }

        GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX(combo_box));
        SPGradient *gr = NULL;
        gtk_tree_model_get (model, &iter, 2, &gr, -1);

        if (gr) {
            GtkWidget *dialog = sp_gradient_vector_editor_new (gr);
            gtk_widget_show (dialog);
        }
    }
}

GtkWidget * gr_change_widget(SPDesktop *desktop)
{
    Inkscape::Selection *selection = sp_desktop_selection (desktop);
    SPDocument *document = sp_desktop_document (desktop);
    SPEventContext *ev = sp_desktop_event_context (desktop);

    SPGradient *gr_selected = NULL;
    bool gr_multi = false;

    SPGradientSpread spr_selected = (SPGradientSpread) INT_MAX; // meaning undefined
    bool spr_multi = false;

    gr_read_selection (selection, ev? ev->get_drag() : 0, gr_selected, gr_multi, spr_selected, spr_multi);

    GtkWidget *widget = gtk_hbox_new(FALSE, FALSE);
    g_object_set_data(G_OBJECT(widget), "dtw", desktop->canvas);
    g_object_set_data (G_OBJECT (widget), "desktop", desktop);

    GtkWidget *combo_box = gr_vector_list(desktop, selection->isEmpty(), gr_selected, gr_multi);
    g_object_set_data(G_OBJECT(widget), "combobox", combo_box);

    gtk_box_pack_start (GTK_BOX (widget), combo_box, TRUE, TRUE, 0);

    {
    GtkWidget *buttons = gtk_hbox_new(FALSE, 1);

    /* Edit... */
    {
        GtkWidget *hb = gtk_hbox_new(FALSE, 1);
        GtkWidget *b = gtk_button_new_with_label(_("Edit..."));
        gtk_widget_set_tooltip_text(b, _("Edit the stops of the gradient"));
        gtk_widget_show(b);
        gtk_container_add(GTK_CONTAINER(hb), b);
        g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(gr_edit), widget);
        gtk_box_pack_start (GTK_BOX(buttons), hb, FALSE, FALSE, 0);
    }

    gtk_box_pack_end (GTK_BOX(widget), buttons, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT(widget), "buttons", buttons);
    gtk_widget_set_sensitive (buttons, (gr_selected && !gr_multi));
    }

    // connect to selection modified and changed signals
    sigc::connection *conn1 = new sigc::connection (selection->connectChanged(
        sigc::bind (
            sigc::ptr_fun(&gr_tb_selection_changed),
            (gpointer)widget )
    ));
    sigc::connection *conn2 = new sigc::connection (selection->connectModified(
        sigc::bind (
            sigc::ptr_fun(&gr_tb_selection_modified),
            (gpointer)widget )
    ));

    sigc::connection *conn3 = new sigc::connection (desktop->connectToolSubselectionChanged(
        sigc::bind (
            sigc::ptr_fun(&gr_drag_selection_changed),
            (gpointer)widget )
    ));

    // when widget is destroyed, disconnect
    g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(gr_disconnect_sigc), conn1);
    g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(gr_disconnect_sigc), conn2);
    g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(gr_disconnect_sigc), conn3);

    // connect to release and modified signals of the defs (i.e. when someone changes gradient)
    sigc::connection *release_connection = new sigc::connection();
    *release_connection = document->getDefs()->connectRelease(sigc::bind<1>(sigc::ptr_fun(&gr_defs_release), widget));
    sigc::connection *modified_connection = new sigc::connection();
    *modified_connection = document->getDefs()->connectModified(sigc::bind<2>(sigc::ptr_fun(&gr_defs_modified), widget));

    // when widget is destroyed, disconnect
    g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(gr_disconnect_sigc), release_connection);
    g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(gr_disconnect_sigc), modified_connection);

    gtk_widget_show_all (widget);
    return widget;
}

GtkWidget *
sp_gradient_toolbox_new(SPDesktop *desktop)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    GtkWidget *tbl = gtk_toolbar_new();

    g_object_set_data(G_OBJECT(tbl), "dtw", desktop->canvas);
    g_object_set_data(G_OBJECT(tbl), "desktop", desktop);

    sp_toolbox_add_label(tbl, _("<b>New:</b>"));

    // TODO replace aux_toolbox_space(tbl, AUX_SPACING);

    {
    GtkWidget *cvbox = gtk_vbox_new (FALSE, 0);
    GtkWidget *cbox = gtk_hbox_new (FALSE, 0);

    {
    GtkWidget *button = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              INKSCAPE_ICON("paint-gradient-linear"),
                                              _("Create linear gradient") );
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (gr_toggle_type), tbl);
    g_object_set_data(G_OBJECT(tbl), "linear", button);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
              prefs->getInt("/tools/gradient/newgradient", SP_GRADIENT_TYPE_LINEAR) == SP_GRADIENT_TYPE_LINEAR);
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    {
    GtkWidget *button = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              INKSCAPE_ICON("paint-gradient-radial"),
                                              _("Create radial (elliptic or circular) gradient"));
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (gr_toggle_type), tbl);
    g_object_set_data(G_OBJECT(tbl), "radial", button);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
              prefs->getInt("/tools/gradient/newgradient", SP_GRADIENT_TYPE_LINEAR) == SP_GRADIENT_TYPE_RADIAL);
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(cvbox), cbox, TRUE, FALSE, 0);
    GtkToolItem *cvbox_toolitem = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(cvbox_toolitem), cvbox);
    gtk_toolbar_insert(GTK_TOOLBAR(tbl), cvbox_toolitem, -1);
    }

    // TODO replace aux_toolbox_space(tbl, AUX_SPACING);

    sp_toolbox_add_label(tbl, _("on"), false);

    // TODO replace aux_toolbox_space(tbl, AUX_SPACING);

    {
        GtkWidget *cvbox = gtk_vbox_new (FALSE, 0);
    GtkWidget *cbox = gtk_hbox_new (FALSE, 0);

    {
    GtkWidget *button = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              INKSCAPE_ICON("object-fill"),
                                              _("Create gradient in the fill"));
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (gr_toggle_fillstroke), tbl);
    g_object_set_data(G_OBJECT(tbl), "fill", button);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                  prefs->getBool("/tools/gradient/newfillorstroke", true));
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    {
    GtkWidget *button = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              INKSCAPE_ICON("object-stroke"),
                                              _("Create gradient in the stroke"));
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (gr_toggle_fillstroke), tbl);
    g_object_set_data(G_OBJECT(tbl), "stroke", button);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                  !prefs->getBool("/tools/gradient/newfillorstroke", true));
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(cvbox), cbox, TRUE, TRUE, 3);
    GtkToolItem *cvbox_toolitem = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(cvbox_toolitem), cvbox);
    gtk_toolbar_insert(GTK_TOOLBAR(tbl), cvbox_toolitem, -1);
    }


    sp_toolbox_add_label(tbl, _("<b>Change:</b>"));

    // TODO replace aux_toolbox_space(tbl, AUX_SPACING);

    {
        GtkWidget *vectors = gr_change_widget (desktop);
        GtkToolItem *vectors_toolitem = gtk_tool_item_new();
        gtk_container_add(GTK_CONTAINER(vectors_toolitem), vectors);
        gtk_toolbar_insert(GTK_TOOLBAR(tbl), vectors_toolitem, -1);
    }

    gtk_widget_show_all(tbl);
    sp_set_font_size_smaller (tbl);

    return tbl;
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
