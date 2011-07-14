#ifndef LIBNR_NR_RECT_H_SEEN
#define LIBNR_NR_RECT_H_SEEN

/** \file
 * Definitions of NRRect and NR::Rect types, and some associated functions \& macros.
 *//*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   MenTaLguY <mental@rydia.net>
 *
 * This code is in public domain
 */

#include <stdexcept>
#include <limits>
#include <boost/optional.hpp>
#include <glib.h>
#include <2geom/rect.h>

#include "libnr/nr-forward.h"
#include "libnr/nr-values.h"
#include "libnr/nr-macros.h"

/* legacy rect stuff */
/* NULL rect is infinite */

struct NRRect {
    NRRect()
    : x0(0), y0(0), x1(0), y1(0)
    {}
    NRRect(double xmin, double ymin, double xmax, double ymax)
    : x0(xmin), y0(ymin), x1(xmax), y1(ymax)
    {}
    explicit NRRect(Geom::OptRect const &rect);
    operator Geom::OptRect() const { return upgrade_2geom(); }
    Geom::OptRect upgrade_2geom() const;

    double x0, y0, x1, y1;
};

// TODO convert to static overloaded functions (pointer and ref) once performance can be tested:
#define nr_rect_l_test_empty_ptr(r) ((r) && NR_RECT_DFLS_TEST_EMPTY(r))
#define nr_rect_l_test_empty(r) NR_RECT_DFLS_TEST_EMPTY_REF(r)

#define nr_rect_d_test_intersect(r0,r1) \
        (!nr_rect_d_test_empty(r0) && !nr_rect_d_test_empty(r1) && \
         !((r0) && (r1) && !NR_RECT_DFLS_TEST_INTERSECT(r0, r1)))

// TODO convert to static overloaded functions (pointer and ref) once performance can be tested:
#define nr_rect_l_test_intersect_ptr(r0,r1) \
        (!nr_rect_l_test_empty_ptr(r0) && !nr_rect_l_test_empty_ptr(r1) && \
         !((r0) && (r1) && !NR_RECT_DFLS_TEST_INTERSECT(r0, r1)))
#define nr_rect_l_test_intersect(r0,r1) \
        (!nr_rect_l_test_empty(r0) && !nr_rect_l_test_empty(r1) && \
         !(!NR_RECT_DFLS_TEST_INTERSECT_REF(r0, r1)))

#define nr_rect_d_point_d_test_inside(r,p) ((p) && (!(r) || (!NR_RECT_DF_TEST_EMPTY(r) && NR_RECT_DF_POINT_DF_TEST_INSIDE(r,p))))
#define nr_rect_l_point_l_test_inside(r,p) ((p) && (!(r) || (!NR_RECT_DFLS_TEST_EMPTY(r) && NR_RECT_LS_POINT_LS_TEST_INSIDE(r,p))))
#define nr_rect_l_test_inside(r,x,y) ((!(r) || (!NR_RECT_DFLS_TEST_EMPTY(r) && NR_RECT_LS_TEST_INSIDE(r,x,y))))

// returns minimal rect which covers all of r0 not covered by r1
NRRectL *nr_rect_l_subtract(NRRectL *d, NRRectL const *r0, NRRectL const *r1);

// returns the area of r
gint32 nr_rect_l_area(NRRectL *r);

/* NULL values are OK for r0 and r1, but not for d */
NRRect *nr_rect_d_intersect(NRRect *d, NRRect const *r0, NRRect const *r1);
NRRectL *nr_rect_l_intersect(NRRectL *d, NRRectL const *r0, NRRectL const *r1);

NRRect *nr_rect_d_union(NRRect *d, NRRect const *r0, NRRect const *r1);
NRRectL *nr_rect_l_union(NRRectL *d, NRRectL const *r0, NRRectL const *r1);

NRRect *nr_rect_union_pt(NRRect *dst, Geom::Point const &p);
NRRect *nr_rect_d_union_xy(NRRect *d, double x, double y);
NRRectL *nr_rect_l_union_xy(NRRectL *d, gint32 x, gint32 y);


#endif /* !LIBNR_NR_RECT_H_SEEN */

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
