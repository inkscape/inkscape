#define __CURVE_C__

/** \file
 * Routines for SPCurve and for NArtBpath arrays in general.
 */

/*
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

#include <string.h>
#include <glib/gmem.h>
#include <display/curve.h>
#include <libnr/n-art-bpath.h>
#include <libnr/nr-point-matrix-ops.h>
#include <libnr/nr-translate-ops.h>
#include <cstring>
#include <string>

#define SP_CURVE_LENSTEP 32

static unsigned sp_bpath_length(NArtBpath const bpath[]);
static bool sp_bpath_closed(NArtBpath const bpath[]);

/* Constructors */

/**
 * The returned curve's state is as if sp_curve_reset has just been called on it.
 */
SPCurve *
sp_curve_new()
{
    return sp_curve_new_sized(SP_CURVE_LENSTEP);
}

/**
 * Like sp_curve_new, but overriding the default initial capacity.
 *
 * The returned curve's state is as if sp_curve_reset has just been called on it.
 *
 * \param length Initial number of NArtBpath elements allocated for bpath (including NR_END
 *    element).
 */
SPCurve *
sp_curve_new_sized(gint length)
{
    g_return_val_if_fail(length > 0, NULL);

    SPCurve *curve = g_new(SPCurve, 1);

    curve->refcount = 1;
    curve->_bpath = g_new(NArtBpath, length);
    curve->_bpath->code = NR_END;
    curve->end = 0;
    curve->length = length;
    curve->substart = 0;
    curve->hascpt = false;
    curve->posSet = false;
    curve->moving = false;
    curve->closed = false;

    return curve;
}

/**
 * Convert NArtBpath object to SPCurve object.
 *
 * \return new SPCurve, or NULL if the curve was not created for some reason.
 */
SPCurve *
sp_curve_new_from_bpath(NArtBpath *bpath)
{
    g_return_val_if_fail(bpath != NULL, NULL);

    SPCurve *curve = sp_curve_new_from_foreign_bpath(bpath);
    g_free(bpath);
    return curve;
}

/**
 * Convert const NArtBpath array to SPCurve.
 *
 * \return new SPCurve, or NULL if the curve was not created for some reason.
 */
SPCurve *sp_curve_new_from_foreign_bpath(NArtBpath const bpath[])
{
    g_return_val_if_fail(bpath != NULL, NULL);

    NArtBpath *new_bpath;
    unsigned const len = sp_bpath_length(bpath);
    new_bpath = g_new(NArtBpath, len);
    memcpy(new_bpath, bpath, len * sizeof(NArtBpath));

    SPCurve *curve = g_new(SPCurve, 1);

    curve->refcount = 1;
    curve->_bpath = new_bpath;
    curve->length = len;
    curve->end = curve->length - 1;
    gint i = curve->end;
    for (; i > 0; i--)
        if ((curve->_bpath[i].code == NR_MOVETO) ||
            (curve->_bpath[i].code == NR_MOVETO_OPEN))
            break;
    curve->substart = i;
    curve->hascpt = false;
    curve->posSet = false;
    curve->moving = false;
    curve->closed = sp_bpath_closed(new_bpath);

    return curve;
}

SPCurve *sp_curve_new_from_rect(NR::Maybe<NR::Rect> const &rect)
{
    g_return_val_if_fail(rect, NULL);

    SPCurve *c = sp_curve_new();

    NR::Point p = rect->corner(0);
    sp_curve_moveto(c, p);

    for (int i=3; i>=0; i--) {
        sp_curve_lineto(c, rect->corner(i));
    }
    sp_curve_closepath_current(c);

    return c;
}

/**
 * Increase refcount of curve.
 *
 * \todo should this be shared with other refcounting code?
 */
SPCurve *
sp_curve_ref(SPCurve *curve)
{
    g_return_val_if_fail(curve != NULL, NULL);

    curve->refcount += 1;

    return curve;
}

/**
 * Decrease refcount of curve, with possible destruction.
 *
 * \todo should this be shared with other refcounting code?
 */
