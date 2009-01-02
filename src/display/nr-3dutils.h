#ifndef __NR_3DUTILS_H__
#define __NR_3DUTILS_H__

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

#include <gdk/gdktypes.h>
#include <2geom/forward.h>

struct NRPixBlock;

namespace NR {

#define X_3D 0
#define Y_3D 1
#define Z_3D 2

/**
 * a type of 3 gdouble components vectors
 */
typedef gdouble Fvector[3];

/**
 * The eye vector
 */
const static Fvector EYE_VECTOR = {0, 0, 1};

/**
 * returns the euclidian norm of the vector v
 *
 * \param v a reference to a vector with double components
 * \return the euclidian norm of v
 */
gdouble norm(const Fvector &v);

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
gdouble scalar_product(const Fvector &a, const Fvector &b);

/**
 * Computes the normalized sum of two Fvectors
 *
 * \param r a Fvector reference where we store the result
 * \param a a Fvector reference
 * \param b a Fvector reference
 */
void normalized_sum(Fvector &r, const Fvector &a, const Fvector &b);

/**
 * Computes the unit suface normal vector of surface given by "in" at (i, j)
 * and store it into N. "in" is a (NRPixBlock *) in mode RGBA but only the alpha
 * channel is considered as a bump map. ss is the altitude when for the alpha
 * value 255. dx and dy are the deltas used to compute in our discrete setting
 *
 * \param N a reference to a Fvector in which we store the unit surface normal
 * \param ss the surface scale
 * \param in a NRPixBlock * whose alpha channel codes the surface
 * \param i the x coordinate of the point at which we compute the normal
 * \param j the y coordinate of the point at which we compute the normal
 * \param dx the delta used in the x coordinate
 * \param dy the delta used in the y coordinate
 */
void compute_surface_normal(Fvector &N, gdouble ss, NRPixBlock *in, int i, int j, int dx, int dy);

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
void convert_coord(gdouble &x, gdouble &y, gdouble &z, Geom::Matrix const &trans);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
