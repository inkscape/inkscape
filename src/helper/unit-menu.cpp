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
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include "helper/sp-marshal.h"
#include "helper/units.h"
#include "unit-menu.h"
#include "widgets/spw-utilities.h"

struct SPUnitSelector {
    GtkHBox box;

    GtkWidget *menu;

    guint bases;
    GSList *units;
    SPUnit const *unit;
    gdouble ctmscale;
    guint plural : 1;
    guint abbr : 1;

    guint update : 1;

    GSList *adjustments;
};

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
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;

    object_class = G_OBJECT_CLASS(klass);
    widget_class = GTK_WIDGET_CLASS(klass);

    unit_selector_parent_class = (GtkHBoxClass*)gtk_type_class(GTK_TYPE_HBOX);

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

    us->menu = gtk_option_menu_new();

    gtk_widget_show(us->menu);
    gtk_box_pack_start(GTK_BOX(us), us->menu, TRUE, TRUE, 0);
}

static void
sp_unit_selector_finalize(GObject *object)
{
    SPUnitSelector *selector = SP_UNIT_SELECTOR(object);

    if (selector->menu) {
        selector->menu = NULL;
    }

    while (selector->adjustments) {
        gtk_object_unref(GTK_OBJECT(selector->adjustments->data));
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
    SPUnitSelector *us = (SPUnitSelector*)gtk_type_new(SP_TYPE_UNIT_SELECTOR);

    sp_unit_selector_set_bases(us, bases);

    return (GtkWidget *) us;
}

void
sp_unit_selector_setsize(GtkWidget *us, guint w, guint h)
{
    gtk_widget_set_size_request(((SPUnitSelector *) us)->menu, w, h);
}

SPUnit const *
sp_unit_selector_get_unit(SPUnitSelector const *us)
{
    g_return_val_if_fail(us != NULL, NULL);
    g_return_val_if_fail(SP_IS_UNIT_SELECTOR(us), NULL);

    return us->unit;
}

static void
spus_unit_activate(GtkWidget *widget, SPUnitSelector *us)
{
    SPUnit const *unit = (SPUnit const *) gtk_object_get_data(GTK_OBJECT(widget), "unit");
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
            gdouble val = adj->value;
#ifdef UNIT_SELECTOR_VERBOSE
            g_print("Old val %g ... ", val);
#endif
            val = sp_convert_distance_full(val, *old, *unit);
#ifdef UNIT_SELECTOR_VERBOSE
            g_print("new val %g\n", val);
#endif
            adj->value = val;
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
            gtk_signal_emit_by_name(GTK_OBJECT(l->data), "value_changed");
        }
    }

    us->update = FALSE;
}

static void
spus_rebuild_menu(SPUnitSelector *us)
{
    if (GTK_OPTION_MENU(us->menu)->menu) {
        gtk_option_menu_remove_menu(GTK_OPTION_MENU(us->menu));
    }

    GtkWidget *m = gtk_menu_new();

    gtk_widget_show(m);

    gint pos = 0;
    gint p = 0;
    for (GSList *l = us->units; l != NULL; l = l->next) {
        SPUnit const *u = (SPUnit*)l->data;

        // use only abbreviations in the menu
        //        i = gtk_menu_item_new_with_label((us->abbr) ? (us->plural) ? u->abbr_plural : u->abbr : (us->plural) ? u->plural : u->name);
        GtkWidget *i = gtk_menu_item_new_with_label( u->abbr );

        gtk_object_set_data(GTK_OBJECT(i), "unit", (gpointer) u);
        gtk_signal_connect(GTK_OBJECT(i), "activate", GTK_SIGNAL_FUNC(spus_unit_activate), us);

        sp_set_font_size_smaller (i);

        gtk_widget_show(i);

        gtk_menu_shell_append(GTK_MENU_SHELL(m), i);
        if (u == us->unit) pos = p;
        p += 1;
    }

    gtk_option_menu_set_menu(GTK_OPTION_MENU(us->menu), m);

    gtk_option_menu_set_history(GTK_OPTION_MENU(us->menu), pos);
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

    gtk_option_menu_set_history(GTK_OPTION_MENU(us->menu), pos);

    SPUnit const *old = us->unit;
    us->unit = unit;

    /* Recalculate adjustments */
    for (GSList *l = us->adjustments; l != NULL; l = l->next) {
        GtkAdjustment *adj = GTK_ADJUSTMENT(l->data);
        gdouble const val = sp_convert_distance_full(adj->value, *old, *unit);
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

    gtk_object_ref(GTK_OBJECT(adj));
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
    gtk_object_unref(GTK_OBJECT(adj));
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
    g_return_val_if_fail(selector != NULL, adj->value);
    g_return_val_if_fail(SP_IS_UNIT_SELECTOR(selector), adj->value);

    return sp_units_get_pixels(adj->value, *(selector->unit));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
