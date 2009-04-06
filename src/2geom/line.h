/**
 * \file
 * \brief  Infinite Straight Line
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

#ifndef _2GEOM_LINE_H_
#define _2GEOM_LINE_H_


#include <cmath>

#include <2geom/bezier-curve.h> // for LineSegment
#include <2geom/crossing.h>
#include <2geom/exception.h>

#include <2geom/ray.h>


namespace Geom
{

class Line
{
  public:
	Line()
		: m_origin(0,0), m_versor(1,0)
	{
	}

	Line(Point const& _origin, Coord angle )
		: m_origin(_origin), m_versor(std::cos(angle), std::sin(angle))
	{
	}

	Line(Point const& A, Point const& B)
	{
		setBy2Points(A, B);
	}

	explicit
	Line(LineSegment const& _segment)
	{
		setBy2Points(_segment.initialPoint(), _segment.finalPoint());
	}

	explicit
	Line(Ray const& _ray)
		: m_origin(_ray.origin()), m_versor(_ray.versor())
	{
	}
    
    static Line fromNormalDistance(Point n, double c) {
        Point P = n*c/(dot(n,n));
    
        return Line(P, P+rot90(n));
    }
    static Line fromPointDirection(Point o, Point v) {
        Line l;
        l.m_origin = o;
        l.m_versor = v;
        return l;
    }

	Line* duplicate() const
	{
		return new Line(*this);
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

	// return the angle described by rotating the X-axis in cw direction
	// until it overlaps the line
	// the returned value is in the interval [0, PI[
	Coord angle() const
	{
		double a = std::atan2(m_versor[Y], m_versor[X]);
		if (a < 0) a += M_PI;
		if (a == M_PI) a = 0;
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
		return m_origin + m_versor * t;
	}

	Coord valueAt(Coord t, Dim2 d) const
	{
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
			result.push_back( (v - m_origin[d]) / m_versor[d] );
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

	Coord timeAtProjection(Point const& _point) const
	{
		if ( isDegenerate() ) return 0;
		return dot( _point - m_origin, m_versor );
	}

	Coord nearestPoint(Point const& _point) const
	{
		return timeAtProjection(_point);
	}

	Line reverse() const
	{
		Line result;
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

	Ray ray(Coord t)
	{
		Ray result;
		result.origin(pointAt(t));
		result.versor(m_versor);
		return result;
	}

	Line derivative() const
	{
		Line result;
		result.origin(m_versor);
		result.versor(Point(0,0));
		return result;
	}

	Line transformed(Matrix const& m) const
	{
		return Line(m_origin * m, (m_origin + m_versor) * m);
	}
    
    static Line from_normal_and_dist(Point const &n, double d) {
        return Line(n*d, n*d + rot90(n));
    }

  private:
	Point m_origin;
	Point m_versor;

}; // end class Line


inline
double distance(Point const& _point, Line const& _line)
{
	if ( _line.isDegenerate() )
	{
		return distance( _point, _line.origin() );
	}
	else
	{
		return fabs( dot(_point - _line.origin(), _line.versor().ccw()) );
	}
}

inline
bool are_near(Point const& _point, Line const& _line, double eps = EPSILON)
{
	return are_near(distance(_point, _line), 0, eps);
}

inline
bool are_parallel(Line const& l1, Line const& l2, double eps = EPSILON)
{
	return ( are_near(l1.versor(), l2.versor(), eps)
			 || are_near(l1.versor(), -l2.versor(), eps) );
}

inline
bool are_same(Line const& l1, Line const& l2, double eps = EPSILON)
{
	return are_parallel(l1, l2, eps) && are_near(l1.origin(), l2, eps);
}

inline
bool are_orthogonal(Line const& l1, Line const& l2, double eps = EPSILON)
{
	return ( are_near(l1.versor(), l2.versor().cw(), eps)
			 || are_near(l1.versor(), l2.versor().ccw(), eps) );
}

inline
bool are_collinear(Point const& p1, Point const& p2, Point const& p3,
		           double eps = EPSILON)
{
	return are_near( cross(p3, p2) - cross(p3, p1) + cross(p2, p1), 0, eps);
}

// evaluate the angle between l1 and l2 rotating l1 in cw direction
// until it overlaps l2
// the returned value is an angle in the interval [0, PI[
inline
double angle_between(Line const& l1, Line const& l2)
{
	double angle = angle_between(l1.versor(), l2.versor());
	if (angle < 0) angle += M_PI;
	if (angle == M_PI) angle = 0;
	return angle;
}

inline
double distance(Point const& _point, LineSegment const& _segment)
{
    double t = _segment.nearestPoint(_point);
    return L2(_point - _segment.pointAt(t));
}

inline
bool are_near(Point const& _point, LineSegment const& _segment,
		      double eps = EPSILON)
{
	return are_near(distance(_point, _segment), 0, eps);
}

// build a line passing by _point and orthogonal to _line
inline
Line make_orthogonal_line(Point const& _point, Line const& _line)
{
	Line l;
	l.origin(_point);
	l.versor(_line.versor().cw());
	return l;
}

// build a line passing by _point and parallel to _line
inline
Line make_parallel_line(Point const& _point, Line const& _line)
{
	Line l(_line);
	l.origin(_point);
	return l;
}

// build a line passing by the middle point of _segment and orthogonal to it.
inline
Line make_bisector_line(LineSegment const& _segment)
{
	return make_orthogonal_line( middle_point(_segment), Line(_segment) );
}

// build the bisector line of the angle between ray(O,A) and ray(O,B)
inline
Line make_angle_bisector_line(Point const& A, Point const& O, Point const& B)
{
	Point M = middle_point(A,B);
	return Line(O,M);
}

// prj(P) = rot(v, Point( rot(-v, P-O)[X], 0 )) + O
inline
Point projection(Point const& _point, Line const& _line)
{
	return _line.pointAt( _line.nearestPoint(_point) );
}

inline
LineSegment projection(LineSegment const& _segment, Line const& _line)
{
	return _line.segment( _line.nearestPoint(_segment.initialPoint()),
					      _line.nearestPoint(_segment.finalPoint()) );
}




namespace detail
{

OptCrossing intersection_impl(Ray const& r1, Line const& l2, unsigned int i);
OptCrossing intersection_impl( LineSegment const& ls1,
                             Line const& l2,
                             unsigned int i );
OptCrossing intersection_impl( LineSegment const& ls1,
                             Ray const& r2,
                             unsigned int i );
}


inline
OptCrossing intersection(Ray const& r1, Line const& l2)
{
    return detail::intersection_impl(r1,  l2, 0);

}

inline
OptCrossing intersection(Line const& l1, Ray const& r2)
{
    return detail::intersection_impl(r2,  l1, 1);
}

inline
OptCrossing intersection(LineSegment const& ls1, Line const& l2)
{
    return detail::intersection_impl(ls1,  l2, 0);
}

inline
OptCrossing intersection(Line const& l1, LineSegment const& ls2)
{
    return detail::intersection_impl(ls2,  l1, 1);
}

inline
OptCrossing intersection(LineSegment const& ls1, Ray const& r2)
{
    return detail::intersection_impl(ls1,  r2, 0);

}

inline
OptCrossing intersection(Ray const& r1, LineSegment const& ls2)
{
    return detail::intersection_impl(ls2,  r1, 1);
}


OptCrossing intersection(Line const& l1, Line const& l2);

OptCrossing intersection(Ray const& r1, Ray const& r2);

OptCrossing intersection(LineSegment const& ls1, LineSegment const& ls2);


} // end namespace Geom


#endif // _2GEOM_LINE_H_


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
