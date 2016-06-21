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

#include "ui/widget/color-preview.h"
#include <glibmm/i18n.h>

#include "desktop.h"
#include "document-undo.h"
#include "document.h"
#include "widgets/ege-adjustment-action.h"
#include "widgets/ege-select-one-action.h"
#include "gradient-chemistry.h"
#include "gradient-drag.h"
#include "gradient-toolbar.h"
#include "widgets/ink-action.h"
#include "macros.h"
#include "preferences.h"
#include "selection.h"
#include "sp-defs.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-stop.h"
#include "style.h"
#include "toolbox.h"
#include "ui/icon-names.h"
#include "ui/tools/gradient-tool.h"
#include "verbs.h"
#include "widgets/gradient-image.h"
#include "widgets/gradient-vector.h"

using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::Tools::ToolBase;

void gr_apply_gradient_to_item( SPItem *item, SPGradient *gr, SPGradientType initialType, Inkscape::PaintTarget initialMode, Inkscape::PaintTarget mode );
void gr_apply_gradient(Inkscape::Selection *selection, GrDrag *drag, SPGradient *gr);
gboolean gr_vector_list(GtkWidget *combo_box, SPDesktop *desktop, bool selection_empty, SPGradient *gr_selected, bool gr_multi);
void gr_get_dt_selected_gradient(Inkscape::Selection *selection, SPGradient *&gr_selected);
void gr_read_selection( Inkscape::Selection *selection, GrDrag *drag, SPGradient *&gr_selected, bool &gr_multi, SPGradientSpread &spr_selected, bool &spr_multi );
static gboolean update_stop_list( GtkWidget *stop_combo, SPGradient *gradient, SPStop *new_stop, GtkWidget *widget, bool gr_multi);
static void sp_gradient_vector_widget_load_gradient(GtkWidget *widget, SPGradient *gradient);
static void select_stop_in_list( GtkWidget *combo_box, SPGradient *gradient, SPStop *new_stop, GtkWidget *data, gboolean block);
static void select_stop_by_drag( GtkWidget *combo_box, SPGradient *gradient, ToolBase *ev, GtkWidget *data);
static void select_drag_by_stop( GtkWidget *combo_box, SPGradient *gradient, ToolBase *ev);
static SPGradient *gr_get_selected_gradient(GtkWidget *widget);
static void gr_stop_set_offset(GtkComboBox *widget, GtkWidget *data);
void add_toolbar_widget(GtkWidget *tbl, GtkWidget *widget);
static GtkWidget * gr_ege_select_one_get_combo(GtkWidget *widget, const gchar *name);
void check_renderer(GtkWidget *combo);
static void gr_tb_selection_changed(Inkscape::Selection *selection, gpointer data);

static gboolean blocked = FALSE;

//########################
//##       Gradient     ##
//########################

void gr_apply_gradient_to_item( SPItem *item, SPGradient *gr, SPGradientType initialType, Inkscape::PaintTarget initialMode, Inkscape::PaintTarget mode )
{
    SPStyle *style = item->style;
    bool isFill = (mode == Inkscape::FOR_FILL);
    if (style
        && (isFill ? style->fill.isPaintserver() : style->stroke.isPaintserver())
        //&& SP_IS_GRADIENT(isFill ? style->getFillPaintServer() : style->getStrokePaintServer()) ) {
        && (isFill ? SP_IS_GRADIENT(style->getFillPaintServer()) : SP_IS_GRADIENT(style->getStrokePaintServer())) ) {
        SPPaintServer *server = isFill ? style->getFillPaintServer() : style->getStrokePaintServer();
        if ( SP_IS_LINEARGRADIENT(server) ) {
            sp_item_set_gradient(item, gr, SP_GRADIENT_TYPE_LINEAR, mode);
        } else if ( SP_IS_RADIALGRADIENT(server) ) {
            sp_item_set_gradient(item, gr, SP_GRADIENT_TYPE_RADIAL, mode);
        }
    }
    else if (initialMode == mode)
    {
        sp_item_set_gradient(item, gr, initialType, mode);
    }
}

/**
Applies gradient vector gr to the gradients attached to the selected dragger of drag, or if none,
to all objects in selection. If there was no previous gradient on an item, uses gradient type and
fill/stroke setting from preferences to create new default (linear: left/right; radial: centered)
gradient.
*/
void gr_apply_gradient(Inkscape::Selection *selection, GrDrag *drag, SPGradient *gr)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    SPGradientType initialType = static_cast<SPGradientType>(prefs->getInt("/tools/gradient/newgradient", SP_GRADIENT_TYPE_LINEAR));
    Inkscape::PaintTarget initialMode = (prefs->getInt("/tools/gradient/newfillorstroke", 1) != 0) ? Inkscape::FOR_FILL : Inkscape::FOR_STROKE;

    // GRADIENTFIXME: make this work for multiple selected draggers.

    // First try selected dragger
    if (drag && !drag->selected.empty()) {
        GrDragger *dragger = *(drag->selected.begin());
        for(std::vector<GrDraggable *>::const_iterator i = dragger->draggables.begin(); i != dragger->draggables.end(); ++i) { //for all draggables of dragger
            GrDraggable *draggable =  *i;
            gr_apply_gradient_to_item(draggable->item, gr, initialType, initialMode, draggable->fill_or_stroke);
        }
        return;
    }

   // If no drag or no dragger selected, act on selection
    std::vector<SPItem*> itemlist=selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
       gr_apply_gradient_to_item(*i, gr, initialType, initialMode, initialMode);
   }
}

