#ifndef SEEN_SP_GRADIENT_CHEMISTRY_H
#define SEEN_SP_GRADIENT_CHEMISTRY_H

/*
 * Various utility methods for gradients
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Authors
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-gradient.h"

class SPCSSAttr;
class SPItem;
typedef unsigned int guint32;

/**
 * Either normalizes given gradient to vector, or returns fresh normalized
 * vector - in latter case, original gradient is flattened and stops cleared.
 * No transrefing - i.e. caller shouldn't hold reference to original and
 * does not get one to new automatically (doc holds ref of every object anyways)
 */
SPGradient *sp_gradient_ensure_vector_normalized(SPGradient *gradient);

 
/**
 * Sets item fill or stroke to the gradient of the specified type with given vector, creating
 * new private gradient, if needed.
 * gr has to be a normalized vector.
 */
SPGradient *sp_item_set_gradient(SPItem *item, SPGradient *gr, SPGradientType type, Inkscape::PaintTarget fill_or_stroke);

/*
 * Get default normalized gradient vector of document, create if there is none
 */
SPGradient *sp_document_default_gradient_vector( SPDocument *document, SPColor const &color, bool singleStop );

/**
 * Return the preferred vector for \a o, made from (in order of preference) its current vector,
 * current fill or stroke color, or from desktop style if \a o is NULL or doesn't have style.
 */
SPGradient *sp_gradient_vector_for_object( SPDocument *doc, SPDesktop *desktop, SPObject *o, Inkscape::PaintTarget fill_or_stroke, bool singleStop = false );

void sp_object_ensure_fill_gradient_normalized (SPObject *object);
void sp_object_ensure_stroke_gradient_normalized (SPObject *object);

SPGradient *sp_gradient_convert_to_userspace (SPGradient *gr, SPItem *item, const char *property);
SPGradient *sp_gradient_reset_to_userspace (SPGradient *gr, SPItem *item);

SPGradient *sp_gradient_fork_vector_if_necessary (SPGradient *gr);
SPGradient *sp_gradient_get_forked_vector_if_necessary(SPGradient *gradient, bool force_vector);


SPStop* sp_last_stop(SPGradient *gradient);
SPStop* sp_get_stop_i(SPGradient *gradient, unsigned int i);
unsigned int sp_number_of_stops(SPGradient const *gradient);
unsigned int sp_number_of_stops_before_stop(SPGradient const *gradient, SPStop *target);

guint32 average_color(guint32 c1, guint32 c2, double p = 0.5);

SPStop *sp_vector_add_stop(SPGradient *vector, SPStop* prev_stop, SPStop* next_stop, gfloat offset);

void sp_gradient_transform_multiply(SPGradient *gradient, Geom::Affine postmul, bool set);

void sp_gradient_reverse_selected_gradients(SPDesktop *desktop);

void sp_gradient_invert_selected_gradients(SPDesktop *desktop, Inkscape::PaintTarget fill_or_stroke);

void sp_gradient_unset_swatch(SPDesktop *desktop, std::string id);

/**
 * Fetches either the fill or the stroke gradient from the given item.
 *
 * @param fill_or_stroke the gradient type (fill/stroke) to get.
 * @return the specified gradient if set, null otherwise.
 */
SPGradient *getGradient(SPItem *item, Inkscape::PaintTarget fill_or_stroke);


void sp_item_gradient_set_coords(SPItem *item, GrPointType point_type, unsigned int point_i, Geom::Point p_desk, Inkscape::PaintTarget fill_or_stroke, bool write_repr, bool scale);

/**
 * Returns the position of point point_type of the gradient applied to item (either fill_or_stroke),
 * in desktop coordinates.
*/
Geom::Point getGradientCoords(SPItem *item, GrPointType point_type, unsigned int point_i, Inkscape::PaintTarget fill_or_stroke);

SPGradient *sp_item_gradient_get_vector(SPItem *item, Inkscape::PaintTarget fill_or_stroke);
SPGradientSpread sp_item_gradient_get_spread(SPItem *item, Inkscape::PaintTarget fill_or_stroke);

void sp_item_gradient_stop_set_style(SPItem *item, GrPointType point_type, unsigned int point_i, Inkscape::PaintTarget fill_or_stroke, SPCSSAttr *stop);
guint32 sp_item_gradient_stop_query_style(SPItem *item, GrPointType point_type, unsigned int point_i, Inkscape::PaintTarget fill_or_stroke);
void sp_item_gradient_edit_stop(SPItem *item, GrPointType point_type, unsigned int point_i, Inkscape::PaintTarget fill_or_stroke);
void sp_item_gradient_reverse_vector(SPItem *item, Inkscape::PaintTarget fill_or_stroke);
void sp_item_gradient_invert_vector_color(SPItem *item, Inkscape::PaintTarget fill_or_stroke);

#endif // SEEN_SP_GRADIENT_CHEMISTRY_H

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
