#define INKSCAPE_LPE_SPIRO_C

/*
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-spiro.h"

#include "display/curve.h"
#include "nodepath.h"
#include <typeinfo>
#include <2geom/pathvector.h>
#include <2geom/matrix.h>
#include <2geom/bezier-curve.h>
#include <2geom/hvlinesegment.h>
#include <2geom/isnan.h>
#include "helper/geom-nodetype.h"
#include "helper/geom-curves.h"

#include "live_effects/bezctx.h"
#include "live_effects/bezctx_intf.h"
#include "live_effects/spiro.h"

// For handling un-continuous paths:
#include "message-stack.h"
#include "inkscape.h"
#include "desktop.h"

#define SPIRO_SHOW_INFINITE_COORDINATE_CALLS

typedef struct {
    bezctx base;
    SPCurve *curve;
    int is_open;
} bezctx_ink;

void bezctx_ink_moveto(bezctx *bc, double x, double y, int /*is_open*/)
{
    bezctx_ink *bi = (bezctx_ink *) bc;
    if ( IS_FINITE(x) && IS_FINITE(y) ) {
        bi->curve->moveto(x, y);
    }
#ifdef SPIRO_SHOW_INFINITE_COORDINATE_CALLS
    else {
        g_message("lpe moveto not finite");
    }
#endif
}

void bezctx_ink_lineto(bezctx *bc, double x, double y)
{
    bezctx_ink *bi = (bezctx_ink *) bc;
    if ( IS_FINITE(x) && IS_FINITE(y) ) {
        bi->curve->lineto(x, y);
    }
#ifdef SPIRO_SHOW_INFINITE_COORDINATE_CALLS
    else {
        g_message("lpe lineto not finite");
    }
#endif
}

void bezctx_ink_quadto(bezctx *bc, double xm, double ym, double x3, double y3)
{
    bezctx_ink *bi = (bezctx_ink *) bc;

    if ( IS_FINITE(xm) && IS_FINITE(ym) && IS_FINITE(x3) && IS_FINITE(y3) ) {
        bi->curve->quadto(xm, ym, x3, y3);
    }
#ifdef SPIRO_SHOW_INFINITE_COORDINATE_CALLS
    else {
        g_message("lpe quadto not finite");
    }
#endif
}

void bezctx_ink_curveto(bezctx *bc, double x1, double y1, double x2, double y2,
		    double x3, double y3)
{
    bezctx_ink *bi = (bezctx_ink *) bc;
    if ( IS_FINITE(x1) && IS_FINITE(y1) && IS_FINITE(x2) && IS_FINITE(y2) ) {
        bi->curve->curveto(x1, y1, x2, y2, x3, y3);
    }
#ifdef SPIRO_SHOW_INFINITE_COORDINATE_CALLS
    else {
        g_message("lpe curveto not finite");
    }
#endif
}

bezctx *
new_bezctx_ink(SPCurve *curve) {
    bezctx_ink *result = g_new(bezctx_ink, 1);
    result->base.moveto = bezctx_ink_moveto;
    result->base.lineto = bezctx_ink_lineto;
    result->base.quadto = bezctx_ink_quadto;
    result->base.curveto = bezctx_ink_curveto;
    result->base.mark_knot = NULL;
    result->curve = curve;
    return &result->base;
}




namespace Inkscape {
namespace LivePathEffect {

LPESpiro::LPESpiro(LivePathEffectObject *lpeobject) :
    Effect(lpeobject)
{
}

LPESpiro::~LPESpiro()
{
}

void
LPESpiro::setup_nodepath(Inkscape::NodePath::Path *np)
{
    Effect::setup_nodepath(np);
    sp_nodepath_show_handles(np, false);
//    sp_nodepath_show_helperpath(np, false);
}

void
LPESpiro::doEffect(SPCurve * curve)
{
    using Geom::X;
    using Geom::Y;

    // Make copy of old path as it is changed during processing
    Geom::PathVector const original_pathv = curve->get_pathvector();
    guint len = curve->get_segment_count() + 2;

    curve->reset();
    bezctx *bc = new_bezctx_ink(curve);
    spiro_cp *path = g_new (spiro_cp, len);
    int ip = 0;

    for(Geom::PathVector::const_iterator path_it = original_pathv.begin(); path_it != original_pathv.end(); ++path_it) {
        if (path_it->empty())
            continue;

        // start of path
        {
            Geom::Point p = path_it->front().pointAt(0);
            path[ip].x = p[X];
            path[ip].y = p[Y];
            path[ip].ty = '{' ;  // for closed paths, this is overwritten
            ip++;
        }

        // midpoints
        Geom::Path::const_iterator curve_it1 = path_it->begin();      // incoming curve
        Geom::Path::const_iterator curve_it2 = ++(path_it->begin());         // outgoing curve

        Geom::Path::const_iterator curve_endit = path_it->end_default(); // this determines when the loop has to stop
        if (path_it->closed()) {
            // if the path is closed, maybe we have to stop a bit earlier because the closing line segment has zerolength.
            if (path_it->back_closed().isDegenerate()) {
                // the closing line segment has zero-length. So stop before that one!
                curve_endit = path_it->end_open();
            }
        }

        while ( curve_it2 != curve_endit )
        {
            /* This deals with the node between curve_it1 and curve_it2.
             * Loop to end_default (so without last segment), loop ends when curve_it2 hits the end
             * and then curve_it1 points to end or closing segment */
            Geom::Point p = curve_it1->finalPoint();
            path[ip].x = p[X];
            path[ip].y = p[Y];

            // Determine type of spiro node this is, determined by the tangents (angles) of the curves
            // TODO: see if this can be simplified by using /helpers/geom-nodetype.cpp:get_nodetype
            bool this_is_line = is_straight_curve(*curve_it1);
            bool next_is_line = is_straight_curve(*curve_it2);

            Geom::NodeType nodetype = Geom::get_nodetype(*curve_it1, *curve_it2);

            if ( nodetype == Geom::NODE_SMOOTH || nodetype == Geom::NODE_SYMM )
            {
                if (this_is_line && !next_is_line) {
                    path[ip].ty = ']';
                } else if (next_is_line && !this_is_line) {
                    path[ip].ty = '[';
                } else {
                    path[ip].ty = 'c';
                }
            } else {
                path[ip].ty = 'v';
            }

            ++curve_it1;
            ++curve_it2;
            ip++;
        }

        // add last point to the spiropath
        Geom::Point p = curve_it1->finalPoint();
        path[ip].x = p[X];
        path[ip].y = p[Y];
        if (path_it->closed()) {
            // curve_it1 points to the (visually) closing segment. determine the match between first and this last segment (the closing node)
            Geom::NodeType nodetype = Geom::get_nodetype(*curve_it1, path_it->front());
            switch (nodetype) {
                case Geom::NODE_NONE: // can't happen! but if it does, it means the path isn't closed :-)
                    path[ip].ty = '}';
                    ip++;
                    break;
                case Geom::NODE_CUSP:
                    path[0].ty = path[ip].ty = 'v';
                    break;
                case Geom::NODE_SMOOTH:
                case Geom::NODE_SYMM:
                    path[0].ty = path[ip].ty = 'c';
                    break;
            }
        } else {
            // set type to path closer
            path[ip].ty = '}';
            ip++;
        }

        // run subpath through spiro
        int sp_len = ip;
        spiro_seg *s = run_spiro(path, sp_len);
        spiro_to_bpath(s, sp_len, bc);
        free(s);
        ip = 0;
    }

    g_free (path);
}

}; //namespace LivePathEffect
}; /* namespace Inkscape */

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
