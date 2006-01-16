#ifndef __SP_BEZIER_UTILS_H__
#define __SP_BEZIER_UTILS_H__

/*
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 *
 * Authors:
 *   Philip J. Schneider
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 1990 Philip J. Schneider
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 *
 * Released under GNU GPL
 */

#include <libnr/nr-forward.h>
#include <glib/gtypes.h>

/* Bezier approximation utils */

gint sp_bezier_fit_cubic(NR::Point bezier[], NR::Point const data[], gint len, gdouble error);

gint sp_bezier_fit_cubic_r(NR::Point bezier[], NR::Point const data[], gint len, gdouble error,
                           unsigned max_beziers);

gint sp_bezier_fit_cubic_full(NR::Point bezier[], int split_points[], NR::Point const data[], gint len,
                              NR::Point const &tHat1, NR::Point const &tHat2,
                              gdouble error, unsigned max_beziers);

NR::Point sp_darray_left_tangent(NR::Point const d[], unsigned const len);
NR::Point sp_darray_left_tangent(NR::Point const d[], unsigned const len, double const tolerance_sq);
NR::Point sp_darray_right_tangent(NR::Point const d[], unsigned const length, double const tolerance_sq);


#endif /* __SP_BEZIER_UTILS_H__ */

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