gboolean gr_vector_list(GtkWidget *combo_box, SPDesktop *desktop, bool selection_empty, SPGradient *gr_selected, bool gr_multi)
{
    gboolean sensitive = FALSE;
    if (blocked) {
        return sensitive;
    }

    SPDocument *document = desktop->getDocument();

    GtkTreeIter iter;
    GtkListStore *store = (GtkListStore *)gtk_combo_box_get_model(GTK_COMBO_BOX(combo_box));

    blocked = TRUE;

    /* Clear old list, if there is any */
    gtk_list_store_clear(store);

    std::vector<SPObject *> gl;
    std::vector<SPObject *> gradients = document->getResourceList( "gradient" );
    for (std::vector<SPObject *>::const_iterator it = gradients.begin(); it != gradients.end(); ++it) {
        SPGradient *grad = SP_GRADIENT(*it);
        if ( grad->hasStops() && !grad->isSolid() ) {
            gl.push_back(*it);
        }
    }

    guint pos = 0;

    if (gl.empty()) {
        // The document has no gradients
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, _("No gradient"), 1, NULL, 2, NULL, -1);
        sensitive = FALSE;

    } else if (selection_empty) {
        // Document has gradients, but nothing is currently selected.
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, _("Nothing selected"), 1, NULL, 2, NULL, -1);
        sensitive = FALSE;

    } else {

        if (gr_selected == NULL) {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, _("No gradient"), 1, NULL, 2, NULL, -1);
            // Dead assignment: Value stored to 'sensitive' is never read
            //sensitive = FALSE;
        }

        if (gr_multi) {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, _("Multiple gradients"), 1, NULL, 2, NULL, -1);
            // Dead assignment: Value stored to 'sensitive' is never read
            //sensitive = FALSE;
        }

        guint idx = 0;
        for (std::vector<SPObject *>::const_iterator it = gl.begin(); it != gl.end(); ++it) {
            SPGradient *gradient = SP_GRADIENT(*it);

            Glib::ustring label = gr_prepare_label(gradient);
            GdkPixbuf *pixb = sp_gradient_to_pixbuf(gradient, 64, 16);
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, label.c_str(), 1, pixb, 2, gradient, -1);

            if (gradient == gr_selected) {
                pos = idx;
            }
            idx ++;
        }
        sensitive = TRUE;

    }

    /* Select the current gradient, or the Multi/Nothing line */
    if (gr_multi || gr_selected == NULL) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box) , 0);
    }
    else {
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box) , pos);
    }

    blocked = FALSE;
    return sensitive;
}

/*
 * Get the gradient of the selected desktop item
 * This is gradient containing the repeat settings, not the underlying "getVector" href linked gradient.
 */
void gr_get_dt_selected_gradient(Inkscape::Selection *selection, SPGradient *&gr_selected)
{
    SPGradient *gradient = 0;

    std::vector<SPItem*> itemlist=selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;// get the items gradient, not the getVector() version
         SPStyle *style = item->style;
         SPPaintServer *server = 0;

         if (style && (style->fill.isPaintserver())) {
             server = item->style->getFillPaintServer();
         }
         if (style && (style->stroke.isPaintserver())) {
             server = item->style->getStrokePaintServer();
         }

         if ( SP_IS_GRADIENT(server) ) {
             gradient = SP_GRADIENT(server);
         }
    }

    if (gradient && gradient->isSolid()) {
        gradient = 0;
    }

    if (gradient) {
        gr_selected = gradient;
    }
}

/*
 * Get the current selection and dragger status from the desktop
 */
void gr_read_selection( Inkscape::Selection *selection,
                        GrDrag *drag,
                        SPGradient *&gr_selected,
                        bool &gr_multi,
                        SPGradientSpread &spr_selected,
                        bool &spr_multi )
{
    if (drag && !drag->selected.empty()) {
        // GRADIENTFIXME: make this work for more than one selected dragger?
        GrDragger *dragger = *(drag->selected.begin());
        for(std::vector<GrDraggable *>::const_iterator i = dragger->draggables.begin(); i != dragger->draggables.end(); ++i) { //for all draggables of dragger
            GrDraggable *draggable = *i;
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
                if (spr_selected != SP_GRADIENT_SPREAD_UNDEFINED) {
                    spr_multi = true;
                } else {
                    spr_selected = spread;
                }
            }
         }
        return;
    }

   // If no selected dragger, read desktop selection
    std::vector<SPItem*> itemlist=selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i){
        SPItem *item = *i;
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
                    if (spr_selected != SP_GRADIENT_SPREAD_UNDEFINED) {
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
                    if (spr_selected != SP_GRADIENT_SPREAD_UNDEFINED) {
                        spr_multi = true;
                    } else {
                        spr_selected = spread;
                    }
                }
            }
        }
    }
 }

/*
 * Core function, setup all the widgets whenever something changes on the desktop
 */
