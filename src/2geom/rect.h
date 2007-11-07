/*
 * rect.h - D2<Interval> specialization to Rect
 *
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

#ifdef _2GEOM_D2  /*This is intentional: we don't actually want anyone to
                    include this, other than D2.h.  If somone else tries, D2
                    won't be defined.  If it is, this will already be included. */
#ifndef _2GEOM_RECT
#define _2GEOM_RECT

#include "matrix.h"
#include <boost/optional/optional.hpp>

namespace Geom {

typedef D2<Interval> Rect;

Rect unify(const Rect &, const Rect &);

template<>
class D2<Interval> {
  private:
    Interval f[2];
  public:
    D2<Interval>() { f[X] = f[Y] = Interval(0, 0); }
    
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

    /** returns the four corners of the rectangle in positive order
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

    /** returns a vector from min to max. */
    inline Point dimensions() const { return Point(f[X].extent(), f[Y].extent()); }
    inline Point midpoint() const { return Point(f[X].middle(), f[Y].middle()); }

    inline double area() const { return f[X].extent() * f[Y].extent(); }
    inline double maxExtent() const { return std::max(f[X].extent(), f[Y].extent()); }

    inline bool isEmpty()                 const { 
	return f[X].isEmpty()        && f[Y].isEmpty(); 
    }
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

    inline void expandBy(double amnt)    { 
	f[X].expandBy(amnt);  f[Y].expandBy(amnt); 
    }
    inline void expandBy(Point const p)  { 
	f[X].expandBy(p[X]);  f[Y].expandBy(p[Y]); 
    }

    /** Transforms the rect by m. Note that it gives correct results only for scales and translates,
        in the case of rotations, the area of the rect will grow as it cannot rotate. */
    inline Rect operator*(Matrix const m) const { 
        return unify(Rect(corner(0) * m, corner(2) * m),
                     Rect(corner(1) * m, corner(3) * m));
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

inline boost::optional<Rect> intersect(Rect const & a, Rect const & b) {
    boost::optional<Interval> x = intersect(a[X], b[X]);
    boost::optional<Interval> y = intersect(a[Y], b[Y]);
    return x && y ? boost::optional<Rect>(Rect(*x, *y)) : boost::optional<Rect>();
}

}

#endif //_2GEOM_RECT
#endif //_2GEOM_D2
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
