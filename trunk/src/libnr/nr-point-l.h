#ifndef SEEN_NR_POINT_L_H
#define SEEN_NR_POINT_L_H

#include <stdexcept>
#include <libnr/nr-i-coord.h>
#include <libnr/nr-point.h>

struct NRPointL {
    NR::ICoord x, y;
};

namespace NR {

class IPoint {
public:
    IPoint()
    { }

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
        if ( i > Y ) {
            throw std::out_of_range("index out of range");
        }
        return _pt[i];
    }

    ICoord &operator[](unsigned i) throw(std::out_of_range) {
        if ( i > Y ) {
            throw std::out_of_range("index out of range");
        }
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

    bool operator==(IPoint const &other) const {
        return _pt[X] == other[X] && _pt[Y] == other[Y];
    }

    bool operator!=(IPoint const &other) const {
        return _pt[X] != other[X] || _pt[Y] != other[Y];
    }

private:
    ICoord _pt[2];
};


}  // namespace NR

#endif /* !SEEN_NR_POINT_L_H */

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