static void gr_tb_selection_changed(Inkscape::Selection * /*selection*/, gpointer data)
{
    if (blocked)
        return;

    GtkWidget *widget = GTK_WIDGET(data);

    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data(G_OBJECT(widget), "desktop"));
    if (!desktop) {
        return;
    }

    Inkscape::Selection *selection = desktop->getSelection(); // take from desktop, not from args
    if (selection) {
        ToolBase *ev = desktop->getEventContext();
        GrDrag *drag = NULL;
        if (ev) {
            drag = ev->get_drag();
        }

        SPGradient *gr_selected = 0;
        SPGradientSpread spr_selected = SP_GRADIENT_SPREAD_UNDEFINED;
        bool gr_multi = false;
        bool spr_multi = false;

        gr_read_selection(selection, drag, gr_selected, gr_multi, spr_selected, spr_multi);

        GtkWidget *gradient_combo = gr_ege_select_one_get_combo(widget, "gradient_select_combo_action");
        if ( gradient_combo ) {
            check_renderer(gradient_combo);
            gboolean sensitive = gr_vector_list(gradient_combo, desktop, selection->isEmpty(), gr_selected, gr_multi);

            EgeSelectOneAction *gradient_action = (EgeSelectOneAction *) g_object_get_data(G_OBJECT(widget), "gradient_select_combo_action");
            gtk_action_set_sensitive( GTK_ACTION(gradient_action), sensitive );
        }

        EgeSelectOneAction* spread = (EgeSelectOneAction *) g_object_get_data(G_OBJECT(widget), "gradient_select_repeat_action");
        gtk_action_set_sensitive( GTK_ACTION(spread), (gr_selected && !gr_multi) );
        if (gr_selected) {
            blocked = TRUE;
            ege_select_one_action_set_active( spread, spr_selected);
            blocked = FALSE;
        }

        InkAction *add = (InkAction *) g_object_get_data(G_OBJECT(widget), "gradient_stops_add_action");
        gtk_action_set_sensitive(GTK_ACTION(add), (gr_selected && !gr_multi && drag && !drag->selected.empty()));

        InkAction *del = (InkAction *) g_object_get_data(G_OBJECT(widget), "gradient_stops_delete_action");
        gtk_action_set_sensitive(GTK_ACTION(del), (gr_selected && !gr_multi && drag && !drag->selected.empty()));

        InkAction *reverse = (InkAction *) g_object_get_data(G_OBJECT(widget), "gradient_stops_reverse_action");
        gtk_action_set_sensitive(GTK_ACTION(reverse), (gr_selected!= NULL));

        EgeSelectOneAction *stops_action = (EgeSelectOneAction *) g_object_get_data(G_OBJECT(widget), "gradient_stops_combo_action");
        gtk_action_set_sensitive( GTK_ACTION(stops_action), (gr_selected && !gr_multi) );

        GtkWidget *stops_combo = gr_ege_select_one_get_combo(widget, "gradient_stops_combo_action");
        if ( stops_combo ) {

            check_renderer(stops_combo);
            update_stop_list(stops_combo, gr_selected, NULL, widget, gr_multi);
            select_stop_by_drag(stops_combo, gr_selected, ev, widget);
        }

        //sp_gradient_vector_widget_load_gradient(widget, gr_selected);

    }

}

static GtkWidget * gr_ege_select_one_get_combo(GtkWidget *widget, const gchar *name)
{
    GtkWidget *combo_box = 0;
    EgeSelectOneAction *act1 = (EgeSelectOneAction *)g_object_get_data( G_OBJECT(widget), name);
    if (act1) {
        gpointer combodata = g_object_get_data( G_OBJECT(act1), "ege-combo-box" );
        if ( GTK_IS_COMBO_BOX(combodata) ) {
            combo_box = GTK_WIDGET(combodata);
        }
    }
    return combo_box;
}

static void gr_tb_selection_modified(Inkscape::Selection *selection, guint /*flags*/, gpointer data)
{
    gr_tb_selection_changed(selection, data);
}

static void gr_drag_selection_changed(gpointer /*dragger*/, gpointer data)
{
    gr_tb_selection_changed(NULL, data);

}

static void gr_defs_release(SPObject * /*defs*/, GtkWidget *widget)
{
    gr_tb_selection_changed(NULL, (gpointer) widget);
}

static void gr_defs_modified(SPObject * /*defs*/, guint /*flags*/, GtkWidget *widget)
{
    gr_tb_selection_changed(NULL, (gpointer) widget);
}

static SPStop *get_selected_stop( GtkWidget *vb)
{
    SPStop *stop = NULL;

    GtkWidget *cb = gr_ege_select_one_get_combo(vb, "gradient_stops_combo_action");
    if ( cb ) {
        GtkTreeIter  iter;
        if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(cb), &iter)) {
            GtkListStore *store = (GtkListStore *)gtk_combo_box_get_model(GTK_COMBO_BOX(cb));
            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 2, &stop, -1);
        }
    }

    return stop;
}


static void sp_gradient_vector_gradient_release(SPObject * /*object*/, GtkWidget *widget)
{
    sp_gradient_vector_widget_load_gradient(widget, NULL);
}

static void sp_gradient_vector_gradient_modified(SPObject *object, guint /*flags*/, GtkWidget *widget)
{
    SPGradient *gradient=SP_GRADIENT(object);
    if (!blocked) {
        blocked = TRUE;
        sp_gradient_vector_widget_load_gradient(widget, gradient);
        blocked = FALSE;
    }
}

