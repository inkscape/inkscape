/**
 * \file
 * \brief  Infinite Straight Ray
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

#ifndef _2GEOM_RAY_H_
#define _2GEOM_RAY_H_

#include <2geom/point.h>
#include <2geom/bezier-curve.h> // for LineSegment
#include <2geom/exception.h>

#include <vector>


namespace Geom
{

class Ray
{
public:
	Ray()
		: m_origin(0,0), m_versor(1,0)
	{
	}

	Ray(Point const& _origin, Coord angle )
		: m_origin(_origin), m_versor(std::cos(angle), std::sin(angle))
	{
	}

	Ray(Point const& A, Point const& B)
	{
		setBy2Points(A, B);
	}

	Point origin() const
	{
		return m_origin;
	}

	Point versor() const
	{
		return m_versor;
	}

	void origin(Point const& _point)
	{
		m_origin = _point;
	}

	void versor(Point const& _versor)
	{
		m_versor = _versor;
	}

	Coord angle() const
	{
		double a = std::atan2(m_versor[Y], m_versor[X]);
		if (a < 0) a += 2*M_PI;
		return a;
	}

	void angle(Coord _angle)
	{
		m_versor[X] = std::cos(_angle);
		m_versor[Y] = std::sin(_angle);
	}

	void setBy2Points(Point const& A, Point const& B)
	{
		m_origin = A;
		m_versor = B - A;
		if ( are_near(m_versor, Point(0,0)) )
			m_versor = Point(0,0);
		else
			m_versor.normalize();
	}

	bool isDegenerate() const
	{
		return ( m_versor[X] == 0 && m_versor[Y] == 0 );
	}

	Point pointAt(Coord t) const
	{
		if (t < 0)	THROW_RANGEERROR("Ray::pointAt, negative t value passed");
		return m_origin + m_versor * t;
	}

	Coord valueAt(Coord t, Dim2 d) const
	{
		if (t < 0)
			THROW_RANGEERROR("Ray::valueAt, negative t value passed");
		if (d < 0 || d > 1)
			THROW_RANGEERROR("Ray::valueAt, dimension argument out of range");
		return m_origin[d] + m_versor[d] * t;
	}

	std::vector<Coord> roots(Coord v, Dim2 d) const
	{
		if (d < 0 || d > 1)
			THROW_RANGEERROR("Ray::roots, dimension argument out of range");
		std::vector<Coord> result;
		if ( m_versor[d] != 0 )
		{
			double t = (v - m_origin[d]) / m_versor[d];
			if (t >= 0)	result.push_back(t);
		}
		// TODO: else ?
		return result;
	}

	// require are_near(_point, *this)
	// on the contrary the result value is meaningless
	Coord timeAt(Point const& _point) const
	{
		Coord t;
		if ( m_versor[X] != 0 )
		{
			t = (_point[X] - m_origin[X]) / m_versor[X];
		}
		else if ( m_versor[Y] != 0 )
		{
			t = (_point[Y] - m_origin[Y]) / m_versor[Y];
		}
		else // degenerate case
		{
			t = 0;
		}
		return t;
	}

	Coord nearestPoint(Point const& _point) const
	{
		if ( isDegenerate() ) return 0;
		double t = dot( _point - m_origin, m_versor );
		if (t < 0) t = 0;
		return t;
	}

	Ray reverse() const
	{
		Ray result;
		result.origin(m_origin);
		result.versor(-m_versor);
		return result;
	}

	Curve* portion(Coord  f, Coord t) const
	{
		LineSegment* seg = new LineSegment(pointAt(f), pointAt(t));
		return seg;
	}

	LineSegment segment(Coord  f, Coord t) const
	{
		return LineSegment(pointAt(f), pointAt(t));
	}

	Ray transformed(Matrix const& m) const
	{
		return Ray(m_origin * m, (m_origin + m_versor) * m);
	}

private:
	Point m_origin;
	Point m_versor;

};  // end class ray

inline
double distance(Point const& _point, Ray const& _ray)
{
	double t = _ray.nearestPoint(_point);
	return distance(_point, _ray.pointAt(t));
}

inline
bool are_near(Point const& _point, Ray const& _ray, double eps = EPSILON)
{
	return are_near(distance(_point, _ray), 0, eps);
}

inline
bool are_same(Ray const& r1, Ray const& r2, double eps = EPSILON)
{
	return are_near(r1.versor(), r2.versor(), eps)
			&& are_near(r1.origin(), r2.origin(), eps);
}

// evaluate the angle between r1 and r2 rotating r1 in cw or ccw direction on r2
// the returned value is an angle in the interval [0, 2PI[
inline
double angle_between(Ray const& r1, Ray const& r2, bool cw = true)
{
	double angle = angle_between(r1.versor(), r2.versor());
	if (angle < 0) angle += 2*M_PI;
	if (!cw) angle = 2*M_PI - angle;
	return angle;
}


inline
Ray make_angle_bisector_ray(Ray const& r1, Ray const& r2)
{
    if ( !are_near(r1.origin(), r2.origin()) )
    {
        THROW_RANGEERROR("passed rays have not the same origin");
    }

    Point M = middle_point(r1.pointAt(1), r2.pointAt(1) );
    if (angle_between(r1, r2) > M_PI)  M = 2 * r1.origin() - M;
    return Ray(r1.origin(), M);
}


}  // end namespace Geom



#endif /*_2GEOM_RAY_H_*/


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
