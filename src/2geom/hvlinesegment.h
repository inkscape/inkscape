/*
 * Horizontal and Vertical Line Segment
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


#ifndef _2GEOM_HVLINESEGMENT_H_
#define _2GEOM_HVLINESEGMENT_H_


#include <2geom/bezier-curve.h>


namespace Geom
{

class HLineSegment : public Curve
{
  public:
	HLineSegment()
	{}
	
	HLineSegment(Coord _x0, Coord _x1, Coord _y)
		: m_line_seg(Point(_x0, _y), Point(_x1, _y))
	{
	}
	
	HLineSegment(Point const& _p, double _length)
		: m_line_seg(_p, Point(_p[X] + _length, _p[Y]))
	{
	}
	
	HLineSegment(Point const& _p0, Point const& _p1)
		: m_line_seg(_p0, _p1)
	{
		if ( _p0[Y] != _p1[Y] )
		{
			THROW_RANGEERROR("HLineSegment::HLineSegment passed points should "
					         "have the same Y value");
		}
	}
	
	Curve* duplicate() const
	{
		return new HLineSegment(*this);
	}
	
	bool isDegenerate() const
	{
		return m_line_seg.isDegenerate();
	}
	
	Point initialPoint() const
	{
		return m_line_seg.initialPoint();
	}
	
	Point finalPoint() const 
	{
		return m_line_seg.finalPoint();
	}
	
	Coord getY()
	{
		return initialPoint()[Y];
	}
	
	void setInitial(Point _p) 
	{ 
		m_line_seg.setInitial( Point(_p[X], initialPoint()[Y]) ); 
	}
	
	void setFinal(Point _p) 
	{ 
		m_line_seg.setFinal( Point(_p[X], finalPoint()[Y]) ); 
	}
	
	void setX0(Coord _x)
	{
		m_line_seg.setInitial( Point(_x, initialPoint()[Y]) );
	}
	
	void setX1(Coord _x)
	{
		m_line_seg.setFinal( Point(_x, finalPoint()[Y]) );
	}
	
	void setY(Coord _y)
	{
		m_line_seg.setInitial( Point(initialPoint()[X], _y) );
		m_line_seg.setFinal( Point(finalPoint()[X], _y) );
	}

	Rect boundsFast() const
	{
		return boundsExact();
	}
	
	Rect boundsExact() const
	{
		return Rect( initialPoint(), finalPoint() );
	}
	
	Rect boundsLocal(Interval i, unsigned deg) const
	{
		return m_line_seg.boundsLocal(i, deg);
	}
	
	int winding(Point p) const
	{
		return m_line_seg.winding(p);
	}
	
	std::vector<double>
	roots(double v, Dim2 d) const
	{
		if (d < 0 || d > 1)
		{
			THROW_RANGEERROR("dimension argument out of range");
		}
		std::vector<double> result;
		if (d == X)
		{
			if ( v >= initialPoint()[X] && v <= finalPoint()[X] )
			{
				double t = 0;
				if (!isDegenerate())
					t = (v - initialPoint()[X]) / (finalPoint()[X] - initialPoint()[X]);
				result.push_back(t);
			}
		}
		else
		{
			if (v == initialPoint()[Y])
			{
				if (!isDegenerate())
					THROW_INFINITESOLUTIONS(0);
				result.push_back(0);
			}
		}
		return result;
	}
	
	double nearestPoint( Point const& p, double from = 0, double to = 1 ) const
	{
		if ( from > to ) std::swap(from, to);
		double xfrom = pointAt(from)[X];
		double xto = pointAt(to)[X];
		if ( xfrom > xto )
		{
		    std::swap(xfrom, xto);
		    std::swap(from, to);
		}
		if ( p[X] > xfrom && p[X] < xto )
		{
			return (p[X] - initialPoint()[X]) / (finalPoint()[X] - initialPoint()[X]);
		}
		else if ( p[X] <= xfrom )
			return from;
		else
			return to;
	}
	
	std::pair<HLineSegment, HLineSegment> subdivide(Coord t) const
	{
		std::pair<HLineSegment, HLineSegment> result;
		Point p = pointAt(t);
		result.first.setInitial(initialPoint());
		result.first.setFinal(p);
		result.second.setInitial(p);
		result.second.setFinal(finalPoint());
		return result;
	}
	
	Curve* portion(double f, double t) const
	{
		Point ip = pointAt(f);
		Point ep = pointAt(t);
		return new HLineSegment(ip[X], ep[X], ip[Y]);
	}
	
	Curve* reverse() const
	{
		return 
		new HLineSegment(finalPoint()[X], initialPoint()[X], initialPoint()[Y]);
	}
	
	Curve* transformed(Matrix const & m) const
	{
		Point ip = initialPoint() * m;
		Point ep = finalPoint() * m;
		return new LineSegment(ip, ep);
	}
	
	Curve* derivative() const
	{
		double x = finalPoint()[X] - initialPoint()[X];
		return new HLineSegment(x, x, 0);
	}
	
	Point pointAt(double t) const
	{
		if ( t < 0 || t > 1 )
			THROW_RANGEERROR("domain parameter out of range");
		double x = initialPoint()[X] + t * (finalPoint()[X] - initialPoint()[X]);
		return Point(x, initialPoint()[Y]);
	}
	
	double valueAt(double t, Dim2 d) const
	{
		if (d < 0 || d > 1)
		{
			THROW_RANGEERROR("dimension argument out of range");
		}
		if ( t < 0 || t > 1 )
			THROW_RANGEERROR("domain parameter out of range");
		
		if (d == Y) return initialPoint()[Y];
		
		return initialPoint()[X] + t * (finalPoint()[X] - initialPoint()[X]);
	}

    std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const
    {
        std::vector<Point> result;
        result.push_back(pointAt(t));
        if (n > 0)
        {
            double x = finalPoint()[X] - initialPoint()[X];
            result.push_back( Point(x, 0) );
        }
        if (n > 1)
        {
            /* higher order derivatives are zero,
             * so the other n-1 vector elements are (0,0) */
            result.insert( result.end(), n-1, Point(0, 0) );
        }
        return result;
    }

	D2<SBasis> toSBasis() const
	{
		return m_line_seg.toSBasis();
	}
	
  private:
	LineSegment m_line_seg;
	
};  // end class HLineSegment