static void sp_gradient_vector_widget_load_gradient(GtkWidget *widget, SPGradient *gradient)
{
    blocked = TRUE;

    SPGradient *old = gr_get_selected_gradient(widget);

    if (old != gradient) {

        g_message("Load gradient");

        sigc::connection *release_connection;
        sigc::connection *modified_connection;

        release_connection = (sigc::connection *)g_object_get_data(G_OBJECT(widget), "gradient_release_connection");
        modified_connection = (sigc::connection *)g_object_get_data(G_OBJECT(widget), "gradient_modified_connection");

        if (old) {
            g_assert( release_connection != NULL );
            g_assert( modified_connection != NULL );
            release_connection->disconnect();
            modified_connection->disconnect();
            sp_signal_disconnect_by_data(old, widget);
        }

        if (gradient) {
            if (!release_connection) {
                release_connection = new sigc::connection();
            }
            if (!modified_connection) {
                modified_connection = new sigc::connection();
            }
            *release_connection = gradient->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_gradient_vector_gradient_release), widget));
            *modified_connection = gradient->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_gradient_vector_gradient_modified), widget));
        } else {
            if (release_connection) {
                delete release_connection;
                release_connection = NULL;
            }
            if (modified_connection) {
                delete modified_connection;
                modified_connection = NULL;
            }
        }

        g_object_set_data(G_OBJECT(widget), "gradient_release_connection", release_connection);
        g_object_set_data(G_OBJECT(widget), "gradient_modified_connection", modified_connection);
    }

    if (gradient) {
        gtk_widget_set_sensitive(widget, TRUE);

        gradient->ensureVector();

        SPStop *stop = get_selected_stop(widget);
        if (!stop) {
            return;
        }

        // Once the user edits a gradient, it stops being auto-collectable
        if (gradient->getRepr()->attribute("inkscape:collect")) {
            SPDocument *document = gradient->document;
            bool saved = DocumentUndo::getUndoSensitive(document);
            DocumentUndo::setUndoSensitive(document, false);
            gradient->getRepr()->setAttribute("inkscape:collect", NULL);
            DocumentUndo::setUndoSensitive(document, saved);
        }
    } else { // no gradient, disable everything
        gtk_widget_set_sensitive(widget, FALSE);
    }

    blocked = FALSE;
}

static void gr_add_stop(GtkWidget * /*button*/, GtkWidget *vb)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data(G_OBJECT(vb), "desktop"));
    if (!desktop) {
        return;
    }

    Inkscape::Selection *selection = desktop->getSelection();
    if (!selection) {
        return;
    }

    ToolBase *ev = desktop->getEventContext();
    Inkscape::UI::Tools::GradientTool *rc = SP_GRADIENT_CONTEXT(ev);

    if (rc) {
        sp_gradient_context_add_stops_between_selected_stops(rc);
    }

}

static void gr_remove_stop(GtkWidget * /*button*/, GtkWidget *vb)
{

    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data(G_OBJECT(vb), "desktop"));
    if (!desktop) {
        return;
    }

    Inkscape::Selection *selection = desktop->getSelection(); // take from desktop, not from args
    if (!selection) {
        return;
    }

    ToolBase *ev = desktop->getEventContext();
    GrDrag *drag = NULL;
    if (ev) {
        drag = ev->get_drag();
    }

    if (drag) {
        drag->deleteSelected();
    }

}

static void gr_linked_changed(GtkToggleAction *act, gpointer /*data*/)
{
    gboolean active = gtk_toggle_action_get_active( act );
    if ( active ) {
        g_object_set( G_OBJECT(act), "iconId", INKSCAPE_ICON("object-locked"), NULL );
    } else {
        g_object_set( G_OBJECT(act), "iconId", INKSCAPE_ICON("object-unlocked"), NULL );
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/options/forkgradientvectors/value", !active);
}

static void gr_reverse(GtkWidget * /*button*/, gpointer data)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(data);
    sp_gradient_reverse_selected_gradients(desktop);
}

/*
 *  Change desktop drag selection to this stop
 */
static void select_drag_by_stop( GtkWidget *data, SPGradient *gradient, ToolBase *ev)
{
    if (blocked || !ev || !gradient)
        return;

    GrDrag *drag = ev->get_drag();
    if (!drag) {
        return;
    }

    SPStop *stop = get_selected_stop(data);

    drag->selectByStop(stop, false, true);
    blocked = FALSE;
}

