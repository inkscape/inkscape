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

#include <glib/gmessages.h>

#include "libnr/nr-pixblock.h"
#include "display/nr-3dutils.h"
#include <cmath>

namespace NR {

#define BEGIN 0 // TOP or LEFT
#define MIDDLE 1
#define END 2 // BOTTOM or RIGHT

#define START(v) ((v)==BEGIN? 1 : 0)
#define FINISH(v) ((v)==END? 1 : 2)

signed char K_X[3][3][3][3] = {
    //K_X[TOP]
    {
        //K_X[TOP][LEFT]
        {
            { 0,  0,  0},
            { 0, -2,  2},
            { 0, -1,  1}
        },
        {
            { 0,  0,  0},
            {-2,  0,  2},
            {-1,  0,  1}
        },
        {
            { 0,  0,  0},
            {-2,  2,  0},
            {-1,  1,  0}
        }
    },
    //K_X[MIDDLE]
    {
        //K_X[MIDDLE][LEFT]
        {
            { 0, -1,  1},
            { 0, -2,  2},
            { 0, -1,  1}
        },
        {
            {-1,  0,  1},
            {-2,  0,  2},
            {-1,  0,  1}
        },
        {
            {-1,  1,  0},
            {-2,  2,  0},
            {-1,  1,  0}
        }
    },
    //K_X[BOTTOM]
    {
        //K_X[BOTTOM][LEFT]
        {
            { 0, -1,  1},
            { 0, -2,  2},
            { 0,  0,  0}
        },
        {
            {-1,  0,  1},
            {-2,  0,  2},
            { 0,  0,  0}
        },
        {
            {-1,  1,  0},
            {-2,  2,  0},
            { 0,  0,  0}
        }
    }
};

//K_Y is obtained by transposing K_X globally and each of its components

gdouble FACTOR_X[3][3] = {
    {2./3, 1./3, 2./3},
    {1./2, 1./4, 1./2},
    {2./3, 1./3, 2./3}
};

//FACTOR_Y is obtained by transposing FACTOR_X

inline
int get_carac(int i, int len, int delta) {
    if (i < delta)
        return BEGIN;
    else if (i > len - 1 - delta)
        return END;
    else
        return MIDDLE;
}

//assumes in is RGBA
//should be made more resistant
void compute_surface_normal(Fvector &N, gdouble ss, NRPixBlock *in, int i, int j, int dx, int dy) {
    int w = in->area.x1 - in->area.x0;
    int h = in->area.y1 - in->area.y0;
    int k, l, alpha_idx, alpha_idx_y;
    int x_carac, y_carac;
    gdouble alpha;
    gdouble accu_x;
    gdouble accu_y;
    unsigned char *data = NR_PIXBLOCK_PX (in);
    g_assert(NR_PIXBLOCK_BPP(in) == 4);
    x_carac = get_carac(j, w, dx); //LEFT, MIDDLE or RIGHT
    y_carac = get_carac(i, h, dy); //TOP, MIDDLE or BOTTOM
    alpha_idx = 4*(i*w + j);
    accu_x = 0;
    accu_y = 0;
    for (k = START(y_carac); k <= FINISH(y_carac); k++) {
        alpha_idx_y = alpha_idx + 4*(k-1)*dy*w;
        for (l = START(x_carac); l <= FINISH(x_carac); l++) {
            alpha = (data + alpha_idx_y + 4*dx*(l-1))[3];
            accu_x += K_X[y_carac][x_carac][k][l] * alpha;
            accu_y += K_X[x_carac][y_carac][l][k] * alpha;
        }
    }
    ss /= 255.0; // Correction for scale of pixel values
    N[X_3D] = -ss * FACTOR_X[y_carac][x_carac] * accu_x / dx;
    N[Y_3D] = -ss * FACTOR_X[x_carac][y_carac] * accu_y / dy;
    N[Z_3D] = 1.0;
    normalize_vector(N);
    //std::cout << "(" << N[X_3D] << ", " << N[Y_3D] << ", " << N[Z_3D] << ")" << std::endl;
}

void convert_coord(gdouble &x, gdouble &y, gdouble &z, Geom::Matrix const &trans) {
    Point p = Point(x, y);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