SPCurve *
sp_curve_unref(SPCurve *curve)
{
    g_return_val_if_fail(curve != NULL, NULL);

    curve->refcount -= 1;

    if (curve->refcount < 1) {
        if (curve->_bpath) {
            g_free(curve->_bpath);
        }
        g_free(curve);
    }

    return NULL;
}

/**
 * Add space for more paths in curve.
 */
static void
sp_curve_ensure_space(SPCurve *curve, gint space)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(space > 0);

    if (curve->end + space < curve->length)
        return;

    if (space < SP_CURVE_LENSTEP)
        space = SP_CURVE_LENSTEP;

    curve->_bpath = g_renew(NArtBpath, curve->_bpath, curve->length + space);

    curve->length += space;
}

/**
 * Create new curve from its own bpath array.
 */
SPCurve *
sp_curve_copy(SPCurve *curve)
{
    g_return_val_if_fail(curve != NULL, NULL);

    return sp_curve_new_from_foreign_bpath(curve->_bpath);
}

/**
 * Return new curve that is the concatenation of all curves in list.
 */
SPCurve *
sp_curve_concat(GSList const *list)
{
    g_return_val_if_fail(list != NULL, NULL);

    gint length = 0;

    for (GSList const *l = list; l != NULL; l = l->next) {
        SPCurve *c = (SPCurve *) l->data;
        length += c->end;
    }

    SPCurve *new_curve = sp_curve_new_sized(length + 1);

    NArtBpath *bp = new_curve->_bpath;

    for (GSList const *l = list; l != NULL; l = l->next) {
        SPCurve *c = (SPCurve *) l->data;
        memcpy(bp, c->_bpath, c->end * sizeof(NArtBpath));
        bp += c->end;
    }

    bp->code = NR_END;

    new_curve->end = length;
    gint i;
    for (i = new_curve->end; i > 0; i--) {
        if ((new_curve->_bpath[i].code == NR_MOVETO)     ||
            (new_curve->_bpath[i].code == NR_MOVETO_OPEN)  )
            break;
    }

    new_curve->substart = i;

    return new_curve;
}

/**
 * Returns a list of new curves corresponding to the subpaths in \a curve.
 */
GSList *
sp_curve_split(SPCurve const *curve)
{
    g_return_val_if_fail(curve != NULL, NULL);

    gint p = 0;
    GSList *l = NULL;

    while (p < curve->end) {
        gint i = 1;
        while ((curve->_bpath[p + i].code == NR_LINETO) ||
               (curve->_bpath[p + i].code == NR_CURVETO))
            i++;
        SPCurve *new_curve = sp_curve_new_sized(i + 1);
        memcpy(new_curve->_bpath, curve->_bpath + p, i * sizeof(NArtBpath));
        new_curve->end = i;
        new_curve->_bpath[i].code = NR_END;
        new_curve->substart = 0;
        new_curve->closed = (new_curve->_bpath->code == NR_MOVETO);
        new_curve->hascpt = (new_curve->_bpath->code == NR_MOVETO_OPEN);
        l = g_slist_prepend(l, new_curve);
        p += i;
    }

    return l;
}

/**
 * Transform all paths in curve, template helper.
 */
template<class M>
static void
tmpl_curve_transform(SPCurve *const curve, M const &m)
{
    g_return_if_fail(curve != NULL);

    for (gint i = 0; i < curve->end; i++) {
        NArtBpath *p = curve->_bpath + i;
        switch (p->code) {
            case NR_MOVETO:
            case NR_MOVETO_OPEN:
            case NR_LINETO: {
                p->setC(3, p->c(3) * m);
                break;
            }
            case NR_CURVETO:
                for (unsigned i = 1; i <= 3; ++i) {
                    p->setC(i, p->c(i) * m);
                }
                break;
            default:
                g_warning("Illegal pathcode %d", p->code);
                break;
        }
    }
}

/**
 * Transform all paths in curve using matrix.
 */
