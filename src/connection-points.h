#ifndef INKSCAPE_CONNECTION_POINT_H
#define INKSCAPE_CONNECTION_POINT_H

#include <2geom/point.h>
//#include <libavoid/vertices.h>
#include <libavoid/connector.h>

#include "svg/stringstream.h"


enum ConnPointType {
    ConnPointDefault = 0,
    ConnPointUserDefined = 1
};
enum ConnPointDefaultPos{
    ConnPointPosTL, // Top Left
    ConnPointPosTC, // Top Centre
    ConnPointPosTR, // Top Right
    ConnPointPosCL, // Centre Left
    ConnPointPosCC, // Centre Centre
    ConnPointPosCR, // Centre Right
    ConnPointPosBL, // Bottom Left
    ConnPointPosBC, // Bottom Centre
    ConnPointPosBR, // Bottom Right
};


struct ConnectionPoint
{
    ConnectionPoint():
        type(ConnPointDefault), // default to a default connection point
        id(ConnPointPosCC), // default to the centre point
        pos(),
        dir(Avoid::ConnDirAll) // allow any direction
    {
    }
    // type of the connection point
    // default or user-defined
    int type;

    /* id of the connection point
       in the case of default
       connection points it specifies
       which of the 9 types the
       connection point is.
    */
    int id;

    /* position related to parent item
       in the case of default connection
       points, these positions should be
       computed by the item's avoidRef
    */
    Geom::Point pos;

    // directions from which connections can occur
    Avoid::ConnDirFlags dir;

    bool operator!=(ConnectionPoint&);
    bool operator==(ConnectionPoint&);
};

namespace Inkscape{

SVGIStringStream& operator>>(SVGIStringStream& istr, ConnectionPoint& cp);
SVGOStringStream& operator<<(SVGOStringStream& ostr, const ConnectionPoint& cp);

}

#endif