#ifndef SEEN_Geom_POINT_L_H
#define SEEN_Geom_POINT_L_H

#include <stdexcept>
#include "point.h"

namespace Geom {

typedef long ICoord;

class IPoint {
    ICoord _pt[2];

  public:
    IPoint() { }

    IPoint(ICoord x, ICoord y) {
        _pt[X] = x;
        _pt[Y] = y;
    }

    IPoint(NRPointL const &p) {
        _pt[X] = p.x;
        _pt[Y] = p.y;
    }

    IPoint(IPoint const &p) {
        for (unsigned i = 0; i < 2; ++i) {
            _pt[i] = p._pt[i];
        }
    }

    IPoint &operator=(IPoint const &p) {
        for (unsigned i = 0; i < 2; ++i) {
            _pt[i] = p._pt[i];
        }
        return *this;
    }

    operator Point() {
        return Point(_pt[X], _pt[Y]);
    }

    ICoord operator[](unsigned i) const throw(std::out_of_range) {
        if ( i > Y ) throw std::out_of_range("index out of range");
        return _pt[i];
    }

    ICoord &operator[](unsigned i) throw(std::out_of_range) {
        if ( i > Y ) throw std::out_of_range("index out of range");
        return _pt[i];
    }

    ICoord operator[](Dim2 d) const throw() { return _pt[d]; }
    ICoord &operator[](Dim2 d) throw() { return _pt[d]; }

    IPoint &operator+=(IPoint const &o) {
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            _pt[i] += o._pt[i];
        }
        return *this;
    }
  
    IPoint &operator-=(IPoint const &o) {
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            _pt[i] -= o._pt[i];
        }
        return *this;
    }
};


}  // namespace Geom

#endif /* !SEEN_Geom_POINT_L_H */

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
