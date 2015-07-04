/*
 * SymmetricMatrix trace
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


#ifndef _NL_TRACE_H_
#define _NL_TRACE_H_


#include <2geom/numeric/matrix.h>
#include <2geom/numeric/symmetric-matrix-fs.h>





namespace Geom { namespace NL {


namespace detail
{

/*
 *  helper routines
 */

inline
int sgn_prod (int x, int y)
{
    if (x == 0 || y == 0)  return 0;
    if (x == y) return 1;
    return -1;
}

inline
bool abs_less (double x, double y)
{
    return (std::fabs(x) < std::fabs(y));
}


/*
 * trace K-th of symmetric matrix S of order N
 */
template <size_t K, size_t N>
struct trace
{
    static double evaluate(const ConstBaseSymmetricMatrix<N> &S);
};

template <size_t N>
struct trace<1,N>
{
    static
    double evaluate (const ConstBaseSymmetricMatrix<N> & S)
    {
        double t = 0;
        for (size_t i = 0; i < N; ++i)
        {
            t += S(i,i);
        }
        return t;
    }
};

template <size_t N>
struct trace<N,N>
{
    static
    double evaluate (const ConstBaseSymmetricMatrix<N> & S)
    {
        Matrix M(S);
        return det(M);
    }
};

/*
 *  trace for symmetric matrix of order 2
 */
template <>
struct trace<1,2>
{
    static
    double evaluate (const ConstBaseSymmetricMatrix<2> & S)
    {
        return (S.get<0,0>() + S.get<1,1>());
    }
};

template <>
struct trace<2,2>
{
    static
    double evaluate (const ConstBaseSymmetricMatrix<2> & S)
    {
        return (S.get<0,0>() * S.get<1,1>() - S.get<0,1>() * S.get<1,0>());
    }
};


/*
 *  trace for symmetric matrix of order 3
 */
template <>
struct trace<1,3>
{
    static
    double evaluate (const ConstBaseSymmetricMatrix<3> & S)
    {
        return (S.get<0,0>() + S.get<1,1>() + S.get<2,2>());
    }
};

template <>
struct trace<2,3>
{
    static
    double evaluate (const ConstBaseSymmetricMatrix<3> & S)
    {
        double a00 = S.get<1,1>() * S.get<2,2>() - S.get<1,2>() * S.get<2,1>();
        double a11 = S.get<0,0>() * S.get<2,2>() - S.get<0,2>() * S.get<2,0>();
        double a22 = S.get<0,0>() * S.get<1,1>() - S.get<0,1>() * S.get<1,0>();
        return (a00 + a11 + a22);
    }
};

template <>
struct trace<3,3>
{
    static
    double evaluate (const ConstBaseSymmetricMatrix<3> & S)
    {
        double d = S.get<0,0>() * S.get<1,1>() * S.get<2,2>();
        d += (2 * S.get<1,0>() * S.get<2,0>() * S.get<2,1>());
        d -= (S.get<0,0>() * S.get<2,1>() * S.get<2,1>());
        d -= (S.get<1,1>() * S.get<2,0>() * S.get<2,0>());
        d -= (S.get<2,2>() * S.get<1,0>() * S.get<1,0>());
        return d;
    }
};


/*
 *  sign of trace K-th
 */
template <size_t K, size_t N>
struct trace_sgn
{
    static
    int evaluate (const ConstBaseSymmetricMatrix<N> & S)
    {
        double d = trace<K, N>::evaluate(S);
        return sgn(d);
    }
};


/*
 *  sign of trace for symmetric matrix of order 2
 */
template <>
struct trace_sgn<2,2>
{
    static
    int evaluate (const ConstBaseSymmetricMatrix<2> & S)
    {
        double m00 = S.get<0,0>();
        double m10 = S.get<1,0>();
        double m11 = S.get<1,1>();

        int sm00 = sgn (m00);
        int sm10 = sgn (m10);
        int sm11 = sgn (m11);

        if (sm10 == 0)
        {
            return sgn_prod (sm00, sm11);
        }
        else
        {
            int sm00m11 = sgn_prod (sm00, sm11);
            if (sm00m11 == 1)
            {
                int e00, e10, e11;
                double f00 = std::frexp (m00, &e00);
                double f10 = std::frexp (m10, &e10);
                double f11 = std::frexp (m11, &e11);

                int e0011 = e00 + e11;
                int e1010 = e10 << 1;
                int ed = e0011 - e1010;

                if (ed > 1)
                {
                    return 1;
                }
                else if (ed < -1)
                {
                    return -1;
                }
                else
                {
                    double d = std::ldexp (f00 * f11, ed) - f10 * f10;
                    //std::cout << "trace_sgn<2,2>: det = " << d << std::endl;
                    double eps = std::ldexp (1, -50);
                    if (std::fabs(d) < eps)  return 0;
                    return sgn (d);
                }
            }
            return -1;
        }
    }
};


/*
 *  sign of trace for symmetric matrix of order 3
 */
