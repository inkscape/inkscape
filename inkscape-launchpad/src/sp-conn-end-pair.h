#ifndef SEEN_SP_CONN_END_PAIR
#define SEEN_SP_CONN_END_PAIR

/*
 * A class for handling connector endpoint movement and libavoid interaction.
 *
 * Authors:
 *   Peter Moulder <pmoulder@mail.csse.monash.edu.au>
 *
 *    * Copyright (C) 2004 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstddef>
#include <sigc++/sigc++.h>

#include "libavoid/connector.h"


class SPConnEnd;
class SPCurve;
class SPPath;
class SPItem;
class SPObject;

namespace Geom { class Point; }
namespace Inkscape {
namespace XML {
class Node;
}
}

extern void recreateCurve(SPCurve *curve, Avoid::ConnRef *connRef, double curvature);

class SPConnEndPair {
public:
    SPConnEndPair(SPPath *);
    ~SPConnEndPair();
    void release();
    void setAttr(unsigned const key, char const *const value);
    void writeRepr(Inkscape::XML::Node *const repr) const;
    void getAttachedItems(SPItem *[2]) const;
    void getEndpoints(Geom::Point endPts[]) const;
    double getCurvature(void) const;
    SPConnEnd** getConnEnds(void);
    bool isOrthogonal(void) const;
    friend void recreateCurve(SPCurve *curve, Avoid::ConnRef *connRef, double curvature);
    void tellLibavoidNewEndpoints(const bool processTransaction = false);
    bool reroutePathFromLibavoid(void);
    void makePathInvalid(void);
    void update(void);
    bool isAutoRoutingConn(void);
    void rerouteFromManipulation(void);

private:
    SPConnEnd *_connEnd[2];
    
    SPPath *_path;

    // libavoid's internal representation of the item.
    Avoid::ConnRef *_connRef;

    int _connType;
    double _connCurvature;
    
    // A sigc connection for transformed signal.
    sigc::connection _transformed_connection;
    
    void storeIds(void);
};


void sp_conn_end_pair_build(SPObject *object);


// _connType options:
enum {
    SP_CONNECTOR_NOAVOID,     // Basic connector - a straight line.
    SP_CONNECTOR_POLYLINE,    // Object avoiding polyline.
    SP_CONNECTOR_ORTHOGONAL   // Object avoiding orthogonal polyline (only horizontal and verical segments).
};


#endif /* !SEEN_SP_CONN_END_PAIR */

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
