#ifndef SEEN_GRADIENT_SELECTOR_H
#define SEEN_GRADIENT_SELECTOR_H

/*
 * Gradient vector and position widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <gtk/gtkvbox.h>
#include "sp-gradient.h"
#include "sp-gradient-spread.h"
#include "sp-gradient-units.h"

class SPGradient;

#define SP_TYPE_GRADIENT_SELECTOR (sp_gradient_selector_get_type ())
#define SP_GRADIENT_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_GRADIENT_SELECTOR, SPGradientSelector))
#define SP_GRADIENT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_GRADIENT_SELECTOR, SPGradientSelectorClass))
#define SP_IS_GRADIENT_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_GRADIENT_SELECTOR))
#define SP_IS_GRADIENT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_GRADIENT_SELECTOR))



struct SPGradientSelector {
    GtkVBox vbox;

    enum SelectorMode {
        MODE_LINEAR,
        MODE_RADIAL
    };

    SelectorMode mode;

    SPGradientUnits gradientUnits;
    SPGradientSpread gradientSpread;

    /* Vector selector */
    GtkWidget *vectors;

    /* Editing buttons */
    GtkWidget *edit;
    GtkWidget *add;

    /* Position widget */
    GtkWidget *position;

    /* Spread selector */
    GtkWidget *spread;

    void setMode(SelectorMode mode);
    void setUnits(SPGradientUnits units);
    void setSpread(SPGradientSpread spread);
    void setVector(SPDocument *doc, SPGradient *vector);
    SPGradientUnits getUnits();
    SPGradientSpread getSpread();
    SPGradient *getVector();
};

struct SPGradientSelectorClass {
    GtkVBoxClass parent_class;

    void (* grabbed) (SPGradientSelector *sel);
    void (* dragged) (SPGradientSelector *sel);
    void (* released) (SPGradientSelector *sel);
    void (* changed) (SPGradientSelector *sel);
};

GType sp_gradient_selector_get_type(void);

GtkWidget *sp_gradient_selector_new (void);

void sp_gradient_selector_set_bbox (SPGradientSelector *sel, gdouble x0, gdouble y0, gdouble x1, gdouble y1);

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
