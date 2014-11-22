#ifndef SEEN_NR_FILTER_UTILS_H
#define SEEN_NR_FILTER_UTILS_H

/**
 * @file
 * Definition of functions needed by several filters.
 */
/*
 * Authors:
 *   Jean-Rene Reinhard <jr@komite.net>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "round.h"

namespace Inkscape {
namespace Filters {

/**
 * Clamps an integer value to a value between 0 and 255. Needed by filters where
 * rendering computations can lead to component values out of bound.
 *
 * \return 0 if the value is smaller than 0, 255 if it is greater 255, else v
 * \param v the value to clamp
 */
inline int clamp(int const val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return val;
}

/**
 * Clamps an integer value to a value between 0 and 255^3.
 *
 * \return 0 if the value is smaller than 0, 255^3 (16581375) if it is greater than 255^3, else v
 * \param v the value to clamp
 */
inline int clamp3(int const val) {
    if (val < 0) return 0;
    if (val > 16581375) return 16581375;
    return val;
}

/**
 * Macro to use the clamp function with double inputs and unsigned char output
 */
#define CLAMP_D_TO_U8(v) (unsigned char) clamp((int)round((v)))

/**
 * Clamps an integer to a value between 0 and alpha. Useful when handling
 * images with premultiplied alpha, as setting some of RGB channels
 * to a value bigger than alpha confuses the alpha blending in Inkscape
 * \return 0 if val is negative, alpha if val is bigger than alpha, val otherwise
 * \param val the value to clamp
 * \param alpha the maximum value to clamp to
 */
inline int clamp_alpha(int const val, int const alpha) {
    if (val < 0) return 0;
    if (val > alpha) return alpha;
    return val;
}

/**
 * Macro to use the clamp function with double inputs and unsigned char output
 */
#define CLAMP_D_TO_U8_ALPHA(v,a) (unsigned char) clamp_alpha((int)round((v)),(a))

} /* namespace Filters */
} /* namespace Inkscape */

#endif /* __NR_FILTER_UTILS_H__ */
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
