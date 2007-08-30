#ifndef SEEN_Geom_TRANSFORMS_H
#define SEEN_Geom_TRANSFORMS_H

#include "matrix.h"
#include <cmath>

namespace Geom {

template <typename T>
struct TransformConcept {
    T t;
    Matrix m;
    Point p;
    void constraints() {
        m = t;  //implicit conversion
        t = t.inverse();
        p = p * t;
        t = t * t;
    }
};


class Rotate;
class Translate {
  private:
    Translate();
    Point vec;
  public:
    explicit Translate(Point const &p) : vec(p) {}
    explicit Translate(Coord const x, Coord const y) : vec(x, y) {}
    inline operator Matrix() const { return Matrix(1, 0, 0, 1, vec[X], vec[Y]); }

    inline Coord operator[](Dim2 const dim) const { return vec[dim]; }
    inline Coord operator[](unsigned const dim) const { return vec[dim]; }
    inline bool operator==(Translate const &o) const { return vec == o.vec; }
    inline bool operator!=(Translate const &o) const { return vec != o.vec; }

    inline Translate inverse() const { return Translate(-vec); }

    friend Point operator*(Point const &v, Translate const &t);
    inline Translate operator*(Translate const &b) const { return Translate(vec + b.vec); }
    
    friend Matrix operator*(Translate const &t, Rotate const &r);
};

inline Point operator*(Point const &v, Translate const &t) { return v + t.vec; }

class Scale {
  private:
    Point vec;
    Scale();
  public:
    explicit Scale(Point const &p) : vec(p) {}
    Scale(Coord const x, Coord const y) : vec(x, y) {}
    explicit Scale(Coord const s) : vec(s, s) {}
    inline operator Matrix() const { return Matrix(vec[X], 0, 0, vec[Y], 0, 0); }

    inline Coord operator[](Dim2 const d) const { return vec[d]; }
    inline Coord operator[](unsigned const d) const { return vec[d]; }
    //TODO: should we keep these mutators? add them to the other transforms?
    inline Coord &operator[](Dim2 const d) { return vec[d]; }
    inline Coord &operator[](unsigned const d) { return vec[d]; }
    inline bool operator==(Scale const &o) const { return vec == o.vec; }
    inline bool operator!=(Scale const &o) const { return vec != o.vec; }

    inline Scale inverse() const { return Scale(1./vec[0], 1./vec[1]); }

    friend Point operator*(Point const &v, Translate const &t);
    inline Scale operator*(Scale const &b) const { return Scale(vec[X]*b[X], vec[Y]*b[Y]); }
};

inline Point operator*(Point const &p, Scale const &s) { return Point(p[X] * s[X], p[Y] * s[Y]); }

/** Notionally an Geom::Matrix corresponding to rotation about the origin.
    Behaves like Geom::Matrix for multiplication.
**/
class Rotate {
  private:
    Rotate();
    Point vec;
  public:    
    explicit Rotate(Coord theta) : vec(std::cos(theta), std::sin(theta)) {}
    Rotate(Point const &p) {Point v = p; v.normalize(); vec = v;} //TODO: UGLY!
    explicit Rotate(Coord x, Coord y) { Rotate(Point(x, y)); }
    inline operator Matrix() const { return Matrix(vec[X], vec[Y], vec[Y], -vec[X], 0, 0); }

    inline Coord operator[](Dim2 const dim) const { return vec[dim]; }
    inline Coord operator[](unsigned const dim) const { return vec[dim]; }
    inline bool operator==(Rotate const &o) const { return vec == o.vec; }
    inline bool operator!=(Rotate const &o) const { return vec != o.vec; }

    Rotate inverse() const { return Rotate( Point(vec[X], -vec[Y]) ); }
    static Rotate from_degrees(Coord deg) {
        Coord rad = (deg / 180.0) * M_PI;
        return Rotate(rad);
    }

    friend Point operator*(Point const &v, Rotate const &r);
    inline Rotate operator*(Rotate const &b) const { return Rotate(vec * b); }
};

inline Point operator*(Point const &v, Rotate const &r) { return v ^ r.vec; }

Matrix operator*(Translate const &t, Scale const &s);
Matrix operator*(Translate const &t, Rotate const &r);

Matrix operator*(Scale const &s, Translate const &t);
Matrix operator*(Scale const &s, Matrix const &m);

Matrix operator*(Matrix const &m, Translate const &t);
Matrix operator*(Matrix const &m, Scale const &s);
Matrix operator*(Matrix const &m, Rotate const &r);
Matrix operator*(Matrix const &m1, Matrix const &m2);

//TODO: matrix to trans/scale/rotate

} /* namespace Geom */


#endif /* !SEEN_Geom_TRANSFORMS_H */

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
