/**
 * \file
 * \brief  D2<Interval> specialization to Rect
 */
/*
 * Copyright 2007 Michael Sloan <mgsloan@gmail.com>
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

/* Authors of original rect class:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   bulia byak <buliabyak@users.sf.net>
 *   MenTaLguY <mental@rydia.net>
 */

#include <2geom/d2.h>

#ifndef _2GEOM_RECT
#define _2GEOM_RECT

#include <2geom/matrix.h>
#include <boost/optional/optional.hpp>

namespace Geom {
/** D2<Interval> specialization to Rect */
typedef D2<Interval> Rect;
class OptRect;

Rect unify(const Rect &, const Rect &);
/**
 * %Rect class.
 * The Rect class is actually a specialisation of D2<Interval>.
 * 
 */
template<>
class D2<Interval> {
  private:
    Interval f[2];
  public:
    /** Best not to use this constructor, do not rely on what it initializes the object to.
     *The default constructor creates a rect of default intervals.
     */
    D2<Interval>() { f[X] = f[Y] = Interval(); }
    
    public:
    D2<Interval>(Interval const &a, Interval const &b) {
        f[X] = a;
        f[Y] = b;
    }

    D2<Interval>(Point const & a, Point const & b) {
        f[X] = Interval(a[X], b[X]);
        f[Y] = Interval(a[Y], b[Y]);
    }

    inline Interval& operator[](unsigned i)              { return f[i]; }
    inline Interval const & operator[](unsigned i) const { return f[i]; }

    inline Point min() const { return Point(f[X].min(), f[Y].min()); }
    inline Point max() const { return Point(f[X].max(), f[Y].max()); }

    /** Returns the four corners of the rectangle in positive order
     *  (clockwise if +Y is up, anticlockwise if +Y is down) */
    Point corner(unsigned i) const {
        switch(i % 4) {
            case 0:  return Point(f[X].min(), f[Y].min());
            case 1:  return Point(f[X].max(), f[Y].min());
            case 2:  return Point(f[X].max(), f[Y].max());
            default: return Point(f[X].min(), f[Y].max());
        }
    }
        
    //We should probably remove these - they're coord sys gnostic
    inline double top() const { return f[Y].min(); }
    inline double bottom() const { return f[Y].max(); }
    inline double left() const { return f[X].min(); }
    inline double right() const { return f[X].max(); }
    
    inline double width() const { return f[X].extent(); }
    inline double height() const { return f[Y].extent(); }

    /** Returns a vector from min to max. */
    inline Point dimensions() const { return Point(f[X].extent(), f[Y].extent()); }
    inline Point midpoint() const { return Point(f[X].middle(), f[Y].middle()); }

/**
 * \brief Compute the area of this rectangle.
 *
 * Note that a zero area rectangle is not empty - just as the interval [0,0] contains one point, the rectangle [0,0] x [0,0] contains 1 point and no area.
 * \retval For a valid return value, the rect must be tested for emptyness first.
 */
    inline double area() const { return f[X].extent() * f[Y].extent(); }
    inline bool hasZeroArea(double eps = EPSILON) const { return (area() <= eps); }

    inline double maxExtent() const { return std::max(f[X].extent(), f[Y].extent()); }
    inline double minExtent() const { return std::min(f[X].extent(), f[Y].extent()); }

//    inline bool isEmpty()                 const { 
//        return f[X].isEmpty()        || f[Y].isEmpty(); 
//    }
    inline bool intersects(Rect const &r) const { 
        return f[X].intersects(r[X]) && f[Y].intersects(r[Y]); 
    }
    inline bool contains(Rect const &r)   const { 
        return f[X].contains(r[X]) && f[Y].contains(r[Y]); 
    }
    inline bool contains(Point const &p)  const {
        return f[X].contains(p[X]) && f[Y].contains(p[Y]);
    }

    inline void expandTo(Point p)        { 
        f[X].extendTo(p[X]);  f[Y].extendTo(p[Y]); 
    }
    inline void unionWith(Rect const &b) { 
        f[X].unionWith(b[X]); f[Y].unionWith(b[Y]); 
    }
    void unionWith(OptRect const &b);

    inline void expandBy(double amnt)    {
        f[X].expandBy(amnt);  f[Y].expandBy(amnt); 
    }
    inline void expandBy(Point const p)  { 
        f[X].expandBy(p[X]);  f[Y].expandBy(p[Y]); 
    }
};

inline Rect unify(Rect const & a, Rect const & b) {
    return Rect(unify(a[X], b[X]), unify(a[Y], b[Y]));
}

inline Rect union_list(std::vector<Rect> const &r) {
    if(r.empty()) return Rect(Interval(0,0), Interval(0,0));
    Rect ret = r[0];
    for(unsigned i = 1; i < r.size(); i++)
        ret.unionWith(r[i]);
    return ret;
}

inline
double distanceSq( Point const& p, Rect const& rect )
{
    double dx = 0, dy = 0;
    if ( p[X] < rect.left() )
    {
        dx = p[X] - rect.left();
    }
    else if ( p[X] > rect.right() )
    {
        dx = rect.right() - p[X];
    }
    if ( p[Y] < rect.top() )
    {
        dy = rect.top() - p[Y];
    }
    else if (  p[Y] > rect.bottom() )
    {
        dy = p[Y] - rect.bottom();
    }
    return dx*dx + dy*dy;
}

/**
 * Returns the smallest distance between p and rect.
 */
inline 
double distance( Point const& p, Rect const& rect )
{
    return std::sqrt(distanceSq(p, rect));
}

/**
 * The OptRect class can represent and empty Rect and non-empty Rects.
 * If OptRect is not empty, it means that both X and Y intervals are not empty.
 * 
 */
class OptRect : public boost::optional<Rect> {
public:
    OptRect() : boost::optional<Rect>() {};
    OptRect(Rect const &a) : boost::optional<Rect>(a) {};

    /**
     * Creates an empty OptRect when one of the argument intervals is empty.
     */
    OptRect(OptInterval const &x_int, OptInterval const &y_int) {
        if (x_int && y_int) {
            *this = Rect(*x_int, *y_int);
        }
        // else, stay empty.
    }

    /**
     * Check whether this OptRect is empty or not.
     */
    inline bool isEmpty() const { return (*this == false); };

    /**
     * If \c this is empty, copy argument \c b. Otherwise, union with it (and do nothing when \c b is empty)
     */
    inline void unionWith(OptRect const &b) {
        if (b) {
            if (*this) { // check that we are not empty
                (**this)[X].unionWith((*b)[X]);
                (**this)[Y].unionWith((*b)[Y]);
            } else {
                *this = b;
            }
        }
    }
};


/** 
 * Returns the smallest rectangle that encloses both rectangles.
 * An empty argument is assumed to be an empty rectangle
 */
inline OptRect unify(OptRect const & a, OptRect const & b) {
    if (!a) {
        return b;
    } else if (!b) {
        return a;
    } else {
        return unify(*a, *b);
    }
}

inline OptRect intersect(Rect const & a, Rect const & b) {
    return OptRect(intersect(a[X], b[X]), intersect(a[Y], b[Y]));
}

inline void Rect::unionWith(OptRect const &b) { 
    if (b) {
        unionWith(*b);
    }
}

} // end namespace Geom

#endif //_2GEOM_RECT

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
