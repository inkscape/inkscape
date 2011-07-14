#define __NR_RECT_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <algorithm>
#include "nr-rect.h"
#include "nr-rect-l.h"

NRRect::NRRect(Geom::OptRect const &rect) {
    if (rect) {
        x0 = rect->min()[Geom::X];
        y0 = rect->min()[Geom::Y];
        x1 = rect->max()[Geom::X];
        y1 = rect->max()[Geom::Y];
    } else {
        *this = NR_RECT_EMPTY;
    }
}

Geom::OptRect NRRect::upgrade_2geom() const {
    if (x0 > x1 || y0 > y1) {
        return Geom::OptRect();
    } else {
        return Geom::Rect(Geom::Point(x0, y0), Geom::Point(x1, y1));
    }
}

/**
 *    \param r0 Rectangle.
 *    \param r1 Another rectangle.
 *    \param d Filled in with the intersection of r0 and r1.
 *    \return d.
 */

NRRectL *nr_rect_l_intersect(NRRectL *d, const NRRectL *r0, const NRRectL *r1)
{
  gint32 t;
  t = std::max(r0->x0, r1->x0);
  d->x1 = std::min(r0->x1, r1->x1);
  d->x0 = t;
  t = std::max(r0->y0, r1->y0);
  d->y1 = std::min(r0->y1, r1->y1);
  d->y0 = t;
	
  return d;
}

NRRect *
nr_rect_d_intersect (NRRect *d, const NRRect *r0, const NRRect *r1)
{
	gint32 t;
	t = MAX (r0->x0, r1->x0);
	d->x1 = MIN (r0->x1, r1->x1);
	d->x0 = t;
	t = MAX (r0->y0, r1->y0);
	d->y1 = MIN (r0->y1, r1->y1);
	d->y0 = t;
	
	return d;
}

// returns minimal rect which covers all of r0 not covered by r1
NRRectL *
nr_rect_l_subtract(NRRectL *d, NRRectL const *r0, NRRectL const *r1)
{
    bool inside1 = nr_rect_l_test_inside(r1, r0->x0, r0->y0);
    bool inside2 = nr_rect_l_test_inside(r1, r0->x1, r0->y0);
    bool inside3 = nr_rect_l_test_inside(r1, r0->x1, r0->y1);
    bool inside4 = nr_rect_l_test_inside(r1, r0->x0, r0->y1);

    if (inside1 && inside2 && inside3) {
        *d = NR_RECT_L_EMPTY;

    } else if (inside1 && inside2) {
        d->x0 = r0->x0;
        d->y0 = r1->y1;

        d->x1 = r0->x1;
        d->y1 = r0->y1;
    } else if (inside2 && inside3) {
        d->x0 = r0->x0;
        d->y0 = r0->y0;

        d->x1 = r1->x0;
        d->y1 = r0->y1;
    } else if (inside3 && inside4) {
        d->x0 = r0->x0;
        d->y0 = r0->y0;

        d->x1 = r0->x1;
        d->y1 = r1->y0;
    } else if (inside4 && inside1) {
        d->x0 = r1->x1;
        d->y0 = r0->y0;

        d->x1 = r0->x1;
        d->y1 = r0->y1;
    } else {
        d->x0 = r0->x0;
        d->y0 = r0->y0;

        d->x1 = r0->x1;
        d->y1 = r0->y1;
    }
    return d;
}

gint32 nr_rect_l_area(NRRectL *r)
{
  if (!r || NR_RECT_DFLS_TEST_EMPTY (r)) {
      return 0;
  }
  return ((r->x1 - r->x0) * (r->y1 - r->y0));
}

NRRect *
nr_rect_d_union (NRRect *d, const NRRect *r0, const NRRect *r1)
{
	if (NR_RECT_DFLS_TEST_EMPTY (r0)) {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			*d = NR_RECT_EMPTY;
		} else {
			*d = *r1;
		}
	} else {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			*d = *r0;
		} else {
			double t;
			t = MIN (r0->x0, r1->x0);
			d->x1 = MAX (r0->x1, r1->x1);
			d->x0 = t;
			t = MIN (r0->y0, r1->y0);
			d->y1 = MAX (r0->y1, r1->y1);
			d->y0 = t;
		}
	}
	return d;
}

NRRectL *
nr_rect_l_union (NRRectL *d, const NRRectL *r0, const NRRectL *r1)
{
	if (NR_RECT_DFLS_TEST_EMPTY (r0)) {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			*d = NR_RECT_L_EMPTY;
		} else {
			*d = *r1;
		}
	} else {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			*d = *r0;
		} else {
			double t;
			t = MIN (r0->x0, r1->x0);
			d->x1 = MAX (r0->x1, r1->x1);
			d->x0 = t;
			t = MIN (r0->y0, r1->y0);
			d->y1 = MAX (r0->y1, r1->y1);
			d->y0 = t;
		}
	}
	return d;
}

NRRect *
nr_rect_union_pt(NRRect *dst, Geom::Point const &p)
{
	return nr_rect_d_union_xy(dst, p[Geom::X], p[Geom::Y]);
}

NRRect *
nr_rect_d_union_xy (NRRect *d, double x, double y)
{
	if ((d->x0 <= d->x1) && (d->y0 <= d->y1)) {
		d->x0 = MIN (d->x0, x);
		d->y0 = MIN (d->y0, y);
		d->x1 = MAX (d->x1, x);
		d->y1 = MAX (d->y1, y);
	} else {
		d->x0 = d->x1 = x;
		d->y0 = d->y1 = y;
	}
	return d;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
