#define __SP_UNIT_MENU_C__

/*
 * Unit selector with autupdate capability
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2000-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noUNIT_SELECTOR_VERBOSE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <gtk/gtk.h>
#include "helper/sp-marshal.h"
#include "helper/units.h"
#include "helper/unit-menu.h"
#include "widgets/spw-utilities.h"

struct SPUnitSelector {
    GtkHBox box;

    GtkWidget *combo_box;
    GtkListStore *store;


    guint bases;
    GSList *units;
    SPUnit const *unit;
    gdouble ctmscale;
    guint plural : 1;
    guint abbr : 1;

    guint update : 1;

    GSList *adjustments;
};

enum {COMBO_COL_LABEL=0, COMBO_COL_UNIT};

struct SPUnitSelectorClass {
    GtkHBoxClass parent_class;

    gboolean (* set_unit)(SPUnitSelector *us, SPUnit const *old, SPUnit const *new_unit);
};

enum {SET_UNIT, LAST_SIGNAL};

static void sp_unit_selector_class_init(SPUnitSelectorClass *klass);
static void sp_unit_selector_init(SPUnitSelector *selector);
static void sp_unit_selector_finalize(GObject *object);

static GtkHBoxClass *unit_selector_parent_class;
static guint signals[LAST_SIGNAL] = {0};

GType sp_unit_selector_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPUnitSelectorClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_unit_selector_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPUnitSelector),
            0, // n_preallocs
            (GInstanceInitFunc)sp_unit_selector_init,
            0 // value_table
        };
        type = g_type_register_static(GTK_TYPE_HBOX, "SPUnitSelector", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

static void
sp_unit_selector_class_init(SPUnitSelectorClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    unit_selector_parent_class = (GtkHBoxClass*)g_type_class_peek_parent(klass);

    signals[SET_UNIT] = g_signal_new("set_unit",
                                     G_TYPE_FROM_CLASS(klass),
                                     G_SIGNAL_RUN_LAST,
                                     G_STRUCT_OFFSET(SPUnitSelectorClass, set_unit),
                                     NULL, NULL,
                                     sp_marshal_BOOLEAN__POINTER_POINTER,
                                     G_TYPE_BOOLEAN, 2,
                                     G_TYPE_POINTER, G_TYPE_POINTER);

    object_class->finalize = sp_unit_selector_finalize;
}

static void
sp_unit_selector_init(SPUnitSelector *us)
{
    us->ctmscale = 1.0;
    us->abbr = FALSE;
    us->plural = TRUE;

    /**
     * Create a combo_box and store with 2 columns,
     * a label and a pointer to a SPUnit
     */
    us->store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_POINTER);
    us->combo_box = gtk_combo_box_new_with_model (GTK_TREE_MODEL (us->store));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer, "scale", 0.8, "scale-set", TRUE, NULL);
    gtk_cell_renderer_set_padding (renderer, 2, 0);
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (us->combo_box), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (us->combo_box), renderer, "text", COMBO_COL_LABEL, NULL);

    gtk_widget_show (us->combo_box);
    gtk_box_pack_start (GTK_BOX(us), us->combo_box, TRUE, TRUE, 0);
}

static void
sp_unit_selector_finalize(GObject *object)
{
    SPUnitSelector *selector = SP_UNIT_SELECTOR(object);

    if (selector->combo_box) {
        selector->combo_box = NULL;
    }

    while (selector->adjustments) {
        g_object_unref(selector->adjustments->data);
        selector->adjustments = g_slist_remove(selector->adjustments, selector->adjustments->data);
    }

    if (selector->units) {
        sp_unit_free_list(selector->units);
    }

    selector->unit = NULL;

    G_OBJECT_CLASS(unit_selector_parent_class)->finalize(object);
}

GtkWidget *
sp_unit_selector_new(guint bases)
{
    SPUnitSelector *us = (SPUnitSelector*)g_object_new(SP_TYPE_UNIT_SELECTOR, NULL);

    sp_unit_selector_set_bases(us, bases);

    return (GtkWidget *) us;
}

