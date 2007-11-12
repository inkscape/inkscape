#ifndef SEEN_Geom_POINT_H
#define SEEN_Geom_POINT_H

/** \file
 * Cartesian point class.
 */

#include <iostream>

#include "coord.h"
#include "utils.h"

namespace Geom {

enum Dim2 { X=0, Y=1 };

class Matrix;

/// Cartesian point.
class Point {
    Coord _pt[2];

  public:
    inline Point()
    { _pt[X] = _pt[Y] = 0; }

    inline Point(Coord x, Coord y) {
        _pt[X] = x; _pt[Y] = y;
    }

    inline Point(Point const &p) {
        for (unsigned i = 0; i < 2; ++i)
            _pt[i] = p._pt[i];
    }

    inline Point &operator=(Point const &p) {
        for (unsigned i = 0; i < 2; ++i)
            _pt[i] = p._pt[i];
        return *this;
    }

    inline Coord operator[](unsigned i) const { return _pt[i]; }
    inline Coord &operator[](unsigned i) { return _pt[i]; }

    Coord operator[](Dim2 d) const throw() { return _pt[d]; }
    Coord &operator[](Dim2 d) throw() { return _pt[d]; }

    static inline Point polar(Coord angle, Coord radius) {
        return Point(radius * std::cos(angle), radius * std::sin(angle));
    }

    inline Coord length() const { return hypot(_pt[0], _pt[1]); }

    /** Return a point like this point but rotated -90 degrees.
        (If the y axis grows downwards and the x axis grows to the
        right, then this is 90 degrees counter-clockwise.)
    **/
    Point ccw() const {
        return Point(_pt[Y], -_pt[X]);
    }

    /** Return a point like this point but rotated +90 degrees.
        (If the y axis grows downwards and the x axis grows to the
        right, then this is 90 degrees clockwise.)
    **/
    Point cw() const {
        return Point(-_pt[Y], _pt[X]);
    }

    /**
        \brief A function to lower the precision of the point
        \param  places  The number of decimal places that should be in
                        the final number.
    */
    inline void round (int places = 0) {
        _pt[X] = (Coord)(decimal_round((double)_pt[X], places));
        _pt[Y] = (Coord)(decimal_round((double)_pt[Y], places));
        return;
    }

    void normalize();

    inline Point operator+(Point const &o) const {
        return Point(_pt[X] + o._pt[X], _pt[Y] + o._pt[Y]);
    }
    inline Point operator-(Point const &o) const {
        return Point(_pt[X] - o._pt[X], _pt[Y] - o._pt[Y]);
    }
    inline Point &operator+=(Point const &o) {
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            _pt[i] += o._pt[i];
        }
        return *this;
    }  
    inline Point &operator-=(Point const &o) {
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            _pt[i] -= o._pt[i];
        }
        return *this;
    }

    inline Point operator-() const {
        return Point(-_pt[X], -_pt[Y]);
    }
    inline Point operator*(double const s) const {
        return Point(_pt[X] * s, _pt[Y] * s);
    }
    inline Point operator/(double const s) const {
        //TODO: s == 0?
        return Point(_pt[X] / s, _pt[Y] / s);
    }
    inline Point &operator*=(double const s) {
        for ( unsigned i = 0 ; i < 2 ; ++i ) _pt[i] *= s;
        return *this;
    }
    inline Point &operator/=(double const s) {
        //TODO: s == 0?
        for ( unsigned i = 0 ; i < 2 ; ++i ) _pt[i] /= s;
        return *this;
    }

    Point &operator*=(Matrix const &m);

    inline int operator == (const Point &in_pnt) {
        return ((_pt[X] == in_pnt[X]) && (_pt[Y] == in_pnt[Y]));
    }

    friend inline std::ostream &operator<< (std::ostream &out_file, const Geom::Point &in_pnt);
};

inline Point operator*(double const s, Point const &p) { return p * s; }

/** A function to print out the Point.  It just prints out the coords
    on the given output stream */
inline std::ostream &operator<< (std::ostream &out_file, const Geom::Point &in_pnt) {
    out_file << "X: " << in_pnt[X] << "  Y: " << in_pnt[Y];
    return out_file;
}

/** This is a rotation (sort of). */
inline Point operator^(Point const &a, Point const &b) {
    Point const ret(a[0] * b[0] - a[1] * b[1],
                    a[1] * b[0] + a[0] * b[1]);
    return ret;
}

//IMPL: boost::EqualityComparableConcept
inline bool operator==(Point const &a, Point const &b) {
    return (a[X] == b[X]) && (a[Y] == b[Y]);
}
inline bool operator!=(Point const &a, Point const &b) {
    return (a[X] != b[X]) || (a[Y] != b[Y]);
}

/** This is a lexicographical ordering for points.  It is remarkably useful for sweepline algorithms*/
inline bool operator<=(Point const &a, Point const &b) {
    return ( ( a[Y] < b[Y] ) ||
             (( a[Y] == b[Y] ) && ( a[X] < b[X] )));
}

Coord L1(Point const &p);

/** Compute the L2, or euclidean, norm of \a p. */
inline Coord L2(Point const &p) { return p.length(); }

/** Compute the square of L2 norm of \a p. Warning: this can overflow where L2 won't.*/
inline Coord L2sq(Point const &p) { return p[0]*p[0] + p[1]*p[1]; }

double LInfty(Point const &p);
bool is_zero(Point const &p);
bool is_unit_vector(Point const &p);

extern double atan2(Point const p);
/** compute the angle turning from a to b (signed). */
extern double angle_between(Point const a, Point const b);

//IMPL: NearConcept
inline bool are_near(Point const &a, Point const &b, double const eps=EPSILON) {
    return ( are_near(a[X],b[X],eps) && are_near(a[Y],b[Y],eps) );
}

/** Returns p * Geom::rotate_degrees(90), but more efficient.
 *
 * Angle direction in Inkscape code: If you use the traditional mathematics convention that y
 * increases upwards, then positive angles are anticlockwise as per the mathematics convention.  If
 * you take the common non-mathematical convention that y increases downwards, then positive angles
 * are clockwise, as is common outside of mathematics.
 *
 * There is no rot_neg90 function: use -rot90(p) instead.
 */
inline Point rot90(Point const &p) { return Point(-p[Y], p[X]); }

/** Given two points and a parameter t \in [0, 1], return a point
 * proportionally from a to b by t.  Akin to 1 degree bezier.*/
inline Point lerp(double const t, Point const a, Point const b) { return (a * (1 - t) + b * t); }

Point unit_vector(Point const &a);

/** compute the dot product (inner product) between the vectors a and b. */
inline Coord dot(Point const &a, Point const &b) { return a[0] * b[0] + a[1] * b[1]; }
/** Defined as dot(a, b.cw()). */
inline Coord cross(Point const &a, Point const &b) { return dot(a, b.cw()); }

/** compute the euclidean distance between points a and b.  TODO: hypot safer/faster? */
inline Coord distance (Point const &a, Point const &b) { return L2(a - b); }

/** compute the square of the distance between points a and b. */
inline Coord distanceSq (Point const &a, Point const &b) { return L2sq(a - b); }

Point abs(Point const &b);

Point operator*(Point const &v, Matrix const &m);

Point operator/(Point const &p, Matrix const &m);

} /* namespace Geom */

#endif /* !SEEN_Geom_POINT_H */

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