void
sp_curve_transform(SPCurve *const curve, NR::Matrix const &m)
{
    tmpl_curve_transform<NR::Matrix>(curve, m);
}

/**
 * Transform all paths in curve using NR::translate.
 */
void
sp_curve_transform(SPCurve *const curve, NR::translate const &m)
{
    tmpl_curve_transform<NR::translate>(curve, m);
}


/* Methods */

/**
 * Set curve to empty curve.
 */
void
sp_curve_reset(SPCurve *curve)
{
    g_return_if_fail(curve != NULL);

    curve->_bpath->code = NR_END;
    curve->end = 0;
    curve->substart = 0;
    curve->hascpt = false;
    curve->posSet = false;
    curve->moving = false;
    curve->closed = false;
}

/* Several consecutive movetos are ALLOWED */

/**
 * Calls sp_curve_moveto() with point made of given coordinates.
 */
void
sp_curve_moveto(SPCurve *curve, gdouble x, gdouble y)
{
    sp_curve_moveto(curve, NR::Point(x, y));
}

/**
 * Perform a moveto to a point, thus starting a new subpath.
 */
void
sp_curve_moveto(SPCurve *curve, NR::Point const &p)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(!curve->moving);

    curve->substart = curve->end;
    curve->hascpt = true;
    curve->posSet = true;
    curve->movePos = p;
}

/**
 * Calls sp_curve_lineto() with a point's coordinates.
 */
void
sp_curve_lineto(SPCurve *curve, NR::Point const &p)
{
    sp_curve_lineto(curve, p[NR::X], p[NR::Y]);
}

/**
 * Adds a line to the current subpath.
 */
void
sp_curve_lineto(SPCurve *curve, gdouble x, gdouble y)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(curve->hascpt);

    if (curve->moving) {
        /* fix endpoint */
        g_return_if_fail(!curve->posSet);
        g_return_if_fail(curve->end > 1);
        NArtBpath *bp = curve->_bpath + curve->end - 1;
        g_return_if_fail(bp->code == NR_LINETO);
        bp->x3 = x;
        bp->y3 = y;
        curve->moving = false;
        return;
    }

    if (curve->posSet) {
        /* start a new segment */
        sp_curve_ensure_space(curve, 2);
        NArtBpath *bp = curve->_bpath + curve->end;
        bp->code = NR_MOVETO_OPEN;
        bp->setC(3, curve->movePos);
        bp++;
        bp->code = NR_LINETO;
        bp->x3 = x;
        bp->y3 = y;
        bp++;
        bp->code = NR_END;
        curve->end += 2;
        curve->posSet = false;
        curve->closed = false;
        return;
    }

    /* add line */

    g_return_if_fail(curve->end > 1);
    sp_curve_ensure_space(curve, 1);
    NArtBpath *bp = curve->_bpath + curve->end;
    bp->code = NR_LINETO;
    bp->x3 = x;
    bp->y3 = y;
    bp++;
    bp->code = NR_END;
    curve->end++;
}

/// Unused
void
sp_curve_lineto_moving(SPCurve *curve, gdouble x, gdouble y)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(curve->hascpt);

    if (curve->moving) {
        /* change endpoint */
        g_return_if_fail(!curve->posSet);
        g_return_if_fail(curve->end > 1);
        NArtBpath *bp = curve->_bpath + curve->end - 1;
        g_return_if_fail(bp->code == NR_LINETO);
        bp->x3 = x;
        bp->y3 = y;
        return;
    }

    if (curve->posSet) {
        /* start a new segment */
        sp_curve_ensure_space(curve, 2);
        NArtBpath *bp = curve->_bpath + curve->end;
        bp->code = NR_MOVETO_OPEN;
        bp->setC(3, curve->movePos);
        bp++;
        bp->code = NR_LINETO;
        bp->x3 = x;
        bp->y3 = y;
        bp++;
        bp->code = NR_END;
        curve->end += 2;
        curve->posSet = false;
        curve->moving = true;
        curve->closed = false;
        return;
    }

    /* add line */

    g_return_if_fail(curve->end > 1);
    sp_curve_ensure_space(curve, 1);
    NArtBpath *bp = curve->_bpath + curve->end;
    bp->code = NR_LINETO;
    bp->x3 = x;
    bp->y3 = y;
    bp++;
    bp->code = NR_END;
    curve->end++;
    curve->moving = true;
}

