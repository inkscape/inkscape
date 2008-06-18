#include "transforms.h"

namespace Geom {

Matrix operator*(Translate const &t, Scale const &s) {
    Matrix ret(s);
    ret[4] = t[X] * s[X];
    ret[5] = t[Y] * s[Y];
    return ret;
}

Matrix operator*(Translate const &t, Rotate const &r) {
    Matrix ret(r);
    ret.setTranslation(t.vec * ret);
    return ret;
}

Matrix operator*(Scale const &s, Translate const &t) {
    return Matrix(s[0], 0,
                  0   , s[1],
                  t[0], t[1]);
}

Matrix operator*(Scale const &s, Matrix const &m) {
    Matrix ret(m);
    ret[0] *= s[X];
    ret[1] *= s[X];
    ret[2] *= s[Y];
    ret[3] *= s[Y];
    return ret;
}

Matrix operator*(Matrix const &m, Translate const &t) {
    Matrix ret(m);
    ret[4] += t[X];
    ret[5] += t[Y];
    return ret;
}

Matrix operator*(Matrix const &m, Scale const &s) {
    Matrix ret(m);
    ret[0] *= s[X]; ret[1] *= s[Y];
    ret[2] *= s[X]; ret[3] *= s[Y];
    ret[4] *= s[X]; ret[5] *= s[Y];
    return ret;
}

Matrix operator*(Matrix const &m, Rotate const &r) {
    // TODO: we just convert the Rotate to a matrix and use the existing operator*(); is there a better way?
    Matrix ret(m);
    ret *= (Matrix) r;
    return ret;
}

Translate pow(Translate const &t, int n) {
    return Translate(t[0]*n, t[1]*n);
}

Coord pow(Coord x, long n) // shamelessly lifted from WP
{
    Coord result = 1;
    while ( n ) {
        if ( n & 1 ) {
            result = result * x;
            n = n-1;
        }
        x = x*x;
        n = n/2;
    }
    return result;
}
Scale pow(Scale const &s, int n) {
    return Scale(pow(s[0],n), pow(s[1],n));

}

Rotate pow(Rotate x, long n)
{
    Rotate result(0,1); // identity
    while ( n ) {
        if ( n & 1 ) {
            result = result * x;
            n = n-1;
        }
        x = x*x;
        n = n/2;
    }
    return result;
}

Matrix pow(Matrix x, long n)
{
    Matrix result;
    result.setIdentity();
    while ( n ) {
        if ( n & 1 ) {
            result = result * x;
            n = n-1;
        }
        x = x*x;
        n = n/2;
    }
    return result;
}

}

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
