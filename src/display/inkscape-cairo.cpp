/*
 * Helper functions to use cairo with inkscape
 *
 * Copyright (C) 2007 bulia byak
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL
 *
 */

#include <cairo.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libnr/nr-pixblock.h>
#include <libnr/nr-convert2geom.h>
#include "../style.h"
#include "nr-arena.h"
#include "sp-canvas.h"
#include <2geom/pathvector.h>
#include <2geom/bezier-curve.h>
#include <2geom/hvlinesegment.h>
#include <2geom/matrix.h>
#include <2geom/point.h>
#include <2geom/path.h>
#include <2geom/transforms.h>
#include <2geom/sbasis-to-bezier.h>
#include "helper/geom-curves.h"

/** Creates a cairo context to render to the given pixblock on the given area */
cairo_t *
nr_create_cairo_context_for_data (NRRectL *area, NRRectL *buf_area, unsigned char *px, unsigned int rowstride)
{
    if (!nr_rect_l_test_intersect_ptr(buf_area, area))
        return NULL;

    NRRectL clip;
    nr_rect_l_intersect (&clip, buf_area, area);
    unsigned char *dpx = px + (clip.y0 - buf_area->y0) * rowstride + 4 * (clip.x0 - buf_area->x0);
    int width = area->x1 - area->x0;
    int height = area->y1 - area->y0;
    // even though cairo cannot draw in nonpremul mode, select ARGB32 for R8G8B8A8N as the closest; later eliminate R8G8B8A8N everywhere
    cairo_surface_t* cst = cairo_image_surface_create_for_data
        (dpx,
         CAIRO_FORMAT_ARGB32,
         width,
         height,
         rowstride);
    cairo_t *ct = cairo_create (cst);

    return ct;
}

/** Creates a cairo context to render to the given SPCanvasBuf on the given area */
cairo_t *
nr_create_cairo_context_canvasbuf (NRRectL */*area*/, SPCanvasBuf *b)
{
    return nr_create_cairo_context_for_data (&(b->rect), &(b->rect), b->buf, b->buf_rowstride);
}


/** Creates a cairo context to render to the given NRPixBlock on the given area */
cairo_t *
nr_create_cairo_context (NRRectL *area, NRPixBlock *pb)
{
    return nr_create_cairo_context_for_data (area, &(pb->area), NR_PIXBLOCK_PX (pb), pb->rs);
}

/*
 * Can be called recursively.
 * If optimize_stroke == false, the view Rect is not used.
 */
static void
feed_curve_to_cairo(cairo_t *cr, Geom::Curve const &c, Geom::Matrix const & trans, Geom::Rect view, bool optimize_stroke)
{
    if( is_straight_curve(c) )
    {
        Geom::Point end_tr = c.finalPoint() * trans;
        if (!optimize_stroke) {
            cairo_line_to(cr, end_tr[0], end_tr[1]);
        } else {
            Geom::Rect swept(c.initialPoint()*trans, end_tr);
            if (swept.intersects(view)) {
                cairo_line_to(cr, end_tr[0], end_tr[1]);
            } else {
                cairo_move_to(cr, end_tr[0], end_tr[1]);
            }
        }
    }
    else if(Geom::QuadraticBezier const *quadratic_bezier = dynamic_cast<Geom::QuadraticBezier const*>(&c)) {
        std::vector<Geom::Point> points = quadratic_bezier->points();
        points[0] *= trans;
        points[1] *= trans;
        points[2] *= trans;
        Geom::Point b1 = points[0] + (2./3) * (points[1] - points[0]);
        Geom::Point b2 = b1 + (1./3) * (points[2] - points[0]);
        if (!optimize_stroke) {
            cairo_curve_to(cr, b1[0], b1[1], b2[0], b2[1], points[2][0], points[2][1]);
        } else {
            Geom::Rect swept(points[0], points[2]);
            swept.expandTo(points[1]);
            if (swept.intersects(view)) {
                cairo_curve_to(cr, b1[0], b1[1], b2[0], b2[1], points[2][0], points[2][1]);
            } else {
                cairo_move_to(cr, points[2][0], points[2][1]);
            }
        }
    }
    else if(Geom::CubicBezier const *cubic_bezier = dynamic_cast<Geom::CubicBezier const*>(&c)) {
        std::vector<Geom::Point> points = cubic_bezier->points();
        //points[0] *= trans; // don't do this one here for fun: it is only needed for optimized strokes
        points[1] *= trans;
        points[2] *= trans;
        points[3] *= trans;
        if (!optimize_stroke) {
            cairo_curve_to(cr, points[1][0], points[1][1], points[2][0], points[2][1], points[3][0], points[3][1]);
        } else {
            points[0] *= trans;  // didn't transform this point yet
            Geom::Rect swept(points[0], points[3]);
            swept.expandTo(points[1]);
            swept.expandTo(points[2]);
            if (swept.intersects(view)) {
                cairo_curve_to(cr, points[1][0], points[1][1], points[2][0], points[2][1], points[3][0], points[3][1]);
            } else {
                cairo_move_to(cr, points[3][0], points[3][1]);
            }
        }
    }
//    else if(Geom::SVGEllipticalArc const *svg_elliptical_arc = dynamic_cast<Geom::SVGEllipticalArc *>(c)) {
//        //TODO: get at the innards and spit them out to cairo
//    }
    else {
        //this case handles sbasis as well as all other curve types
        Geom::Path sbasis_path = Geom::cubicbezierpath_from_sbasis(c.toSBasis(), 0.1);

        //recurse to convert the new path resulting from the sbasis to svgd
        for(Geom::Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
            feed_curve_to_cairo(cr, *iter, trans, view, optimize_stroke);
        }
    }
}


