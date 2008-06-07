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
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL
 */

#include "display/curve.h"

#include <string.h>
#include <glib/gmem.h>
#include "libnr/nr-point.h"
#include "libnr/nr-rect.h"
#include <libnr/n-art-bpath.h>
#include <libnr/nr-point-matrix-ops.h>
#include <libnr/nr-translate-ops.h>
#include <libnr/n-art-bpath-2geom.h>
#include <libnr/nr-convert2geom.h>
#include <cstring>
#include <string>
#include <2geom/pathvector.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/sbasis-to-bezier.h>
#include "svg/svg.h"

static unsigned sp_bpath_length(NArtBpath const bpath[]);
static bool sp_bpath_closed(NArtBpath const bpath[]);

#define NO_CHECKS   // define this to disable the checking for unequal paths in SPCurve, improves performance by a lot!


#ifndef NO_CHECKS
static void debug_out( char const * text, Geom::PathVector const & pathv) {
    char * str = sp_svg_write_path(pathv);
    g_message("%s : %s", text, str);
    g_free(str);
}
#endif

#ifndef NO_CHECKS
static void debug_out( char const * text, NArtBpath const * bpath) {
    char * str = sp_svg_write_path(bpath);
    g_message("%s : %s", text, str);
    g_free(str);
}
#endif