/**
 * Calls sp_curve_curveto() with coordinates of three points.
 */
void
sp_curve_curveto(SPCurve *curve, NR::Point const &p0, NR::Point const &p1, NR::Point const &p2)
{
    using NR::X;
    using NR::Y;
    sp_curve_curveto(curve,
                     p0[X], p0[Y],
                     p1[X], p1[Y],
                     p2[X], p2[Y]);
}

/**
 * Adds a bezier segment to the current subpath.
 */
void
sp_curve_curveto(SPCurve *curve, gdouble x0, gdouble y0, gdouble x1, gdouble y1, gdouble x2, gdouble y2)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(curve->hascpt);
    g_return_if_fail(!curve->moving);

    if (curve->posSet) {
        /* start a new segment */
        sp_curve_ensure_space(curve, 2);
        NArtBpath *bp = curve->_bpath + curve->end;
        bp->code = NR_MOVETO_OPEN;
        bp->setC(3, curve->movePos);
        bp++;
        bp->code = NR_CURVETO;
        bp->x1 = x0;
        bp->y1 = y0;
        bp->x2 = x1;
        bp->y2 = y1;
        bp->x3 = x2;
        bp->y3 = y2;
        bp++;
        bp->code = NR_END;
        curve->end += 2;
        curve->posSet = false;
        curve->closed = false;
        return;
    }

    /* add curve */

    g_return_if_fail(curve->end > 1);
    sp_curve_ensure_space(curve, 1);
    NArtBpath *bp = curve->_bpath + curve->end;
    bp->code = NR_CURVETO;
    bp->x1 = x0;
    bp->y1 = y0;
    bp->x2 = x1;
    bp->y2 = y1;
    bp->x3 = x2;
    bp->y3 = y2;
    bp++;
    bp->code = NR_END;
    curve->end++;
}

/**
 * Close current subpath by possibly adding a line between start and end.
 */
void
sp_curve_closepath(SPCurve *curve)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(curve->hascpt);
    g_return_if_fail(!curve->posSet);
    g_return_if_fail(!curve->moving);
    g_return_if_fail(!curve->closed);
    /* We need at least moveto, curveto, end. */
    g_return_if_fail(curve->end - curve->substart > 1);

    {
        NArtBpath *bs = curve->_bpath + curve->substart;
        NArtBpath *be = curve->_bpath + curve->end - 1;

        if (bs->c(3) != be->c(3)) {
            sp_curve_lineto(curve, bs->c(3));
            bs = curve->_bpath + curve->substart;
        }

        bs->code = NR_MOVETO;
    }
    curve->closed = true;

    for (NArtBpath const *bp = curve->_bpath; bp->code != NR_END; bp++) {
        /** \todo
         * effic: Maintain a count of NR_MOVETO_OPEN's (e.g. instead of
         * the closed boolean).
         */
        if (bp->code == NR_MOVETO_OPEN) {
            curve->closed = false;
            break;
        }
    }

    curve->hascpt = false;
}

