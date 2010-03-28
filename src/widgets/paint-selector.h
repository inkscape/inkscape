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

#include "fill-or-stroke.h"
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


/// Generic paint selector widget
struct SPPaintSelector {
    GtkVBox vbox;

    enum Mode {
        MODE_EMPTY,
        MODE_MULTIPLE,
        MODE_NONE,
        MODE_COLOR_RGB,
        MODE_COLOR_CMYK,
        MODE_GRADIENT_LINEAR,
        MODE_GRADIENT_RADIAL,
        MODE_PATTERN,
        MODE_SWATCH,
        MODE_UNSET
    } ;

    enum FillRule {
        FILLRULE_NONZERO,
        FILLRULE_EVENODD
    } ;

    guint update : 1;

    Mode mode;

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

    static Mode getModeForStyle(SPStyle const & style, FillOrStroke kind);

    void setMode( Mode mode );
    void setFillrule( FillRule fillrule );

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

    void (* mode_changed) (SPPaintSelector *psel, SPPaintSelector::Mode mode);

    void (* grabbed) (SPPaintSelector *psel);
    void (* dragged) (SPPaintSelector *psel);
    void (* released) (SPPaintSelector *psel);
    void (* changed) (SPPaintSelector *psel);
    void (* fillrule_changed) (SPPaintSelector *psel, SPPaintSelector::FillRule fillrule);
};

GtkType sp_paint_selector_get_type (void);

SPPaintSelector *sp_paint_selector_new(FillOrStroke kind);



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
