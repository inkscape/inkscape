/**
 * 3x4 transformation matrix to map points from projective 3-space into the projective plane
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007  Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_PROJ_PT_H
#define SEEN_PROJ_PT_H

#include <2geom/point.h>
#include <cstdio>

namespace Proj {

const double epsilon = 1E-6;

// TODO: Catch the case when the constructors are called with only zeros
class Pt2 {
public:
    Pt2 () { pt[0] = 0; pt[1] = 0; pt[2] = 1.0; } // we default to (0 : 0 : 1)
    Pt2 (double x, double y, double w) { pt[0] = x; pt[1] = y; pt[2] = w; }
    Pt2 (Geom::Point const &point) { pt[0] = point[Geom::X]; pt[1] = point[Geom::Y]; pt[2] = 1; }
    Pt2 (const char *coord_str);

    inline double operator[] (unsigned int index) const {
        if (index > 2) { return Geom::infinity(); }
        return pt[index];
    }
    inline double &operator[] (unsigned int index) {
        // FIXME: How should we handle wrong indices?
        //if (index > 2) { return Geom::infinity(); }
        return pt[index];
    }
    inline bool operator== (Pt2 &rhs) {
        normalize();
        rhs.normalize();
        return (fabs(pt[0] - rhs.pt[0]) < epsilon &&
                fabs(pt[1] - rhs.pt[1]) < epsilon &&
                fabs(pt[2] - rhs.pt[2]) < epsilon);
    }
    inline bool operator!= (Pt2 &rhs) {
        return !((*this) == rhs);
    }

    /*** For convenience, we define addition/subtraction etc. as "affine" operators (i.e.,
         the result for finite points is the same as if the affine points were addes ***/
    inline Pt2 &operator+(Pt2 &rhs) const {
        Pt2 *result = new Pt2 (*this);
        result->normalize();
        rhs.normalize();
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            result->pt[i] += rhs.pt[i];
        }
        return *result;
    }

    inline Pt2 &operator-(Pt2 &rhs) const {
        Pt2 *result = new Pt2 (*this);
        result->normalize();
        rhs.normalize();
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            result->pt[i] -= rhs.pt[i];
        }
        return *result;
    }

    inline Pt2 &operator*(double const s) const {
        Pt2 *result = new Pt2 (*this);
        result->normalize();
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            result->pt[i] *= s;
        }
        return *result;
    }

    void normalize();
    Geom::Point affine();
    inline bool is_finite() { return pt[2] != 0; } // FIXME: Should we allow for some tolerance?
    char *coord_string();
    inline void print(char const *s) const { printf ("%s(%8.2f : %8.2f : %8.2f)\n", s, pt[0], pt[1], pt[2]); }

private:
    double pt[3];
};


class Pt3 {
public:
    Pt3 () { pt[0] = 0; pt[1] = 0; pt[2] = 0; pt[3] = 1.0; } // we default to (0 : 0 : 0 : 1)
    Pt3 (double x, double y, double z, double w) { pt[0] = x; pt[1] = y; pt[2] = z; pt[3] = w; }
    Pt3 (const char *coord_str);

    inline bool operator== (Pt3 &rhs) {
        normalize();
        rhs.normalize();
        return (fabs(pt[0] - rhs.pt[0]) < epsilon &&
                fabs(pt[1] - rhs.pt[1]) < epsilon &&
                fabs(pt[2] - rhs.pt[2]) < epsilon &&
                fabs(pt[3] - rhs.pt[3]) < epsilon);
    }

    /*** For convenience, we define addition/subtraction etc. as "affine" operators (i.e.,
         the result for finite points is the same as if the affine points were addes ***/
    inline Pt3 &operator+(Pt3 &rhs) const {
        Pt3 *result = new Pt3 (*this);
        result->normalize();
        rhs.normalize();
        for ( unsigned i = 0 ; i < 3 ; ++i ) {
            result->pt[i] += rhs.pt[i];
        }
        return *result;
    }

    inline Pt3 &operator-(Pt3 &rhs) const {
        Pt3 *result = new Pt3 (*this);
        result->normalize();
        rhs.normalize();
        for ( unsigned i = 0 ; i < 3 ; ++i ) {
            result->pt[i] -= rhs.pt[i];
        }
        return *result;
    }

    inline Pt3 &operator*(double const s) const {
        Pt3 *result = new Pt3 (*this);
        result->normalize();
        for ( unsigned i = 0 ; i < 3 ; ++i ) {
            result->pt[i] *= s;
        }
        return *result;
    }
    
    inline double operator[] (unsigned int index) const {
        if (index > 3) { return Geom::infinity(); }
        return pt[index];
    }
    inline double &operator[] (unsigned int index) {
        // FIXME: How should we handle wrong indices?
        //if (index > 3) { return Geom::infinity(); }
        return pt[index];
    }
    void normalize();
    inline bool is_finite() { return pt[3] != 0; } // FIXME: Should we allow for some tolerance?
    char *coord_string();
    inline void print(char const *s) const {
        printf ("%s(%8.2f : %8.2f : %8.2f : %8.2f)\n", s, pt[0], pt[1], pt[2], pt[3]);
    }

private:
    double pt[4];
};

} // namespace Proj

#endif // !SEEN_PROJ_PT_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
