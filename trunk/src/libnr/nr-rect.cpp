#define __NR_RECT_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include "nr-rect-l.h"
#include <algorithm>

NRRect::NRRect(NR::Rect const &rect)
: x0(rect.min()[NR::X]), y0(rect.min()[NR::Y]),
  x1(rect.max()[NR::X]), y1(rect.max()[NR::Y])
{}

NRRect::NRRect(boost::optional<NR::Rect> const &rect) {
    if (rect) {
        x0 = rect->min()[NR::X];
        y0 = rect->min()[NR::Y];
        x1 = rect->max()[NR::X];
        y1 = rect->max()[NR::Y];
    } else {
        nr_rect_d_set_empty(this);
    }
}

NRRect::NRRect(Geom::OptRect const &rect) {
    if (rect) {
        x0 = rect->min()[Geom::X];
        y0 = rect->min()[Geom::Y];
        x1 = rect->max()[Geom::X];
        y1 = rect->max()[Geom::Y];
    } else {
        nr_rect_d_set_empty(this);
    }
}

boost::optional<NR::Rect> NRRect::upgrade() const {
    if (nr_rect_d_test_empty_ptr(this)) {
        return boost::optional<NR::Rect>();
    } else {
        return NR::Rect(NR::Point(x0, y0), NR::Point(x1, y1));
    }
}

Geom::OptRect NRRect::upgrade_2geom() const {
    if (nr_rect_d_test_empty_ptr(this)) {
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
  NR::ICoord t;
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
	NR::Coord t;
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
        nr_rect_l_set_empty (d);

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

NR::ICoord nr_rect_l_area(NRRectL *r)
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
			nr_rect_d_set_empty (d);
		} else {
			*d = *r1;
		}
	} else {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			*d = *r0;
		} else {
			NR::Coord t;
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
			nr_rect_l_set_empty (d);
		} else {
			*d = *r1;
		}
	} else {
		if (NR_RECT_DFLS_TEST_EMPTY (r1)) {
			*d = *r0;
		} else {
			NR::ICoord t;
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
nr_rect_union_pt(NRRect *dst, NR::Point const &p)
{
	using NR::X;
	using NR::Y;

	return nr_rect_d_union_xy(dst, p[X], p[Y]);
}

NRRect *
nr_rect_d_union_xy (NRRect *d, NR::Coord x, NR::Coord y)
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

NRRect *
nr_rect_d_matrix_transform(NRRect *d, NRRect const *const s, NR::Matrix const &m)
{
    using NR::X;
    using NR::Y;

    if (nr_rect_d_test_empty_ptr(s)) {
        nr_rect_d_set_empty(d);
    } else {
        NR::Point const c00(NR::Point(s->x0, s->y0) * m);
        NR::Point const c01(NR::Point(s->x0, s->y1) * m);
        NR::Point const c10(NR::Point(s->x1, s->y0) * m);
        NR::Point const c11(NR::Point(s->x1, s->y1) * m);
        d->x0 = std::min(std::min(c00[X], c01[X]),
                         std::min(c10[X], c11[X]));
        d->y0 = std::min(std::min(c00[Y], c01[Y]),
                         std::min(c10[Y], c11[Y]));
        d->x1 = std::max(std::max(c00[X], c01[X]),
                         std::max(c10[X], c11[X]));
        d->y1 = std::max(std::max(c00[Y], c01[Y]),
                         std::max(c10[Y], c11[Y]));
    }
    return d;
}

NRRect *
nr_rect_d_matrix_transform(NRRect *d, NRRect const *s, NR::Matrix const *m)
{
    return nr_rect_d_matrix_transform(d, s, *m);
}

/** Enlarges the rectangle given amount of pixels to all directions */
NRRectL *
nr_rect_l_enlarge(NRRectL *d, int amount)
{
    d->x0 -= amount;
    d->y0 -= amount;
    d->x1 += amount;
    d->y1 += amount;
    return d;
}

namespace NR {

Rect::Rect(const Point &p0, const Point &p1)
: _min(std::min(p0[X], p1[X]), std::min(p0[Y], p1[Y])),
  _max(std::max(p0[X], p1[X]), std::max(p0[Y], p1[Y]))
{}

/** returns the four corners of the rectangle in the correct winding order */
Point Rect::corner(unsigned i) const {
	switch (i % 4) {
	case 0:
		return _min;
	case 1:
		return Point(_max[X], _min[Y]);
	case 2:
		return _max;
	default: /* i.e. 3 */
		return Point(_min[X], _max[Y]);
	}
}

/** returns the midpoint of this rectangle */
Point Rect::midpoint() const {
	return ( _min + _max ) / 2;
}

Point Rect::cornerFarthestFrom(Point const &p) const {
    Point m = midpoint();
    unsigned i = 0;
    if (p[X] < m[X]) {
        i = 1;
    }
    if (p[Y] < m[Y]) {
        i = 3 - i;
    }
    return corner(i);
}

/** returns a vector from topleft to bottom right. */
Point Rect::dimensions() const {
	return _max - _min;
}

/** Translates the rectangle by p. */
void Rect::offset(Point p) {
	_min += p;
	_max += p;
}

/** Makes this rectangle large enough to include the point p. */
void Rect::expandTo(Point p) {
	for ( int i=0 ; i < 2 ; i++ ) {
		_min[i] = std::min(_min[i], p[i]);
		_max[i] = std::max(_max[i], p[i]);
	}
}

void Rect::growBy(double size) {
  for ( unsigned d = 0 ; d < 2 ; d++ ) {
    _min[d] -= size;
    _max[d] += size;
    if ( _min[d] > _max[d] ) {
      _min[d] = _max[d] = ( _min[d] + _max[d] ) / 2;
    }
  }
} 

/** Returns the set of points shared by both rectangles. */
boost::optional<Rect> intersection(boost::optional<Rect> const & a, boost::optional<Rect> const & b) {
    if ( !a || !b ) {
        return boost::optional<Rect>();
    } else {
        Rect r;
        for ( int i=0 ; i < 2 ; i++ ) {
            r._min[i] = std::max(a->_min[i], b->_min[i]);
            r._max[i] = std::min(a->_max[i], b->_max[i]);
            if ( r._min[i] > r._max[i] ) {
            return boost::optional<Rect>();
        }
	}
	return r;
    }
}

/** returns the smallest rectangle containing both rectangles */
Rect union_bounds(Rect const &a, Rect const &b) {
    Rect r;
    for ( int i=0 ; i < 2 ; i++ ) {
        r._min[i] = std::min(a._min[i], b._min[i]);
        r._max[i] = std::max(a._max[i], b._max[i]);
    }
    return r;
}

}  // namespace NR


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