#ifndef NO_CHECKS
void SPCurve::debug_check( char const * text, SPCurve const * curve) {
    char * pathv_str = sp_svg_write_path(curve->_pathv);
    char * bpath_str = sp_svg_write_path(curve->_bpath);
    if ( strcmp(pathv_str, bpath_str) ) {
        g_message("%s : unequal paths", text);
        g_message("bpath : %s", bpath_str);
        g_message("pathv : %s", pathv_str);
    }
    g_free(pathv_str);
    g_free(bpath_str);
#else
void SPCurve::debug_check( char const *, SPCurve const *) {
#endif
}

#ifndef NO_CHECKS
void SPCurve::debug_check( char const * text, bool a) {
    if ( !a ) {
        g_message("%s : bool fail", text);
    }
#else
void SPCurve::debug_check( char const *, bool) {
#endif
}

/* Constructors */

/**
 * The returned curve's state is as if SPCurve::reset has just been called on it.
 * \param length Initial number of NArtBpath elements allocated for bpath (including NR_END
 *    element).
 * 2GEOMproof
 */
SPCurve::SPCurve(guint length)
  : _refcount(1),
    _bpath(NULL),
    _pathv(),
    _end(0),
    _length(length),
    _substart(0),
    _hascpt(false),
    _posSet(false),
    _moving(false),
    _closed(false)
{
    if (length <= 0) {
        g_error("SPCurve::SPCurve called with invalid length parameter");
        throw;
    }

    _bpath = g_new(NArtBpath, length);
    _bpath->code = NR_END;

    _pathv.clear();

    debug_check("SPCurve::SPCurve(guint length)", this);
}

SPCurve::SPCurve(Geom::PathVector const& pathv)
  : _refcount(1),
    _bpath(NULL),
    _pathv(pathv),
    _end(0),
    _length(0),
    _substart(0),
    _hascpt(false),
    _posSet(false),
    _moving(false),
    _closed(false)
{
    // temporary code to convert to _bpath as well:
    _bpath = BPath_from_2GeomPath(_pathv);
    unsigned const len = sp_bpath_length(_bpath);
    _length = len;
    _end = _length - 1;
    gint i = _end;
    for (; i > 0; i--)
        if ((_bpath[i].code == NR_MOVETO) ||
            (_bpath[i].code == NR_MOVETO_OPEN))
            break;
    _substart = i;
    _closed = sp_bpath_closed(_bpath);

    debug_check("SPCurve::SPCurve(Geom::PathVector const& pathv)", this);
}

// * 2GEOMproof
SPCurve *
SPCurve::new_from_foreign_bpath(NArtBpath const *bpath)
{
    g_return_val_if_fail(bpath != NULL, NULL);

    NArtBpath *new_bpath;
    unsigned const len = sp_bpath_length(bpath);
    new_bpath = g_new(NArtBpath, len);
    memcpy(new_bpath, bpath, len * sizeof(NArtBpath));

    SPCurve *curve = new SPCurve();

    curve->_bpath = new_bpath;
    curve->_length = len;
    curve->_end = curve->_length - 1;
    gint i = curve->_end;
    for (; i > 0; i--)
        if ((curve->_bpath[i].code == NR_MOVETO) ||
            (curve->_bpath[i].code == NR_MOVETO_OPEN))
            break;
    curve->_substart = i;
    curve->_closed = sp_bpath_closed(new_bpath);

    curve->_pathv = BPath_to_2GeomPath(curve->_bpath);

    debug_check("SPCurve::new_from_foreign_bpath", curve);

    return curve;
}

/**
 * Convert NArtBpath object to SPCurve object.
 *
 * \return new SPCurve, or NULL if the curve was not created for some reason.
 * 2GEOMproof
 */
SPCurve *
SPCurve::new_from_bpath(NArtBpath *bpath)
{
    g_return_val_if_fail(bpath != NULL, NULL);

    SPCurve *curve = SPCurve::new_from_foreign_bpath(bpath);
    g_free(bpath);

    debug_check("SPCurve::new_from_bpath", curve);

    return curve;
}

// * 2GEOMproof
SPCurve *
SPCurve::new_from_rect(NR::Maybe<NR::Rect> const &rect)
{
    g_return_val_if_fail(rect, NULL);

    SPCurve *c =  new SPCurve();

    NR::Point p = rect->corner(0);
    c->moveto(p);

    for (int i=3; i>=0; i--) {
        c->lineto(rect->corner(i));
    }
    c->closepath_current();

    debug_check("SPCurve::new_from_rect", c);

    return c;
}

// * 2GEOMproof
SPCurve::~SPCurve()
{
    if (_bpath) {
        g_free(_bpath);
        _bpath = NULL;
    }
}

/* Methods */

void
SPCurve::set_pathv(Geom::PathVector const & new_pathv)
{
    _pathv = new_pathv;

    _hascpt = false;
    _posSet = false;
    _moving = false;

    // temporary code to convert to _bpath as well:
    if (_bpath) {
        g_free(_bpath);
        _bpath = NULL;
    }
    _bpath = BPath_from_2GeomPath(_pathv);
    unsigned const len = sp_bpath_length(_bpath);
    _length = len;
    _end = _length - 1;
    gint i = _end;
    for (; i > 0; i--)
        if ((_bpath[i].code == NR_MOVETO) ||
            (_bpath[i].code == NR_MOVETO_OPEN))
            break;
    _substart = i;
    _closed = sp_bpath_closed(_bpath);

    debug_check("SPCurve::set_pathv", this);
}

/**
 * Get pointer to bpath data. Don't keep this reference too long, because the path might change by another function.
 */
NArtBpath const *
SPCurve::get_bpath() const
{
    debug_check("SPCurve::get_bpath", this);
    return _bpath;
};

Geom::PathVector const &
SPCurve::get_pathvector() const
{
    debug_check("SPCurve::get_pathvector", this);
    return _pathv;
}

/**
 *Returns index in bpath[] of NR_END element.
 * remove for 2geom
 */
guint
SPCurve::get_length() const
{
//    g_message("SPCurve::get_length must be removed");

    return _end;
}

/**
 * Increase _refcount of curve.
 *
 * \todo should this be shared with other refcounting code?
 * 2GEOMproof
 */
SPCurve *
SPCurve::ref()
{
    g_return_val_if_fail(this != NULL, NULL);

    _refcount += 1;

    return this;
}

/**
 * Decrease refcount of curve, with possible destruction.
 *
 * \todo should this be shared with other refcounting code?
 * 2GEOMproof
 */
SPCurve *
SPCurve::unref()
{
    g_return_val_if_fail(this != NULL, NULL);

    _refcount -= 1;

    if (_refcount < 1) {
        delete this;
    }

    return NULL;
}

/**
 * Add space for more paths in curve.
 * This function has no meaning for 2geom representation, other than maybe for optimization issues (enlargening the vector for what is to come)
 * 2GEOMproof
 */
void
SPCurve::ensure_space(guint space)
{
    g_return_if_fail(this != NULL);
    g_return_if_fail(space > 0);

    if (_end + space < _length)
        return;

    if (space < SP_CURVE_LENSTEP)
        space = SP_CURVE_LENSTEP;

    _bpath = g_renew(NArtBpath, _bpath, _length + space);

    _length += space;
}

/**
 * Create new curve from its own bpath array.
 * 2GEOMproof
 */
SPCurve *
SPCurve::copy() const
{
    g_return_val_if_fail(this != NULL, NULL);

    return SPCurve::new_from_foreign_bpath(_bpath);
}

/**
 * Return new curve that is the concatenation of all curves in list.
 * 2GEOMified
 */
SPCurve *
SPCurve::concat(GSList const *list)
{
    g_return_val_if_fail(list != NULL, NULL);

    gint length = 0;

    for (GSList const *l = list; l != NULL; l = l->next) {
        SPCurve *c = (SPCurve *) l->data;
        length += c->_end;
    }

    SPCurve *new_curve = new SPCurve(length + 1);

    NArtBpath *bp = new_curve->_bpath;

    for (GSList const *l = list; l != NULL; l = l->next) {
        SPCurve *c = (SPCurve *) l->data;
        memcpy(bp, c->_bpath, c->_end * sizeof(NArtBpath));
        bp += c->_end;
    }

    bp->code = NR_END;

    new_curve->_end = length;
    gint i;
    for (i = new_curve->_end; i > 0; i--) {
        if ((new_curve->_bpath[i].code == NR_MOVETO)     ||
            (new_curve->_bpath[i].code == NR_MOVETO_OPEN)  )
            break;
    }

    new_curve->_substart = i;

    for (GSList const *l = list; l != NULL; l = l->next) {
        SPCurve *c = (SPCurve *) l->data;
        new_curve->_pathv.insert( new_curve->_pathv.end(), c->get_pathvector().begin(), c->get_pathvector().end() );
    }

    debug_check("SPCurve::concat", new_curve);

    return new_curve;
}

/**
 * Returns a list of new curves corresponding to the subpaths in \a curve.
 * 2geomified
 */
GSList *
SPCurve::split() const
{
    g_return_val_if_fail(this != NULL, NULL);

    guint p = 0;
    GSList *l = NULL;

    gint pathnr = 0;
    while (p < _end) {
        gint i = 1;
        while ((_bpath[p + i].code == NR_LINETO) ||
               (_bpath[p + i].code == NR_CURVETO))
            i++;
        SPCurve *new_curve = new SPCurve(i + 1);
        memcpy(new_curve->_bpath, _bpath + p, i * sizeof(NArtBpath));
        new_curve->_end = i;
        new_curve->_bpath[i].code = NR_END;
        new_curve->_substart = 0;
        new_curve->_closed = (new_curve->_bpath->code == NR_MOVETO);
        new_curve->_hascpt = (new_curve->_bpath->code == NR_MOVETO_OPEN);
        new_curve->_pathv = Geom::PathVector(1, _pathv[pathnr]);
        l = g_slist_prepend(l, new_curve);
        p += i;
        pathnr++;
    }

    return l;
}

/**
 * Transform all paths in curve, template helper.
 */
template<class M>
static void
tmpl_curve_transform(SPCurve * curve, M const &m)
{
    g_return_if_fail(curve != NULL);

    for (guint i = 0; i < curve->_end; i++) {
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
 * 2GEOMified, can be deleted when completely 2geom
 */
void
SPCurve::transform(NR::Matrix const &m)
{
    tmpl_curve_transform<NR::Matrix>(this, m);

    _pathv = _pathv * to_2geom(m);

    debug_check("SPCurve::transform(NR::Matrix const &m)", this);
}

/**
 * Transform all paths in curve using matrix.
 */
void
SPCurve::transform(Geom::Matrix const &m)
{
    tmpl_curve_transform<NR::Matrix>(this, from_2geom(m));

    _pathv = _pathv * m;

    debug_check("SPCurve::transform(Geom::Matrix const &m)", this);
}

/**
 * Transform all paths in curve using NR::translate.
 * 2GEOMified, can be deleted when completely 2geom
 */
void
SPCurve::transform(NR::translate const &m)
{
    tmpl_curve_transform<NR::translate>(this, m);

    _pathv = _pathv * to_2geom(m);

    debug_check("SPCurve::transform(NR::translate const &m)", this);
}

/**
 * Set curve to empty curve.
 * 2GEOMified
 */
void
SPCurve::reset()
{
    g_return_if_fail(this != NULL);

    _bpath->code = NR_END;
    _end = 0;
    _substart = 0;
    _hascpt = false;
    _posSet = false;
    _moving = false;
    _closed = false;

    _pathv.clear();

    debug_check("SPCurve::reset", this);
}

/* Several consecutive movetos are ALLOWED */

/**
 * Calls SPCurve::moveto() with point made of given coordinates.
 */
void
SPCurve::moveto(gdouble x, gdouble y)
{
    moveto(NR::Point(x, y));
}
/**
 * Calls SPCurve::moveto() with point made of given coordinates.
 */
void
SPCurve::moveto(Geom::Point const &p)
{
    moveto(from_2geom(p));
}
/**
 * Perform a moveto to a point, thus starting a new subpath.
 * 2GEOMified
 */
void
SPCurve::moveto(NR::Point const &p)
{
    g_return_if_fail(this != NULL);
    g_return_if_fail(!_moving);

    _substart = _end;
    _hascpt = true;
    _posSet = true;
    _movePos = p;
    _pathv.push_back( Geom::Path() );  // for some reason Geom::Path(p) does not work...
    _pathv.back().start(to_2geom(p));

    // the output is not the same. This is because SPCurve *incorrectly* coaslesces multiple moveto's into one for NArtBpath.
//    debug_check("SPCurve::moveto", this);
}

/**
 * Calls SPCurve::lineto() with a point's coordinates.
 */
void
SPCurve::lineto(Geom::Point const &p)
{
    lineto(p[Geom::X], p[Geom::Y]);
}
/**
 * Calls SPCurve::lineto() with a point's coordinates.
 */
void
SPCurve::lineto(NR::Point const &p)
{
    lineto(p[NR::X], p[NR::Y]);
}
/**
 * Adds a line to the current subpath.
 * 2GEOMified
 */
void
SPCurve::lineto(gdouble x, gdouble y)
{
    g_return_if_fail(this != NULL);
    g_return_if_fail(_hascpt);

    if (_moving) {
        /* fix endpoint */
        g_return_if_fail(!_posSet);
        g_return_if_fail(_end > 1);
        NArtBpath *bp = _bpath + _end - 1;
        g_return_if_fail(bp->code == NR_LINETO);
        bp->x3 = x;
        bp->y3 = y;
        _moving = false;

        Geom::Path::iterator it = _pathv.back().end();
        if ( Geom::LineSegment const *last_line_segment = dynamic_cast<Geom::LineSegment const *>( &(*it) )) {
            Geom::LineSegment new_seg( *last_line_segment );
            new_seg.setFinal( Geom::Point(x,y) );
            _pathv.back().replace(it, new_seg);
        }
    } else if (_posSet) {
        /* start a new segment */
        ensure_space(2);
        NArtBpath *bp = _bpath + _end;
        bp->code = NR_MOVETO_OPEN;
        bp->setC(3, _movePos);
        bp++;
        bp->code = NR_LINETO;
        bp->x3 = x;
        bp->y3 = y;
        bp++;
        bp->code = NR_END;
        _end += 2;
        _posSet = false;
        _closed = false;

        _pathv.back().appendNew<Geom::LineSegment>( Geom::Point(x,y) );
        return;
    } else {
        /* add line */

        g_return_if_fail(_end > 1);
        ensure_space(1);
        NArtBpath *bp = _bpath + _end;
        bp->code = NR_LINETO;
        bp->x3 = x;
        bp->y3 = y;
        bp++;
        bp->code = NR_END;
        _end++;
        _pathv.back().appendNew<Geom::LineSegment>( Geom::Point(x,y) );
    }

    debug_check("SPCurve::lineto", this);
}

/**
 * Calls SPCurve::curveto() with coordinates of three points.
 */
void
SPCurve::curveto(Geom::Point const &p0, Geom::Point const &p1, Geom::Point const &p2)
{
    using Geom::X;
    using Geom::Y;
    curveto( p0[X], p0[Y],
             p1[X], p1[Y],
             p2[X], p2[Y] );
}
/**
 * Calls SPCurve::curveto() with coordinates of three points.
 */
void
SPCurve::curveto(NR::Point const &p0, NR::Point const &p1, NR::Point const &p2)
{
    using NR::X;
    using NR::Y;
    curveto( p0[X], p0[Y],
             p1[X], p1[Y],
             p2[X], p2[Y] );
}
/**
 * Adds a bezier segment to the current subpath.
 * 2GEOMified
 */
void
SPCurve::curveto(gdouble x0, gdouble y0, gdouble x1, gdouble y1, gdouble x2, gdouble y2)
{
    g_return_if_fail(this != NULL);
    g_return_if_fail(_hascpt);
    g_return_if_fail(!_moving);

    if (_posSet) {
        /* start a new segment */
        ensure_space(2);
        NArtBpath *bp = _bpath + _end;
        bp->code = NR_MOVETO_OPEN;
        bp->setC(3, _movePos);
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
        _end += 2;
        _posSet = false;
        _closed = false;
        _pathv.back().appendNew<Geom::CubicBezier>( Geom::Point(x0,y0), Geom::Point(x1,y1), Geom::Point(x2,y2) );
    } else {
        /* add curve */

        g_return_if_fail(_end > 1);
        ensure_space(1);
        NArtBpath *bp = _bpath + _end;
        bp->code = NR_CURVETO;
        bp->x1 = x0;
        bp->y1 = y0;
        bp->x2 = x1;
        bp->y2 = y1;
        bp->x3 = x2;
        bp->y3 = y2;
        bp++;
        bp->code = NR_END;
        _end++;
        if (_pathv.empty())  g_message("leeg");
        else _pathv.back().appendNew<Geom::CubicBezier>( Geom::Point(x0,y0), Geom::Point(x1,y1), Geom::Point(x2,y2) );
    }

    debug_check("SPCurve::curveto", this);
}

/**
 * Close current subpath by possibly adding a line between start and end.
  * 2GEOMified
 */
void
SPCurve::closepath()
{
    g_return_if_fail(this != NULL);
    g_return_if_fail(_hascpt);
    g_return_if_fail(!_posSet);
    g_return_if_fail(!_moving);
    g_return_if_fail(!_closed);
    /* We need at least moveto, curveto, end. */
    g_return_if_fail(_end - _substart > 1);

    {
        NArtBpath *bs = _bpath + _substart;
        NArtBpath *be = _bpath + _end - 1;

        if (bs->c(3) != be->c(3)) {
            lineto(bs->c(3));
            bs = _bpath + _substart;
        }

        bs->code = NR_MOVETO;
    }
    // Inkscape always manually adds the closing line segment to SPCurve with a lineto.
    // This lineto is removed in the writing function for NArtBpath, 
    // so when path is closed and the last segment is a lineto, the closing line segment must really be removed first!
    // TODO: fix behavior in Inkscape!
    if ( /*Geom::LineSegment const *line_segment = */ dynamic_cast<Geom::LineSegment const  *>(&_pathv.back().back())) {
        _pathv.back().erase_last();
    }
    _pathv.back().close(true);
    _closed = true;

    for (Geom::PathVector::const_iterator it = _pathv.begin(); it != _pathv.end(); it++) {
         if ( ! it->closed() ) {
            _closed = false;
            break;
        }
    }

    for (NArtBpath const *bp = _bpath; bp->code != NR_END; bp++) {
        /** \todo
         * effic: Maintain a count of NR_MOVETO_OPEN's (e.g. instead of
         * the closed boolean).
         */
        if (bp->code == NR_MOVETO_OPEN) {
            _closed = false;
            break;
        }
    }

    _hascpt = false;

    debug_check("SPCurve::closepath", this);
}

/** Like SPCurve::closepath() but sets the end point of the current
    command to the subpath start point instead of adding a new lineto.

    Used for freehand drawing when the user draws back to the start point.
  
  2GEOMified
**/
void
SPCurve::closepath_current()
{
    g_return_if_fail(this != NULL);
    g_return_if_fail(_hascpt);
    g_return_if_fail(!_posSet);
    g_return_if_fail(!_closed);
    /* We need at least moveto, curveto, end. */
    g_return_if_fail(_end - _substart > 1);

    {
        NArtBpath *bs = _bpath + _substart;
        NArtBpath *be = _bpath + _end - 1;

        be->x3 = bs->x3;
        be->y3 = bs->y3;

        bs->code = NR_MOVETO;
    }
    // Inkscape always manually adds the closing line segment to SPCurve with a lineto.
    // This lineto is removed in the writing function for NArtBpath, 
    // so when path is closed and the last segment is a lineto, the closing line segment must really be removed first!
    // TODO: fix behavior in Inkscape!
    if ( /*Geom::LineSegment const *line_segment = */ dynamic_cast<Geom::LineSegment const  *>(&_pathv.back().back())) {
        _pathv.back().erase_last();
    }
    _pathv.back().close(true);
    _closed = true;

    for (Geom::PathVector::const_iterator it = _pathv.begin(); it != _pathv.end(); it++) {
         if ( ! it->closed() ) {
            _closed = false;
            break;
        }
    }

    for (NArtBpath const *bp = _bpath; bp->code != NR_END; bp++) {
        /** \todo
         * effic: Maintain a count of NR_MOVETO_OPEN's (e.g. instead of
         * the closed boolean).
         */
        if (bp->code == NR_MOVETO_OPEN) {
            _closed = false;
            break;
        }
    }

    _hascpt = false;
    _moving = false;

    debug_check("SPCurve::closepath_current", this);
}

/**
 * True if no paths are in curve.
 * 2GEOMproof
 */
bool
SPCurve::is_empty() const
{
    g_return_val_if_fail(this != NULL, TRUE);

    if (!_bpath)
        return true;

    bool empty = _pathv.empty(); /* || _pathv.front().empty(); */
    debug_check("SPCurve::is_empty", (_bpath->code == NR_END)  ==  empty );

    return (_bpath->code == NR_END);
}

/**
 * True iff all subpaths are closed.
 * 2GEOMproof
 */
bool
SPCurve::is_closed() const
{
    bool closed = true;
    for (Geom::PathVector::const_iterator it = _pathv.begin(); it != _pathv.end(); it++) {
         if ( ! it->closed() ) {
            closed = false;
            break;
        }
    }
    debug_check("SPCurve::is_closed", (closed)  ==  (_closed) );

    return _closed;
}

/**
 * Return last subpath or NULL.
 */
NArtBpath const *
SPCurve::last_bpath() const
{
    g_return_val_if_fail(this != NULL, NULL);

    if (_end == 0) {
        return NULL;
    }

    return _bpath + _end - 1;
}

/**
 * Return first subpath or NULL.
 */
NArtBpath const *
SPCurve::first_bpath() const
{
    g_return_val_if_fail(this != NULL, NULL);

    if (_end == 0) {
        return NULL;
    }

    return _bpath;
}

/**
 * Return first point of first subpath or (0,0).  TODO: shouldn't this be (NR_HUGE, NR_HUGE) to be able to tell it apart from normal (0,0) ?
 */
NR::Point
SPCurve::first_point() const
{
    NArtBpath const * bpath = first_bpath();
    g_return_val_if_fail(bpath != NULL, NR::Point(0, 0));
    if (is_empty())
        return NR::Point(0, 0);

    debug_check("SPCurve::first_point", bpath->c(3) == _pathv.front().initialPoint() );

    return bpath->c(3);
    // return from_2geom( _pathv.front().initialPoint() );
}

/**
 * Return the second point of first subpath or _movePos if curve too short.
 */
NR::Point
SPCurve::second_point() const
{
    g_return_val_if_fail(this != NULL, NR::Point(0, 0));

    if (_end < 1) {
        return _movePos;
    }

    NArtBpath *bpath = NULL;
    if (_end < 2) {
        bpath = _bpath;
    } else {
        bpath = _bpath + 1;
    }
    g_return_val_if_fail(bpath != NULL, NR::Point(0, 0));

    debug_check("SPCurve::second_point", bpath->c(3) == _pathv.front()[0].finalPoint() );

    return bpath->c(3);
}

/**
 * Return the second-last point of last subpath or _movePos if curve too short.
 */
NR::Point
SPCurve::penultimate_point() const
{
    g_return_val_if_fail(this != NULL, NR::Point(0, 0));

    if (_end < 2) {
        return _movePos;
    }

    NArtBpath *const bpath = _bpath + _end - 2;
    g_return_val_if_fail(bpath != NULL, NR::Point(0, 0));
    
    Geom::Point p(NR_HUGE, NR_HUGE);
    Geom::Curve const& back = _pathv.back().back();
    if (_pathv.back().closed()) {
        p = back.finalPoint();
    } else {
        p = back.initialPoint();
    }

    debug_check("SPCurve::penultimate_point", bpath->c(3) == p );
    return bpath->c(3);
}

/**
 * Return last point of last subpath or (0,0).  TODO: shouldn't this be (NR_HUGE, NR_HUGE) to be able to tell it apart from normal (0,0) ?
 */
NR::Point
SPCurve::last_point() const
{
    NArtBpath const * bpath = last_bpath();
    g_return_val_if_fail(bpath != NULL, NR::Point(0, 0));
    if (is_empty())
        return NR::Point(0, 0);

    debug_check("SPCurve::last_point", bpath->c(3) == _pathv.back().finalPoint() );
    return bpath->c(3);
    // return from_2geom( _pathv.back().finalPoint() );
}

inline static bool
is_moveto(NRPathcode const c)
{
    return c == NR_MOVETO || c == NR_MOVETO_OPEN;
}

/**
 * Returns a *new* \a curve but drawn in the opposite direction.
 * Should result in the same shape, but
 * with all its markers drawn facing the other direction.
 * Reverses the order of subpaths as well
 * 2GEOMified
 **/
SPCurve *
SPCurve::create_reverse() const
{
    /* We need at least moveto, curveto, end. */
    g_return_val_if_fail(_end - _substart > 1, NULL);

    NArtBpath const *be = _bpath + _end - 1;

    g_assert(is_moveto(_bpath[_substart].code));
    g_assert(is_moveto(_bpath[0].code));
    g_assert((be+1)->code == NR_END);

    SPCurve  *new_curve = new SPCurve(_length);
    new_curve->moveto(be->c(3));

    for (NArtBpath const *bp = be; ; --bp) {
        switch (bp->code) {
            case NR_MOVETO:
                g_assert(new_curve->_bpath[new_curve->_substart].code == NR_MOVETO_OPEN);
                new_curve->_bpath[new_curve->_substart].code = NR_MOVETO;
                /* FALL-THROUGH */
            case NR_MOVETO_OPEN:
                if (bp == _bpath) {
                    return new_curve;
                }
                new_curve->moveto((bp-1)->c(3));
                break;

            case NR_LINETO:
                new_curve->lineto((bp-1)->c(3));
                break;

            case NR_CURVETO:
                new_curve->curveto(bp->c(2), bp->c(1), (bp-1)->c(3));
                break;

            default:
                g_assert_not_reached();
        }
    }

    new_curve->_pathv = Geom::reverse_paths_and_order(_pathv);

    debug_check("SPCurve::create_reverse", new_curve);
}

/**
 * Append \a curve2 to \a this.
 * If \a use_lineto is false, simply add all paths in \a curve2 to \a this;
 * if \a use_lineto is true, combine \a this's last path and \a curve2's first path and add the rest of the paths in \a curve2 to \a this.
 * 2GEOMified
 */
void
SPCurve::append(SPCurve const *curve2,
                bool use_lineto)
{
    g_return_if_fail(this != NULL);
    g_return_if_fail(curve2 != NULL);

    if (curve2->is_empty())
        return;
    if (curve2->_end < 1)
        return;

    NArtBpath const *bs = curve2->_bpath;

    bool closed = this->_closed;

    for (NArtBpath const *bp = bs; bp->code != NR_END; bp++) {
        switch (bp->code) {
            case NR_MOVETO_OPEN:
                if (use_lineto && _hascpt) {
                    lineto(bp->x3, bp->y3);
                    use_lineto = FALSE;
                } else {
                    if (closed) closepath();
                    moveto(bp->x3, bp->y3);
                }
                closed = false;
                break;

            case NR_MOVETO:
                if (use_lineto && _hascpt) {
                    lineto(bp->x3, bp->y3);
                    use_lineto = FALSE;
                } else {
                    if (closed) closepath();
                    moveto(bp->x3, bp->y3);
                }
                closed = true;
                break;

            case NR_LINETO:
                lineto(bp->x3, bp->y3);
                break;

            case NR_CURVETO:
                curveto(bp->x1, bp->y1, bp->x2, bp->y2, bp->x3, bp->y3);
                break;

            case NR_END:
                g_assert_not_reached();
        }
    }

    if (closed) {
        closepath();
    }

    debug_check("SPCurve::append", this);

    /* 2GEOM code when code above is removed:
    if (use_lineto) {
        Geom::PathVector::const_iterator it = curve2->_pathv.begin();
        if ( ! _pathv.empty() ) {
            Geom::Path & lastpath = _pathv.back();
            lastpath.appendNew<Geom::LineSegment>( (*it).initialPoint() );
            lastpath.append( (*it) );
        } else {
            _pathv.push_back( (*it) );
        }

        for (it++; it != curve2->_pathv.end(); it++) {
            _pathv.push_back( (*it) );
        }
    } else {
        for (Geom::PathVector::const_iterator it = curve2->_pathv.begin(); it != curve2->_pathv.end(); it++) {
            _pathv.push_back( (*it) );
        }
    }
    */
}

/**
 * Append \a c1 to \a this with possible fusing of close endpoints.
 * 2GEOMproof. Needs to be recoded when NArtBpath is no longer there. Right now, it applies the same changes to bpath and pathv depending on bpath
 */
SPCurve *
SPCurve::append_continuous(SPCurve const *c1, gdouble tolerance)
{
    g_return_val_if_fail(this != NULL, NULL);
    g_return_val_if_fail(c1 != NULL, NULL);
    g_return_val_if_fail(!_closed, NULL);
    g_return_val_if_fail(!c1->_closed, NULL);

    if (c1->_end < 1) {
        return this;
    }

    debug_check("SPCurve::append_continuous 11", this);

    NArtBpath const *be = last_bpath();
    if (be) {
        NArtBpath const *bs = c1->first_bpath();
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
                        if (closed) closepath();
                        moveto(bs->x3, bs->y3);
                        closed = false;
                        break;
                    case NR_MOVETO:
                        if (closed) closepath();
                        moveto(bs->x3, bs->y3);
                        closed = true;
                        break;
                    case NR_LINETO:
                        lineto(bs->x3, bs->y3);
                        break;
                    case NR_CURVETO:
                        curveto(bs->x1, bs->y1, bs->x2, bs->y2, bs->x3, bs->y3);
                        break;
                    case NR_END:
                        g_assert_not_reached();
                }
            }
        } else {
            append(c1, TRUE);
        }
    } else {
        append(c1, TRUE);
    }

    debug_check("SPCurve::append_continuous", this);

    return this;
}

/**
 * Remove last segment of curve.
 * (Only used once in /src/pen-context.cpp)
 * 2GEOMified
 */
void
SPCurve::backspace()
{
    g_return_if_fail(this != NULL);

    if ( is_empty() )
        return;

    if (_end > 0) {
        _end -= 1;
        if (_end > 0) {
            NArtBpath *bp = _bpath + _end - 1;
            if ((bp->code == NR_MOVETO)     ||
                (bp->code == NR_MOVETO_OPEN)  )
            {
                _hascpt = true;
                _posSet = true;
                _closed = false;
                _movePos = bp->c(3);
                _end -= 1;
            }
        }
        _bpath[_end].code = NR_END;
    }

    if ( !_pathv.back().empty() ) {
        _pathv.back().erase_last();
        _pathv.back().close(false);
    }

    debug_check("SPCurve::backspace", this);
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
double
sp_curve_distance_including_space(SPCurve const *const curve, double seg2len[])
{
    g_return_val_if_fail(curve != NULL, 0.);

    double ret = 0.0;

    if ( curve->_bpath->code == NR_END ) {
        return ret;
    }

    NR::Point prev(curve->_bpath->c(3));
    for (guint i = 1; i < curve->_end; ++i) {
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
double
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

/**
 * 2GEOMified
 */
void
SPCurve::stretch_endpoints(NR::Point const &new_p0, NR::Point const &new_p1)
{
    if (is_empty()) {
        return;
    }
    g_assert(unsigned(SP_CURVE_LENGTH(this)) + 1 == sp_bpath_length(_bpath));
    unsigned const nSegs = SP_CURVE_LENGTH(this) - 1;
    g_assert(nSegs != 0);
    double *const seg2len = new double[nSegs];
    double const tot_len = sp_curve_nonzero_distance_including_space(this, seg2len);
    NR::Point const offset0( new_p0 - first_point() );
    NR::Point const offset1( new_p1 - last_point() );
    _bpath->setC(3, new_p0);
    double begin_dist = 0.;
    for (unsigned si = 0; si < nSegs; ++si) {
        double const end_dist = begin_dist + seg2len[si];
        NArtBpath &p = _bpath[1 + si];
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
    g_assert(L1(_bpath[nSegs].c(3) - new_p1) < 1.);
    /* Explicit set for better numerical properties. */
    _bpath[nSegs].setC(3, new_p1);
    delete [] seg2len;

    Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2 = _pathv.front().toPwSb();
    Geom::Piecewise<Geom::SBasis> arclength = Geom::arcLengthSb(pwd2);
    if ( arclength.lastValue() <= 0 ) {
        g_error("SPCurve::stretch_endpoints - arclength <= 0");
        throw;
    }
    arclength *= 1./arclength.lastValue();
    Geom::Point const A( to_2geom(offset0) );
    Geom::Point const B( to_2geom(offset1) );
    Geom::Piecewise<Geom::SBasis> offsetx = (arclength*-1.+1)*A[0] + arclength*B[0];
    Geom::Piecewise<Geom::SBasis> offsety = (arclength*-1.+1)*A[1] + arclength*B[1];
    Geom::Piecewise<Geom::D2<Geom::SBasis> > offsetpath = Geom::sectionize( Geom::D2<Geom::Piecewise<Geom::SBasis> >(offsetx, offsety) );
    pwd2 += offsetpath;
    _pathv = Geom::path_from_piecewise( pwd2, 0.001 );

    debug_check("SPCurve::stretch_endpoints", this);
}

/**
 *  sets start of first path to new_p0, and end of first path to  new_p1
 * 2GEOMified
 */
void
SPCurve::move_endpoints(NR::Point const &new_p0, NR::Point const &new_p1)
{
    if (is_empty()) {
        return;
    }
    unsigned const nSegs = SP_CURVE_LENGTH(this) - 1;
    g_assert(nSegs != 0);

    _bpath->setC(3, new_p0);
    _bpath[nSegs].setC(3, new_p1);

    _pathv.front().setInitial(to_2geom(new_p0));
    _pathv.front().setFinal(to_2geom(new_p1));

    debug_check("SPCurve::move_endpoints", this);
}

/**
 * returns the number of nodes in a path, used for statusbar text when selecting an spcurve.
 * 2GEOMified
 */
guint
SPCurve::nodes_in_path() const
{
    gint r = _end;
    gint i = _length - 1;
    if (i > r) i = r; // sometimes after switching from node editor length is wrong, e.g. f6 - draw - f2 - tab - f1, this fixes it
    for (; i >= 0; i --)
        if (_bpath[i].code == NR_MOVETO)
            r --;

    guint nr = 0;
    for(Geom::PathVector::const_iterator it = _pathv.begin(); it != _pathv.end(); ++it) {
        nr += (*it).size();

        nr++; // count last node (this works also for closed paths because although they don't have a 'last node', they do have an extra segment
    }

    debug_check("SPCurve::nodes_in_path", r == (gint)nr);

    return r;
}

/**
 *  Adds p to the last point (and last handle if present) of the last path
 * 2GEOMified
 */
void
SPCurve::last_point_additive_move(Geom::Point const & p)
{
    if (is_empty()) {
        return;
    }
    if (_end == 0) {
        return;
    }
    NArtBpath * path = _bpath + _end - 1;

    if (path->code == NR_CURVETO) {
        path->x2 += p[Geom::X];
        path->y2 += p[Geom::Y];
    }
    path->x3 += p[Geom::X];
    path->y3 += p[Geom::Y];

    _pathv.back().setFinal( _pathv.back().finalPoint() + p );

    // Move handle as well when the last segment is a cubic bezier segment:
    // TODO: what to do for quadratic beziers?
    if ( Geom::CubicBezier const *lastcube = dynamic_cast<Geom::CubicBezier const *>(&_pathv.back().back()) ) {
        Geom::CubicBezier newcube( *lastcube );
        newcube.setPoint(2, newcube[2] + p);
        _pathv.back().replace( --_pathv.back().end(), newcube );
    }

    debug_check("SPCurve::last_point_additive_move", this);
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
