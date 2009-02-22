/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2004-2006  Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
*/

#ifndef AVOID_CONNECTOR_H
#define AVOID_CONNECTOR_H

#include "libavoid/router.h"
#include "libavoid/geometry.h"
#include "libavoid/shape.h"
#include <list>


namespace Avoid {


static const int ConnType_PolyLine   = 1;
static const int ConnType_Orthogonal = 2;


class ConnRef
{
    public:
        ConnRef(Router *router, const unsigned int id);
        ConnRef(Router *router, const unsigned int id,
                const Point& src, const Point& dst);
        virtual ~ConnRef();
        
        void setType(unsigned int type);
        PolyLine& route(void);
        bool needsReroute(void);
        void freeRoute(void);
        void calcRouteDist(void);
        void updateEndPoint(const unsigned int type, const Point& point);
        void setEndPointId(const unsigned int type, const unsigned int id);
        unsigned int getSrcShapeId(void);
        unsigned int getDstShapeId(void);
        void makeActive(void);
        void makeInactive(void);
        void lateSetup(const Point& src, const Point& dst);
        unsigned int id(void);
        VertInf *src(void);
        VertInf *dst(void);
        void removeFromGraph(void);
        bool isInitialised(void);
        void unInitialise(void);
        void setCallback(void (*cb)(void *), void *ptr);
        void handleInvalid(void);
        int generatePath(Point p0, Point p1);
        void makePathInvalid(void);
        Router *router(void);
        void setHateCrossings(bool value);
        bool doesHateCrossings(void);
        
        friend void Router::attachedShapes(IntList &shapes,
                const unsigned int shapeId, const unsigned int type);
        friend void Router::attachedConns(IntList &conns,
                const unsigned int shapeId, const unsigned int type);
        friend void Router::markConnectors(ShapeRef *shape);
        
    private:
        Router *_router;
        unsigned int _id;
        unsigned int _type;
        unsigned int _srcId, _dstId;
        bool _needs_reroute_flag;
        bool _false_path;
        bool _active;
        PolyLine _route;
        double _route_dist;
        ConnRefList::iterator _pos;
        VertInf *_srcVert;
        VertInf *_dstVert;
        bool _initialised;
        void (*_callback)(void *);
        void *_connector;
        bool _hateCrossings;
};


}


#endif


