/*
 * SVG Elliptical Arc Class
 *
 * Copyright 2008  Marco Cecchetti <mrcekets at gmail.com>
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


#include "path.h"


namespace Geom
{

D2<SBasis> SVGEllipticalArc::toSBasis() const
{
    // the interval of parametrization has to be [0,1]
    Coord et = start_angle() + ( sweep_flag() ? sweep_angle() : -sweep_angle() );
    Linear param(start_angle(), et);
    Coord cos_rot_angle = std::cos(rotation_angle());
    Coord sin_rot_angle = std::sin(rotation_angle());
    // order = 4 seems to be enough to get a perfect looking elliptical arc
    // should it be choosen in function of the arc length anyway ?
    // or maybe a user settable parameter: toSBasis(unsigned int order) ?
    SBasis arc_x = ray(X) * cos(param,4);
    SBasis arc_y = ray(Y) * sin(param,4);
    D2<SBasis> arc;
    arc[0] = arc_x * cos_rot_angle - arc_y * sin_rot_angle + Linear(center(X),center(X));
    arc[1] = arc_x * sin_rot_angle + arc_y * cos_rot_angle + Linear(center(Y),center(Y));
    return arc;
}

double SVGEllipticalArc::valueAt(Coord t, Dim2 d) const
{
    Coord tt = from_01_to_02PI(t);
    double sin_rot_angle = std::sin(rotation_angle());
    double cos_rot_angle = std::cos(rotation_angle());
    if ( d == X )
    {
        return    ray(X) * cos_rot_angle * std::cos(tt) 
                - ray(Y) * sin_rot_angle * std::sin(tt) 
                + center(X);
    }
    else
    {
        return    ray(X) * sin_rot_angle * std::cos(tt) 
                + ray(Y) * cos_rot_angle * std::sin(tt) 
                + center(Y);
    }
}


Curve* SVGEllipticalArc::portion(double f, double t) const 
{
	if (f < 0) f = 0;
	if (f > 1) f = 1;
	if (t < 0) t = 0;
	if (t > 1) t = 1;
    SVGEllipticalArc* arc = new SVGEllipticalArc( *this );
    arc->m_initial_point = pointAt(f);
    arc->m_final_point = pointAt(t);
    double sa = sweep_flag() ? sweep_angle() : -sweep_angle();
    arc->m_start_angle = m_start_angle + sa * f;
    if ( !(arc->m_start_angle < 2*M_PI) )
        arc->m_start_angle -= 2*M_PI;
    if ( !(arc->m_start_angle > 0) )
    	arc->m_start_angle += 2*M_PI;
    arc->m_end_angle = m_start_angle + sa * t;
    if ( !(arc->m_end_angle < 2*M_PI) )
        arc->m_end_angle -= 2*M_PI;
    if ( !(arc->m_end_angle > 0) )
    	arc->m_end_angle += 2*M_PI;
    if ( f > t ) arc->m_sweep = !sweep_flag();
    if ( large_arc_flag() && (arc->sweep_angle() < M_PI) )
        arc->m_large_arc = false;
    return arc;
}

// NOTE: doesn't work with 360 deg arcs
void SVGEllipticalArc::calculate_center_and_extreme_angles() throw(RangeError)
{
    double sin_rot_angle = std::sin(rotation_angle());
    double cos_rot_angle = std::cos(rotation_angle());

    Point sp = sweep_flag() ? initialPoint() : finalPoint();
    Point ep = sweep_flag() ? finalPoint() : initialPoint();

    Matrix m( ray(X) * cos_rot_angle, ray(X) * sin_rot_angle,
             -ray(Y) * sin_rot_angle, ray(Y) * cos_rot_angle,
              0,                      0 );
    Matrix im = m.inverse();
    Point sol = (ep - sp) * im;
    double half_sum_angle = std::atan2(-sol[X], sol[Y]);
    double half_diff_angle;
    if ( are_near(std::fabs(half_sum_angle), M_PI/2) )
    {
        double anti_sgn_hsa = (half_sum_angle > 0) ? -1 : 1;
        double arg = anti_sgn_hsa * sol[X] / 2;
        // if |arg| is a little bit > 1 acos returns nan
        if ( are_near(arg, 1) )
            half_diff_angle = 0;
        else if ( are_near(arg, -1) )
            half_diff_angle = M_PI;
        else
        {
        	if ( !(-1 < arg && arg < 1) )
        		throwRangeError("there is no ellipse that satisfies the given "
        						"constraints");
            // assert( -1 < arg && arg < 1 );
            // if it fails 
            // => there is no ellipse that satisfies the given constraints
            half_diff_angle = std::acos( arg );
        }

        half_diff_angle = M_PI/2 - half_diff_angle;
    }
    else
    {
        double  arg = sol[Y] / ( 2 * std::cos(half_sum_angle) );
        // if |arg| is a little bit > 1 asin returns nan
        if ( are_near(arg, 1) ) 
            half_diff_angle = M_PI/2;
        else if ( are_near(arg, -1) )
            half_diff_angle = -M_PI/2;
        else
        {
        	if ( !(-1 < arg && arg < 1) )
        		throwRangeError("there is no ellipse that satisfies the given "
        						"constraints");
            // assert( -1 < arg && arg < 1 );
            // if it fails 
            // => there is no ellipse that satisfies the given constraints
            half_diff_angle = std::asin( arg );
        }
    }

    if (   ( m_large_arc && half_diff_angle > 0 ) 
        || (!m_large_arc && half_diff_angle < 0 ) )
    {
        half_diff_angle = -half_diff_angle;
    }
    if ( half_sum_angle < 0 ) half_sum_angle += 2*M_PI;
    if ( half_diff_angle < 0 ) half_diff_angle += M_PI;
    
    m_start_angle = half_sum_angle - half_diff_angle;
    m_end_angle =  half_sum_angle + half_diff_angle;
    // 0 <= m_start_angle, m_end_angle < 2PI
    if ( m_start_angle < 0 ) m_start_angle += 2*M_PI;
    if( !(m_end_angle < 2*M_PI) ) m_end_angle -= 2*M_PI;
    sol[0] = std::cos(m_start_angle);
    sol[1] = std::sin(m_start_angle);
    m_center = sp - sol * m;
    if ( !sweep_flag() )
    {
        double angle = m_start_angle;
        m_start_angle = m_end_angle;
        m_end_angle = angle;
    }
}

Coord SVGEllipticalArc::from_01_to_02PI(Coord t) const
{
    if ( sweep_flag() )
    {
        Coord angle = start_angle() + sweep_angle() * t;
        if ( !(angle < 2*M_PI) )
            angle -= 2*M_PI;
        return angle;
    }
    else
    {
        Coord angle = start_angle() - sweep_angle() * t;
        if ( angle < 0 ) angle += 2*M_PI;
        return angle;
    }
}


} // end namespace Geom


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


