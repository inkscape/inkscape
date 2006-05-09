#ifndef SEEN_DISPLAY_CURVE_H
#define SEEN_DISPLAY_CURVE_H

/** \file
 * Wrapper around an array of NArtBpath objects.
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

#include <glib/gtypes.h>
#include <glib/gslist.h>

#include "libnr/nr-forward.h"
#include "libnr/nr-point.h"

/// Wrapper around NArtBpath.
struct SPCurve {
    gint refcount;
    NArtBpath *_bpath;
    
    /// Index in bpath[] of NR_END element.
    gint end;

    /// Allocated size (i.e., capacity) of bpath[] array.  Not to be confused 
    /// with the SP_CURVE_LENGTH macro, which returns the logical length of 
    /// the path (i.e., index of NR_END).
    gint length;

    /// Index in bpath[] of the start (i.e., moveto element) of the last 
    /// subpath in this path.
    gint substart;

    /// Previous moveto position.
    /// \note This is used for coalescing moveto's, whereas if we're to 
    /// conform to the SVG spec then we mustn't coalesce movetos if we have 
    /// midpoint markers.  Ref:
    /// http://www.w3.org/TR/SVG11/implnote.html#PathElementImplementationNotes
    /// (first subitem of the item about zero-length path segments)
    NR::Point movePos;

    /// True iff bpath points to read-only, static storage (see callers of
    /// sp_curve_new_from_static_bpath), in which case we shouldn't free 
    /// bpath and shouldn't write through it.
    bool sbpath : 1;
    
    /// True iff current point is defined.  Initially false for a new curve; 
    /// becomes true after moveto; becomes false on closepath.  Curveto, 
    /// lineto etc. require hascpt; hascpt remains true after lineto/curveto.
    bool hascpt : 1;
    
    /// True iff previous was moveto.
    bool posSet : 1;

    /// True iff bpath end is moving.
    bool moving : 1;
    
    /// True iff all subpaths are closed.
    bool closed : 1;
};

#define SP_CURVE_LENGTH(c) (((SPCurve const *)(c))->end)
#define SP_CURVE_BPATH(c) (((SPCurve const *)(c))->_bpath)
#define SP_CURVE_SEGMENT(c,i) (((SPCurve const *)(c))->_bpath + (i))

/* Constructors */

SPCurve *sp_curve_new();
SPCurve *sp_curve_new_sized(gint length);
SPCurve *sp_curve_new_from_bpath(NArtBpath *bpath);
SPCurve *sp_curve_new_from_static_bpath(NArtBpath const *bpath);
SPCurve *sp_curve_new_from_foreign_bpath(NArtBpath const bpath[]);

SPCurve *sp_curve_ref(SPCurve *curve);
SPCurve *sp_curve_unref(SPCurve *curve);

SPCurve *sp_curve_copy(SPCurve *curve);
SPCurve *sp_curve_concat(GSList const *list);
GSList *sp_curve_split(SPCurve const *curve);
void sp_curve_transform(SPCurve *curve, NR::Matrix const &);
void sp_curve_transform(SPCurve *curve, NR::translate const &);
void sp_curve_stretch_endpoints(SPCurve *curve, NR::Point const &, NR::Point const &);
void sp_curve_move_endpoints(SPCurve *curve, NR::Point const &,
        NR::Point const &);

/* Methods */

void sp_curve_reset(SPCurve *curve);

void sp_curve_moveto(SPCurve *curve, NR::Point const &p);
void sp_curve_moveto(SPCurve *curve, gdouble x, gdouble y);
void sp_curve_lineto(SPCurve *curve, NR::Point const &p);
void sp_curve_lineto(SPCurve *curve, gdouble x, gdouble y);
void sp_curve_lineto_moving(SPCurve *curve, gdouble x, gdouble y);
void sp_curve_curveto(SPCurve *curve, NR::Point const &p0, NR::Point const &p1, NR::Point const &p2);
void sp_curve_curveto(SPCurve *curve, gdouble x0, gdouble y0, gdouble x1, gdouble y1, gdouble x2, gdouble y2);
void sp_curve_closepath(SPCurve *curve);
void sp_curve_closepath_current(SPCurve *curve);

SPCurve *sp_curve_append_continuous(SPCurve *c0, SPCurve const *c1, gdouble tolerance);

#define sp_curve_is_empty sp_curve_empty
bool sp_curve_empty(SPCurve *curve);
NArtBpath *sp_curve_last_bpath(SPCurve const *curve);
NArtBpath *sp_curve_first_bpath(SPCurve const *curve);
NR::Point sp_curve_first_point(SPCurve const *curve);
NR::Point sp_curve_last_point(SPCurve const *curve);
NR::Point sp_curve_second_point(SPCurve const *curve);
NR::Point sp_curve_penultimate_point(SPCurve const *curve);

void sp_curve_append(SPCurve *curve, SPCurve const *curve2, bool use_lineto);
SPCurve *sp_curve_reverse(SPCurve const *curve);
void sp_curve_backspace(SPCurve *curve);


#endif /* !SEEN_DISPLAY_CURVE_H */

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
