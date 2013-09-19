/*  This file is part of the libdepixelize project
    Copyright (C) 2013 Vinícius dos Santos Oliveira <vini.ipsmaker@gmail.com>

    GNU Lesser General Public License Usage
    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License as published by the
    Free Software Foundation; either version 2.1 of the License, or (at your
    option) any later version.
    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.

    GNU General Public License Usage
    Alternatively, this library may be used under the terms of the GNU General
    Public License as published by the Free Software Foundation, either version
    2 of the License, or (at your option) any later version.
    You should have received a copy of the GNU General Public License along with
    this library.  If not, see <http://www.gnu.org/licenses/>.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
*/

#ifndef LIBDEPIXELIZE_TRACER_YUV_H
#define LIBDEPIXELIZE_TRACER_YUV_H

#include <glib.h>

namespace Tracer {
namespace colorspace {

/**
 * The same algorithm used in hqx filter
 */
inline void rgb2yuv(guint8 r, guint8 g, guint8 b,
                    guint8 &y, guint8 &u, guint8 &v)
{
    y = 0.299 * r + 0.587 * g + 0.114 * b;
    u = guint16(-0.169 * r - 0.331 * g + 0.5 * b) + 128;
    v = guint16(0.5 * r - 0.419 * g - 0.081 * b) + 128;
}

inline void rgb2yuv(const guint8 rgb[], guint8 yuv[])
{
    rgb2yuv(rgb[0], rgb[1], rgb[2], yuv[0], yuv[1], yuv[2]);
}

inline bool same_color(const guint8 (&a)[4], const guint8 (&b)[4])
{
    return (a[0] == b[0]
            && a[1] == b[1]
            && a[2] == b[2]
            && a[3] == b[3]);
}

inline bool dissimilar_colors(const guint8 a[], const guint8 b[])
{
    // C uses row-major order, so 
    // A[2][3] = { {1, 2, 3}, {4, 5, 6} } = {1, 2, 3, 4, 5, 6}
    guint8 yuv[2][3];
    rgb2yuv(a, yuv[0]);
    rgb2yuv(b, yuv[1]);

    // Magic numbers taken from hqx algorithm
    // Only used to describe the level of tolerance
    return abs(yuv[0][0] - yuv[1][0]) > 0x30
        || abs(yuv[0][1] - yuv[1][1]) > 7
        || abs(yuv[0][2] - yuv[1][2]) > 6;
}

inline bool similar_colors(const guint8 a[], const guint8 b[])
{
    return !dissimilar_colors(a, b);
}

inline bool shading_edge(const guint8 a[], const guint8 b[])
{
    // C uses row-major order, so 
    // A[2][3] = { {1, 2, 3}, {4, 5, 6} } = {1, 2, 3, 4, 5, 6}
    guint8 yuv[2][3];
    rgb2yuv(a, yuv[0]);
    rgb2yuv(b, yuv[1]);

    // Magic numbers taken from Kopf-Lischinski algorithm
    // Only used to describe the level of tolerance
    return abs(yuv[0][0] - yuv[1][0]) <= 100
        && abs(yuv[0][1] - yuv[1][1]) <= 100
        && abs(yuv[0][2] - yuv[1][2]) <= 100;
}

inline bool contour_edge(const guint8 a[], const guint8 b[])
{
    return !shading_edge(a, b);
}

} // namespace colorspace
} // namespace Tracer

#endif // LIBDEPIXELIZE_TRACER_YUV_H

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
