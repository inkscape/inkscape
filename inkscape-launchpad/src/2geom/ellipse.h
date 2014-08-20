/**
 * \file
 * \brief  Ellipse Curve
 *
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


#ifndef _2GEOM_ELLIPSE_H_
#define _2GEOM_ELLIPSE_H_

#include <vector>
#include <2geom/point.h>
#include <2geom/exception.h>
#include <2geom/affine.h>

namespace Geom
{

class EllipticalArc;
class Circle;

class Ellipse
{
  public:
    Ellipse():
       m_centre(),
       m_ray(),
       m_angle(0)
    {}

    Ellipse(double cx, double cy, double rx, double ry, double a)
        : m_centre(cx, cy), m_ray(rx, ry), m_angle(a)
    {
    }

    // build an ellipse by its implicit equation:
    // Ax^2 + Bxy + Cy^2 + Dx + Ey + F = 0
    Ellipse(double A, double B, double C, double D, double E, double F)
    {
        set(A, B, C, D, E, F);
    }

    Ellipse(std::vector<Point> const& points)
    {
        set(points);
    }
    
    Ellipse(Geom::Circle const &c);

    void set(double cx, double cy, double rx, double ry, double a)
    {
        m_centre[X] = cx;
        m_centre[Y] = cy;
        m_ray[X] = rx;
        m_ray[Y] = ry;
        m_angle = a;
    }

    // build an ellipse by its implicit equation:
    // Ax^2 + Bxy + Cy^2 + Dx + Ey + F = 0
    void set(double A, double B, double C, double D, double E, double F);

    // biuld up the best fitting ellipse wrt the passed points
    // prerequisite: at least 5 points must be passed
    void set(std::vector<Point> const& points);

    EllipticalArc *
    arc(Point const& initial, Point const& inner, Point const& final, bool svg_compliant = true);

    Point center() const
    {
        return m_centre;
    }

    Coord center(Dim2 d) const
    {
        return m_centre[d];
    }

    Coord ray(Dim2 d) const
    {
        return m_ray[d];
    }

    Coord rot_angle() const
    {
        return m_angle;
    }

    std::vector<double> implicit_form_coefficients() const;

    Ellipse transformed(Affine const& m) const;

  private:
    Point m_centre, m_ray;
    double m_angle;
};


} // end namespace Geom



#endif // _2GEOM_ELLIPSE_H_


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
