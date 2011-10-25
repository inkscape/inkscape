#ifndef INKSCAPE_HELPER_GEOM_NODETYPE_H
#define INKSCAPE_HELPER_GEOM_NODETYPE_H

/**
 * @file
 * Specific nodetype geometry functions for Inkscape, not provided my lib2geom.
 */
/*
 * Author:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL
 */

#include <2geom/forward.h>

namespace Geom {

/**
 *  What kind of node is this?  This is the value for the node->type
 *  field.  NodeType indicates the degree of continuity required for
 *  the node.  I think that the corresponding integer indicates which
 *  derivate is connected. (Thus 2 means that the node is continuous
 *  to the second derivative, i.e. has matching endpoints and tangents)
 */
typedef enum {
/**  Discontinuous node, usually either start or endpoint of a path */
    NODE_NONE,
/**  This node continuously joins two segments, but the unit tangent is discontinuous.*/
    NODE_CUSP,
/**  This node continuously joins two segments, with continuous *unit* tangent. */
    NODE_SMOOTH,
/**  This node is symmetric. I.e. continously joins two segments with continuous derivative */
    NODE_SYMM
} NodeType;


NodeType get_nodetype(Curve const &c_incoming, Curve const &c_outgoing);


} // namespace Geom

#endif  // INKSCAPE_HELPER_GEOM_NODETYPE_H

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
