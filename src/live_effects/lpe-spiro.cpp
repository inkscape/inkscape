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

#include "live_effects/bezctx.h"
#include "live_effects/bezctx_intf.h"
#include "live_effects/spiro.h"

// For handling un-continuous paths:
#include "message-stack.h"
#include "inkscape.h"
#include "desktop.h"

typedef struct {
    bezctx base;
    SPCurve *curve;
    int is_open;
} bezctx_ink;

void bezctx_ink_moveto(bezctx *bc, double x, double y, int /*is_open*/)
{
    bezctx_ink *bi = (bezctx_ink *) bc;
    bi->curve->moveto(x, y);
}

void bezctx_ink_lineto(bezctx *bc, double x, double y)
{
    bezctx_ink *bi = (bezctx_ink *) bc;
    bi->curve->lineto(x, y);
}

void bezctx_ink_quadto(bezctx *bc, double xm, double ym, double x3, double y3)
{
    bezctx_ink *bi = (bezctx_ink *) bc;

    double x0, y0;
    double x1, y1;
    double x2, y2;

    NR::Point last = bi->curve->last_point();
    x0 = last[NR::X];
    y0 = last[NR::Y];
    x1 = xm + (1./3) * (x0 - xm);
    y1 = ym + (1./3) * (y0 - ym);
    x2 = xm + (1./3) * (x3 - xm);
    y2 = ym + (1./3) * (y3 - ym);

    bi->curve->curveto(x1, y1, x2, y2, x3, y3);
}

void bezctx_ink_curveto(bezctx *bc, double x1, double y1, double x2, double y2,
		    double x3, double y3)
{
    bezctx_ink *bi = (bezctx_ink *) bc;
    bi->curve->curveto(x1, y1, x2, y2, x3, y3);
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
            path[ip].ty = path_it->closed() ? 'c' : '{' ;  // this is changed later when the path is closed
            ip++;
        }

        // midpoints
        Geom::Path::const_iterator curve_it1 = path_it->begin();      // incoming curve
        Geom::Path::const_iterator curve_it2 = ++(path_it->begin());         // outgoing curve

        int d = 1;
        while ( curve_it2 != path_it->end_default() )
        {
            /* This deals with the node between curve_it1 and curve_it2.
             * Loop to end_default (so without last segment), loop ends when curve_it2 hits the end
             * and then curve_it1 points to end or closing segment */
            Geom::Point p = curve_it1->finalPoint();
            path[ip].x = p[X];
            path[ip].y = p[Y];

            // Determine type of spiro node this is, determined by the tangents (angles) of the curves
            // TODO: see if this can be simplified by using /helpers/geom-nodetype.cpp:get_nodetype
            bool this_is_line = ( dynamic_cast<Geom::LineSegment const *>(&*curve_it1)  ||
                                  dynamic_cast<Geom::HLineSegment const *>(&*curve_it1) ||
                                  dynamic_cast<Geom::VLineSegment const *>(&*curve_it1) );
            bool next_is_line = ( dynamic_cast<Geom::LineSegment const *>(&*curve_it2)  ||
                                  dynamic_cast<Geom::HLineSegment const *>(&*curve_it2) ||
                                  dynamic_cast<Geom::VLineSegment const *>(&*curve_it2) );
            Geom::Point deriv_1 = curve_it1->unitTangentAt(1);
            Geom::Point deriv_2 = curve_it2->unitTangentAt(0);
            double this_angle_L2 = Geom::L2(deriv_1);
            double next_angle_L2 = Geom::L2(deriv_2);
            double both_angles_L2 = Geom::L2(deriv_1 + deriv_2);
            if ( (this_angle_L2 > 1e-6) &&
                 (next_angle_L2 > 1e-6) &&
                 ((this_angle_L2 + next_angle_L2 - both_angles_L2) < 1e-3) )
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

        if (path_it->closed()) {
            // curve_it1 points to closing segment, which is always a straight line
            if (curve_it1->initialPoint() == curve_it1->finalPoint()) {
                // zero length closing segment, so  last handled seg already closed the path
                // When the original path was *not* visibly closed by the straight line "svgd-'z'" segment (default closing segment)
                // this means the last handled segment already closed the original path.
                // FIXME: how to correctly handle this case??
                path[0].ty = path[ip-1].ty;
            } else {
                // When the original path was visibly closed by the straight line "svgd-'z'" segment (default closing segment)
                // then close the spiro path with the same straight line path
                path[0].ty = 'v';
            }
        } else {
            // add point to the path
            Geom::Point p = curve_it1->finalPoint();
            path[ip].x = p[X];
            path[ip].y = p[Y];
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

    // FIXME: refactor the spiro code such that it cannot generate non-continous paths!
    // sometimes, the code above generates a path that is not continuous: caused by chaotic algorithm?
    // The continuity error always happens after a lot of curveto calls (a big path probably that spins to infinity?)
    // if so, undo the effect by resetting the original path.
    try {
        curve->get_pathvector() * Geom::identity(); // tests for continuity, this makes LPE Spiro slower of course :-(
    }
    catch (Geom::ContinuityError & e) {
        g_warning("Exception during LPE Spiro execution. \n %s", e.what());
        SP_ACTIVE_DESKTOP->messageStack()->flash( Inkscape::WARNING_MESSAGE,
            _("An exception occurred during execution of the Spiro Path Effect.") );
        curve->set_pathvector(original_pathv);
    }
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