static void select_stop_by_drag(GtkWidget *combo_box, SPGradient *gradient, ToolBase *ev, GtkWidget *data)
{
    if (blocked || !ev || !gradient)
        return;

    GrDrag *drag = ev->get_drag();

    if (!drag || drag->selected.empty()) {
        blocked = TRUE;
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box) , 0);
        gr_stop_set_offset(GTK_COMBO_BOX(combo_box), data);
        blocked = FALSE;
        return;
    }

    gint n = 0;

    // for all selected draggers
    for(std::set<GrDragger *>::const_iterator i = drag->selected.begin(); i != drag->selected.end(); ++i) { //for all draggables of dragger
        GrDragger *dragger = *i;
        for(std::vector<GrDraggable *>::const_iterator j = dragger->draggables.begin(); j != dragger->draggables.end(); ++j) { //for all draggables of dragger
            GrDraggable *draggable = *j; 

            if (draggable->point_type != POINT_RG_FOCUS) {
                n++;
            }
            if (n > 1) {

                // Mulitple stops selected
                GtkListStore *store = (GtkListStore *)gtk_combo_box_get_model(GTK_COMBO_BOX(combo_box));
                if (!store) {
                    return;
                }
                GtkTreeIter iter;
                gtk_list_store_prepend(store, &iter);
                gtk_list_store_set(store, &iter, 0, _("Multiple stops"), 1, NULL, 2, NULL, -1);
                gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box) , 0);

                EgeAdjustmentAction* act = (EgeAdjustmentAction *)g_object_get_data( G_OBJECT(data), "offset_action");
                if (act) {
                    gtk_action_set_sensitive( GTK_ACTION(act), FALSE);
                }

                return;
            }


            SPGradient *vector = gradient->getVector();
            if (!vector)
                return;

            SPStop *stop = vector->getFirstStop();

            switch (draggable->point_type) {
                case POINT_LG_MID:
                case POINT_RG_MID1:
                case POINT_RG_MID2:
                    {
                        stop = sp_get_stop_i(vector, draggable->point_i);
                    }
                    break;
                 case POINT_LG_END:
                 case POINT_RG_R1:
                 case POINT_RG_R2:
                     {
                        stop = sp_last_stop(vector);
                     }
                     break;
                 default:
                     break;
            }

            select_stop_in_list( combo_box, gradient, stop, data, TRUE);
        }
    }
}

static void select_stop_in_list( GtkWidget *combo_box, SPGradient *gradient, SPStop *new_stop, GtkWidget *data, gboolean block)
{
    int i = 0;
    for ( SPObject *ochild = gradient->firstChild() ; ochild ; ochild = ochild->getNext() ) {
        if (SP_IS_STOP(ochild)) {
            if (ochild == new_stop) {
                blocked = block;
                gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box) , i);
                gr_stop_set_offset(GTK_COMBO_BOX(combo_box), data);
                blocked = FALSE;
                return;
            }
            i++;
        }
    }
}

static gboolean update_stop_list( GtkWidget *stop_combo, SPGradient *gradient, SPStop *new_stop, GtkWidget *widget, bool gr_multi)
{
    gboolean sensitive = FALSE;

    if (!stop_combo) {
        return sensitive;
    }

    GtkListStore *store = (GtkListStore *)gtk_combo_box_get_model(GTK_COMBO_BOX(stop_combo));
    if (!store) {
        return sensitive;
    }

    blocked = TRUE;

    /* Clear old list, if there is any */
    gtk_list_store_clear(store);
    GtkTreeIter iter;

    if (!SP_IS_GRADIENT(gradient)) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, _("No gradient"), 1, NULL, 2, NULL, -1);
        gtk_combo_box_set_active(GTK_COMBO_BOX(stop_combo) , 0);
        sensitive = FALSE;
        blocked = FALSE;
        return sensitive;
    }

    /* Populate the combobox store */
    std::vector<SPObject *> sl;
    if ( gradient->hasStops() ) {
        for ( SPObject *ochild = gradient->firstChild() ; ochild ; ochild = ochild->getNext() ) {
            if (SP_IS_STOP(ochild)) {
                sl.push_back(ochild);
            }
        }
    }
    if (sl.empty()) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, _("No stops in gradient"), 1, NULL, 2, NULL, -1);
        sensitive = FALSE;

    } else {

        for (std::vector<SPObject *>::const_iterator it = sl.begin(); it != sl.end(); ++it) {
            if (SP_IS_STOP(*it)){
                SPStop *stop = SP_STOP(*it);
                Inkscape::XML::Node *repr = reinterpret_cast<SPItem *>(*it)->getRepr();
                Inkscape::UI::Widget::ColorPreview *cpv = Gtk::manage(new Inkscape::UI::Widget::ColorPreview(stop->get_rgba32()));
                GdkPixbuf *pb = cpv->toPixbuf(32, 16);
                Glib::ustring label = gr_ellipsize_text(repr->attribute("id"), 25);

                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter, 0, label.c_str(), 1, pb, 2, stop, -1);
                // Dead assignment: Value stored to 'sensitive' is never read
                //sensitive = FALSE;
            }
        }

        sensitive = TRUE;
    }

    if (gr_multi) {
        sensitive = FALSE;
    }

    if (new_stop == NULL) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(stop_combo) , 0);
    } else {
        select_stop_in_list(stop_combo, gradient, new_stop, widget, TRUE);
    }

    blocked = FALSE;

    return sensitive;
}

static SPGradient *gr_get_selected_gradient(GtkWidget *widget)
{
    SPGradient *gr = NULL;
    EgeSelectOneAction* act1 = (EgeSelectOneAction *)g_object_get_data( G_OBJECT(widget), "gradient_select_combo_action");
    if (act1) {
        gint n = ege_select_one_action_get_active(act1);
        GtkTreeModel *model = ege_select_one_action_get_model(act1);
        GtkTreeIter  iter;
        if (gtk_tree_model_iter_nth_child(model, &iter, NULL, n)) {
            gtk_tree_model_get(model, &iter, 2, &gr, -1);
            return gr;
        }
    }

    return gr;
}

/*
static void gr_edit(GtkWidget *button, GtkWidget *widget)
{
    SPGradient *gr = gr_get_selected_gradient(widget);
    if (gr) {
        GtkWidget *dialog = sp_gradient_vector_editor_new(gr);
        gtk_widget_show(dialog);
    }
}
*/

