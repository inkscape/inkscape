#ifndef SEEN_NR_3DUTILS_H
#define SEEN_NR_3DUTILS_H

/*
 * 3D utils. Definition of gdouble vectors of dimension 3 and of some basic
 * functions.
 *   This looks redundant, why not just use Geom::Point for this?
 *
 * Authors:
 *   Jean-Rene Reinhard <jr@komite.net>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/forward.h>

namespace NR {

#define X_3D 0
#define Y_3D 1
#define Z_3D 2

/**
 * a type of 3 gdouble components vectors
 */
struct Fvector {
    Fvector() {
        v[0] = v[1] = v[2] = 0.0;
    }
    Fvector(double x, double y, double z) {
        v[0] = x;
        v[1] = y;
        v[2] = z;
    }
    double v[3];
    double &operator[](unsigned i) { return v[i]; }
    double operator[](unsigned i) const { return v[i]; }
};

/**
 * The eye vector
 */
const static Fvector EYE_VECTOR(0, 0, 1);

/**
 * returns the euclidian norm of the vector v
 *
 * \param v a reference to a vector with double components
 * \return the euclidian norm of v
 */
double norm(const Fvector &v);

/**
 * Normalizes a vector
 *
 * \param v a reference to a vector to normalize
 */
void normalize_vector(Fvector &v);

/**
 * Computes the scalar product between two Fvectors
 *
 * \param a a Fvector reference
 * \param b a Fvector reference
 * \return the scalar product of a and b
 */
double scalar_product(const Fvector &a, const Fvector &b);

/**
 * Computes the normalized sum of two Fvectors
 *
 * \param r a Fvector reference where we store the result
 * \param a a Fvector reference
 * \param b a Fvector reference
 */
void normalized_sum(Fvector &r, const Fvector &a, const Fvector &b);

/**
 * Applies the transformation matrix to (x, y, z). This function assumes that
 * trans[0] = trans[3]. x and y are transformed according to trans, z is
 * multiplied by trans[0].
 *
 * \param x a reference to a x coordinate
 * \param y a reference to a y coordinate
 * \param z a reference to a z coordinate
 * \param z a reference to a transformation matrix
 */
void convert_coord(double &x, double &y, double &z, Geom::Affine const &trans);

} /* namespace NR */

#endif /* __NR_3DUTILS_H__ */
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
