#ifndef __SP_PAINT_SELECTOR_H__
#define __SP_PAINT_SELECTOR_H__

/** \file
 * Generic paint selector widget
 *
 * Copyright (C) Lauris 2002
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
	GtkWidget *none, *solid, *gradient, *radial, *pattern, *unset;

	GtkWidget *fillrulebox;
	GtkWidget *evenodd, *nonzero;

	GtkWidget *frame, *selector;

	SPColor color;
	float alpha;
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

void sp_paint_selector_set_mode (SPPaintSelector *psel, SPPaintSelectorMode mode);
void sp_paint_selector_set_fillrule (SPPaintSelector *psel, SPPaintSelectorFillRule fillrule);

void sp_paint_selector_set_color_alpha (SPPaintSelector *psel, const SPColor *color, float alpha);

void sp_paint_selector_set_gradient_linear (SPPaintSelector *psel, SPGradient *vector);

void sp_paint_selector_set_gradient_radial (SPPaintSelector *psel, SPGradient *vector);

void sp_paint_selector_set_gradient_properties (SPPaintSelector *psel, SPGradientUnits units, SPGradientSpread spread);
void sp_paint_selector_get_gradient_properties (SPPaintSelector *psel, SPGradientUnits *units, SPGradientSpread *spread);

void sp_gradient_selector_attrs_to_gradient (SPGradient *gr, SPPaintSelector *psel);

void sp_paint_selector_get_color_alpha (SPPaintSelector *psel, SPColor *color, gfloat *alpha);

SPGradient *sp_paint_selector_get_gradient_vector (SPPaintSelector *psel);

void sp_paint_selector_system_color_set (SPPaintSelector *psel, const SPColor *color, float opacity);

SPPattern * sp_paint_selector_get_pattern (SPPaintSelector *psel);

void sp_update_pattern_list ( SPPaintSelector *psel, SPPattern *pat);

void sp_paint_selector_set_flat_color (SPPaintSelector *psel, SPDesktop *desktop, const gchar *color_property, const gchar *opacity_property);

SPPaintSelectorMode sp_style_determine_paint_selector_mode (SPStyle *style, bool isfill);

#endif
