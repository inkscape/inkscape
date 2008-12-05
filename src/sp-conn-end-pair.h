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
#include <glib/gtypes.h>

#include "forward.h"
#include "libnr/nr-point.h"
#include <sigc++/connection.h>
#include <sigc++/functors/slot.h>
#include <sigc++/signal.h>
#include "libavoid/connector.h"


class SPConnEnd;
namespace Inkscape {
namespace XML {
class Node;
}
}


class SPConnEndPair {
public:
    SPConnEndPair(SPPath *);
    ~SPConnEndPair();
    void release();
    void setAttr(unsigned const key, gchar const *const value);
    void writeRepr(Inkscape::XML::Node *const repr) const;
    void getAttachedItems(SPItem *[2]) const;
    void getEndpoints(Geom::Point endPts[]) const;
    void reroutePath(void);
    void makePathInvalid(void);
    void update(void);
    bool isAutoRoutingConn(void);
    void rerouteFromManipulation(void);
    void reroute(void);
    sigc::connection connectInvalidPath(sigc::slot<void, SPPath *> slot);

    // A signal emited by a call back from libavoid.  Used to let 
    // connectors know when they need to reroute themselves.
    sigc::signal<void, SPPath *> _invalid_path_signal;
    // A sigc connection to listen for connector path invalidation.
    sigc::connection _invalid_path_connection;

private:
    SPConnEnd *_connEnd[2];
    
    SPPath *_path;

    // libavoid's internal representation of the item.
    Avoid::ConnRef *_connRef;

    int _connType;
    
    // A sigc connection for transformed signal.
    sigc::connection _transformed_connection;
    
    void storeIds(void);
};


void sp_conn_end_pair_build(SPObject *object);


// _connType options:
enum {
    SP_CONNECTOR_NOAVOID,    // Basic connector - a straight line.
    SP_CONNECTOR_POLYLINE    // Object avoiding polyline.
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