static void gr_stop_set_offset(GtkComboBox * /*widget*/, GtkWidget *data)
{
    SPStop *stop = get_selected_stop(data);
    if (!stop) {
        return;
    }

    EgeAdjustmentAction* act = (EgeAdjustmentAction *)g_object_get_data( G_OBJECT(data), "offset_action");
    if (!act) {
        return;
    }

    GtkAdjustment *adj = ege_adjustment_action_get_adjustment(act);

    bool isEndStop = false;

    SPStop *prev = NULL;
    prev = stop->getPrevStop();
    if (prev != NULL )  {
        gtk_adjustment_set_lower(adj, prev->offset);
    } else {
        isEndStop = true;
        gtk_adjustment_set_lower(adj, 0);
    }

    SPStop *next = NULL;
    next = stop->getNextStop();
    if (next != NULL ) {
        gtk_adjustment_set_upper(adj, next->offset);
    } else {
        isEndStop = true;
        gtk_adjustment_set_upper(adj, 1.0);
    }

    blocked = TRUE;
    gtk_adjustment_set_value(adj, stop->offset);
    gtk_action_set_sensitive( GTK_ACTION(act), !isEndStop );
    gtk_adjustment_changed(adj);
    blocked = FALSE;

}


/*
 * Callback functions for user actions
 */

static void gr_new_type_changed( EgeSelectOneAction *act, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gint typemode = ege_select_one_action_get_active( act ) == 0 ? SP_GRADIENT_TYPE_LINEAR : SP_GRADIENT_TYPE_RADIAL;
    prefs->setInt("/tools/gradient/newgradient", typemode);
}

static void gr_new_fillstroke_changed( EgeSelectOneAction *act, GObject * /*tbl*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Inkscape::PaintTarget fsmode = (ege_select_one_action_get_active( act ) == 0) ? Inkscape::FOR_FILL : Inkscape::FOR_STROKE;
    prefs->setInt("/tools/gradient/newfillorstroke", (fsmode == Inkscape::FOR_FILL) ? 1 : 0);
}

/*
 * User selected a gradient from the combobox
 */
static void gr_gradient_combo_changed(EgeSelectOneAction *act, gpointer data)
{
    if (blocked) {
        return;
    }

    SPGradient *gr = NULL;
    gint n = ege_select_one_action_get_active(act);
    GtkTreeModel *model = ege_select_one_action_get_model(act);
    GtkTreeIter  iter;
    if (gtk_tree_model_iter_nth_child(model, &iter, NULL, n)) {
        gtk_tree_model_get(model, &iter, 2, &gr, -1);
    }

    if (gr) {
        gr = sp_gradient_ensure_vector_normalized(gr);

        SPDesktop *desktop = static_cast<SPDesktop *>(data);
        Inkscape::Selection *selection = desktop->getSelection();
        ToolBase *ev = desktop->getEventContext();

        gr_apply_gradient(selection, ev? ev->get_drag() : NULL, gr);

        DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_GRADIENT,
                   _("Assign gradient to object"));
    }

}

static void gr_spread_change(EgeSelectOneAction *act, GtkWidget *widget)
{
    if (blocked) {
        return;
    }

    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data(G_OBJECT(widget), "desktop"));
    Inkscape::Selection *selection = desktop->getSelection();
    SPGradient *gradient = 0;
    gr_get_dt_selected_gradient(selection, gradient);

    if (gradient) {
        SPGradientSpread spread = (SPGradientSpread) ege_select_one_action_get_active(act);
        gradient->setSpread(spread);
        gradient->updateRepr();

        DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_GRADIENT,
                   _("Set gradient repeat"));
    }
}


/*
 * User selected a stop from the combobox
 */
static void gr_stop_combo_changed(GtkComboBox * /*widget*/, GtkWidget *data)
{
    if (blocked) {
        return;
    }

    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data(G_OBJECT(data), "desktop"));
    ToolBase *ev = desktop->getEventContext();
    SPGradient *gr = gr_get_selected_gradient(data);

    select_drag_by_stop(data, gr, ev);
}

/*
 * User changed the offset
 */
static void gr_stop_offset_adjustment_changed(GtkAdjustment *adj, GObject *tbl)
{
    if (blocked) {
        return;
    }

    blocked = TRUE;

    SPStop *stop = get_selected_stop(GTK_WIDGET(tbl));
    if (stop) {
        stop->offset = gtk_adjustment_get_value(adj);
        sp_repr_set_css_double(stop->getRepr(), "offset", stop->offset);

        DocumentUndo::maybeDone(stop->document, "gradient:stop:offset", SP_VERB_CONTEXT_GRADIENT,
                                _("Change gradient stop offset"));

    }

    blocked = FALSE;
}

/*
 * Change the combobox to show the icon/image on the left of the text
 */
void check_renderer(GtkWidget *combo)
{
    gpointer rendered = g_object_get_data( G_OBJECT(combo), "renderers" );
    if (!rendered) {

        gtk_cell_layout_clear(GTK_CELL_LAYOUT(combo));

        GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();
        gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, FALSE);
        gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "pixbuf", 1,  NULL);
        gtk_cell_renderer_set_padding(renderer, 5, 0);

        renderer = gtk_cell_renderer_text_new();
        gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
        gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "text", 0, NULL);

        g_object_set_data(G_OBJECT(combo), "renderers", renderer);
    }
}

