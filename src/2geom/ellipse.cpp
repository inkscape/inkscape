/*
 * Ellipse Curve
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


#include <2geom/ellipse.h>
#include <2geom/svg-elliptical-arc.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>


namespace Geom
{

void Ellipse::set(double A, double B, double C, double D, double E, double F)
{
    double den = 4*A*C - B*B;
    if ( den == 0 )
    {
        THROW_LOGICALERROR("den == 0, while computing ellipse centre");
    }
    m_centre[X] = (B*E - 2*C*D) / den;
    m_centre[Y] = (B*D - 2*A*E) / den;

    // evaluate the a coefficient of the ellipse equation in normal form
    // E(x,y) = a*(x-cx)^2 + b*(x-cx)*(y-cy) + c*(y-cy)^2 = 1
    // where b = a*B , c = a*C, (cx,cy) == centre
    double num =   A * sqr(m_centre[X])
                 + B * m_centre[X] * m_centre[Y]
                 + C * sqr(m_centre[Y])
                 - A * F;


    //evaluate ellipse rotation angle
    double rot = std::atan2( -B, -(A - C) )/2;
//      std::cerr << "rot = " << rot << std::endl;
    bool swap_axes = false;
    if ( are_near(rot, 0) ) rot = 0;
    if ( are_near(rot, M_PI/2)  || rot < 0 )
    {
        swap_axes = true;
    }

    // evaluate the length of the ellipse rays
    double cosrot = std::cos(rot);
    double sinrot = std::sin(rot);
    double cos2 = cosrot * cosrot;
    double sin2 = sinrot * sinrot;
    double cossin = cosrot * sinrot;

    den = A * cos2 + B * cossin + C * sin2;
    if ( den == 0 )
    {
        THROW_LOGICALERROR("den == 0, while computing 'rx' coefficient");
    }
    double rx2 =  num/den;
    if ( rx2 < 0 )
    {
        THROW_LOGICALERROR("rx2 < 0, while computing 'rx' coefficient");
    }
    double rx = std::sqrt(rx2);

    den = C * cos2 - B * cossin + A * sin2;
    if ( den == 0 )
    {
        THROW_LOGICALERROR("den == 0, while computing 'ry' coefficient");
    }
    double ry2 =  num/den;
    if ( ry2 < 0 )
    {
        THROW_LOGICALERROR("ry2 < 0, while computing 'rx' coefficient");
    }
    double ry = std::sqrt(ry2);

    // the solution is not unique so we choose always the ellipse
    // with a rotation angle between 0 and PI/2
    if ( swap_axes ) std::swap(rx, ry);
    if (    are_near(rot,  M_PI/2)
         || are_near(rot, -M_PI/2)
         || are_near(rx, ry)       )
    {
        rot = 0;
    }
    else if ( rot < 0 )
    {
        rot += M_PI/2;
    }

    m_ray[X] = rx;
    m_ray[Y] = ry;
    m_angle = rot;
}


void Ellipse::set(std::vector<Point> const& points)
{
    size_t sz = points.size();
    if (sz < 5)
    {
        THROW_RANGEERROR("fitting error: too few points passed");
    }
    NL::LFMEllipse model;
    NL::least_squeares_fitter<NL::LFMEllipse> fitter(model, sz);

    for (size_t i = 0; i < sz; ++i)
    {
        fitter.append(points[i]);
    }
    fitter.update();

    NL::Vector z(sz, 0.0);
    model.instance(*this, fitter.result(z));
}


SVGEllipticalArc
Ellipse::arc(Point const& initial, Point const& inner, Point const& final,
             bool _svg_compliant)
{
    Point sp_cp = initial - center();
    Point ep_cp = final   - center();
    Point ip_cp = inner   - center();

    double angle1 = angle_between(sp_cp, ep_cp);
    double angle2 = angle_between(sp_cp, ip_cp);
    double angle3 = angle_between(ip_cp, ep_cp);

    bool large_arc_flag = true;
    bool sweep_flag = true;

    if ( angle1 > 0 )
    {
        if ( angle2 > 0 && angle3 > 0 )
        {
            large_arc_flag = false;
            sweep_flag = true;
        }
        else
        {
            large_arc_flag = true;
            sweep_flag = false;
        }
    }
    else
    {
        if ( angle2 < 0 && angle3 < 0 )
        {
            large_arc_flag = false;
            sweep_flag = false;
        }
        else
        {
            large_arc_flag = true;
            sweep_flag = true;
        }
    }

    SVGEllipticalArc ea( initial, ray(X), ray(Y), rot_angle(),
                      large_arc_flag, sweep_flag, final, _svg_compliant);
    return ea;
}


}  // end namespace Geom


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


