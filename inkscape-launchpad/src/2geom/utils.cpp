/** Various utility functions.
 *
 * Copyright 2008 Marco Cecchetti <mrcekets at gmail.com>
 * Copyright 2007 Johan Engelen <goejendaagh@zonnet.nl>
 * Copyright 2006 Michael G. Sloan <mgsloan@gmail.com>
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


#include <2geom/utils.h>


namespace Geom 
{

// return a vector that contains all the binomial coefficients of degree n 
void binomial_coefficients(std::vector<size_t>& bc, std::size_t n)
{
    size_t s = n+1;
    bc.clear();
    bc.resize(s);
    bc[0] = 1;
    for (size_t i = 1; i < n; ++i)
    {
        size_t k = i >> 1;
        if (i & 1u)
        {
            bc[k+1] = bc[k] << 1;
        }
        for (size_t j = k; j > 0; --j)
        {
            bc[j] += bc[j-1];
        }
    }
    s >>= 1;
    for (size_t i = 0; i < s; ++i)
    {
        bc[n-i] = bc[i];
    }
}

} // end namespace Geom











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
