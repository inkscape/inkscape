#define __NR_PATH_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <glib/gmem.h>
#include "n-art-bpath.h"
#include "nr-rect.h"
#include "nr-path.h"

static void nr_curve_bbox (NR::Coord x000, NR::Coord y000, NR::Coord x001, NR::Coord y001, NR::Coord x011, NR::Coord y011, NR::Coord x111, NR::Coord y111, NRRect *bbox);

static void nr_curve_bbox(NR::Point const p000, NR::Point const p001,
			  NR::Point const p011, NR::Point const p111,
			  NRRect *bbox);

NRBPath *nr_path_duplicate_transform(NRBPath *d, const_NRBPath *s, NR::Matrix const *transform)
{
	int i;

	if (!s->path) {
		d->path = NULL;
		return d;
	}

	i = 0;
	while (s->path[i].code != NR_END) i += 1;

	d->path = g_new (NArtBpath, i + 1);

	i = 0;
	while (s->path[i].code != NR_END) {
		d->path[i].code = s->path[i].code;
		if (s->path[i].code == NR_CURVETO) {
            d->path[i].setC(1, s->path[i].c(1) * (*transform));
            d->path[i].setC(2, s->path[i].c(2) * (*transform));
		}
        d->path[i].setC(3, s->path[i].c(3) * (*transform));
		i += 1;
	}
	d->path[i].code = NR_END;

	return d;
}

NRBPath *nr_path_duplicate_transform(NRBPath *d, const_NRBPath *s, NR::Matrix const transform) {
	NR::Matrix tr = transform;
	return nr_path_duplicate_transform(d, s, &tr);
}

NArtBpath* nr_artpath_affine(NArtBpath const *s, NR::Matrix const &aff) {
    const_NRBPath bp;
    bp.path = s;
    NRBPath abp;
    nr_path_duplicate_transform(&abp, &bp, aff);
    return abp.path;
}

static void
nr_line_wind_distance (NR::Coord x0, NR::Coord y0, NR::Coord x1, NR::Coord y1, NR::Point &pt, int *wind, NR::Coord *best)
{
	NR::Coord Ax, Ay, Bx, By, Dx, Dy, s;
	NR::Coord dist2;

	/* Find distance */
	Ax = x0;
	Ay = y0;
	Bx = x1;
	By = y1;
	Dx = x1 - x0;
	Dy = y1 - y0;
	const NR::Coord Px = pt[NR::X];
	const NR::Coord Py = pt[NR::Y];

	if (best) {
		s = ((Px - Ax) * Dx + (Py - Ay) * Dy) / (Dx * Dx + Dy * Dy);
		if (s <= 0.0) {
			dist2 = (Px - Ax) * (Px - Ax) + (Py - Ay) * (Py - Ay);
		} else if (s >= 1.0) {
			dist2 = (Px - Bx) * (Px - Bx) + (Py - By) * (Py - By);
		} else {
			NR::Coord Qx, Qy;
			Qx = Ax + s * Dx;
			Qy = Ay + s * Dy;
			dist2 = (Px - Qx) * (Px - Qx) + (Py - Qy) * (Py - Qy);
		}

		if (dist2 < (*best * *best)) *best = sqrt (dist2);
	}

	if (wind) {
		/* Find wind */
		if ((Ax >= Px) && (Bx >= Px)) return;
		if ((Ay >= Py) && (By >= Py)) return;
		if ((Ay < Py) && (By < Py)) return;
		if (Ay == By) return;
		/* Ctach upper y bound */
		if (Ay == Py) {
			if (Ax < Px) *wind -= 1;
			return;
		} else if (By == Py) {
			if (Bx < Px) *wind += 1;
			return;
		} else {
			NR::Coord Qx;
			/* Have to calculate intersection */
			Qx = Ax + Dx * (Py - Ay) / Dy;
			if (Qx < Px) {
				*wind += (Dy > 0.0) ? 1 : -1;
			}
		}
	}
}

