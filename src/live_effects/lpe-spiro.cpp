#define INKSCAPE_LPE_SPIRO_C

/*
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-spiro.h"

#include "display/curve.h"
#include <typeinfo>
#include <2geom/pathvector.h>
#include <2geom/affine.h>
#include <2geom/curves.h>
#include "helper/geom-nodetype.h"
#include "helper/geom-curves.h"

#include "live_effects/spiro.h"

// For handling un-continuous paths:
#include "message-stack.h"
#include "inkscape.h"
#include "desktop.h"


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
LPESpiro::doEffect(SPCurve * curve)
{
    sp_spiro_do_effect(curve);
}

void sp_spiro_do_effect(SPCurve *curve){
    using Geom::X;
    using Geom::Y;

    // Make copy of old path as it is changed during processing
    Geom::PathVector const original_pathv = curve->get_pathvector();
    guint len = curve->get_segment_count() + 2;

    curve->reset();
    Spiro::spiro_cp *path = g_new (Spiro::spiro_cp, len);
    int ip = 0;

    for(Geom::PathVector::const_iterator path_it = original_pathv.begin(); path_it != original_pathv.end(); ++path_it) {
        if (path_it->empty())
            continue;

        // start of path
        {
            Geom::Point p = path_it->initialPoint();
            path[ip].x = p[X];
            path[ip].y = p[Y];
            path[ip].ty = '{' ;  // for closed paths, this is overwritten
            ip++;
        }

        // midpoints
        Geom::Path::const_iterator curve_it1 = path_it->begin();      // incoming curve
        Geom::Path::const_iterator curve_it2 = ++(path_it->begin());         // outgoing curve
        Geom::Path::const_iterator curve_endit = path_it->end_default(); // this determines when the loop has to stop

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
        Spiro::spiro_run(path, sp_len, *curve);
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
