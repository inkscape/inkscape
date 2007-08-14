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

}
