#ifndef SEEN_SP_PAINT_SELECTOR_H
#define SEEN_SP_PAINT_SELECTOR_H

/** \file
 * Generic paint selector widget
 *
 * Authors:
 *   Lauris
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) Lauris 2002
 * Copyright (C) 2010 Authors
 *
 */

#include <glib.h>
#include "sp-gradient-spread.h"
#include "sp-gradient-units.h"

class SPGradient;

#define SP_TYPE_PAINT_SELECTOR (sp_paint_selector_get_type ())
#define SP_PAINT_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_PAINT_SELECTOR, SPPaintSelector))
#define SP_PAINT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_PAINT_SELECTOR, SPPaintSelectorClass))
#define SP_IS_PAINT_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_PAINT_SELECTOR))
#define SP_IS_PAINT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_PAINT_SELECTOR))

#include <gtk/gtkvbox.h>

#include "../forward.h"
#include <color.h>
#include <libnr/nr-forward.h>

typedef enum {
    SP_PAINT_SELECTOR_MODE_EMPTY,
    SP_PAINT_SELECTOR_MODE_MULTIPLE,
    SP_PAINT_SELECTOR_MODE_NONE,
    SP_PAINT_SELECTOR_MODE_COLOR_RGB,
    SP_PAINT_SELECTOR_MODE_COLOR_CMYK,
    SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR,
    SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL,
    SP_PAINT_SELECTOR_MODE_PATTERN,
    SP_PAINT_SELECTOR_MODE_SWATCH,
    SP_PAINT_SELECTOR_MODE_UNSET
} SPPaintSelectorMode;

typedef enum {
    SP_PAINT_SELECTOR_FILLRULE_NONZERO,
    SP_PAINT_SELECTOR_FILLRULE_EVENODD
} SPPaintSelectorFillRule;

/// Generic paint selector widget
struct SPPaintSelector {
    GtkVBox vbox;

    guint update : 1;

    SPPaintSelectorMode mode;

    GtkWidget *style;
    GtkWidget *none;
    GtkWidget *solid;
    GtkWidget *gradient;
    GtkWidget *radial;
    GtkWidget *pattern;
    GtkWidget *swatch;
    GtkWidget *unset;

    GtkWidget *fillrulebox;
    GtkWidget *evenodd, *nonzero;

    GtkWidget *frame, *selector;

    SPColor color;
    float alpha;


    void setMode( SPPaintSelectorMode mode );
    void setFillrule( SPPaintSelectorFillRule fillrule );

    void setColorAlpha( SPColor const &color, float alpha );
    void getColorAlpha( SPColor &color, gfloat &alpha ) const;

    void setGradientLinear( SPGradient *vector );
    void setGradientRadial( SPGradient *vector );
    void setSwatch( SPGradient *vector );

    void setGradientProperties( SPGradientUnits units, SPGradientSpread spread );
    void getGradientProperties( SPGradientUnits &units, SPGradientSpread &spread ) const;

    void pushAttrsToGradient( SPGradient *gr ) const;
    SPGradient *getGradientVector();
    SPPattern * getPattern();
    void updatePatternList( SPPattern *pat );

    // TODO move this elsewhere:
    void setFlatColor( SPDesktop *desktop, const gchar *color_property, const gchar *opacity_property );
};

/// The SPPaintSelector vtable
struct SPPaintSelectorClass {
    GtkVBoxClass parent_class;

    void (* mode_changed) (SPPaintSelector *psel, SPPaintSelectorMode mode);

    void (* grabbed) (SPPaintSelector *psel);
    void (* dragged) (SPPaintSelector *psel);
    void (* released) (SPPaintSelector *psel);
    void (* changed) (SPPaintSelector *psel);
    void (* fillrule_changed) (SPPaintSelector *psel, SPPaintSelectorFillRule fillrule);
};

GtkType sp_paint_selector_get_type (void);

GtkWidget *sp_paint_selector_new (bool is_fill);



SPPaintSelectorMode sp_style_determine_paint_selector_mode (SPStyle *style, bool isfill);

#endif // SEEN_SP_PAINT_SELECTOR_H

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
