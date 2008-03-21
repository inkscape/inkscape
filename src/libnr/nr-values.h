#ifndef __NR_VALUES_H__
#define __NR_VALUES_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-forward.h>

#define NR_EPSILON 1e-18

#define NR_HUGE   1e18
#define NR_HUGE_L (0x7fffffff)
#define NR_HUGE_S (0x7fff)

/*
The following predefined objects are for reference
and comparison.  They are defined in nr-values.cpp
*/
extern NR::Matrix NR_MATRIX_IDENTITY;
extern NRRect   NR_RECT_EMPTY;
extern NRRectL  NR_RECT_L_EMPTY;
extern NRRectL  NR_RECT_S_EMPTY;

/** component_vectors[i] has 1.0 at position i, and 0.0 elsewhere
    (i.e. in the other position). */
extern NR::Point const component_vectors[2];

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