/** Feeds path-creating calls to the cairo context translating them from the Path */
static void
feed_path_to_cairo (cairo_t *ct, Geom::Path const &path)
{
    if (path.empty())
        return;

    cairo_move_to(ct, path.initialPoint()[0], path.initialPoint()[1] );

    for(Geom::Path::const_iterator cit = path.begin(); cit != path.end_open(); ++cit) {
        feed_curve_to_cairo(ct, *cit, Geom::identity(), Geom::Rect(), false); // optimize_stroke is false, so the view rect is not used
    }

    if (path.closed()) {
        cairo_line_to(ct, path.initialPoint()[0], path.initialPoint()[1]);
//        cairo_close_path(ct);
        /* I think we should use cairo_close_path(ct) here but it doesn't work. (the closing line is not rendered completely)
           According to cairo documentation:
           The behavior of cairo_close_path() is distinct from simply calling cairo_line_to() with the equivalent coordinate
           in the case of stroking. When a closed sub-path is stroked, there are no caps on the ends of the sub-path. Instead,
           there is a line join connecting the final and initial segments of the sub-path. 
        */
    }
}

/** Feeds path-creating calls to the cairo context translating them from the Path, with the given transform and shift */
static void
feed_path_to_cairo (cairo_t *ct, Geom::Path const &path, Geom::Matrix trans, boost::optional<Geom::Rect> area, bool optimize_stroke, double stroke_width)
{
    if (!area || area->isEmpty())
        return;
    if (path.empty())
        return;

    // Transform all coordinates to coords within "area"
    Geom::Point shift = area->min();
    Geom::Rect view = *area;
    view.expandBy (stroke_width);
    view = view * (Geom::Matrix)Geom::Translate(-shift);
    //  Pass transformation to feed_curve, so that we don't need to create a whole new path.
    Geom::Matrix transshift(trans * Geom::Translate(-shift));

    Geom::Point initial = path.initialPoint() * transshift;
    cairo_move_to(ct, initial[0], initial[1] );

    for(Geom::Path::const_iterator cit = path.begin(); cit != path.end_open(); ++cit) {
        feed_curve_to_cairo(ct, *cit, transshift, view, optimize_stroke);
    }

    if (path.closed()) {
        cairo_line_to(ct, initial[0], initial[1]);
        /* We cannot use cairo_close_path(ct) here because some parts of the path may have been
           clipped and not drawn (maybe the before last segment was outside view area), which 
           would result in closing the "subpath" after the last interruption, not the entire path.

           However, according to cairo documentation:
           The behavior of cairo_close_path() is distinct from simply calling cairo_line_to() with the equivalent coordinate
           in the case of stroking. When a closed sub-path is stroked, there are no caps on the ends of the sub-path. Instead,
           there is a line join connecting the final and initial segments of the sub-path. 

           The correct fix will be possible when cairo introduces methods for moving without
           ending/starting subpaths, which we will use for skipping invisible segments; then we
           will be able to use cairo_close_path here. This issue also affects ps/eps/pdf export,
           see bug 168129
        */
    }
}

/** Feeds path-creating calls to the cairo context translating them from the PathVector, with the given transform and shift
 *  One must have done cairo_new_path(ct); before calling this function. */
void
feed_pathvector_to_cairo (cairo_t *ct, Geom::PathVector const &pathv, Geom::Matrix trans, boost::optional<Geom::Rect> area, bool optimize_stroke, double stroke_width)
{
    if (!area || area->isEmpty())
        return;
    if (pathv.empty())
        return;

    for(Geom::PathVector::const_iterator it = pathv.begin(); it != pathv.end(); ++it) {
        feed_path_to_cairo(ct, *it, trans, area, optimize_stroke, stroke_width);
    }
}

/** Feeds path-creating calls to the cairo context translating them from the PathVector
 *  One must have done cairo_new_path(ct); before calling this function. */
void
feed_pathvector_to_cairo (cairo_t *ct, Geom::PathVector const &pathv)
{
    if (pathv.empty())
        return;

    for(Geom::PathVector::const_iterator it = pathv.begin(); it != pathv.end(); ++it) {
        feed_path_to_cairo(ct, *it);
    }
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
