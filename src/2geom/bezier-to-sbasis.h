/**
 * \file
 * \brief Conversion between Bezier control points and SBasis curves
 *//*
 * Copyright 2006 Nathan Hurst <njh@mail.csse.monash.edu.au>
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
 *
 */

#ifndef LIB2GEOM_SEEN_BEZIER_TO_SBASIS_H
#define LIB2GEOM_SEEN_BEZIER_TO_SBASIS_H

#include <2geom/coord.h>
#include <2geom/point.h>
#include <2geom/d2.h>
#include <2geom/sbasis-to-bezier.h>

namespace Geom
{

#if 0
inline SBasis bezier_to_sbasis(Coord const *handles, unsigned order) {
    if(order == 0)
        return Linear(handles[0]);
    else if(order == 1)
        return Linear(handles[0], handles[1]);
    else
        return multiply(Linear(1, 0), bezier_to_sbasis(handles, order-1)) +
            multiply(Linear(0, 1), bezier_to_sbasis(handles+1, order-1));
}


template <typename T>
inline D2<SBasis> handles_to_sbasis(T const &handles, unsigned order)
{
    double v[2][order+1];
    for(unsigned i = 0; i <= order; i++)
        for(unsigned j = 0; j < 2; j++)
             v[j][i] = handles[i][j];
    return D2<SBasis>(bezier_to_sbasis(v[0], order),
                      bezier_to_sbasis(v[1], order));
}
#endif


template <typename T>
inline
D2<SBasis> handles_to_sbasis(T const& handles, unsigned order)
{
    D2<SBasis> sbc;
    size_t sz = order + 1;
    std::vector<Point> v;
    v.reserve(sz);
    for (size_t i = 0; i < sz; ++i)
        v.push_back(handles[i]);
    bezier_to_sbasis(sbc, v);
    return sbc;
}

} // end namespace Geom

#endif // LIB2GEOM_SEEN_BEZIER_TO_SBASIS_H
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
