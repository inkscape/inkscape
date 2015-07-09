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

#ifndef LIB2GEOM_SEEN_VIEWBOX_H
#define LIB2GEOM_SEEN_VIEWBOX_H

#include <2geom/affine.h>
#include <2geom/rect.h>

namespace Geom {

/** Values for the <align> parameter of preserveAspectRatio.
 * See: http://www.w3.org/TR/SVG/coords.html#PreserveAspectRatioAttribute */
enum Align {
    ALIGN_NONE,
    ALIGN_XMIN_YMIN,
    ALIGN_XMID_YMIN,
    ALIGN_XMAX_YMIN,
    ALIGN_XMIN_YMID,
    ALIGN_XMID_YMID,
    ALIGN_XMAX_YMID,
    ALIGN_XMIN_YMAX,
    ALIGN_XMID_YMAX,
    ALIGN_XMAX_YMAX
};

/** Values for the <meetOrSlice> parameter of preserveAspectRatio.
 * See: http://www.w3.org/TR/SVG/coords.html#PreserveAspectRatioAttribute */
enum Expansion {
    EXPANSION_MEET,
    EXPANSION_SLICE
};

Point align_factors(Align align);

class ViewBox {
    OptRect _box;
    Align _align;
    Expansion _expansion;

public:
    explicit ViewBox(OptRect const &r = OptRect(), Align a = ALIGN_XMID_YMID, Expansion ex = EXPANSION_MEET)
        : _box(r)
        , _align(a)
        , _expansion(ex)
    {}

    void setBox(OptRect const &r) { _box = r; }
    void setAlign(Align a) { _align = a; }
    void setExpansion(Expansion ex) { _expansion = ex; }
    OptRect const &box() const { return _box; }
    Align align() const { return _align; }
    Expansion expansion() const { return _expansion; }

    /** Obtain transformation from the viewbox to the specified viewport. */
    Affine transformTo(Geom::Rect const &viewport) const;
};

} // namespace Geom

#endif // !LIB2GEOM_SEEN_VIEWBOX_H

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