static void
nr_curve_bbox_wind_distance (NR::Coord x000, NR::Coord y000,
			     NR::Coord x001, NR::Coord y001,
			     NR::Coord x011, NR::Coord y011,
			     NR::Coord x111, NR::Coord y111,
			     NR::Point &pt,
			     NRRect *bbox, int *wind, NR::Coord *best,
			     NR::Coord tolerance)
{
	NR::Coord x0, y0, x1, y1, len2;
	int needdist, needwind, needline;

	const NR::Coord Px = pt[NR::X];
	const NR::Coord Py = pt[NR::Y];

	needdist = 0;
	needwind = 0;
	needline = 0;

	if (bbox) nr_curve_bbox (x000, y000, x001, y001, x011, y011, x111, y111, bbox);

	x0 = MIN (x000, x001);
	x0 = MIN (x0, x011);
	x0 = MIN (x0, x111);
	y0 = MIN (y000, y001);
	y0 = MIN (y0, y011);
	y0 = MIN (y0, y111);
	x1 = MAX (x000, x001);
	x1 = MAX (x1, x011);
	x1 = MAX (x1, x111);
	y1 = MAX (y000, y001);
	y1 = MAX (y1, y011);
	y1 = MAX (y1, y111);

	if (best) {
		/* Quicly adjust to endpoints */
		len2 = (x000 - Px) * (x000 - Px) + (y000 - Py) * (y000 - Py);
		if (len2 < (*best * *best)) *best = (NR::Coord) sqrt (len2);
		len2 = (x111 - Px) * (x111 - Px) + (y111 - Py) * (y111 - Py);
		if (len2 < (*best * *best)) *best = (NR::Coord) sqrt (len2);

		if (((x0 - Px) < *best) && ((y0 - Py) < *best) && ((Px - x1) < *best) && ((Py - y1) < *best)) {
			/* Point is inside sloppy bbox */
			/* Now we have to decide, whether subdivide */
			/* fixme: (Lauris) */
			if (((y1 - y0) > 5.0) || ((x1 - x0) > 5.0)) {
				needdist = 1;
			} else {
				needline = 1;
			}
		}
	}
	if (!needdist && wind) {
		if ((y1 >= Py) && (y0 < Py) && (x0 < Px)) {
			/* Possible intersection at the left */
			/* Now we have to decide, whether subdivide */
			/* fixme: (Lauris) */
			if (((y1 - y0) > 5.0) || ((x1 - x0) > 5.0)) {
				needwind = 1;
			} else {
				needline = 1;
			}
		}
	}

	if (needdist || needwind) {
		NR::Coord x00t, x0tt, xttt, x1tt, x11t, x01t;
		NR::Coord y00t, y0tt, yttt, y1tt, y11t, y01t;
		NR::Coord s, t;

		t = 0.5;
		s = 1 - t;

		x00t = s * x000 + t * x001;
		x01t = s * x001 + t * x011;
		x11t = s * x011 + t * x111;
		x0tt = s * x00t + t * x01t;
		x1tt = s * x01t + t * x11t;
		xttt = s * x0tt + t * x1tt;

		y00t = s * y000 + t * y001;
		y01t = s * y001 + t * y011;
		y11t = s * y011 + t * y111;
		y0tt = s * y00t + t * y01t;
		y1tt = s * y01t + t * y11t;
		yttt = s * y0tt + t * y1tt;

		nr_curve_bbox_wind_distance (x000, y000, x00t, y00t, x0tt, y0tt, xttt, yttt, pt, NULL, wind, best, tolerance);
		nr_curve_bbox_wind_distance (xttt, yttt, x1tt, y1tt, x11t, y11t, x111, y111, pt, NULL, wind, best, tolerance);
	} else if (1 || needline) {
		nr_line_wind_distance (x000, y000, x111, y111, pt, wind, best);
	}
}

