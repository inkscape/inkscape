/*
 * Inkscape::Util::FixedPoint - fixed point type
 *
 * Authors:
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.net>
 *
 * Copyright (C) 2006 Jasper van de Gronde
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_FIXED_POINT_H
#define SEEN_INKSCAPE_UTIL_FIXED_POINT_H

#include "util/reference.h"
#include <math.h>
#include <algorithm>
#include <limits>

namespace Inkscape {

namespace Util {

template <typename T, unsigned int precision>
class FixedPoint {
public:
    FixedPoint() {}
    FixedPoint(const FixedPoint& value) : v(value.v) {}
    FixedPoint(char value) : v(static_cast<T>(value)<<precision) {}
    FixedPoint(unsigned char value) : v(static_cast<T>(value)<<precision) {}
    FixedPoint(short value) : v(static_cast<T>(value)<<precision) {}
    FixedPoint(unsigned short value) : v(static_cast<T>(value)<<precision) {}
    FixedPoint(int value) : v(static_cast<T>(value)<<precision) {}
    FixedPoint(unsigned int value) : v(static_cast<T>(value)<<precision) {}
    FixedPoint(double value) : v(static_cast<T>(floor(value*(1<<precision)))) {}

    FixedPoint& operator+=(FixedPoint val) { v += val.v; return *this; }
    FixedPoint& operator-=(FixedPoint val) { v -= val.v; return *this; }
    FixedPoint& operator*=(FixedPoint val) {
        const unsigned int half_size = 8*sizeof(T)/2;
        const T al = v&((1<<half_size)-1), bl = val.v&((1<<half_size)-1);
        const T ah = v>>half_size, bh = val.v>>half_size;
        v = static_cast<unsigned int>(al*bl)>>precision;
        if ( half_size >= precision ) {
            v += ((al*bh)+(ah*bl)+((ah*bh)<<half_size))<<(half_size-precision);
        } else {
            v += ((al*bh)+(ah*bl))>>(precision-half_size);
            v += (ah*bh)<<(2*half_size-precision);
        }
        return *this;
    }

    FixedPoint& operator*=(char val) { v *= val; return *this; }
    FixedPoint& operator*=(unsigned char val) { v *= val; return *this; }
    FixedPoint& operator*=(short val) { v *= val; return *this; }
    FixedPoint& operator*=(unsigned short val) { v *= val; return *this; }
    FixedPoint& operator*=(int val) { v *= val; return *this; }
    FixedPoint& operator*=(unsigned int val) { v *= val; return *this; }

    FixedPoint operator+(FixedPoint val) const { FixedPoint r(*this); return r+=val; }
    FixedPoint operator-(FixedPoint val) const { FixedPoint r(*this); return r-=val; }
    FixedPoint operator*(FixedPoint val) const { FixedPoint r(*this); return r*=val; }

    FixedPoint operator*(char val) const { FixedPoint r(*this); return r*=val; }
    FixedPoint operator*(unsigned char val) const { FixedPoint r(*this); return r*=val; }
    FixedPoint operator*(short val) const { FixedPoint r(*this); return r*=val; }
    FixedPoint operator*(unsigned short val) const { FixedPoint r(*this); return r*=val; }
    FixedPoint operator*(int val) const { FixedPoint r(*this); return r*=val; }
    FixedPoint operator*(unsigned int val) const { FixedPoint r(*this); return r*=val; }

    float operator*(float val) const { return static_cast<float>(*this)*val; }
    double operator*(double val) const { return static_cast<double>(*this)*val; }

    operator char() const { return v>>precision; }
    operator unsigned char() const { return v>>precision; }
    operator short() const { return v>>precision; }
    operator unsigned short() const { return v>>precision; }
    operator int() const { return v>>precision; }
    operator unsigned int() const { return v>>precision; }

    operator float() const { return ldexpf(v,-precision); }
    operator double() const { return ldexp(v,-precision); }
private:
    T v;
};

template<typename T, unsigned int precision> FixedPoint<T,precision> operator *(char a, FixedPoint<T,precision> b) { return b*=a; }
template<typename T, unsigned int precision> FixedPoint<T,precision> operator *(unsigned char a, FixedPoint<T,precision> b) { return b*=a; }
template<typename T, unsigned int precision> FixedPoint<T,precision> operator *(short a, FixedPoint<T,precision> b) { return b*=a; }
template<typename T, unsigned int precision> FixedPoint<T,precision> operator *(unsigned short a, FixedPoint<T,precision> b) { return b*=a; }
template<typename T, unsigned int precision> FixedPoint<T,precision> operator *(int a, FixedPoint<T,precision> b) { return b*=a; }
template<typename T, unsigned int precision> FixedPoint<T,precision> operator *(unsigned int a, FixedPoint<T,precision> b) { return b*=a; }

template<typename T, unsigned int precision> float operator *(float a, FixedPoint<T,precision> b) { return b*a; }
template<typename T, unsigned int precision> double operator *(double a, FixedPoint<T,precision> b) { return b*a; }

}

}

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
