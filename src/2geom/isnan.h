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
 * 2005 modification hereby placed in public domain.  Probably supercedes the 2004 copyright
 * for the code itself.
 */

#include <cmath>
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
#elif defined(isnan) || defined(__FreeBSD__)
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
#elif defined(isfinite)
# define IS_FINITE(_a) (isfinite(_a))
#else
# define IS_FINITE(_a) (std::isfinite(_a))
#endif
/* If the above doesn't work, then try (finite(_a) && !isNaN(_a)) or (!isNaN((_a) - (_a))).
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
