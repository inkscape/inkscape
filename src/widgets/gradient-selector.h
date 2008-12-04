#ifndef SEEN_GRADIENT_SELECTOR_H
#define SEEN_GRADIENT_SELECTOR_H

/*
 * Gradient vector and position widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include "sp-gradient-spread.h"
#include "sp-gradient-units.h"
class SPGradient;

#define SP_TYPE_GRADIENT_SELECTOR (sp_gradient_selector_get_type ())
#define SP_GRADIENT_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_GRADIENT_SELECTOR, SPGradientSelector))
#define SP_GRADIENT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_GRADIENT_SELECTOR, SPGradientSelectorClass))
#define SP_IS_GRADIENT_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_GRADIENT_SELECTOR))
#define SP_IS_GRADIENT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_GRADIENT_SELECTOR))

#include <libnr/nr-forward.h>
#include <gtk/gtkvbox.h>
#include "../forward.h"
#include <sp-gradient.h>

enum {
    SP_GRADIENT_SELECTOR_MODE_LINEAR,
    SP_GRADIENT_SELECTOR_MODE_RADIAL
};

struct SPGradientSelector {
    GtkVBox vbox;

    guint mode : 1;

    SPGradientUnits gradientUnits : 1;
    SPGradientSpread gradientSpread : 2;

    /* Vector selector */
    GtkWidget *vectors;
    /* Editing buttons */
    GtkWidget *edit, *add;
    /* Position widget */
    GtkWidget *position;
    /* Spread selector */
    GtkWidget *spread;
};

struct SPGradientSelectorClass {
    GtkVBoxClass parent_class;

    void (* grabbed) (SPGradientSelector *sel);
    void (* dragged) (SPGradientSelector *sel);
    void (* released) (SPGradientSelector *sel);
    void (* changed) (SPGradientSelector *sel);
};

GtkType sp_gradient_selector_get_type (void);

GtkWidget *sp_gradient_selector_new (void);

void sp_gradient_selector_set_mode (SPGradientSelector *sel, guint mode);
void sp_gradient_selector_set_units (SPGradientSelector *sel, guint units);
void sp_gradient_selector_set_spread (SPGradientSelector *sel, guint spread);
void sp_gradient_selector_set_vector (SPGradientSelector *sel, SPDocument *doc, SPGradient *vector);
void sp_gradient_selector_set_bbox (SPGradientSelector *sel, gdouble x0, gdouble y0, gdouble x1, gdouble y1);

SPGradientUnits sp_gradient_selector_get_units (SPGradientSelector *sel);
SPGradientSpread sp_gradient_selector_get_spread (SPGradientSelector *sel);

SPGradient *sp_gradient_selector_get_vector (SPGradientSelector *sel);

#endif // SEEN_GRADIENT_SELECTOR_H


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