void
nr_path_matrix_point_bbox_wind_distance (const_NRBPath const *bpath, NR::Matrix const &m, NR::Point &pt,
					     NRRect *bbox, int *wind, NR::Coord *dist,
						 NR::Coord tolerance, NR::Rect *viewbox)
{
	if (!bpath->path) {
		if (wind) *wind = 0;
		if (dist) *dist = NR_HUGE;
		return;
	}

	NR::Coord x0 = 0;
	NR::Coord y0 = 0;
	NR::Coord x1 = 0;
	NR::Coord y1 = 0;
	NR::Coord x2 = 0;
	NR::Coord y2 = 0;
	NR::Coord x3 = 0;
	NR::Coord y3 = 0;

	// remembering the start of subpath
	NR::Coord x_start = 0, y_start = 0; bool start_set = false;
	NR::Rect swept;

	for (const NArtBpath *p = bpath->path; p->code != NR_END; p+= 1) {
		switch (p->code) {
		case NR_MOVETO_OPEN:
		case NR_MOVETO:
			if (start_set) { // this is a new subpath
				if (wind && (x0 != x_start || y0 != y_start)) // for correct fill picking, each subpath must be closed
					nr_line_wind_distance (x0, y0, x_start, y_start, pt, wind, dist);
			}
			x0 = m[0] * p->x3 + m[2] * p->y3 + m[4];
			y0 = m[1] * p->x3 + m[3] * p->y3 + m[5];
			x_start = x0; y_start = y0; start_set = true;
			if (bbox) {
				bbox->x0 = (NR::Coord) MIN (bbox->x0, x0);
				bbox->y0 = (NR::Coord) MIN (bbox->y0, y0);
				bbox->x1 = (NR::Coord) MAX (bbox->x1, x0);
				bbox->y1 = (NR::Coord) MAX (bbox->y1, y0);
			}
			break;
		case NR_LINETO:
			x3 = m[0] * p->x3 + m[2] * p->y3 + m[4];
			y3 = m[1] * p->x3 + m[3] * p->y3 + m[5];
			if (bbox) {
				bbox->x0 = (NR::Coord) MIN (bbox->x0, x3);
				bbox->y0 = (NR::Coord) MIN (bbox->y0, y3);
				bbox->x1 = (NR::Coord) MAX (bbox->x1, x3);
				bbox->y1 = (NR::Coord) MAX (bbox->y1, y3);
			}
			if (dist || wind) {
				if (wind) { // we need to pick fill, so do what we're told
					nr_line_wind_distance (x0, y0, x3, y3, pt, wind, dist);
				} else { // only stroke is being picked; skip this segment if it's totally outside the viewbox
					swept = NR::Rect(NR::Point(x0, y0), NR::Point(x3, y3));
					//std::cout << "swept: " << swept;
					//std::cout << "view: " << *viewbox;
					//std::cout << "intersects? " << (swept.intersects(*viewbox)? "YES" : "NO") << "\n";
					if (!viewbox || swept.intersects(*viewbox))
   					nr_line_wind_distance (x0, y0, x3, y3, pt, wind, dist);
				}
			}
			x0 = x3;
			y0 = y3;
			break;
		case NR_CURVETO:
			{
			x3 = m[0] * p->x3 + m[2] * p->y3 + m[4];
			y3 = m[1] * p->x3 + m[3] * p->y3 + m[5];
			x1 = m[0] * p->x1 + m[2] * p->y1 + m[4];
			y1 = m[1] * p->x1 + m[3] * p->y1 + m[5];
			x2 = m[0] * p->x2 + m[2] * p->y2 + m[4];
			y2 = m[1] * p->x2 + m[3] * p->y2 + m[5];

			swept = NR::Rect(NR::Point(x0, y0), NR::Point(x3, y3));
			swept.expandTo(NR::Point(x1, y1));
			swept.expandTo(NR::Point(x2, y2));

			if (!viewbox || swept.intersects(*viewbox)) { // we see this segment, so do full processing
				nr_curve_bbox_wind_distance (
                       x0, y0,
                       x1, y1,
                       x2, y2,
						     x3, y3,
						     pt,
						     bbox, wind, dist, tolerance);
			} else {
				if (wind) { // if we need fill, we can just pretend it's a straight line
					nr_line_wind_distance (x0, y0, x3, y3, pt, wind, dist);
				} else { // otherwise, skip it completely
				}
			}
			x0 = x3;
			y0 = y3;
			}
			break;
		default:
			break;
		}
	}

	if (start_set) { 
		if (wind && (x0 != x_start || y0 != y_start)) // for correct picking, each subpath must be closed
			nr_line_wind_distance (x0, y0, x_start, y_start, pt, wind, dist);
	}
}

static void
nr_curve_bbox(NR::Point const p000, NR::Point const p001,
	      NR::Point const p011, NR::Point const p111,
	      NRRect *bbox)
{
	using NR::X;
	using NR::Y;

	nr_curve_bbox(p000[X], p000[Y],
		      p001[X], p001[Y],
		      p011[X], p011[Y],
		      p111[X], p111[Y],
		      bbox);
}

