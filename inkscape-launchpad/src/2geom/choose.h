/**
 * \file
 * \brief Calculation of binomial cefficients
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

#ifndef LIB2GEOM_SEEN_CHOOSE_H
#define LIB2GEOM_SEEN_CHOOSE_H

#include <vector>

namespace Geom {

// XXX: Can we keep only the left terms easily?
// this would more than halve the array
// row index becomes n2 = n/2, row2 = n2*(n2+1)/2, row = row2*2+(n&1)?n2:0
// we could also leave off the ones

template <typename T>
T choose(unsigned n, unsigned k) {
    static std::vector<T> pascals_triangle;
    static unsigned rows_done = 0;
    // indexing is (0,0,), (1,0), (1,1), (2, 0)...
    // to get (i, j) i*(i+1)/2 + j
    if(/*k < 0 ||*/ k > n) return 0;
    if(rows_done <= n) {// we haven't got there yet
        if(rows_done == 0) {
            pascals_triangle.push_back(1);
            rows_done = 1;
        }
        while(rows_done <= n) {
            unsigned p = pascals_triangle.size() - rows_done;
            pascals_triangle.push_back(1);
            for(unsigned i = 0; i < rows_done-1; i++) {
                pascals_triangle.push_back(pascals_triangle[p]
                                           + pascals_triangle[p+1]);
		p++;
            }
            pascals_triangle.push_back(1);
            rows_done ++;
        }
    }
    unsigned row = (n*(n+1))/2;
    return pascals_triangle[row+k];
}

// Is it faster to store them or compute them on demand?
/*template <typename T>
T choose(unsigned n, unsigned k) {
	T r = 1;
	for(unsigned i = 1; i <= k; i++)
		r = (r*(n-k+i))/i;
	return r;
	}*/



template <typename ValueType>
class BinomialCoefficient
{
  public:
    typedef ValueType value_type;
    typedef std::vector<value_type> container_type;

    BinomialCoefficient(unsigned int _n)
        : n(_n), m(n >> 1)
    {
        coefficients.reserve(m+1);
        coefficients.push_back(1);
        int h = m + 1;
        value_type bct = 1;
        for (int i = 1; i < h; ++i)
        {
            bct *= (n-i+1);
            bct /= i;
            coefficients.push_back(bct);
        }
    }

    unsigned int size() const
    {
        return degree() +1;
    }

    unsigned int degree() const
    {
        return n;
    }

    value_type operator[] (unsigned int k) const
    {
        if (k > m)  k = n - k;
        return coefficients[k];
    }

  private:
    const int n;
    const unsigned int m;
    container_type coefficients;
};

} // end namespace Geom

#endif // LIB2GEOM_SEEN_CHOOSE_H

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
