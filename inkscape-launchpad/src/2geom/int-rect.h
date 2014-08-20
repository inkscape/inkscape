/**
 *  \file
 *  \brief Axis-aligned rectangle with integer coordinates
 *//*
 * Copyright 2011 Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#ifndef LIB2GEOM_SEEN_INT_RECT_H
#define LIB2GEOM_SEEN_INT_RECT_H

#include <2geom/coord.h>
#include <2geom/int-interval.h>
#include <2geom/generic-rect.h>

namespace Geom {

typedef GenericRect<IntCoord> IntRect;
typedef GenericOptRect<IntCoord> OptIntRect;

// the functions below do not work when defined generically
inline OptIntRect operator&(IntRect const &a, IntRect const &b) {
    OptIntRect ret(a);
    ret.intersectWith(b);
    return ret;
}
inline OptIntRect intersect(IntRect const &a, IntRect const &b) {
    return a & b;
}
inline OptIntRect intersect(OptIntRect const &a, OptIntRect const &b) {
    return a & b;
}
inline IntRect unify(IntRect const &a, IntRect const &b) {
    return a | b;
}
inline OptIntRect unify(OptIntRect const &a, OptIntRect const &b) {
    return a | b;
}

} // end namespace Geom

#endif // !LIB2GEOM_SEEN_INT_RECT_H

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
