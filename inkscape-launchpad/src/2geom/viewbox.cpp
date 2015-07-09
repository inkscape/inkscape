/**
 * \file
 * \brief Convenience class for SVG viewBox handling
 *//*
 *  Authors:
 *    Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2013 Authors
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

#include <2geom/transforms.h>
#include <2geom/viewbox.h>

namespace Geom {

/** Convert an align specification to coordinate fractions. */
Point align_factors(Align g) {
    Point p;
    switch (g) {
    case ALIGN_XMIN_YMIN:
        p[X] = 0.0;
        p[Y] = 0.0;
        break;
    case ALIGN_XMID_YMIN:
        p[X] = 0.5;
        p[Y] = 0.0;
        break;
    case ALIGN_XMAX_YMIN:
        p[X] = 1.0;
        p[Y] = 0.0;
        break;
    case ALIGN_XMIN_YMID:
        p[X] = 0.0;
        p[Y] = 0.5;
        break;
    case ALIGN_XMID_YMID:
        p[X] = 0.5;
        p[Y] = 0.5;
        break;
    case ALIGN_XMAX_YMID:
        p[X] = 1.0;
        p[Y] = 0.5;
        break;
    case ALIGN_XMIN_YMAX:
        p[X] = 0.0;
        p[Y] = 1.0;
        break;
    case ALIGN_XMID_YMAX:
        p[X] = 0.5;
        p[Y] = 1.0;
        break;
    case ALIGN_XMAX_YMAX:
        p[X] = 1.0;
        p[Y] = 1.0;
        break;
    default:
        break;
    }
    return p;
}

/** Obtain transformation from the viewbox to the specified viewport. */
Affine ViewBox::transformTo(Geom::Rect const &viewport) const
{
    if (!_box) {
        return Geom::Affine::identity();
    }

    // 1. translate viewbox to origin
    Geom::Affine total = Translate(-_box->min());

    // 2. compute scale
    Geom::Point vdims = viewport.dimensions();
    Geom::Point bdims = _box->dimensions();
    Geom::Scale scale(vdims[X] / bdims[X], vdims[Y] / bdims[Y]);

    if (_align == ALIGN_NONE) {
        // apply non-uniform scale
        // = Scale(_box->dimensions()).inverse() * Scale(viewport.dimensions())
        total *= scale * Translate(viewport.min());
    } else {
        double uscale = 0;
        if (_expansion == EXPANSION_MEET) {
            uscale = std::min(scale[X], scale[Y]);
        } else {
            uscale = std::max(scale[X], scale[Y]);
        }
        scale = Scale(uscale);

        // compute offset for align
        Geom::Point offset = bdims * scale - vdims;
        offset *= Scale(align_factors(_align));
        total *= Translate(-offset);
    }

    return total;
}

} // namespace Geom

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
