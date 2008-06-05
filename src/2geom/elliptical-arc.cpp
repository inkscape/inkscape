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


#include "elliptical-arc.h"
#include "bezier-curve.h"
#include "poly.h"

#include <cfloat>
#include <limits>




namespace Geom
{


Rect EllipticalArc::boundsExact() const
{
	std::vector<double> extremes(4);
	double cosrot = std::cos(rotation_angle());
	double sinrot = std::sin(rotation_angle());
	extremes[0] = std::atan2( -ray(Y) * sinrot, ray(X) * cosrot );
	extremes[1] = extremes[0] + M_PI;
	if ( extremes[0] < 0 ) extremes[0] += 2*M_PI;	
	extremes[2] = std::atan2( ray(Y) * cosrot, ray(X) * sinrot );
	extremes[3] = extremes[2] + M_PI;
	if ( extremes[2] < 0 ) extremes[2] += 2*M_PI;
	
	
	std::vector<double>arc_extremes(4);
	arc_extremes[0] = initialPoint()[X];
	arc_extremes[1] = finalPoint()[X];
	if ( arc_extremes[0] < arc_extremes[1] ) 
		std::swap(arc_extremes[0], arc_extremes[1]);
	arc_extremes[2] = initialPoint()[Y];
	arc_extremes[3] = finalPoint()[Y];
	if ( arc_extremes[2] < arc_extremes[3] ) 
		std::swap(arc_extremes[2], arc_extremes[3]);
	
	
	if ( start_angle() < end_angle() )
	{
		if ( sweep_flag() )
		{
			for ( unsigned int i = 0; i < extremes.size(); ++i )
			{
				if ( start_angle() < extremes[i] && extremes[i] < end_angle() )
				{
					arc_extremes[i] = pointAtAngle(extremes[i])[i >> 1];
				}
			}
		}
		else
		{
			for ( unsigned int i = 0; i < extremes.size(); ++i )
			{
				if ( start_angle() > extremes[i] || extremes[i] > end_angle() )
				{
					arc_extremes[i] = pointAtAngle(extremes[i])[i >> 1];
				}
			}
		}
	}
	else
	{
		if ( sweep_flag() )
		{
			for ( unsigned int i = 0; i < extremes.size(); ++i )
			{
				if ( start_angle() < extremes[i] || extremes[i] < end_angle() )
				{
					arc_extremes[i] = pointAtAngle(extremes[i])[i >> 1];
				}
			}
		}
		else
		{
			for ( unsigned int i = 0; i < extremes.size(); ++i )
			{
				if ( start_angle() > extremes[i] && extremes[i] > end_angle() )
				{
					arc_extremes[i] = pointAtAngle(extremes[i])[i >> 1];
				}
			}		
		}
	}
	
	return Rect( Point(arc_extremes[1], arc_extremes[3]) , 
			     Point(arc_extremes[0], arc_extremes[2]) );

}


std::vector<double> 
EllipticalArc::roots(double v, Dim2 d) const
{
	if ( d > Y )
	{
		THROW_RANGEERROR("dimention out of range");
	}
	
	std::vector<double> sol;
	if ( are_near(ray(X), 0) && are_near(ray(Y), 0) )
	{
		if ( center(d) == v )
			sol.push_back(0);
		return sol;
	}
	
	const char* msg[2][2] = 
	{
		{ "d == X; ray(X) == 0; "
		  "s = (v - center(X)) / ( -ray(Y) * std::sin(rotation_angle()) ); "
		  "s should be contained in [-1,1]",
		  "d == X; ray(Y) == 0; "
		  "s = (v - center(X)) / ( ray(X) * std::cos(rotation_angle()) ); "
		  "s should be contained in [-1,1]"
		},
		{ "d == Y; ray(X) == 0; "
		  "s = (v - center(X)) / ( ray(Y) * std::cos(rotation_angle()) ); "
		  "s should be contained in [-1,1]",
		  "d == Y; ray(Y) == 0; "
		  "s = (v - center(X)) / ( ray(X) * std::sin(rotation_angle()) ); "
		  "s should be contained in [-1,1]"
		},
	};	  
	
	for ( unsigned int dim = 0; dim < 2; ++dim )
	{
		if ( are_near(ray(dim), 0) )
		{
			
			if ( initialPoint()[d] == v && finalPoint()[d] == v )
			{
				THROW_INFINITESOLUTIONS(0);
			}
			if ( (initialPoint()[d] < finalPoint()[d])
				 && (initialPoint()[d] > v || finalPoint()[d] < v) )
			{
				return sol;
			}
			if ( (initialPoint()[d] > finalPoint()[d])
				 && (finalPoint()[d] > v || initialPoint()[d] < v) )
			{
				return sol;
			}
			double ray_prj;
			switch(d)
			{
				case X:		
					switch(dim)
					{	
						case X: ray_prj = -ray(Y) * std::sin(rotation_angle());
								break;
						case Y: ray_prj = ray(X) * std::cos(rotation_angle());
								break;
					}
					break;
				case Y:
					switch(dim)
					{	
						case X: ray_prj = ray(Y) * std::cos(rotation_angle());
								break;
						case Y: ray_prj = ray(X) * std::sin(rotation_angle());
								break;
					}
					break;
			}
			
			double s = (v - center(d)) / ray_prj;
			if ( s < -1 || s > 1 )
			{
				THROW_LOGICALERROR(msg[d][dim]);
			}
			switch(dim)
			{	
				case X: 
					s = std::asin(s); // return a value in [-PI/2,PI/2]
					if ( logical_xor( sweep_flag(), are_near(start_angle(), M_PI/2) )  )
					{
						if ( s < 0 ) s += 2*M_PI;
					}
					else
					{
						s = M_PI - s;
						if (!(s < 2*M_PI) ) s -= 2*M_PI;
					}
					break;
				case Y: 
					s = std::acos(s); // return a value in [0,PI]
					if ( logical_xor( sweep_flag(), are_near(start_angle(), 0) ) )
					{
						s = 2*M_PI - s;
						if ( !(s < 2*M_PI) ) s -= 2*M_PI;
					}
					break;
			}
			
			//std::cerr << "s = " << rad_to_deg(s);
			s = map_to_01(s);
			//std::cerr << " -> t: " << s << std::endl;
			if ( !(s < 0 || s > 1) )
				sol.push_back(s);
			return sol;
		}
	}
		
	double rotx, roty;
	switch(d)
	{
		case X: 
			rotx = std::cos(rotation_angle());
			roty = -std::sin(rotation_angle());
			break;
		case Y:
			rotx = std::sin(rotation_angle());
			roty = std::cos(rotation_angle());
			break;
	}
	double rxrotx = ray(X) * rotx;
	double c_v = center(d) - v;

	double a = -rxrotx + c_v;
	double b = ray(Y) * roty;
	double c = rxrotx + c_v;
	//std::cerr << "a = " << a << std::endl;
	//std::cerr << "b = " << b << std::endl;
	//std::cerr << "c = " << c << std::endl;
	
	if ( are_near(a,0) )
	{
		sol.push_back(M_PI);
		if ( !are_near(b,0) )
		{
			double s = 2 * std::atan(-c/(2*b));
			if ( s < 0 ) s += 2*M_PI;
			sol.push_back(s);
		}
	}
	else
	{
		double delta = b * b - a * c;
		//std::cerr << "delta = " << delta << std::endl;
		if ( are_near(delta, 0) )
		{
			double s = 2 * std::atan(-b/a);
			if ( s < 0 ) s += 2*M_PI;
			sol.push_back(s);
		}
		else if ( delta > 0 )
		{		
			double sq = std::sqrt(delta);
			double s = 2 * std::atan( (-b - sq) / a );
			if ( s < 0 ) s += 2*M_PI;
			sol.push_back(s);
			s = 2 * std::atan( (-b + sq) / a );
			if ( s < 0 ) s += 2*M_PI;
			sol.push_back(s);
		}
	}
	
	std::vector<double> arc_sol;
	for (unsigned int i = 0; i < sol.size(); ++i )
	{
		//std::cerr << "s = " << rad_to_deg(sol[i]);
		sol[i] = map_to_01(sol[i]);
		//std::cerr << " -> t: " << sol[i] << std::endl;
		if ( !(sol[i] < 0 || sol[i] > 1) )
			arc_sol.push_back(sol[i]);
	}
	return arc_sol;
	
	
//	return SBasisCurve(toSBasis()).roots(v, d);
}

// D(E(t,C),t) = E(t+PI/2,O)
Curve* EllipticalArc::derivative() const
{
	EllipticalArc* result = new EllipticalArc(*this);
	result->m_center[X] = result->m_center[Y] = 0;
	result->m_start_angle += M_PI/2;
	if( !( result->m_start_angle < 2*M_PI ) )
	{
		result->m_start_angle -= 2*M_PI;
	}
	result->m_end_angle += M_PI/2;
	if( !( result->m_end_angle < 2*M_PI ) )
	{
		result->m_end_angle -= 2*M_PI;
	}
	result->m_initial_point = result->pointAtAngle( result->start_angle() );
	result->m_final_point = result->pointAtAngle( result->end_angle() );
	return result;
	
}

std::vector<Point> 
EllipticalArc::pointAndDerivatives(Coord t, unsigned int n) const
{
	std::vector<Point> result;
	result.reserve(n);
	double angle = map_unit_interval_on_circular_arc(t, start_angle(), 
			                                         end_angle(), sweep_flag());
	EllipticalArc ea(*this);
	ea.m_center = Point(0,0);
	unsigned int m = std::min(n, 4u);
	for ( unsigned int i = 0; i < m; ++i )
	{
		result.push_back( ea.pointAtAngle(angle) );
		angle += M_PI/2;
		if ( !(angle < 2*M_PI) ) angle -= 2*M_PI;
	}
	m = n / 4;
	for ( unsigned int i = 1; i < m; ++i )
	{
		for ( unsigned int j = 0; j < 4; ++j )
			result.push_back( result[j] );
	}
	m = n - 4 * m;
	for ( unsigned int i = 0; i < m; ++i )
	{
		result.push_back( result[i] );
	}
	if ( !result.empty() ) // n != 0
		result[0] = pointAtAngle(angle);
	return result;
}

D2<SBasis> EllipticalArc::toSBasis() const
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


bool EllipticalArc::containsAngle(Coord angle) const
{
	if ( sweep_flag() )
		if ( start_angle() < end_angle() )
			return ( !( angle < start_angle() || angle > end_angle() ) );
		else
			return ( !( angle < start_angle() && angle > end_angle() ) );
	else
		if ( start_angle() > end_angle() )
			return ( !( angle > start_angle() || angle < end_angle() ) );
		else
			return ( !( angle > start_angle() && angle < end_angle() ) );
}


double EllipticalArc::valueAtAngle(Coord t, Dim2 d) const
{
    double sin_rot_angle = std::sin(rotation_angle());
    double cos_rot_angle = std::cos(rotation_angle());
    if ( d == X )
    {
        return    ray(X) * cos_rot_angle * std::cos(t) 
                - ray(Y) * sin_rot_angle * std::sin(t) 
                + center(X);
    }
    else if ( d == Y )
    {
        return    ray(X) * sin_rot_angle * std::cos(t) 
                + ray(Y) * cos_rot_angle * std::sin(t) 
                + center(Y);
    }
    THROW_RANGEERROR("dimension parameter out of range");
}


Curve* EllipticalArc::portion(double f, double t) const 
{
	if (f < 0) f = 0;
	if (f > 1) f = 1;
	if (t < 0) t = 0;
	if (t > 1) t = 1;
	if ( are_near(f, t) )
	{
		EllipticalArc* arc = new EllipticalArc();
		arc->m_center = arc->m_initial_point = arc->m_final_point = pointAt(f);
		arc->m_start_angle = arc->m_end_angle = m_start_angle;
		arc->m_rot_angle = m_rot_angle;
		arc->m_sweep = m_sweep;
		arc->m_large_arc = m_large_arc;
	}
    EllipticalArc* arc = new EllipticalArc( *this );
    arc->m_initial_point = pointAt(f);
    arc->m_final_point = pointAt(t);
    double sa = sweep_flag() ? sweep_angle() : -sweep_angle();
    arc->m_start_angle = m_start_angle + sa * f;
    if ( !(arc->m_start_angle < 2*M_PI) )
        arc->m_start_angle -= 2*M_PI;
    if ( arc->m_start_angle < 0 )
    	arc->m_start_angle += 2*M_PI;
    arc->m_end_angle = m_start_angle + sa * t;
    if ( !(arc->m_end_angle < 2*M_PI) )
        arc->m_end_angle -= 2*M_PI;
    if ( arc->m_end_angle < 0 )
    	arc->m_end_angle += 2*M_PI;
    if ( f > t ) arc->m_sweep = !sweep_flag();
    if ( large_arc_flag() && (arc->sweep_angle() < M_PI) )
        arc->m_large_arc = false;
    return arc;
}

// NOTE: doesn't work with 360 deg arcs
void EllipticalArc::calculate_center_and_extreme_angles()
{
    if ( are_near(initialPoint(), finalPoint()) )
    {
    	if ( are_near(ray(X), 0) && are_near(ray(Y), 0) )
    	{
    		m_start_angle = m_end_angle = 0;
    		m_center = initialPoint();
    		return;
    	}
    	else
    	{
    		THROW_RANGEERROR("initial and final point are the same");
    	}
    }
	if ( are_near(ray(X), 0) && are_near(ray(Y), 0) )
	{ // but initialPoint != finalPoint
		THROW_RANGEERROR(
			"there is no ellipse that satisfies the given constraints: "
			"ray(X) == 0 && ray(Y) == 0 but initialPoint != finalPoint"
		);
	}
	if ( are_near(ray(Y), 0) )
	{
		Point v = initialPoint() - finalPoint();
		if ( are_near(L2sq(v), 4*ray(X)*ray(X)) )
		{
			double angle = std::atan2(v[Y], v[X]);
			if (angle < 0) angle += 2*M_PI;
			if ( are_near( angle, rotation_angle() ) )
			{
				m_start_angle = 0;
				m_end_angle = M_PI;
				m_center = v/2 + finalPoint();
				return;
			}
			angle -= M_PI;
			if ( angle < 0 ) angle += 2*M_PI;
			if ( are_near( angle, rotation_angle() ) )
			{
				m_start_angle = M_PI;
				m_end_angle = 0;
				m_center = v/2 + finalPoint();
				return;
			}
			THROW_RANGEERROR(
				"there is no ellipse that satisfies the given constraints: "
				"ray(Y) == 0 "
				"and slope(initialPoint - finalPoint) != rotation_angle "
				"and != rotation_angle + PI"
			);
		}
		if ( L2sq(v) > 4*ray(X)*ray(X) )
		{
			THROW_RANGEERROR(
				"there is no ellipse that satisfies the given constraints: "
				"ray(Y) == 0 and distance(initialPoint, finalPoint) > 2*ray(X)"
			);
		}
		else
		{
			THROW_RANGEERROR(
				"there is infinite ellipses that satisfy the given constraints: "
				"ray(Y) == 0  and distance(initialPoint, finalPoint) < 2*ray(X)"
			);
		}
		
	}
	
	if ( are_near(ray(X), 0) )
	{
		Point v = initialPoint() - finalPoint();
		if ( are_near(L2sq(v), 4*ray(Y)*ray(Y)) )
		{
			double angle = std::atan2(v[Y], v[X]);
			if (angle < 0) angle += 2*M_PI;
			double rot_angle = rotation_angle() + M_PI/2;
			if ( !(rot_angle < 2*M_PI) ) rot_angle -= 2*M_PI;
			if ( are_near( angle, rot_angle ) )
			{
				m_start_angle = M_PI/2;
				m_end_angle = 3*M_PI/2;
				m_center = v/2 + finalPoint();
				return;
			}
			angle -= M_PI;
			if ( angle < 0 ) angle += 2*M_PI;
			if ( are_near( angle, rot_angle ) )
			{
				m_start_angle = 3*M_PI/2;
				m_end_angle = M_PI/2;
				m_center = v/2 + finalPoint();
				return;
			}
			THROW_RANGEERROR(
				"there is no ellipse that satisfies the given constraints: "
				"ray(X) == 0 "
				"and slope(initialPoint - finalPoint) != rotation_angle + PI/2 "
				"and != rotation_angle + (3/2)*PI"
			);
		}
		if ( L2sq(v) > 4*ray(Y)*ray(Y) )
		{
			THROW_RANGEERROR(
				"there is no ellipse that satisfies the given constraints: "
				"ray(X) == 0 and distance(initialPoint, finalPoint) > 2*ray(Y)"
			);
		}
		else
		{
			THROW_RANGEERROR(
				"there is infinite ellipses that satisfy the given constraints: "
				"ray(X) == 0  and distance(initialPoint, finalPoint) < 2*ray(Y)"
			);
		}
		
	}
	
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
        		THROW_RANGEERROR(
        			"there is no ellipse that satisfies the given constraints"
        		);
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
        		THROW_RANGEERROR(
        			"there is no ellipse that satisfies the given constraints"
        		);
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

Coord EllipticalArc::map_to_02PI(Coord t) const
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

Coord EllipticalArc::map_to_01(Coord angle) const 
{
	return map_circular_arc_on_unit_interval(angle, start_angle(), 
			                                 end_angle(), sweep_flag());
}


std::vector<double> EllipticalArc::
allNearestPoints( Point const& p, double from, double to ) const
{
	if ( from > to ) std::swap(from, to);
	if ( from < 0 || to > 1 )
	{
		THROW_RANGEERROR("[from,to] interval out of range");
	}
	std::vector<double> result;
	if ( ( are_near(ray(X), 0) && are_near(ray(Y), 0) )  || are_near(from, to) )
	{
		result.push_back(from);
		return result;
	}
	else if ( are_near(ray(X), 0) || are_near(ray(Y), 0) )
	{
		LineSegment seg(pointAt(from), pointAt(to));
		Point np = seg.pointAt( seg.nearestPoint(p) );
		if ( are_near(ray(Y), 0) )
		{
			if ( are_near(rotation_angle(), M_PI/2) 
				 || are_near(rotation_angle(), 3*M_PI/2) )
			{
				result = roots(np[Y], Y);
			}
			else
			{
				result = roots(np[X], X);
			}
		}
		else
		{
			if ( are_near(rotation_angle(), M_PI/2) 
				 || are_near(rotation_angle(), 3*M_PI/2) )
			{
				result = roots(np[X], X);
			}
			else
			{
				result = roots(np[Y], Y);
			}
		}
		return result;
	}
	else if ( are_near(ray(X), ray(Y)) )
	{
		Point r = p - center();
		if ( are_near(r, Point(0,0)) )
		{
			THROW_INFINITESOLUTIONS(0);
		}
		// TODO: implement case r != 0
//		Point np = ray(X) * unit_vector(r);
//		std::vector<double> solX = roots(np[X],X);
//		std::vector<double> solY = roots(np[Y],Y);
//		double t;
//		if ( are_near(solX[0], solY[0]) || are_near(solX[0], solY[1]))
//		{
//			t = solX[0];
//		}
//		else
//		{
//			t = solX[1];
//		}
//		if ( !(t < from || t > to) )
//		{
//			result.push_back(t);
//		}
//		else
//		{
//			
//		}
	}
	
	// solve the equation <D(E(t),t)|E(t)-p> == 0
	// that provides min and max distance points 
	// on the ellipse E wrt the point p
	// after the substitutions: 
	// cos(t) = (1 - s^2) / (1 + s^2)
	// sin(t) = 2t / (1 + s^2)
	// where s = tan(t/2)
	// we get a 4th degree equation in s
	/*
	 *	ry s^4 ((-cy + py) Cos[Phi] + (cx - px) Sin[Phi]) + 
	 *	ry ((cy - py) Cos[Phi] + (-cx + px) Sin[Phi]) + 
	 *	2 s^3 (rx^2 - ry^2 + (-cx + px) rx Cos[Phi] + (-cy + py) rx Sin[Phi]) + 
	 *	2 s (-rx^2 + ry^2 + (-cx + px) rx Cos[Phi] + (-cy + py) rx Sin[Phi])
	 */

	Point p_c = p - center();
	double rx2_ry2 = (ray(X) - ray(Y)) * (ray(X) + ray(Y));
	double cosrot = std::cos( rotation_angle() );
	double sinrot = std::sin( rotation_angle() );
	double expr1 = ray(X) * (p_c[X] * cosrot + p_c[Y] * sinrot);
	Poly coeff;
	coeff.resize(5);
	coeff[4] = ray(Y) * ( p_c[Y] * cosrot - p_c[X] * sinrot );
	coeff[3] = 2 * ( rx2_ry2 + expr1 );
	coeff[2] = 0;
	coeff[1] = 2 * ( -rx2_ry2 + expr1 );
	coeff[0] = -coeff[4];
	
//	for ( unsigned int i = 0; i < 5; ++i )
//		std::cerr << "c[" << i << "] = " << coeff[i] << std::endl;
	
	std::vector<double> real_sol;
	// gsl_poly_complex_solve raises an error 
	// if the leading coefficient is zero
	if ( are_near(coeff[4], 0) )  
	{
		real_sol.push_back(0);
		if ( !are_near(coeff[3], 0) )
		{
			double sq = -coeff[1] / coeff[3];
			if ( sq > 0 )
			{
				double s = std::sqrt(sq);
				real_sol.push_back(s);
				real_sol.push_back(-s);
			}
		}
	}
	else
	{
		real_sol = solve_reals(coeff);
	}
//	else
//	{
//		double sol[8];
//		gsl_poly_complex_workspace * w = gsl_poly_complex_workspace_alloc(5);
//		gsl_poly_complex_solve(coeff, 5, w, sol );
//		gsl_poly_complex_workspace_free(w);
//		
//		for ( unsigned int i = 0; i < 4; ++i )
//		{
//			if ( sol[2*i+1] == 0 ) real_sol.push_back(sol[2*i]);
//		}
//	}
		
	for ( unsigned int i = 0; i < real_sol.size(); ++i )
	{
		real_sol[i] = 2 * std::atan(real_sol[i]);
		if ( real_sol[i] < 0 ) real_sol[i] += 2*M_PI;
	}
	// when s -> Infinity then <D(E)|E-p> -> 0 iff coeff[4] == 0
	// so we add M_PI to the solutions being lim arctan(s) = PI when s->Infinity
	if ( (real_sol.size() % 2) != 0 )
	{
		real_sol.push_back(M_PI);
	}
	
	double mindistsq1 = std::numeric_limits<double>::max();
	double mindistsq2 = std::numeric_limits<double>::max();
	double dsq;
	unsigned int mi1, mi2;
	for ( unsigned int i = 0; i < real_sol.size(); ++i )
	{
		dsq = distanceSq(p, pointAtAngle(real_sol[i]));
		if ( mindistsq1 > dsq )
		{
			mindistsq2 = mindistsq1;
			mi2 = mi1;
			mindistsq1 = dsq;
			mi1 = i;
		}
		else if ( mindistsq2 > dsq )
		{
			mindistsq2 = dsq;
			mi2 = i;
		}
	}
	
	double t = map_to_01( real_sol[mi1] );
	if ( !(t < from || t > to) )
	{
		result.push_back(t);
	}
	
	bool second_sol = false; 
	t = map_to_01( real_sol[mi2] );
   	if ( real_sol.size() == 4 && !(t < from || t > to) )
   	{
     	if ( result.empty() || are_near(mindistsq1, mindistsq2) )
    	{
    		result.push_back(t);
    		second_sol = true;
    	}
   	}
	
   	// we need to test extreme points too
	double dsq1 = distanceSq(p, pointAt(from));
	double dsq2 = distanceSq(p, pointAt(to));
	if ( second_sol )
	{
		if ( mindistsq2 > dsq1 )
		{
			result.clear();
			result.push_back(from);
			mindistsq2 = dsq1;
		}
		else if ( are_near(mindistsq2, dsq) )
		{
			result.push_back(from);
		}
		if ( mindistsq2 > dsq2 )
		{
			result.clear();
			result.push_back(to);
		}
		else if ( are_near(mindistsq2, dsq2) )
		{
			result.push_back(to);
		}
		
	}
	else
	{
		if ( result.empty() )
		{
			if ( are_near(dsq1, dsq2) )
			{
				result.push_back(from);
				result.push_back(to);
			}
			else if ( dsq2 > dsq1 )
			{
				result.push_back(from);
			}
			else
			{
				result.push_back(to);
			}
		}
	}
	
	return result;
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


