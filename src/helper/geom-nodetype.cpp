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
 * If one of the curves has zero length, but the other doesn't, then the returned type
 * is NODE_SMOOTH. If both have zero length, the returned type is NODE_SYMM. There is no
 * good reason for this. Feel free to change, but check all uses of this method such
 * that it doesn't break anything!
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
    while ( (deriv1[n1] == Point(0.,0.)) && (n1 <= 3) ) {
        n1++;
    }
    int n2 = 1;
    while ( (deriv2[n2] == Point(0.,0.)) && (n2 <= 3) ) {
        n2++;
    }

    // if one of the paths still has zero derivative
    if ( (n1 > 3) || (n2 > 3) ) {
        if (n1 == n2)
            return NODE_SYMM;
        else
            return NODE_SMOOTH;
    }

    // get unit derivatives, so the errors do not depend on absolute lengths of derivatives
    Geom::Point const d1 = - deriv1[n1] / deriv1[n1].length(); // reverse sign because it is taken in "wrong direction"
    Geom::Point const d2 = deriv2[n2]  / deriv2[n2].length();

    double crossproduct = Geom::cross(d1, d2);
    if ( are_near( crossproduct , 0.) && (Geom::dot(d1, d2) > 0.) ) {
        // Apparently, the derivatives are colinear and in same direction but do they match exactly?
        if ( (n1 == n2) && (Geom::are_near(-deriv1[n1], deriv2[n2])) )
            return NODE_SYMM;
        else
            return NODE_SMOOTH;
    }

    return NODE_CUSP;
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