void
sp_unit_selector_setsize(GtkWidget *us, guint w, guint h)
{
    gtk_widget_set_size_request(((SPUnitSelector *) us)->combo_box, w, h);
}

SPUnit const *
sp_unit_selector_get_unit(SPUnitSelector const *us)
{
    g_return_val_if_fail(us != NULL, NULL);
    g_return_val_if_fail(SP_IS_UNIT_SELECTOR(us), NULL);

    return us->unit;
}


static void
on_combo_box_changed (GtkComboBox *widget, SPUnitSelector *us)
{
    GtkTreeIter  iter;
    if (!gtk_combo_box_get_active_iter (widget, &iter)) {
        return;
    }

    SPUnit const *unit = NULL;
    gtk_tree_model_get ((GtkTreeModel *)us->store, &iter, COMBO_COL_UNIT, &unit, -1);

    g_return_if_fail(unit != NULL);

#ifdef UNIT_SELECTOR_VERBOSE
    g_print("Old unit %s new unit %s\n", us->unit->name, unit->name);
#endif

    SPUnit const *old = us->unit;
    us->unit = unit;

    us->update = TRUE;

    gboolean consumed = FALSE;
    g_signal_emit(G_OBJECT(us), signals[SET_UNIT], 0, old, unit, &consumed);

    if ( !consumed
         && ( unit->base == old->base
              || ( unit->base == SP_UNIT_ABSOLUTE && old->base == SP_UNIT_DEVICE )
              || ( old->base == SP_UNIT_ABSOLUTE && unit->base == SP_UNIT_DEVICE ) ) ) {
        // Either the same base, or absolute<->device:
        /* Recalculate adjustments. */
        for (GSList *l = us->adjustments; l != NULL; l = g_slist_next(l)) {
            GtkAdjustment *adj = GTK_ADJUSTMENT(l->data);
            gdouble val = gtk_adjustment_get_value (adj);
#ifdef UNIT_SELECTOR_VERBOSE
            g_print("Old val %g ... ", val);
#endif
            val = sp_convert_distance_full(val, *old, *unit);
#ifdef UNIT_SELECTOR_VERBOSE
            g_print("new val %g\n", val);
#endif
            gtk_adjustment_set_value (adj, val);
        }
        /* need to separate the value changing from the notification
         * or else the unit changes can break the calculations */
        for (GSList *l = us->adjustments; l != NULL; l = g_slist_next(l)) {
            gtk_adjustment_value_changed(GTK_ADJUSTMENT(l->data));
        }
    } else if (!consumed && unit->base != old->base) {
        /* when the base changes, signal all the adjustments to get them
         * to recalculate */
        for (GSList *l = us->adjustments; l != NULL; l = g_slist_next(l)) {
            g_signal_emit_by_name(G_OBJECT(l->data), "value_changed");
        }
    }

    us->update = FALSE;

}

static void
spus_rebuild_menu(SPUnitSelector *us)
{

    gtk_list_store_clear(us->store);

    GtkTreeIter iter;

    gint pos = 0;
    gint p = 0;
    for (GSList *l = us->units; l != NULL; l = l->next) {
        SPUnit const *u = (SPUnit*)l->data;

        // use only abbreviations in the menu
        //        i = gtk_menu_item_new_with_label((us->abbr) ? (us->plural) ? u->abbr_plural : u->abbr : (us->plural) ? u->plural : u->name);
        gtk_list_store_append (us->store, &iter);
        gtk_list_store_set (us->store, &iter, COMBO_COL_LABEL, u->abbr, COMBO_COL_UNIT, (gpointer) u, -1);

        if (u == us->unit) {
            pos = p;
        }

        p += 1;
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(us->combo_box), pos);
    g_signal_connect (G_OBJECT (us->combo_box), "changed", G_CALLBACK (on_combo_box_changed), us);
}

