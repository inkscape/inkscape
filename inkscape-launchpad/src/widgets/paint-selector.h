#ifndef SEEN_SP_PAINT_SELECTOR_H
#define SEEN_SP_PAINT_SELECTOR_H
/*
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include "color.h"
#include "fill-or-stroke.h"
#include "sp-gradient-spread.h"
#include "sp-gradient-units.h"
#include "ui/selected-color.h"

class SPGradient;
class SPDesktop;
class SPPattern;
class SPStyle;

#define SP_TYPE_PAINT_SELECTOR (sp_paint_selector_get_type ())
#define SP_PAINT_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_PAINT_SELECTOR, SPPaintSelector))
#define SP_PAINT_SELECTOR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), SP_TYPE_PAINT_SELECTOR, SPPaintSelectorClass))
#define SP_IS_PAINT_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_PAINT_SELECTOR))
#define SP_IS_PAINT_SELECTOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), SP_TYPE_PAINT_SELECTOR))

/**
 * Generic paint selector widget.
 */
struct SPPaintSelector {
#if GTK_CHECK_VERSION(3,0,0)
    GtkBox  vbox;
#else
    GtkVBox vbox;
#endif

    enum Mode {
        MODE_EMPTY,
        MODE_MULTIPLE,
        MODE_NONE,
        MODE_SOLID_COLOR,
        MODE_GRADIENT_LINEAR,
        MODE_GRADIENT_RADIAL,
#ifdef WITH_MESH
        MODE_GRADIENT_MESH,
#endif
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
#ifdef WITH_MESH
    GtkWidget *mesh;
#endif
    GtkWidget *pattern;
    GtkWidget *swatch;
    GtkWidget *unset;

    GtkWidget *fillrulebox;
    GtkWidget *evenodd, *nonzero;

    GtkWidget *frame, *selector;
    GtkWidget *label;

    Inkscape::UI::SelectedColor *selected_color;
    bool updating_color;

    static Mode getModeForStyle(SPStyle const & style, FillOrStroke kind);

    void setMode( Mode mode );
    void setFillrule( FillRule fillrule );

    void setColorAlpha( SPColor const &color, float alpha );
    void getColorAlpha( SPColor &color, gfloat &alpha ) const;

    void setGradientLinear( SPGradient *vector );
    void setGradientRadial( SPGradient *vector );
#ifdef WITH_MESH
    void setGradientMesh(SPGradient *vector);
#endif
    void setSwatch( SPGradient *vector );

    void setGradientProperties( SPGradientUnits units, SPGradientSpread spread );
    void getGradientProperties( SPGradientUnits &units, SPGradientSpread &spread ) const;

    void pushAttrsToGradient( SPGradient *gr ) const;
    SPGradient *getGradientVector();
    SPPattern * getPattern();
    void updatePatternList( SPPattern *pat );

    static gboolean isSeparator (GtkTreeModel *model, GtkTreeIter *iter, gpointer data);

    // TODO move this elsewhere:
    void setFlatColor( SPDesktop *desktop, const gchar *color_property, const gchar *opacity_property );

    void onSelectedColorGrabbed();
    void onSelectedColorDragged();
    void onSelectedColorReleased();
    void onSelectedColorChanged();
};

enum {COMBO_COL_LABEL=0, COMBO_COL_STOCK=1, COMBO_COL_PATTERN=2, COMBO_COL_SEP=3, COMBO_N_COLS=4};


/// The SPPaintSelector vtable
struct SPPaintSelectorClass {
#if GTK_CHECK_VERSION(3,0,0)
    GtkBoxClass parent_class;
#else
    GtkVBoxClass parent_class;
#endif

    void (* mode_changed) (SPPaintSelector *psel, SPPaintSelector::Mode mode);

    void (* grabbed) (SPPaintSelector *psel);
    void (* dragged) (SPPaintSelector *psel);
    void (* released) (SPPaintSelector *psel);
    void (* changed) (SPPaintSelector *psel);
    void (* fillrule_changed) (SPPaintSelector *psel, SPPaintSelector::FillRule fillrule);
};

GType sp_paint_selector_get_type (void);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
