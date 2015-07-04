/**
 * @file
 * @brief  Path sink for Cairo contexts
 *//*
 * Copyright 2014 Krzysztof Kosiński
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, output to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 */

#include <cairo.h>
#include <2geom/cairo-path-sink.h>
#include <2geom/elliptical-arc.h>

namespace Geom {

CairoPathSink::CairoPathSink(cairo_t *cr)
    : _cr(cr)
{}

void CairoPathSink::moveTo(Point const &p)
{
    cairo_move_to(_cr, p[X], p[Y]);
    _current_point = p;
}

void CairoPathSink::lineTo(Point const &p)
{
    cairo_line_to(_cr, p[X], p[Y]);
    _current_point = p;
}

void CairoPathSink::curveTo(Point const &p1, Point const &p2, Point const &p3)
{
    cairo_curve_to(_cr, p1[X], p1[Y], p2[X], p2[Y], p3[X], p3[Y]);
    _current_point = p3;
}

void CairoPathSink::quadTo(Point const &p1, Point const &p2)
{
    // degree-elevate to cubic Bezier, since Cairo doesn't do quad Beziers
    // google "Bezier degree elevation" for more info
    Point q1 = (1./3.) * _current_point + (2./3.) * p1;
    Point q2 = (2./3.) * p1 + (1./3.) * p2;
    // q3 = p2
    cairo_curve_to(_cr, q1[X], q1[Y], q2[X], q2[Y], p2[X], p2[Y]);
    _current_point = p2;
}

void CairoPathSink::arcTo(double rx, double ry, double angle,
                          bool large_arc, bool sweep, Point const &p)
{
    EllipticalArc arc(_current_point, rx, ry, angle, large_arc, sweep, p);
    // Cairo only does circular arcs.
    // To do elliptical arcs, we must use a temporary transform.
    Affine uct = arc.unitCircleTransform();

    // TODO move Cairo-2Geom matrix conversion into a common location
    cairo_matrix_t cm;
    cm.xx = uct[0];
    cm.xy = uct[2];
    cm.x0 = uct[4];
    cm.yx = uct[1];
    cm.yy = uct[3];
    cm.y0 = uct[5];

    cairo_save(_cr);
    cairo_transform(_cr, &cm);
    if (sweep) {
        cairo_arc(_cr, 0, 0, 1, arc.initialAngle(), arc.finalAngle());
    } else {
        cairo_arc_negative(_cr, 0, 0, 1, arc.initialAngle(), arc.finalAngle());
    }
    _current_point = p;
    cairo_restore(_cr);

    /* Note that an extra linear segment will be inserted before the arc
     * if Cairo considers the current point distinct from the initial point
     * of the arc; we could partially alleviate this by not emitting
     * linear segments that are followed by arc segments, but this would require
     * buffering the input curves. */
}
           
void CairoPathSink::closePath()
{
    cairo_close_path(_cr);
}

void CairoPathSink::flush() {}

} // namespace Geom

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