class VLineSegment : public Curve
{
  public:
	VLineSegment()
	{}
	
	VLineSegment(Coord _x, Coord _y0, Coord _y1)
		: m_line_seg(Point(_x, _y0), Point(_x, _y1))
	{
	}
	
	VLineSegment(Point const& _p, double _length)
		: m_line_seg(_p, Point(_p[X], _p[Y] + _length))
	{
	}
	
	VLineSegment(Point const& _p0, Point const& _p1)
		: m_line_seg(_p0, _p1)
	{
		if ( _p0[X] != _p1[X] )
		{
			THROW_RANGEERROR("VLineSegment::VLineSegment passed points should "
					         "have the same X value");
		}
	}
	
	Curve* duplicate() const
	{
		return new VLineSegment(*this);
	}
	
	bool isDegenerate() const
	{
		return m_line_seg.isDegenerate();
	}
	
	Point initialPoint() const
	{
		return m_line_seg.initialPoint();
	}
	
	Point finalPoint() const 
	{
		return m_line_seg.finalPoint();
	}
	
	Coord getX()
	{
		return initialPoint()[X];
	}
	
	void setInitial(Point _p) 
	{ 
		m_line_seg.setInitial( Point(initialPoint()[X], _p[Y]) ); 
	}
	
	void setFinal(Point _p) 
	{ 
		m_line_seg.setFinal( Point(finalPoint()[X], _p[Y]) ); 
	}
	
	void setY0(Coord _y)
	{
		m_line_seg.setInitial( Point(initialPoint()[X], _y) );
	}
	
	void setY1(Coord _y)
	{
		m_line_seg.setFinal( Point(finalPoint()[Y], _y) );
	}
	
	void setX(Coord _x)
	{
		m_line_seg.setInitial( Point(_x, initialPoint()[Y]) );
		m_line_seg.setFinal( Point(_x, finalPoint()[Y]) );
	}

	Rect boundsFast() const
	{
		return boundsExact();
	}
	