template <>
struct trace_sgn<2,3>
{
    static
    int evaluate (const ConstBaseSymmetricMatrix<3> & S)
    {
        double eps = std::ldexp (1, -50);
        double t[6];

        t[0] = S.get<1,1>() * S.get<2,2>();
        t[1] = - S.get<1,2>() * S.get<2,1>();
        t[2] = S.get<0,0>() * S.get<2,2>();
        t[3] = - S.get<0,2>() * S.get<2,0>();
        t[4] = S.get<0,0>() * S.get<1,1>();
        t[5] = - S.get<0,1>() * S.get<1,0>();


        double* maxp = std::max_element (t, t+6, abs_less);
        int em;
        std::frexp(*maxp, &em);
        double d = 0;
        for (size_t i = 0; i < 6; ++i)
        {
            d += t[i];
        }
        double r = std::fabs (std::ldexp (d, -em));  // relative error
        //std::cout << "trace_sgn<2,3>: d = " << d << std::endl;
        //std::cout << "trace_sgn<2,3>: r = " << r << std::endl;
        if (r < eps) return 0;
        if (d > 0) return 1;
        return -1;
    }
};

template <>
struct trace_sgn<3,3>
{
    static
    int evaluate (const ConstBaseSymmetricMatrix<3> & S)
    {

        double eps = std::ldexp (1, -48);
        double t[5];

        t[0] = S.get<0,0>() * S.get<1,1>() * S.get<2,2>();
        t[1] = 2 * S.get<1,0>() * S.get<2,0>() * S.get<2,1>();
        t[2] = -(S.get<0,0>() * S.get<2,1>() * S.get<2,1>());
        t[3] = -(S.get<1,1>() * S.get<2,0>() * S.get<2,0>());
        t[4] = -(S.get<2,2>() * S.get<1,0>() * S.get<1,0>());

        double* maxp = std::max_element (t, t+5, abs_less);
        int em;
        std::frexp(*maxp, &em);
        double d = 0;
        for (size_t i = 0; i < 5; ++i)
        {
            d += t[i];
        }
        //std::cout << "trace_sgn<3,3>: d = " << d << std::endl;
        double r = std::fabs (std::ldexp (d, -em));  // relative error
        //std::cout << "trace_sgn<3,3>: r = " << r << std::endl;

        if (r < eps)  return 0;
        if (d > 0) return 1;
        return -1;
    }
}; // end struct trace_sgn<3,3>

} // end namespace detail


template <size_t K, size_t N>
inline
double trace (const ConstBaseSymmetricMatrix<N> & _matrix)
{
    return detail::trace<K, N>::evaluate(_matrix);
}

template <size_t N>
inline
double trace (const ConstBaseSymmetricMatrix<N> & _matrix)
{
    return detail::trace<1, N>::evaluate(_matrix);
}

template <size_t N>
inline
double det (const ConstBaseSymmetricMatrix<N> & _matrix)
{
    return detail::trace<N, N>::evaluate(_matrix);
}


template <size_t K, size_t N>
inline
int trace_sgn (const ConstBaseSymmetricMatrix<N> & _matrix)
{
    return detail::trace_sgn<K, N>::evaluate(_matrix);
}

template <size_t N>
inline
int trace_sgn (const ConstBaseSymmetricMatrix<N> & _matrix)
{
    return detail::trace_sgn<1, N>::evaluate(_matrix);
}

template <size_t N>
inline
int det_sgn (const ConstBaseSymmetricMatrix<N> & _matrix)
{
    return detail::trace_sgn<N, N>::evaluate(_matrix);
}

/*
template <size_t N>
inline
size_t rank (const ConstBaseSymmetricMatrix<N> & S)
{
    THROW_NOTIMPLEMENTED();
    return 0;
}

template <>
inline
size_t rank<2> (const ConstBaseSymmetricMatrix<2> & S)
{
    if (S.is_zero())  return 0;
    double d = S.get<0,0>() * S.get<1,1>() - S.get<0,1>() * S.get<1,0>();
    if (d != 0)  return 2;
    return 1;
}

template <>
inline
size_t rank<3> (const ConstBaseSymmetricMatrix<3> & S)
{
    if (S.is_zero())  return 0;

    double a20 = S.get<0,1>() * S.get<1,2>() - S.get<0,2>() * S.get<1,1>();
    double a21 = S.get<0,2>() * S.get<1,0>() - S.get<0,0>() * S.get<1,2>();
    double a22 = S.get<0,0>() * S.get<1,1>() - S.get<0,1>() * S.get<1,0>();
    double d = a20 * S.get<2,0>() + a21 * S.get<2,1>() + a22 * S.get<2,2>();

    if (d != 0)  return 3;

    if (a20 != 0 || a21 != 0 || a22 != 0)  return 2;

    double a00 = S.get<1,1>() * S.get<2,2>() - S.get<1,2>() * S.get<2,1>();
    if (a00 != 0)  return 2;

    double a10 = S.get<0,2>() * S.get<2,1>() - S.get<0,1>() * S.get<2,2>();
    if (a10 != 0)  return 2;

    double a11 = S.get<0,0>() * S.get<2,2>() - S.get<0,2>() * S.get<2,0>();
    if (a11 != 0)  return 2;

    return 1;
}
*/

} /* end namespace NL*/ } /* end namespace Geom*/




#endif // _NL_TRACE_H_




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
