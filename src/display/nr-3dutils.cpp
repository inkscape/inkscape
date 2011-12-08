/*
 * 3D utils.
 *
 * Authors:
 *   Jean-Rene Reinhard <jr@komite.net>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#include "display/nr-3dutils.h"
#include <cmath>
#include <2geom/point.h>
#include <2geom/affine.h>

namespace NR {

void convert_coord(gdouble &x, gdouble &y, gdouble &z, Geom::Affine const &trans) {
    Geom::Point p = Geom::Point(x, y);
    p *= trans;
    x = p[Geom::X];
    y = p[Geom::Y];
    z *= trans[0];
}

gdouble norm(const Fvector &v) {
    return sqrt(v[X_3D]*v[X_3D] + v[Y_3D]*v[Y_3D] + v[Z_3D]*v[Z_3D]);
}

void normalize_vector(Fvector &v) {
    gdouble nv = norm(v);
    //TODO test nv == 0
    for (int j = 0; j < 3; j++) {
        v[j] /= nv;
    }
}

gdouble scalar_product(const Fvector &a, const Fvector &b) {
    return  a[X_3D] * b[X_3D] +
            a[Y_3D] * b[Y_3D] +
            a[Z_3D] * b[Z_3D];
}

void normalized_sum(Fvector &r, const Fvector &a, const Fvector &b) {
    r[X_3D] = a[X_3D] + b[X_3D];
    r[Y_3D] = a[Y_3D] + b[Y_3D];
    r[Z_3D] = a[Z_3D] + b[Z_3D];
    normalize_vector(r);
}

}/* namespace NR */

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
