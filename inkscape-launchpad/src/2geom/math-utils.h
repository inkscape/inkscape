/**
 * \file
 * \brief Low level math functions and compatibility wrappers
 *//*
 * Authors:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   Michael G. Sloan <mgsloan@gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * Copyright 2006-2009 Authors
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

#ifndef LIB2GEOM_SEEN_MATH_UTILS_H
#define LIB2GEOM_SEEN_MATH_UTILS_H

#include "config.h"
#include <math.h> // sincos is usually only available in math.h
#include <cmath>
#include <utility> // for std::pair
#include <boost/math/special_functions/fpclassify.hpp>

namespace Geom {

/** @brief Sign function - indicates the sign of a numeric type.
 * Mathsy people will know this is basically the derivative of abs, except for the fact
 * that it is defined on 0.
 * @return -1 when x is negative, 1 when positive, and 0 if equal to 0. */
template <class T> inline int sgn(const T& x) {
    return (x < 0 ? -1 : (x > 0 ? 1 : 0) );
// can we 'optimize' this with:
//    return ( (T(0) < x) - (x < T(0)) );
}

template <class T> inline T sqr(const T& x) {return x * x;}
template <class T> inline T cube(const T& x) {return x * x * x;}

/** Between function - returns true if a number x is within a range: (min < x) && (max > x).
 * The values delimiting the range and the number must have the same type.
 */
template <class T> inline const T& between (const T& min, const T& max, const T& x)
    { return (min < x) && (max > x); }

/** @brief Returns @a x rounded to the nearest multiple of \f$10^{p}\f$.

    Implemented in terms of round, i.e. we make no guarantees as to what happens if x is
    half way between two rounded numbers.
    
    Note: places is the number of decimal places without using scientific (e) notation, not the
    number of significant figures.  This function may not be suitable for values of x whose
    magnitude is so far from 1 that one would want to use scientific (e) notation.

    places may be negative: e.g. places = -2 means rounding to a multiple of .01
**/
inline double decimal_round(double x, int p) {
    //TODO: possibly implement with modulus instead?
    double const multiplier = ::pow(10.0, p);
    return ::round( x * multiplier ) / multiplier;
}

/** @brief Simultaneously compute a sine and a cosine of the same angle.
 * This function can be up to 2 times faster than separate computation, depending
 * on the platform. It uses the standard library function sincos() if available.
 * @param angle Angle
 * @param sin_ Variable that will store the sine
 * @param cos_ Variable that will store the cosine */
inline void sincos(double angle, double &sin_, double &cos_) {
#ifdef HAVE_SINCOS
    ::sincos(angle, &sin_, &cos_);
#else
    sin_ = ::sin(angle);
    cos_ = ::cos(angle);
#endif
}

/* Temporary fix for various misdefinitions of isnan().
 * isnan() is becoming undef'd in some .h files. 
 * #include this last in your .cpp file to get it right.
 *
 * The problem is that isnan and isfinite are part of C99 but aren't part of
 * the C++ standard (which predates C99).
 */

#if defined(__isnan)
# define IS_NAN(_a) (__isnan(_a))
#elif defined(__APPLE__) && __GNUC__ == 3
# define IS_NAN(_a) (__isnan(_a))	/* MacOSX/Darwin definition < 10.4 */
#elif defined(WIN32) || defined(_isnan)
# define IS_NAN(_a) (_isnan(_a)) 	/* Win32 definition */
#elif defined(isnan) || defined(__FreeBSD__) || defined(__osf__)
# define IS_NAN(_a) (isnan(_a))		/* GNU definition */
#elif defined (SOLARIS)
# define IS_NAN(_a) (isnan(_a))		/* GNU definition */
#else
# define IS_NAN(_a) (boost::math::isnan(_a))
#endif
/* If the above doesn't work, then try (a != a). */


#if defined(__isfinite)
# define IS_FINITE(_a) (__isfinite(_a))
#elif defined(__APPLE__) && __GNUC__ == 3
# define IS_FINITE(_a) (__isfinite(_a))	/* MacOSX/Darwin definition < 10.4 */
#elif defined(__sgi)
# define IS_FINITE(_a) (_isfinite(_a))
#elif defined(isfinite)
# define IS_FINITE(_a) (isfinite(_a))
#elif defined(__osf__)
# define IS_FINITE(_a) (finite(_a) && !IS_NAN(_a))
#elif defined (SOLARIS)
#include  <ieeefp.h>
#define IS_FINITE(_a) (finite(_a) && !IS_NAN(_a))
#else
# define IS_FINITE(_a) (boost::math::isfinite(_a))
#endif

} // end namespace Geom

#endif // LIB2GEOM_SEEN_MATH_UTILS_H

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