	Rect boundsExact() const
	{
		return Rect( initialPoint(), finalPoint() );
	}
	
	Rect boundsLocal(Interval i, unsigned deg) const
	{
		return m_line_seg.boundsLocal(i, deg);
	}
	
	int winding(Point p) const
	{
		return m_line_seg.winding(p);
	}
	
	std::vector<double>
	roots(double v, Dim2 d) const
	{
		if (d < 0 || d > 1)
		{
			THROW_RANGEERROR("dimension argument out of range");
		}
		std::vector<double> result;
		if (d == Y)
		{
			if ( v >= initialPoint()[Y] && v <= finalPoint()[Y] )
			{
				double t = 0;
				if (!isDegenerate())
					t = (v - initialPoint()[Y]) / (finalPoint()[Y] - initialPoint()[Y]);
				result.push_back(t);
			}
		}
		else
		{
			if (v == initialPoint()[X])
			{
				if (!isDegenerate())
					THROW_INFINITESOLUTIONS(0);
				result.push_back(0);
			}
		}
		return result;
	}
	
	double nearestPoint( Point const& p, double from = 0, double to = 1 ) const
	{
		if ( from > to ) std::swap(from, to);
		double yfrom = pointAt(from)[Y];
		double yto = pointAt(to)[Y];
		if (yfrom > yto)
		{
		    std::swap(yfrom, yto);
		    std::swap(from, to);
		}
		if ( p[Y] > yfrom && p[Y] < yto )
		{
			return (p[Y] - initialPoint()[Y]) / (finalPoint()[Y] - initialPoint()[Y]);
		}
		else if ( p[Y] <= yfrom )
			return from;
		else
			return to;
	}
	
	std::pair<VLineSegment, VLineSegment> subdivide(Coord t) const
	{
		std::pair<VLineSegment, VLineSegment> result;
		Point p = pointAt(t);
		result.first.setInitial(initialPoint());
		result.first.setFinal(p);
		result.second.setInitial(p);
		result.second.setFinal(finalPoint());
		return result;
	}
	
	Curve* portion(double f, double t) const
	{
		Point ip = pointAt(f);
		Point ep = pointAt(t);
		return new VLineSegment(ip[X], ip[Y], ep[Y]);
	}
	
	Curve* reverse() const
	{
		return 
		new VLineSegment(initialPoint()[X], finalPoint()[Y], initialPoint()[Y]);
	}
	
	Curve* transformed(Matrix const & m) const
	{
		Point ip = initialPoint() * m;
		Point ep = finalPoint() * m;
		return new LineSegment(ip, ep);
	}
	
	Curve* derivative() const
	{
		double y = finalPoint()[Y] - initialPoint()[Y];
		return new VLineSegment(0, y, y);
	}
	
	Point pointAt(double t) const
	{
		if ( t < 0 || t > 1 )
			THROW_RANGEERROR("domain parameter out of range");
		double y = initialPoint()[Y] + t * (finalPoint()[Y] - initialPoint()[Y]);
		return Point(initialPoint()[X], y);
	}
	
	double valueAt(double t, Dim2 d) const
	{
		if (d < 0 || d > 1)
		{
			THROW_RANGEERROR("dimension argument out of range");
		}
		if ( t < 0 || t > 1 )
			THROW_RANGEERROR("domain parameter out of range");
		
		if (d == X) return initialPoint()[X];
		
		return initialPoint()[Y] + t * (finalPoint()[Y] - initialPoint()[Y]);
	}

    std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const
    {
        std::vector<Point> result;
        result.push_back(pointAt(t));
        if (n > 0)
        {
            double y = finalPoint()[Y] - initialPoint()[Y];
            result.push_back( Point(0, y) );
        }
        if (n > 1)
        {
            /* higher order derivatives are zero,
             * so the other n-1 vector elements are (0,0) */
            result.insert( result.end(), n-1, Point(0, 0) );
        }
        return result;
    }

	D2<SBasis> toSBasis() const
	{
		return m_line_seg.toSBasis();
	}
	
  private:
	LineSegment m_line_seg;
	
}; // end class VLineSegment



}  // end namespace Geom


#endif // _2GEOM_HVLINESEGMENT_H_ 


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