static void gradient_toolbox_check_ec(SPDesktop* dt, Inkscape::UI::Tools::ToolBase* ec, GObject* holder);

/**
 * Gradient auxiliary toolbar construction and setup.
 *
 */
void sp_gradient_toolbox_prep(SPDesktop * desktop, GtkActionGroup* mainActions, GObject* holder)
{
    Inkscape::IconSize secondarySize = ToolboxFactory::prefToSize("/toolbox/secondary", 1);

    /* New gradient linear or radial */
    {
        GtkListStore* model = gtk_list_store_new( 3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

        GtkTreeIter iter;
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                0, _("linear"), 1, _("Create linear gradient"), 2, INKSCAPE_ICON("paint-gradient-linear"), -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter,
                0, _("radial"), 1, _("Create radial (elliptic or circular) gradient"), 2, INKSCAPE_ICON("paint-gradient-radial"), -1 );

        EgeSelectOneAction* act = ege_select_one_action_new( "GradientNewTypeAction", (""), (""), NULL, GTK_TREE_MODEL(model) );
        g_object_set( act, "short_label", _("New:"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        g_object_set_data( holder, "gradient_new_type_action", act );

        ege_select_one_action_set_appearance( act, "full" );
        ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
        g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
        ege_select_one_action_set_icon_column( act, 2 );
        ege_select_one_action_set_tooltip_column( act, 1  );

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        gint mode = prefs->getInt("/tools/gradient/newgradient", SP_GRADIENT_TYPE_LINEAR) != SP_GRADIENT_TYPE_LINEAR;
        ege_select_one_action_set_active( act, mode );
        g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(gr_new_type_changed), holder );
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

        EgeSelectOneAction* act = ege_select_one_action_new( "GradientNewFillStrokeAction", (""), (""), NULL, GTK_TREE_MODEL(model) );
        g_object_set( act, "short_label", _("on:"), NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act) );
        g_object_set_data( holder, "gradient_new_fillstroke_action", act );

        ege_select_one_action_set_appearance( act, "full" );
        ege_select_one_action_set_radio_action_type( act, INK_RADIO_ACTION_TYPE );
        g_object_set( G_OBJECT(act), "icon-property", "iconId", NULL );
        ege_select_one_action_set_icon_column( act, 2 );
        ege_select_one_action_set_tooltip_column( act, 1  );

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        Inkscape::PaintTarget fsmode = (prefs->getInt("/tools/gradient/newfillorstroke", 1) != 0) ? Inkscape::FOR_FILL : Inkscape::FOR_STROKE;
        ege_select_one_action_set_active( act, (fsmode == Inkscape::FOR_FILL) ? 0 : 1 );
        g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(gr_new_fillstroke_changed), holder );
    }


    /* Gradient Select list*/
    {
        GtkListStore *store = gtk_list_store_new(3, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_POINTER);

        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, _("No gradient"), 1, NULL, 2, NULL, -1);

        EgeSelectOneAction* act1 = ege_select_one_action_new( "GradientSelectGradientAction", _("Select"), (_("Choose a gradient")), NULL, GTK_TREE_MODEL(store) );
        g_object_set( act1, "short_label", _("Select:"), NULL );
        ege_select_one_action_set_appearance( act1, "compact" );
        gtk_action_set_sensitive( GTK_ACTION(act1), FALSE );
        g_signal_connect( G_OBJECT(act1), "changed", G_CALLBACK(gr_gradient_combo_changed), desktop );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act1) );
        g_object_set_data( holder, "gradient_select_combo_action", act1 );

    }

    // Gradient Repeat type
    {
        GtkListStore* model = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_INT );

        GtkTreeIter iter;
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter, 0, C_("Gradient repeat type", "None"), 1, SP_GRADIENT_SPREAD_PAD, -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter, 0, _("Reflected"), 1, SP_GRADIENT_SPREAD_REFLECT, -1 );

        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter, 0, _("Direct"), 1, SP_GRADIENT_SPREAD_REPEAT, -1 );

        EgeSelectOneAction* act1 = ege_select_one_action_new( "GradientSelectRepeatAction", _("Repeat"),
                (// TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/pservers.html#LinearGradientSpreadMethodAttribute
                        _("Whether to fill with flat color beyond the ends of the gradient vector "
                          "(spreadMethod=\"pad\"), or repeat the gradient in the same direction "
                          "(spreadMethod=\"repeat\"), or repeat the gradient in alternating opposite "
                          "directions (spreadMethod=\"reflect\")")),
                NULL, GTK_TREE_MODEL(model) );
        g_object_set( act1, "short_label", _("Repeat:"), NULL );
        ege_select_one_action_set_appearance( act1, "compact" );
        gtk_action_set_sensitive( GTK_ACTION(act1), FALSE );
        g_signal_connect( G_OBJECT(act1), "changed", G_CALLBACK(gr_spread_change), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act1) );
        g_object_set_data( holder, "gradient_select_repeat_action", act1 );
    }

    /* Gradient Stop list */
    {
        GtkListStore *store = gtk_list_store_new(3, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_POINTER);

        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, _("No stops"), 1, NULL, 2, NULL, -1);

        EgeSelectOneAction* act1 = ege_select_one_action_new( "GradientEditStopsAction", _("Stops"), _("Select a stop for the current gradient"), NULL, GTK_TREE_MODEL(store) );
        g_object_set( act1, "short_label", _("Stops:"), NULL );
        ege_select_one_action_set_appearance( act1, "compact" );
        gtk_action_set_sensitive( GTK_ACTION(act1), FALSE );
        g_signal_connect( G_OBJECT(act1), "changed", G_CALLBACK(gr_stop_combo_changed), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(act1) );
        g_object_set_data( holder, "gradient_stops_combo_action", act1 );
    }

    /* Offset */
    {
        EgeAdjustmentAction* eact = 0;
        eact = create_adjustment_action( "GradientEditOffsetAction",
                                         _("Offset"), C_("Gradient", "Offset:"), _("Offset of selected stop"),
                                         "/tools/gradient/stopoffset", 0,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         0.0, 1.0, 0.01, 0.1,
                                         0, 0, 0,
                                         gr_stop_offset_adjustment_changed,
                                         NULL /*unit tracker*/,
                                         0.01, 2, 1.0);

        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        g_object_set_data( holder, "offset_action", eact );
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );

    }

    /* Add stop */
    {
        InkAction* inky = ink_action_new( "GradientEditAddAction",
                                          _("Insert new stop"),
                                          _("Insert new stop"),
                                          INKSCAPE_ICON("node-add"),
                                          secondarySize );
        g_object_set( inky, "short_label", _("Delete"), NULL );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(gr_add_stop), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        gtk_action_set_sensitive( GTK_ACTION(inky), FALSE );
        g_object_set_data( holder, "gradient_stops_add_action", inky );
    }

    /* Delete stop */
    {
        InkAction* inky = ink_action_new( "GradientEditDeleteAction",
                                          _("Delete stop"),
                                          _("Delete stop"),
                                          INKSCAPE_ICON("node-delete"),
                                          secondarySize );
        g_object_set( inky, "short_label", _("Delete"), NULL );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(gr_remove_stop), holder );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        gtk_action_set_sensitive( GTK_ACTION(inky), FALSE );
        g_object_set_data( holder, "gradient_stops_delete_action", inky );
    }

    /* Reverse */
    {
        InkAction* inky = ink_action_new( "GradientEditReverseAction",
                                          _("Reverse"),
                                          _("Reverse the direction of the gradient"),
                                          INKSCAPE_ICON("object-flip-horizontal"),
                                          secondarySize );
        g_object_set( inky, "short_label", _("Delete"), NULL );
        g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(gr_reverse), desktop );
        gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        gtk_action_set_sensitive( GTK_ACTION(inky), FALSE );
        g_object_set_data( holder, "gradient_stops_reverse_action", inky );

    }

    // Gradients Linked toggle
    {
        InkToggleAction* itact = ink_toggle_action_new( "GradientEditLinkAction",
                                                        _("Link gradients"),
                                                        _("Link gradients to change all related gradients"),
                                                        INKSCAPE_ICON("object-unlocked"),
                                                        Inkscape::ICON_SIZE_DECORATION );
        g_object_set( itact, "short_label", "Lock", NULL );
        g_signal_connect_after( G_OBJECT(itact), "toggled", G_CALLBACK(gr_linked_changed), desktop) ;
        gtk_action_group_add_action( mainActions, GTK_ACTION(itact) );

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        bool linkedmode = prefs->getBool("/options/forkgradientvectors/value", true);
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(itact), !linkedmode );
    }

    g_object_set_data(holder, "desktop", desktop);

    desktop->connectEventContextChanged(sigc::bind(sigc::ptr_fun(&gradient_toolbox_check_ec), holder));
}

