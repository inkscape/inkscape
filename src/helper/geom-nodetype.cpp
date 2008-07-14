#define INKSCAPE_HELPER_GEOM_NODETYPE_CPP

/**
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
 * Returns the nodetype between c_incoming and c_outgoing. Location of the node is
 * at c_incoming.pointAt(1) == c_outgoing.pointAt(0). If these two are unequal, 
 * the returned type is NODE_NONE.
 * This method uses exact floating point comparison, so the final and initial points of
 * the two input curves should match exactly!
 */
NodeType get_nodetype(Curve const &c_incoming, Curve const &c_outgoing)
{
    // FIXME: this should be exact floating point match, not are_near!
    if ( !are_near(c_incoming.pointAt(1), c_outgoing.pointAt(0)) )
        return NODE_NONE;

    Curve * c1_reverse = c_incoming.reverse();
    std::vector<Point> deriv1 = c1_reverse->pointAndDerivatives(0, 3);
    delete c1_reverse;
    std::vector<Point> deriv2 = c_outgoing.pointAndDerivatives(0, 3);

    // Determine lowest derivative that is non-zero
    int n1 = 1;
    while ( (deriv1[n1] == Point(0,0)) && (n1 < 3) ) {
        n1++;
    }
    int n2 = 1;
    while ( (deriv2[n2] == Point(0,0)) && (n2 < 3) ) {
        n2++;
    }

    double const angle1 = Geom::atan2(-deriv1[n1]);
    double const angle2 = Geom::atan2(deriv2[n2]);

    if ( !are_near(angle1, angle2) )
        return NODE_CUSP;   // derivatives are not colinear

    // Apparently, the derivatives are colinear but does the order of the derivatives match?
    if (n1 != n2)
        return NODE_SMOOTH;
    else
        return NODE_SYMM;
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
