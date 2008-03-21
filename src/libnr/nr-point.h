#ifndef SEEN_NR_POINT_H
#define SEEN_NR_POINT_H

/** \file
 * Cartesian point class.
 */

//#include <math.h>
//#include <stdexcept>
#include <iostream>
//#include <iomanip>

#include <libnr/nr-coord.h>
#include <libnr/nr-dim2.h>

//#include "round.h"
#include "decimal-round.h"

#include <2geom/point.h>

namespace NR {

class Matrix;

/// Cartesian point.
class Point {
public:
    inline Point()
    { _pt[X] = _pt[Y] = 0; }

    inline Point(Coord x, Coord y) {
        _pt[X] = x;
        _pt[Y] = y;
    }

    inline Point(Point const &p) {
        for (unsigned i = 0; i < 2; ++i) {
            _pt[i] = p._pt[i];
        }
    }

    inline Point(Geom::Point const &p) {
        _pt[X] = p[Geom::X];
        _pt[Y] = p[Geom::Y];
    }

    inline Point &operator=(Point const &p) {
        for (unsigned i = 0; i < 2; ++i) {
            _pt[i] = p._pt[i];
        }
        return *this;
    }

    inline Coord operator[](unsigned i) const {
        return _pt[i];
    }

    inline Coord &operator[](unsigned i) {
        return _pt[i];
    }

    Coord operator[](Dim2 d) const throw() { return _pt[d]; }
    Coord &operator[](Dim2 d) throw() { return _pt[d]; }

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
        _pt[X] = (Coord)(Inkscape::decimal_round((double)_pt[X], places));
        _pt[Y] = (Coord)(Inkscape::decimal_round((double)_pt[Y], places));
        return;
    }

    void normalize();

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
  
    inline Point &operator/=(double const s) {
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            _pt[i] /= s;
        }
        return *this;
    }

    inline Point &operator*=(double const s) {
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            _pt[i] *= s;
        }
        return *this;
    }

    Point &operator*=(Matrix const &m);

    inline int operator == (const Point &in_pnt) {
        return ((_pt[X] == in_pnt[X]) && (_pt[Y] == in_pnt[Y]));
    }

    friend inline std::ostream &operator<< (std::ostream &out_file, const NR::Point &in_pnt);

    inline Geom::Point to_2geom() const {
        return Geom::Point(_pt[X], _pt[Y]);
    }

private:
    Coord _pt[2];
};

/** A function to print out the Point.  It just prints out the coords
    on the given output stream */
inline std::ostream &operator<< (std::ostream &out_file, const NR::Point &in_pnt) {
    out_file << "X: " << in_pnt[X] << "  Y: " << in_pnt[Y];
    return out_file;
}

} /* namespace NR */

#endif /* !SEEN_NR_POINT_H */

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