/** Like sp_curve_closepath() but sets the end point of the current
    command to the subpath start point instead of adding a new lineto.

    Used for freehand drawing when the user draws back to the start point.
**/
void
sp_curve_closepath_current(SPCurve *curve)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(curve->hascpt);
    g_return_if_fail(!curve->posSet);
    g_return_if_fail(!curve->closed);
    /* We need at least moveto, curveto, end. */
    g_return_if_fail(curve->end - curve->substart > 1);

    {
        NArtBpath *bs = curve->_bpath + curve->substart;
        NArtBpath *be = curve->_bpath + curve->end - 1;

        be->x3 = bs->x3;
        be->y3 = bs->y3;

        bs->code = NR_MOVETO;
    }
    curve->closed = true;

    for (NArtBpath const *bp = curve->_bpath; bp->code != NR_END; bp++) {
        /** \todo
         * effic: Maintain a count of NR_MOVETO_OPEN's (e.g. instead of
         * the closed boolean).
         */
        if (bp->code == NR_MOVETO_OPEN) {
            curve->closed = false;
            break;
        }
    }

    curve->hascpt = false;
    curve->moving = false;
}

/**
 * True if no paths are in curve.
 */
bool
sp_curve_empty(SPCurve *curve)
{
    g_return_val_if_fail(curve != NULL, TRUE);

    return (curve->_bpath->code == NR_END);
}

/**
 * Return last subpath or NULL.
 */
NArtBpath *
sp_curve_last_bpath(SPCurve const *curve)
{
    g_return_val_if_fail(curve != NULL, NULL);

    if (curve->end == 0) {
        return NULL;
    }

    return curve->_bpath + curve->end - 1;
}

/**
 * Return first subpath or NULL.
 */
NArtBpath *
sp_curve_first_bpath(SPCurve const *curve)
{
    g_return_val_if_fail(curve != NULL, NULL);

    if (curve->end == 0) {
        return NULL;
    }

    return curve->_bpath;
}

/**
 * Return first point of first subpath or (0,0).
 */
NR::Point
sp_curve_first_point(SPCurve const *const curve)
{
    NArtBpath *const bpath = sp_curve_first_bpath(curve);
    g_return_val_if_fail(bpath != NULL, NR::Point(0, 0));
    return bpath->c(3);
}

/**
 * Return the second point of first subpath or curve->movePos if curve too short.
 */
NR::Point
sp_curve_second_point(SPCurve const *const curve)
{
    g_return_val_if_fail(curve != NULL, NR::Point(0, 0));

    if (curve->end < 1) {
        return curve->movePos;
    }

    NArtBpath *bpath = NULL;
    if (curve->end < 2) {
        bpath = curve->_bpath;
    } else {
        bpath = curve->_bpath + 1;
    }
    g_return_val_if_fail(bpath != NULL, NR::Point(0, 0));
    return bpath->c(3);
}

/**
 * Return the second-last point of last subpath or curve->movePos if curve too short.
 */
NR::Point
sp_curve_penultimate_point(SPCurve const *const curve)
{
    g_return_val_if_fail(curve != NULL, NR::Point(0, 0));

    if (curve->end < 2) {
        return curve->movePos;
    }

    NArtBpath *const bpath = curve->_bpath + curve->end - 2;
    g_return_val_if_fail(bpath != NULL, NR::Point(0, 0));
    return bpath->c(3);
}

/**
 * Return last point of last subpath or (0,0).
 */
NR::Point
sp_curve_last_point(SPCurve const *const curve)
{
    NArtBpath *const bpath = sp_curve_last_bpath(curve);
    g_return_val_if_fail(bpath != NULL, NR::Point(0, 0));
    return bpath->c(3);
}

inline static bool
is_moveto(NRPathcode const c)
{
    return c == NR_MOVETO || c == NR_MOVETO_OPEN;
}

/**
 * Returns \a curve but drawn in the opposite direction.
 * Should result in the same shape, but
 * with all its markers drawn facing the other direction.
 **/
