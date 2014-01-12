/**
 * \file
 * \brief Circles
 *//*
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
 *
 * Copyright 2008  authors
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
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
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
 */

#ifndef LIB2GEOM_SEEN_CIRCLE_H
#define LIB2GEOM_SEEN_CIRCLE_H

#include <vector>
#include <2geom/point.h>
#include <2geom/exception.h>
#include <2geom/path.h>

namespace Geom {

class EllipticalArc;

class Circle
{
  public:
    Circle()
    {}

    Circle(double cx, double cy, double r)
        : m_centre(cx, cy), m_ray(r)
    {
    }

    Circle(Point center, double r)
        : m_centre(center), m_ray(r)
    {
    }

    Circle(double A, double B, double C, double D)
    {
        set(A, B, C, D);
    }

    Circle(std::vector<Point> const& points)
    {
        set(points);
    }

    void set(double cx, double cy, double r)
    {
        m_centre[X] = cx;
        m_centre[Y] = cy;
        m_ray = r;
    }


    // build a circle by its implicit equation:
    // Ax^2 + Ay^2 + Bx + Cy + D = 0
    void set(double A, double B, double C, double D);

    // build up the best fitting circle wrt the passed points
    // prerequisite: at least 3 points must be passed
    void set(std::vector<Point> const& points);

    EllipticalArc *
    arc(Point const& initial, Point const& inner, Point const& final,
        bool _svg_compliant = true);

    D2<SBasis> toSBasis();
    void getPath(std::vector<Path> &path_out);

    Point center() const
    {
        return m_centre;
    }

    Coord center(Dim2 d) const
    {
        return m_centre[d];
    }

    Coord ray() const
    {
        return m_ray;
    }


  private:
    Point m_centre;
    Coord m_ray;
};

} // end namespace Geom

#endif // LIB2GEOM_SEEN_CIRCLE_H

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
