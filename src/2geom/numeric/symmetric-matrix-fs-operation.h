/*
 * SymmetricMatrix basic operation
 *
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
 *
 * Copyright 2009  authors
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

#ifndef _NL_SYMMETRIC_MATRIX_FS_OPERATION_H_
#define _NL_SYMMETRIC_MATRIX_FS_OPERATION_H_


#include <2geom/numeric/symmetric-matrix-fs.h>
#include <2geom/numeric/symmetric-matrix-fs-trace.h>




namespace Geom { namespace NL {

template <size_t N>
SymmetricMatrix<N> adj(const ConstBaseSymmetricMatrix<N> & S);

template <>
inline
SymmetricMatrix<2> adj(const ConstBaseSymmetricMatrix<2> & S)
{
    SymmetricMatrix<2> result;
    result.get<0,0>() = S.get<1,1>();
    result.get<1,0>() = -S.get<1,0>();
    result.get<1,1>() = S.get<0,0>();
    return result;
}

template <>
inline
SymmetricMatrix<3> adj(const ConstBaseSymmetricMatrix<3> & S)
{
    SymmetricMatrix<3> result;

    result.get<0,0>() = S.get<1,1>() * S.get<2,2>() - S.get<1,2>() * S.get<2,1>();
    result.get<1,0>() = S.get<0,2>() * S.get<2,1>() - S.get<0,1>() * S.get<2,2>();
    result.get<1,1>() = S.get<0,0>() * S.get<2,2>() - S.get<0,2>() * S.get<2,0>();
    result.get<2,0>() = S.get<0,1>() * S.get<1,2>() - S.get<0,2>() * S.get<1,1>();
    result.get<2,1>() = S.get<0,2>() * S.get<1,0>() - S.get<0,0>() * S.get<1,2>();
    result.get<2,2>() = S.get<0,0>() * S.get<1,1>() - S.get<0,1>() * S.get<1,0>();
    return result;
}

template <size_t N>
inline
SymmetricMatrix<N> inverse(const ConstBaseSymmetricMatrix<N> & S)
{
    SymmetricMatrix<N> result = adj(S);
    double d = det(S);
    assert (d != 0);
    result.scale (1/d);
    return result;
}

} /* end namespace NL*/ } /* end namespace Geom*/


#endif // _NL_SYMMETRIC_MATRIX_FS_OPERATION_H_




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
