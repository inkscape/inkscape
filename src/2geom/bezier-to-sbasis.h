/*
 * bezier-to-sbasis.h
 *
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

#ifndef _BEZIER_TO_SBASIS
#define _BEZIER_TO_SBASIS

#include "coord.h"

#include "d2.h"
#include "point.h"

namespace Geom{

template <unsigned order>
struct bezier_to_sbasis_impl {
    static inline SBasis compute(Coord const *handles) {
        return multiply(Linear(1, 0), bezier_to_sbasis_impl<order-1>::compute(handles)) +
               multiply(Linear(0, 1), bezier_to_sbasis_impl<order-1>::compute(handles+1));
    }
};

template <>
struct bezier_to_sbasis_impl<1> {
    static inline SBasis compute(Coord const *handles) {
        return Linear(handles[0], handles[1]);
    }
};

template <>
struct bezier_to_sbasis_impl<0> {
    static inline SBasis compute(Coord const *handles) {
        return Linear(handles[0], handles[0]);
    }
};

template <unsigned order>
inline SBasis bezier_to_sbasis(Coord const *handles) {
    return bezier_to_sbasis_impl<order>::compute(handles);
}

template <unsigned order, typename T>
inline D2<SBasis> handles_to_sbasis(T const &handles) {
    double v[2][order+1];
    for(unsigned i = 0; i <= order; i++)
        for(unsigned j = 0; j < 2; j++)
             v[j][i] = handles[i][j];
    return D2<SBasis>(bezier_to_sbasis<order>(v[0]),
                      bezier_to_sbasis<order>(v[1]));
}

};
#endif
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
