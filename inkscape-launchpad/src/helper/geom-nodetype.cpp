/*
 * Specific nodetype geometry functions for Inkscape, not provided my lib2geom.
 *
 * Author:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL
 */

#include "helper/geom-nodetype.h"

#include <2geom/curve.h>
#include <2geom/point.h>
#include <vector>

namespace Geom {

/*
 * NOTE: THIS METHOD NEVER RETURNS "NODE_SYMM".
 * Returns the nodetype between c_incoming and c_outgoing. Location of the node is
 * at c_incoming.pointAt(1) == c_outgoing.pointAt(0). If these two are unequal, 
 * the returned type is NODE_NONE.
 * Comparison is based on the unitTangent, does not work for NODE_SYMM!
 */
NodeType get_nodetype(Curve const &c_incoming, Curve const &c_outgoing)
{
    if ( !are_near(c_incoming.pointAt(1), c_outgoing.pointAt(0)) )
        return NODE_NONE;

    Geom::Curve *crv = c_incoming.reverse();
    Geom::Point deriv_1 = -crv->unitTangentAt(0);
    delete crv;
    Geom::Point deriv_2 = c_outgoing.unitTangentAt(0);
    double this_angle_L2 = Geom::L2(deriv_1);
    double next_angle_L2 = Geom::L2(deriv_2);
    double both_angles_L2 = Geom::L2(deriv_1 + deriv_2);
    if ( (this_angle_L2 > 1e-6) &&
         (next_angle_L2 > 1e-6) &&
         ((this_angle_L2 + next_angle_L2 - both_angles_L2) < 1e-3) )
    {
        return NODE_SMOOTH;
    }

    return NODE_CUSP;
}

} // end namespace Geom

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