SPCurve *
sp_curve_reverse(SPCurve const *curve)
{
    /* We need at least moveto, curveto, end. */
    g_return_val_if_fail(curve->end - curve->substart > 1, NULL);

    NArtBpath const *be = curve->_bpath + curve->end - 1;

    g_assert(is_moveto(curve->_bpath[curve->substart].code));
    g_assert(is_moveto(curve->_bpath[0].code));
    g_assert((be+1)->code == NR_END);

    SPCurve  *new_curve = sp_curve_new_sized(curve->length);
    sp_curve_moveto(new_curve, be->c(3));

    for (NArtBpath const *bp = be; ; --bp) {
        switch (bp->code) {
            case NR_MOVETO:
                g_assert(new_curve->_bpath[new_curve->substart].code == NR_MOVETO_OPEN);
                new_curve->_bpath[new_curve->substart].code = NR_MOVETO;
                /* FALL-THROUGH */
            case NR_MOVETO_OPEN:
                if (bp == curve->_bpath) {
                    return new_curve;
                }
                sp_curve_moveto(new_curve, (bp-1)->c(3));
                break;

            case NR_LINETO:
                sp_curve_lineto(new_curve, (bp-1)->c(3));
                break;

            case NR_CURVETO:
                sp_curve_curveto(new_curve, bp->c(2), bp->c(1), (bp-1)->c(3));
                break;

            default:
                g_assert_not_reached();
        }
    }
}

/**
 * Append \a curve2 to \a curve.
 */
void
sp_curve_append(SPCurve *curve,
                SPCurve const *curve2,
                bool use_lineto)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(curve2 != NULL);

    if (curve2->end < 1)
        return;

    NArtBpath const *bs = curve2->_bpath;

    bool closed = curve->closed;

    for (NArtBpath const *bp = bs; bp->code != NR_END; bp++) {
        switch (bp->code) {
            case NR_MOVETO_OPEN:
                if (use_lineto && curve->hascpt) {
                    sp_curve_lineto(curve, bp->x3, bp->y3);
                    use_lineto = FALSE;
                } else {
                    if (closed) sp_curve_closepath(curve);
                    sp_curve_moveto(curve, bp->x3, bp->y3);
                }
                closed = false;
                break;

            case NR_MOVETO:
                if (use_lineto && curve->hascpt) {
                    sp_curve_lineto(curve, bp->x3, bp->y3);
                    use_lineto = FALSE;
                } else {
                    if (closed) sp_curve_closepath(curve);
                    sp_curve_moveto(curve, bp->x3, bp->y3);
                }
                closed = true;
                break;

            case NR_LINETO:
                sp_curve_lineto(curve, bp->x3, bp->y3);
                break;

            case NR_CURVETO:
                sp_curve_curveto(curve, bp->x1, bp->y1, bp->x2, bp->y2, bp->x3, bp->y3);
                break;

            case NR_END:
                g_assert_not_reached();
        }
    }

    if (closed) {
        sp_curve_closepath(curve);
    }
}

/**
 * Append \a c1 to \a c0 with possible fusing of close endpoints.
 */
SPCurve *
sp_curve_append_continuous(SPCurve *c0, SPCurve const *c1, gdouble tolerance)
{
    g_return_val_if_fail(c0 != NULL, NULL);
    g_return_val_if_fail(c1 != NULL, NULL);
    g_return_val_if_fail(!c0->closed, NULL);
    g_return_val_if_fail(!c1->closed, NULL);

    if (c1->end < 1) {
        return c0;
    }

    NArtBpath *be = sp_curve_last_bpath(c0);
    if (be) {
        NArtBpath const *bs = sp_curve_first_bpath(c1);
        if ( bs
             && ( fabs( bs->x3 - be->x3 ) <= tolerance )
             && ( fabs( bs->y3 - be->y3 ) <= tolerance ) )
        {
            /** \todo
             * fixme: Strictly we mess in case of multisegment mixed
             * open/close curves
             */
            bool closed = false;
            for (bs = bs + 1; bs->code != NR_END; bs++) {
                switch (bs->code) {
                    case NR_MOVETO_OPEN:
                        if (closed) sp_curve_closepath(c0);
                        sp_curve_moveto(c0, bs->x3, bs->y3);
                        closed = false;
                        break;
                    case NR_MOVETO:
                        if (closed) sp_curve_closepath(c0);
                        sp_curve_moveto(c0, bs->x3, bs->y3);
                        closed = true;
                        break;
                    case NR_LINETO:
                        sp_curve_lineto(c0, bs->x3, bs->y3);
                        break;
                    case NR_CURVETO:
                        sp_curve_curveto(c0, bs->x1, bs->y1, bs->x2, bs->y2, bs->x3, bs->y3);
                        break;
                    case NR_END:
                        g_assert_not_reached();
                }
            }
        } else {
            sp_curve_append(c0, c1, TRUE);
        }
    } else {
        sp_curve_append(c0, c1, TRUE);
    }

    return c0;
}

