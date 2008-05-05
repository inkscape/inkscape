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
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL
 */

#include <glib/gtypes.h>
#include <glib/gslist.h>

#include "libnr/nr-forward.h"
#include "libnr/nr-rect.h"

#define SP_CURVE_LENSTEP 32

/// Wrapper around NArtBpath.
class SPCurve {
public:
    /* Constructors */
    SPCurve(gint length = SP_CURVE_LENSTEP);
    static SPCurve * new_from_bpath(NArtBpath *bpath);
    static SPCurve * new_from_foreign_bpath(NArtBpath const *bpath);
    static SPCurve * new_from_rect(NR::Maybe<NR::Rect> const &rect);

    virtual ~SPCurve();

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

    SPCurve * ref();
    SPCurve * unref();

    SPCurve * copy() const;

    GSList * split() const;
    void transform(NR::Matrix const &);
    void transform(NR::translate const &);
    void stretch_endpoints(NR::Point const &, NR::Point const &);
    void move_endpoints(NR::Point const &, NR::Point const &);

    void reset();

    void moveto(NR::Point const &p);
    void moveto(gdouble x, gdouble y);
    void lineto(NR::Point const &p);
    void lineto(gdouble x, gdouble y);
    void lineto_moving(gdouble x, gdouble y);
    void curveto(NR::Point const &p0, NR::Point const &p1, NR::Point const &p2);
    void curveto(gdouble x0, gdouble y0, gdouble x1, gdouble y1, gdouble x2, gdouble y2);
    void closepath();
    void closepath_current();

    SPCurve * append_continuous(SPCurve const *c1, gdouble tolerance);

    bool is_empty() const;
    NArtBpath * last_bpath() const;
    NArtBpath * first_bpath() const;
    NR::Point first_point() const;
    NR::Point last_point() const;
    NR::Point second_point() const;
    NR::Point penultimate_point() const;

    void append(SPCurve const *curve2, bool use_lineto);
    SPCurve * reverse() const;
    void backspace();

    static SPCurve * concat(GSList const *list);

private:
    // Don't implement these:
    SPCurve(const SPCurve&);
    SPCurve& operator=(const SPCurve&);
};

#define SP_CURVE_LENGTH(c) (((SPCurve const *)(c))->end)
#define SP_CURVE_BPATH(c) (((SPCurve const *)(c))->_bpath)
#define SP_CURVE_SEGMENT(c,i) (((SPCurve const *)(c))->_bpath + (i))

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
