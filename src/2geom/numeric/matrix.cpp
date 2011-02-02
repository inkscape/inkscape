/*
 * Matrix, MatrixView, ConstMatrixView classes wrap the gsl matrix routines;
 * "views" mimic the semantic of C++ references: any operation performed
 * on a "view" is actually performed on the "viewed object"
 *
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
 *
 * Copyright 2008  authors
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


#include <2geom/numeric/matrix.h>
#include <2geom/numeric/vector.h>


namespace Geom { namespace NL {

Vector operator*( detail::BaseMatrixImpl const& A,
                  detail::BaseVectorImpl const& v )
{
    assert(A.columns() == v.size());

    Vector result(A.rows(), 0.0);
    for (size_t i = 0; i < A.rows(); ++i)
        for (size_t j = 0; j < A.columns(); ++j)
            result[i] += A(i,j) * v[j];

    return result;
}

Matrix operator*( detail::BaseMatrixImpl const& A,
                  detail::BaseMatrixImpl const& B )
{
    assert(A.columns() == B.rows());

    Matrix C(A.rows(), B.columns(), 0.0);
    for (size_t i = 0; i < C.rows(); ++i)
        for (size_t j = 0; j < C.columns(); ++j)
            for (size_t k = 0; k < A.columns(); ++k)
                C(i,j) += A(i,k) * B(k, j);

    return C;
}

Matrix pseudo_inverse(detail::BaseMatrixImpl const& A)
{

    Matrix U(A);
    Matrix V(A.columns(), A.columns());
    Vector s(A.columns());
    gsl_vector* work = gsl_vector_alloc(A.columns());

    gsl_linalg_SV_decomp( U.get_gsl_matrix(),
                          V.get_gsl_matrix(),
                          s.get_gsl_vector(),
                          work );

    Matrix P(A.columns(), A.rows(), 0.0);

    int sz = s.size();
    while ( sz-- > 0 && s[sz] == 0 ) {}
    ++sz;
    if (sz == 0) return P;
    VectorView sv(s, sz);

    for (size_t i = 0; i < sv.size(); ++i)
    {
        VectorView v = V.column_view(i);
        v.scale(1/sv[i]);
        for (size_t h = 0; h < P.rows(); ++h)
            for (size_t k = 0; k < P.columns(); ++k)
                P(h,k) += V(h,i) * U(k,i);
    }

    return P;
}


double trace (detail::BaseMatrixImpl const& A)
{
    if (A.rows() != A.columns())
    {
        THROW_RANGEERROR ("NL::Matrix: computing trace: "
                          "rows() != columns()");
    }
    double t = 0;
    for (size_t i = 0; i < A.rows(); ++i)
    {
        t += A(i,i);
    }
    return t;
}


double det (detail::BaseMatrixImpl const& A)
{
    if (A.rows() != A.columns())
    {
        THROW_RANGEERROR ("NL::Matrix: computing determinant: "
                          "rows() != columns()");
    }

    Matrix LU(A);
    int s;
    gsl_permutation * p = gsl_permutation_alloc(LU.rows());
    gsl_linalg_LU_decomp (LU.get_gsl_matrix(), p, &s);

    double t = 1;
    for (size_t i = 0; i < LU.rows(); ++i)
    {
        t *= LU(i,i);
    }

    gsl_permutation_free(p);
    return t;
}


} }  // end namespaces

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