/**
 * Remove last segment of curve.
 */
void
sp_curve_backspace(SPCurve *curve)
{
    g_return_if_fail(curve != NULL);

    if (curve->end > 0) {
        curve->end -= 1;
        if (curve->end > 0) {
            NArtBpath *bp = curve->_bpath + curve->end - 1;
            if ((bp->code == NR_MOVETO)     ||
                (bp->code == NR_MOVETO_OPEN)  )
            {
                curve->hascpt = true;
                curve->posSet = true;
                curve->closed = false;
                curve->movePos = bp->c(3);
                curve->end -= 1;
            }
        }
        curve->_bpath[curve->end].code = NR_END;
    }
}

/* Private methods */

/**
 * Returns index of first NR_END bpath in array.
 */
static unsigned sp_bpath_length(NArtBpath const bpath[])
{
    g_return_val_if_fail(bpath != NULL, FALSE);

    unsigned ret = 0;
    while ( bpath[ret].code != NR_END ) {
        ++ret;
    }
    ++ret;

    return ret;
}

/**
 * \brief
 *
 * \todo
 * fixme: this is bogus -- it doesn't check for nr_moveto, which will indicate
 * a closing of the subpath it's nonsense to talk about a path as a whole
 * being closed, although maybe someone would want that for some other reason?
 * Oh, also, if the bpath just ends, then it's *open*.  I hope nobody is using
 * this code for anything.
 */
static bool sp_bpath_closed(NArtBpath const bpath[])
{
    g_return_val_if_fail(bpath != NULL, FALSE);

    for (NArtBpath const *bp = bpath; bp->code != NR_END; bp++) {
        if (bp->code == NR_MOVETO_OPEN) {
            return false;
        }
    }

    return true;
}

/**
 * Returns length of bezier segment.
 */
static double
bezier_len(NR::Point const &c0,
           NR::Point const &c1,
           NR::Point const &c2,
           NR::Point const &c3,
           double const threshold)
{
    /** \todo
     * The SVG spec claims that a closed form exists, but for the moment I'll
     * use a stupid algorithm.
     */
    double const lbound = L2( c3 - c0 );
    double const ubound = L2( c1 - c0 ) + L2( c2 - c1 ) + L2( c3 - c2 );
    double ret;
    if ( ubound - lbound <= threshold ) {
        ret = .5 * ( lbound + ubound );
    } else {
        NR::Point const a1( .5 * ( c0 + c1 ) );
        NR::Point const b2( .5 * ( c2 + c3 ) );
        NR::Point const c12( .5 * ( c1 + c2 ) );
        NR::Point const a2( .5 * ( a1 + c12 ) );
        NR::Point const b1( .5 * ( c12 + b2 ) );
        NR::Point const midpoint( .5 * ( a2 + b1 ) );
        double const rec_threshold = .625 * threshold;
        ret = bezier_len(c0, a1, a2, midpoint, rec_threshold) + bezier_len(midpoint, b1, b2, c3, rec_threshold);
        if (!(lbound - 1e-2 <= ret && ret <= ubound + 1e-2)) {
            using NR::X; using NR::Y;
            g_warning("ret=%f outside of expected bounds [%f, %f] for {(%.0f %.0f) (%.0f %.0f) (%.0f %.0f) (%.0f %.0f)}",
                      ret, lbound, ubound, c0[X], c0[Y], c1[X], c1[Y], c2[X], c2[Y], c3[X], c3[Y]);
        }
    }
    return ret;
}

