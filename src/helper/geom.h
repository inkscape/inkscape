#ifndef INKSCAPE_HELPER_GEOM_H
#define INKSCAPE_HELPER_GEOM_H

/**
 * Specific geometry functions for Inkscape, not provided my lib2geom.
 *
 * Author:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL
 */

#include <2geom/forward.h>
#include <libnr/nr-forward.h>
#include <libnr/nr-coord.h>

Geom::Rect bounds_fast_transformed(Geom::PathVector const & pv, Geom::Matrix const & t);
Geom::Rect bounds_exact_transformed(Geom::PathVector const & pv, Geom::Matrix const & t);

void pathv_matrix_point_bbox_wind_distance ( Geom::PathVector const & pathv, NR::Matrix const &m, NR::Point const &pt,
                                             NR::Rect *bbox, int *wind, NR::Coord *dist,
                                             NR::Coord tolerance, NR::Rect const *viewbox) __attribute__ ((deprecated));
void pathv_matrix_point_bbox_wind_distance ( Geom::PathVector const & pathv, Geom::Matrix const &m, Geom::Point const &pt,
                                             Geom::Rect *bbox, int *wind, Geom::Coord *dist,
                                             Geom::Coord tolerance, Geom::Rect const *viewbox);

Geom::PathVector pathv_to_linear_and_cubic_beziers( Geom::PathVector const &pathv );

#endif  // INKSCAPE_HELPER_GEOM_H

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