void
sp_unit_selector_set_bases(SPUnitSelector *us, guint bases)
{
    g_return_if_fail(us != NULL);
    g_return_if_fail(SP_IS_UNIT_SELECTOR(us));

    if (bases == us->bases) return;

    GSList *units = sp_unit_get_list(bases);
    g_return_if_fail(units != NULL);
    sp_unit_free_list(us->units);
    us->units = units;
    us->unit = (SPUnit*)units->data;

    spus_rebuild_menu(us);
}

void
sp_unit_selector_add_unit(SPUnitSelector *us, SPUnit const *unit, int position)
{
    if (!g_slist_find(us->units, (gpointer) unit)) {
        us->units = g_slist_insert(us->units, (gpointer) unit, position);

        spus_rebuild_menu(us);
    }
}

void
sp_unit_selector_set_unit(SPUnitSelector *us, SPUnit const *unit)
{
    g_return_if_fail(us != NULL);
    g_return_if_fail(SP_IS_UNIT_SELECTOR(us));

    if (unit == NULL) {
        return; // silently return, by default a newly created selector uses pt
    }
    if (unit == us->unit) {
        return;
    }

    gint const pos = g_slist_index(us->units, (gpointer) unit);
    g_return_if_fail(pos >= 0);

    gtk_combo_box_set_active(GTK_COMBO_BOX(us->combo_box), pos);

    SPUnit const *old = us->unit;
    us->unit = unit;

    /* Recalculate adjustments */
    for (GSList *l = us->adjustments; l != NULL; l = l->next) {
        GtkAdjustment *adj = GTK_ADJUSTMENT(l->data);
        gdouble const val = sp_convert_distance_full(gtk_adjustment_get_value (adj), *old, *unit);
        gtk_adjustment_set_value(adj, val);
    }
}

void
sp_unit_selector_add_adjustment(SPUnitSelector *us, GtkAdjustment *adj)
{
    g_return_if_fail(us != NULL);
    g_return_if_fail(SP_IS_UNIT_SELECTOR(us));
    g_return_if_fail(adj != NULL);
    g_return_if_fail(GTK_IS_ADJUSTMENT(adj));

    g_return_if_fail(!g_slist_find(us->adjustments, adj));

    g_object_ref(adj);
    us->adjustments = g_slist_prepend(us->adjustments, adj);
}

void
sp_unit_selector_remove_adjustment(SPUnitSelector *us, GtkAdjustment *adj)
{
    g_return_if_fail(us != NULL);
    g_return_if_fail(SP_IS_UNIT_SELECTOR(us));
    g_return_if_fail(adj != NULL);
    g_return_if_fail(GTK_IS_ADJUSTMENT(adj));

    g_return_if_fail(g_slist_find(us->adjustments, adj));

    us->adjustments = g_slist_remove(us->adjustments, adj);
    g_object_unref(adj);
}

gboolean
sp_unit_selector_update_test(SPUnitSelector const *selector)
{
    g_return_val_if_fail(selector != NULL, FALSE);
    g_return_val_if_fail(SP_IS_UNIT_SELECTOR(selector), FALSE);

    return selector->update;
}

double
sp_unit_selector_get_value_in_pixels(SPUnitSelector const *selector, GtkAdjustment *adj)
{
    g_return_val_if_fail(selector != NULL, gtk_adjustment_get_value (adj));
    g_return_val_if_fail(SP_IS_UNIT_SELECTOR(selector), gtk_adjustment_get_value (adj));

    return sp_units_get_pixels(gtk_adjustment_get_value (adj), *(selector->unit));
}

void
sp_unit_selector_set_value_in_pixels(SPUnitSelector *selector, GtkAdjustment *adj, double value)
{
    g_return_if_fail(selector != NULL);
    g_return_if_fail(SP_IS_UNIT_SELECTOR(selector));

    gtk_adjustment_set_value(adj, sp_pixels_get_units(value, *(selector->unit)));
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