// lp:1327267
/**
 * Checks the current tool and connects gradient aux toolbox signals if it happens to be the gradient tool.
 * Called every time the current tool changes by signal emission.
 */
static void gradient_toolbox_check_ec(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* ec, GObject* holder)
{
    static sigc::connection connChanged;
    static sigc::connection connModified;
    static sigc::connection connSubselectionChanged;
    static sigc::connection connDefsRelease;
    static sigc::connection connDefsModified;

    if (SP_IS_GRADIENT_CONTEXT(ec)) {
        Inkscape::Selection *selection = desktop->getSelection();
        SPDocument *document = desktop->getDocument();

        // connect to selection modified and changed signals
        connChanged = selection->connectChanged(sigc::bind(sigc::ptr_fun(&gr_tb_selection_changed), holder));
        connModified = selection->connectModified(sigc::bind(sigc::ptr_fun(&gr_tb_selection_modified), holder));
        connSubselectionChanged = desktop->connectToolSubselectionChanged(sigc::bind(sigc::ptr_fun(&gr_drag_selection_changed), holder));

        // Is this necessary? Couldn't hurt.
        gr_tb_selection_changed(selection, holder);

        // connect to release and modified signals of the defs (i.e. when someone changes gradient)
        connDefsRelease = document->getDefs()->connectRelease(sigc::bind<1>(sigc::ptr_fun(&gr_defs_release), GTK_WIDGET(holder)));
        connDefsModified = document->getDefs()->connectModified(sigc::bind<2>(sigc::ptr_fun(&gr_defs_modified), GTK_WIDGET(holder)));
    } else {
        if (connChanged)
            connChanged.disconnect();
        if (connModified)
            connModified.disconnect();
        if (connSubselectionChanged)
            connSubselectionChanged.disconnect();
        if (connDefsRelease)
            connDefsRelease.disconnect();
        if (connDefsModified)
            connDefsModified.disconnect();
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