/* Fast bbox calculation */
/* Thanks to Nathan Hurst for suggesting it */

static void
nr_curve_bbox (NR::Coord x000, NR::Coord y000, NR::Coord x001, NR::Coord y001, NR::Coord x011, NR::Coord y011, NR::Coord x111, NR::Coord y111, NRRect *bbox)
{
	NR::Coord a, b, c, D;

	bbox->x0 = (NR::Coord) MIN (bbox->x0, x111);
	bbox->y0 = (NR::Coord) MIN (bbox->y0, y111);
	bbox->x1 = (NR::Coord) MAX (bbox->x1, x111);
	bbox->y1 = (NR::Coord) MAX (bbox->y1, y111);

	/*
	 * xttt = s * (s * (s * x000 + t * x001) + t * (s * x001 + t * x011)) + t * (s * (s * x001 + t * x011) + t * (s * x011 + t * x111))
	 * xttt = s * (s2 * x000 + s * t * x001 + t * s * x001 + t2 * x011) + t * (s2 * x001 + s * t * x011 + t * s * x011 + t2 * x111)
	 * xttt = s * (s2 * x000 + 2 * st * x001 + t2 * x011) + t * (s2 * x001 + 2 * st * x011 + t2 * x111)
	 * xttt = s3 * x000 + 2 * s2t * x001 + st2 * x011 + s2t * x001 + 2st2 * x011 + t3 * x111
	 * xttt = s3 * x000 + 3s2t * x001 + 3st2 * x011 + t3 * x111
	 * xttt = s3 * x000 + (1 - s) 3s2 * x001 + (1 - s) * (1 - s) * 3s * x011 + (1 - s) * (1 - s) * (1 - s) * x111
	 * xttt = s3 * x000 + (3s2 - 3s3) * x001 + (3s - 6s2 + 3s3) * x011 + (1 - 2s + s2 - s + 2s2 - s3) * x111
	 * xttt = (x000 - 3 * x001 + 3 * x011 -     x111) * s3 +
	 *        (       3 * x001 - 6 * x011 + 3 * x111) * s2 +
	 *        (                  3 * x011 - 3 * x111) * s  +
	 *        (                                 x111)
	 * xttt' = (3 * x000 - 9 * x001 +  9 * x011 - 3 * x111) * s2 +
	 *         (           6 * x001 - 12 * x011 + 6 * x111) * s  +
	 *         (                       3 * x011 - 3 * x111)
	 */

	a = 3 * x000 - 9 * x001 + 9 * x011 - 3 * x111;
	b = 6 * x001 - 12 * x011 + 6 * x111;
	c = 3 * x011 - 3 * x111;

	/*
	 * s = (-b +/- sqrt (b * b - 4 * a * c)) / 2 * a;
	 */
	if (fabs (a) < NR_EPSILON) {
		/* s = -c / b */
		if (fabs (b) > NR_EPSILON) {
			double s, t, xttt;
			s = -c / b;
			if ((s > 0.0) && (s < 1.0)) {
				t = 1.0 - s;
				xttt = s * s * s * x000 + 3 * s * s * t * x001 + 3 * s * t * t * x011 + t * t * t * x111;
				bbox->x0 = (float) MIN (bbox->x0, xttt);
				bbox->x1 = (float) MAX (bbox->x1, xttt);
			}
		}
	} else {
		/* s = (-b +/- sqrt (b * b - 4 * a * c)) / 2 * a; */
		D = b * b - 4 * a * c;
		if (D >= 0.0) {
			NR::Coord d, s, t, xttt;
			/* Have solution */
			d = sqrt (D);
			s = (-b + d) / (2 * a);
			if ((s > 0.0) && (s < 1.0)) {
				t = 1.0 - s;
				xttt = s * s * s * x000 + 3 * s * s * t * x001 + 3 * s * t * t * x011 + t * t * t * x111;
				bbox->x0 = (NR::Coord) MIN (bbox->x0, xttt);
				bbox->x1 = (NR::Coord) MAX (bbox->x1, xttt);
			}
			s = (-b - d) / (2 * a);
			if ((s > 0.0) && (s < 1.0)) {
				t = 1.0 - s;
				xttt = s * s * s * x000 + 3 * s * s * t * x001 + 3 * s * t * t * x011 + t * t * t * x111;
				bbox->x0 = (NR::Coord) MIN (bbox->x0, xttt);
				bbox->x1 = (NR::Coord) MAX (bbox->x1, xttt);
			}
		}
	}

	a = 3 * y000 - 9 * y001 + 9 * y011 - 3 * y111;
	b = 6 * y001 - 12 * y011 + 6 * y111;
	c = 3 * y011 - 3 * y111;

	if (fabs (a) < NR_EPSILON) {
		/* s = -c / b */
		if (fabs (b) > NR_EPSILON) {
			double s, t, yttt;
			s = -c / b;
			if ((s > 0.0) && (s < 1.0)) {
				t = 1.0 - s;
				yttt = s * s * s * y000 + 3 * s * s * t * y001 + 3 * s * t * t * y011 + t * t * t * y111;
				bbox->y0 = (float) MIN (bbox->y0, yttt);
				bbox->y1 = (float) MAX (bbox->y1, yttt);
			}
		}
	} else {
		/* s = (-b +/- sqrt (b * b - 4 * a * c)) / 2 * a; */
		D = b * b - 4 * a * c;
		if (D >= 0.0) {
			NR::Coord d, s, t, yttt;
			/* Have solution */
			d = sqrt (D);
			s = (-b + d) / (2 * a);
			if ((s > 0.0) && (s < 1.0)) {
				t = 1.0 - s;
				yttt = s * s * s * y000 + 3 * s * s * t * y001 + 3 * s * t * t * y011 + t * t * t * y111;
				bbox->y0 = (NR::Coord) MIN (bbox->y0, yttt);
				bbox->y1 = (NR::Coord) MAX (bbox->y1, yttt);
			}
			s = (-b - d) / (2 * a);
			if ((s > 0.0) && (s < 1.0)) {
				t = 1.0 - s;
				yttt = s * s * s * y000 + 3 * s * s * t * y001 + 3 * s * t * t * y011 + t * t * t * y111;
				bbox->y0 = (NR::Coord) MIN (bbox->y0, yttt);
				bbox->y1 = (NR::Coord) MAX (bbox->y1, yttt);
			}
		}
	}
}

