/**
 * \file
 * \brief  \todo brief description
 *
 * Authors:
 *      ? <?@?.?>
 * 
 * Copyright ?-?  authors
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

#ifndef _2GEOM_ISNAN_H__
#define _2GEOM_ISNAN_H__

/*
 * Temporary fix for various misdefinitions of isnan().
 * isnan() is becoming undef'd in some .h files. 
 * #include this last in your .cpp file to get it right.
 *
 * The problem is that isnan and isfinite are part of C99 but aren't part of
 * the C++ standard (which predates C99).
 *
 * Authors:
 *   Inkscape groupies and obsessive-compulsives
 *
 * Copyright (C) 2004 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 * 2005 modification hereby placed in public domain.  Probably supercedes 
 * the 2004 copyright for the code itself.
 */

#include <math.h>
/* You might try changing the above to <cmath> if you have problems.
 * Whether you use math.h or cmath, you may need to edit the .cpp file
 * and/or other .h files to use the same header file.
 */

#if defined(__isnan)
# define IS_NAN(_a) (__isnan(_a))
#elif defined(__APPLE__) && __GNUC__ == 3
# define IS_NAN(_a) (__isnan(_a))	/* MacOSX/Darwin definition < 10.4 */
#elif defined(WIN32) || defined(_isnan)
# define IS_NAN(_a) (_isnan(_a)) 	/* Win32 definition */
#elif defined(isnan) || defined(__FreeBSD__) || defined(__osf__)
# define IS_NAN(_a) (isnan(_a))		/* GNU definition */
#elif defined (SOLARIS_2_8) && __GNUC__ == 3 && __GNUC_MINOR__ == 2
# define IS_NAN(_a) (isnan(_a))		/* GNU definition */
#else
# define IS_NAN(_a) (std::isnan(_a))
#endif
/* If the above doesn't work, then try (a != a).
 * Also, please report a bug as per http://www.inkscape.org/report_bugs.php,
 * giving information about what platform and compiler version you're using.
 */


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
#elif defined (SOLARIS_2_8) && __GNUC__ == 3 && __GNUC_MINOR__ == 2
#include  <ieeefp.h>
#define IS_FINITE(_a) (finite(_a) && !IS_NAN(_a))
#else
# define IS_FINITE(_a) (std::isfinite(_a))
#endif
/* If the above doesn't work, then try (finite(_a) && !IS_NAN(_a)) or 
 * (!IS_NAN((_a) - (_a))).
 * Also, please report a bug as per http://www.inkscape.org/report_bugs.php,
 * giving information about what platform and compiler version you're using.
 */


#endif /* _2GEOM_ISNAN_H__ */

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
