#define INKSCAPE_LPE_SPIRO_C

/*
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-spiro.h"
#include "display/curve.h"
#include <libnr/n-art-bpath.h>
#include "nodepath.h"

#include "live_effects/bezctx.h"
#include "live_effects/bezctx_intf.h"
#include "live_effects/spiro.h"

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
    SPCurve *csrc = curve->copy();
    curve->reset();
    bezctx *bc = new_bezctx_ink(curve);
    int len = SP_CURVE_LENGTH(csrc);
    spiro_cp *path = g_new (spiro_cp, len + 1);
    NArtBpath const *bpath = csrc->get_bpath();
    int ib = 0;
    int ip = 0;
    bool closed = false;
    NR::Point pt(0, 0);
    NArtBpath const *first_in_subpath = NULL;
    while(ib <= len) {
        path [ip].x = bpath[ib].x3;
        path [ip].y = bpath[ib].y3;
        // std::cout << "==" << bpath[ib].code << "   ip" << ip << "  ib" << ib << "\n";
        if (bpath[ib].code == NR_END || bpath[ib].code == NR_MOVETO_OPEN || bpath[ib].code == NR_MOVETO) {
            if (ip != 0) { // run prev subpath
                int sp_len = 0;
                if (!closed) {
                     path[ip - 1].ty = '}';
                    sp_len = ip;
                } else {
                    sp_len = ip - 1;
                }
                spiro_seg *s = NULL;
                //for (int j = 0; j <= sp_len; j ++) printf ("%c\n", path[j].ty);
                s = run_spiro(path, sp_len);
                spiro_to_bpath(s, sp_len, bc);
                free(s);
                path[0].x = path[ip].x;
                path[0].y = path[ip].y;
                ip = 0;
            }
            if (bpath[ib].code == NR_MOVETO_OPEN) {
                closed = false;
                path[ip].ty = '{';
            } else {
                closed = true;
                if (ib  < len)
                    first_in_subpath = &(bpath[ib + 1]);
                path[ip].ty = 'c';
            }
        } else {
                // this point is not last, so makes sense to find a proper type for it
                NArtBpath const *next = NULL;
                if (ib < len && (bpath[ib+1].code == NR_END || bpath[ib+1].code == NR_MOVETO_OPEN || bpath[ib+1].code == NR_MOVETO)) { // end of subpath
                    if (closed)
                        next = first_in_subpath;
                } else {
                    if (ib < len)
                        next = &(bpath[ib+1]);
                    else if (closed)
                        next = first_in_subpath;
                }
                if (next) {
                    bool this_is_line = bpath[ib].code == NR_LINETO ||
                        (NR::L2(NR::Point(bpath[ib].x3, bpath[ib].y3) - NR::Point(bpath[ib].x2, bpath[ib].y2)) < 1e-6);
                    bool next_is_line = next->code == NR_LINETO ||
                        (NR::L2(NR::Point(bpath[ib].x3, bpath[ib].y3) - NR::Point(next->x1, next->y1)) < 1e-6);
                    NR::Point this_angle (0, 0);
                    if (this_is_line) {
                        this_angle = NR::Point (bpath[ib].x3 - pt[NR::X], bpath[ib].y3 - pt[NR::Y]);
                    } else if (bpath[ib].code == NR_CURVETO) {
                        this_angle = NR::Point (bpath[ib].x3 - bpath[ib].x2, bpath[ib].y3 - bpath[ib].y2);
                    }
                    NR::Point next_angle (0, 0);
                    if (next_is_line) {
                        next_angle = NR::Point (next->x3 - bpath[ib].x3, next->y3 - bpath[ib].y3);
                    } else if (next->code == NR_CURVETO) {
                        next_angle = NR::Point (next->x1 - bpath[ib].x3, next->y1 - bpath[ib].y3);
                    }
                    double this_angle_L2 = NR::L2(this_angle);
                    double next_angle_L2 = NR::L2(next_angle);
                    double both_angles_L2 = NR::L2(this_angle + next_angle);
                    if (this_angle_L2 > 1e-6 &&
                        next_angle_L2 > 1e-6 &&
                        this_angle_L2 + next_angle_L2 - both_angles_L2 < 1e-3) {
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
                    if (closed && next == first_in_subpath) {
                        path[0].ty = path[ip].ty;
                    }
                }
        }
        pt  = NR::Point(bpath[ib].x3, bpath[ib].y3);
        ip++;
        ib++;
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