void
nr_path_matrix_bbox_union(const_NRBPath *bpath, NR::Matrix const &m,
			  NRRect *bbox)
{
    using NR::X;
    using NR::Y;

    if (!bpath->path) {
	return;
    }

    NR::Point prev0(0, 0);
    for (NArtBpath const *p = bpath->path; p->code != NR_END; ++p) {
	switch (p->code) {
	    case NR_MOVETO_OPEN:
	    case NR_MOVETO: {
		NR::Point const c3(p->x3,
				   p->y3);
		prev0 = c3 * m;
		nr_rect_union_pt(bbox, prev0);
		break;
	    }

	    case NR_LINETO: {
		NR::Point const c3(p->x3,
				   p->y3);
		NR::Point const endPt( c3 * m );
		nr_rect_union_pt(bbox, endPt);
		prev0 = endPt;
		break;
	    }

	    case NR_CURVETO: {
		NR::Point const c1(p->x1, p->y1);
		NR::Point const c2(p->x2, p->y2);
		NR::Point const c3(p->x3, p->y3);
		NR::Point const endPt( c3 * m );
		nr_curve_bbox(prev0,
			      c1 * m,
			      c2 * m,
			      endPt,
			      bbox);
		prev0 = endPt;
		break;
	    }

	    default:
	        break;
	}
    }
}

NArtBpath *nr_path_from_rect(NRRect const &r)
{
    NArtBpath *path = g_new (NArtBpath, 6);

    path[0].code = NR_MOVETO;
    path[0].setC(3, NR::Point(r.x0, r.y0));
    path[1].code = NR_LINETO;
    path[1].setC(3, NR::Point(r.x1, r.y0)); 
    path[2].code = NR_LINETO;
    path[2].setC(3, NR::Point(r.x1, r.y1)); 
    path[3].code = NR_LINETO;
    path[3].setC(3, NR::Point(r.x0, r.y1));
    path[4].code = NR_LINETO;
    path[4].setC(3, NR::Point(r.x0, r.y0));    
    path[5].code = NR_END;

    return path;
}

