//D2<Interval> specialization to Rect:

 /* Authors of original rect class:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   bulia byak <buliabyak@users.sf.net>
 *   MenTaLguY <mental@rydia.net>
 */

#ifdef _2GEOM_D2  /*This is intentional: we don't actually want anyone to
                    include this, other than D2.h */
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
    D2<Interval>();// { f[X] = f[Y] = Interval(0, 0); }

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

    /** returns the four corners of the rectangle in order
     *  (clockwise if +Y is up, anticlockwise if +Y is down) */
    Point corner(unsigned i) const {
      switch(i % 4) {
        case 0: return Point(f[X].min(), f[Y].min());
        case 1: return Point(f[X].max(), f[Y].min());
        case 2: return Point(f[X].max(), f[Y].max());        case 3: return Point(f[X].min(), f[Y].max());
      }
    }

    inline double width() const { return f[X].extent(); }
    inline double height() const { return f[Y].extent(); }

    /** returns a vector from min to max. */
    inline Point dimensions() const { return Point(f[X].extent(), f[Y].extent()); }
    inline Point midpoint() const { return Point(f[X].middle(), f[Y].middle()); }

    inline double area() const { return f[X].extent() * f[Y].extent(); }
    inline double maxExtent() const { return std::max(f[X].extent(), f[Y].extent()); }

    inline bool isEmpty()                 const { return f[X].isEmpty()        && f[Y].isEmpty(); }
    inline bool intersects(Rect const &r) const { return f[X].intersects(r[X]) && f[Y].intersects(r[Y]); }
    inline bool contains(Rect const &r)   const { return f[X].contains(r[X])   && f[Y].contains(r[Y]); }
    inline bool contains(Point const &p)  const { return f[X].contains(p[X])   && f[Y].contains(p[Y]); }

    inline void expandTo(Point p)        { f[X].extendTo(p[X]);  f[Y].extendTo(p[Y]); }
    inline void unionWith(Rect const &b) { f[X].unionWith(b[X]); f[Y].unionWith(b[Y]); }

    inline void expandBy(double amnt)    { f[X].expandBy(amnt);  f[Y].expandBy(amnt); }
    inline void expandBy(Point const p)  { f[X].expandBy(p[X]);  f[Y].expandBy(p[Y]); }

    /** Transforms the rect by m. Note that it gives correct results only for scales and translates,
        in the case of rotations, the area of the rect will grow as it cannot rotate. */
    inline Rect operator*(Matrix const m) const { return unify(Rect(corner(0) * m, corner(2) * m),
                                                               Rect(corner(1) * m, corner(3) * m)); }
};

inline Rect unify(const Rect & a, const Rect & b) {
    return Rect(unify(a[X], b[X]), unify(a[Y], b[Y]));
}

inline boost::optional<Rect> intersect(const Rect & a, const Rect & b) {
    boost::optional<Interval> x = intersect(a[X], b[X]);
    boost::optional<Interval> y = intersect(a[Y], b[Y]);
    return x && y ? boost::optional<Rect>(Rect(*x, *y)) : boost::optional<Rect>();
}

}

#endif //_2GEOM_RECT
#endif //_2GEOM_D2
