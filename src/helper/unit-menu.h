#ifndef __SP_UNIT_MENU_H__
#define __SP_UNIT_MENU_H__

/*
 * SPUnitMenu
 *
 * Generic (and quite unintelligent) grid item for gnome canvas
 *
 * Copyright (C) Lauris Kaplinski 2000
 *
 */

#include <glib/gtypes.h>
#include <gtk/gtkoptionmenu.h>

#include <helper/helper-forward.h>


/* Unit selector Widget */

#define SP_TYPE_UNIT_SELECTOR (sp_unit_selector_get_type())
#define SP_UNIT_SELECTOR(o) (GTK_CHECK_CAST((o), SP_TYPE_UNIT_SELECTOR, SPUnitSelector))
#define SP_UNIT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST((k), SP_TYPE_UNIT_SELECTOR, SPUnitSelectorClass))
#define SP_IS_UNIT_SELECTOR(o) (GTK_CHECK_TYPE((o), SP_TYPE_UNIT_SELECTOR))
#define SP_IS_UNIT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE((k), SP_TYPE_UNIT_SELECTOR))

GType sp_unit_selector_get_type(void);

GtkWidget *sp_unit_selector_new(guint bases);
void sp_unit_selector_setsize(GtkWidget *us, guint w, guint h);

SPUnit const *sp_unit_selector_get_unit(SPUnitSelector const *selector);

void sp_unit_selector_set_bases(SPUnitSelector *selector, guint bases);
void sp_unit_selector_add_unit(SPUnitSelector *selector, SPUnit const *unit, int position);

void sp_unit_selector_set_unit(SPUnitSelector *selector, SPUnit const *unit);
void sp_unit_selector_add_adjustment(SPUnitSelector *selector, GtkAdjustment *adjustment);
void sp_unit_selector_remove_adjustment(SPUnitSelector *selector, GtkAdjustment *adjustment);

gboolean sp_unit_selector_update_test(SPUnitSelector const *selector);

double sp_unit_selector_get_value_in_pixels(SPUnitSelector const *selector, GtkAdjustment *adj);
void sp_unit_selector_set_value_in_pixels(SPUnitSelector *selector, GtkAdjustment *adj, double value);



#endif

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