/**
 * Returns total length of curve, excluding length of closepath segments.
 */
static double
sp_curve_distance_including_space(SPCurve const *const curve, double seg2len[])
{
    g_return_val_if_fail(curve != NULL, 0.);

    double ret = 0.0;

    if ( curve->_bpath->code == NR_END ) {
        return ret;
    }

    NR::Point prev(curve->_bpath->c(3));
    for (gint i = 1; i < curve->end; ++i) {
        NArtBpath &p = curve->_bpath[i];
        double seg_len = 0;
        switch (p.code) {
            case NR_MOVETO_OPEN:
            case NR_MOVETO:
            case NR_LINETO:
                seg_len = L2(p.c(3) - prev);
                break;

            case NR_CURVETO:
                seg_len = bezier_len(prev, p.c(1), p.c(2), p.c(3), 1.);
                break;

            case NR_END:
                return ret;
        }
        seg2len[i - 1] = seg_len;
        ret += seg_len;
        prev = p.c(3);
    }
    g_assert(!(ret < 0));
    return ret;
}

/**
 * Like sp_curve_distance_including_space(), but ensures that the
 * result >= 1e-18:  uses 1 per segment if necessary.
 */
static double
sp_curve_nonzero_distance_including_space(SPCurve const *const curve, double seg2len[])
{
    double const real_dist(sp_curve_distance_including_space(curve, seg2len));
    if (real_dist >= 1e-18) {
        return real_dist;
    } else {
        unsigned const nSegs = SP_CURVE_LENGTH(curve) - 1;
        for (unsigned i = 0; i < nSegs; ++i) {
            seg2len[i] = 1.;
        }
        return (double) nSegs;
    }
}

void
sp_curve_stretch_endpoints(SPCurve *curve, NR::Point const &new_p0, NR::Point const &new_p1)
{
    if (sp_curve_empty(curve)) {
        return;
    }
    g_assert(unsigned(SP_CURVE_LENGTH(curve)) + 1 == sp_bpath_length(curve->_bpath));
    unsigned const nSegs = SP_CURVE_LENGTH(curve) - 1;
    g_assert(nSegs != 0);
    double *const seg2len = new double[nSegs];
    double const tot_len = sp_curve_nonzero_distance_including_space(curve, seg2len);
    NR::Point const offset0( new_p0 - sp_curve_first_point(curve) );
    NR::Point const offset1( new_p1 - sp_curve_last_point(curve) );
    curve->_bpath->setC(3, new_p0);
    double begin_dist = 0.;
    for (unsigned si = 0; si < nSegs; ++si) {
        double const end_dist = begin_dist + seg2len[si];
        NArtBpath &p = curve->_bpath[1 + si];
        switch (p.code) {
            case NR_LINETO:
            case NR_MOVETO:
            case NR_MOVETO_OPEN:
                p.setC(3, p.c(3) + NR::Lerp(end_dist / tot_len, offset0, offset1));
                break;

            case NR_CURVETO:
                for (unsigned ci = 1; ci <= 3; ++ci) {
                    p.setC(ci, p.c(ci) + Lerp((begin_dist + ci * seg2len[si] / 3.) / tot_len, offset0, offset1));
                }
                break;

            default:
                g_assert_not_reached();
        }

        begin_dist = end_dist;
    }
    g_assert(L1(curve->_bpath[nSegs].c(3) - new_p1) < 1.);
    /* Explicit set for better numerical properties. */
    curve->_bpath[nSegs].setC(3, new_p1);
    delete [] seg2len;
}

void
sp_curve_move_endpoints(SPCurve *curve, NR::Point const &new_p0,
        NR::Point const &new_p1)
{
    if (sp_curve_empty(curve)) {
        return;
    }
    unsigned const nSegs = SP_CURVE_LENGTH(curve) - 1;
    g_assert(nSegs != 0);

    curve->_bpath->setC(3, new_p0);
    curve->_bpath[nSegs].setC(3, new_p1);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
