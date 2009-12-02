/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2009  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the 
 * library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * Author(s):   Michael Wybrow <mjwybrow@users.sourceforge.net>
*/

//! @file    shape.h
//! @brief   Contains the interface for the ConnRef class.


#ifndef AVOID_CONNECTOR_H
#define AVOID_CONNECTOR_H

#include <list>
#include <vector>

#include "libavoid/vertices.h"
#include "libavoid/geometry.h"
#include "libavoid/shape.h"


namespace Avoid {

class Router;
class ConnRef;
typedef std::list<ConnRef *> ConnRefList;


//! @brief  Describes the type of routing that is performed for each 
//!         connector.
enum ConnType {
    ConnType_None       = 0,
    //! @brief  The connector path will be a shortest-path poly-line that
    //!         routes around obstacles.
    ConnType_PolyLine   = 1,
    //! @brief  The connector path will be a shortest-path orthogonal 
    //!         poly-line (only vertical and horizontal line segments) that
    //!         routes around obstacles.
    ConnType_Orthogonal = 2
};

//! @brief  Flags that can be passed to the ConnEnd constructor to specify
//!         which sides of a shape this point should have visibility to if
//!         it is located within the shape's area.
//!
//! Like SVG, libavoid considers the Y-axis to point downwards, that is, 
//! like screen coordinates the coordinates increase from left-to-right and 
//! also from top-to-bottom.
//!
enum ConnDirFlag {
    ConnDirNone  = 0,
    //! @brief  This option specifies the point should be given visibility 
    //!         to the top of the shape that it is located within.
    ConnDirUp    = 1,
    //! @brief  This option specifies the point should be given visibility 
    //!         to the bottom of the shape that it is located within.
    ConnDirDown  = 2,
    //! @brief  This option specifies the point should be given visibility 
    //!         to the left side of the shape that it is located within.
    ConnDirLeft  = 4,
    //! @brief  This option specifies the point should be given visibility 
    //!         to the right side of the shape that it is located within.
    ConnDirRight = 8,
    //! @brief  This option, provided for convenience, specifies the point 
    //!         should be given visibility to all four sides of the shape 
    //!         that it is located within.
    ConnDirAll   = 15
};
//! @brief  One or more Avoid::ConnDirFlag options.
//!
typedef unsigned int ConnDirFlags;


static const double ATTACH_POS_TOP = 0;
static const double ATTACH_POS_CENTER = 0.5;
static const double ATTACH_POS_BOTTOM = 1;
static const double ATTACH_POS_LEFT = ATTACH_POS_TOP;
static const double ATTACH_POS_RIGHT = ATTACH_POS_BOTTOM;


//! @brief  The ConnEnd class represents different possible endpoints for 
//!         connectors.
//!
//! Currently this class just allows free-floating endpoints, but in future
//! will be capable of representing attachments to connection points on shapes.
//! 
class ConnEnd 
{
    public:
        //! @brief Constructs a ConnEnd from a free-floating point.
        //!
        //! @param[in]  point  The position of the connector endpoint.
        //!
        ConnEnd(const Point& point);

        //! @brief Constructs a ConnEnd from a free-floating point as well
        //!        as a set of flags specifying visibility for this point 
        //!        if it is located inside a shape.
        //!
        //! @param[in]  point    The position of the connector endpoint.
        //! @param[in]  visDirs  One or more Avoid::ConnDirFlag options 
        //!                      specifying the directions that this point 
        //!                      should be given visibility if it is inside 
        //!                      a shape.
        //!
        ConnEnd(const Point& point, const ConnDirFlags visDirs);

        ConnEnd(ShapeRef *shapeRef, const double x_pos, const double y_pos,
                const double insideOffset = 0.0,
                const ConnDirFlags visDirs = ConnDirNone);

        //! @brief Returns the position of this connector endpoint
        //!
        //! @return The position of this connector endpoint.
        const Point point(void) const;

        ConnDirFlags directions(void) const;
    private:
        Point _point;
        ConnDirFlags _directions;
        
        // For referencing ConnEnds
        ShapeRef *_shapeRef;
        double _xPosition;
        double _yPosition;
        double _insideOffset;
};


//! @brief   The ConnRef class represents a connector object.
//!
//! Connectors are a (possible multi-segment) line between two points.
//! They are routed intelligently so as not to overlap any of the shape
//! objects in the Router scene.
//! 
//! Routing penalties can be applied, resulting in more aesthetically pleasing
//! connector paths with fewer segments or less severe bend-points.
//!
//! You can set a function to be called when the connector has been rerouted
//! and needs to be redrawn.  Alternatively, you can query the connector's
//! needsRepaint() function to determine this manually.
//!
//! Usually, it is expected that you would create a ConnRef for each connector 
//! in your diagram and keep that reference in your own connector class.
//!
class ConnRef
{
    public:
        //! @brief Constructs a connector with no endpoints specified.
        //!
        //! @param[in]  router  The router scene to place the connector into.
        //! @param[in]  id      A unique positive integer ID for the connector.  
        //!
        //! If an ID is not specified, then one will be assigned to the shape.
        //! If assigning an ID yourself, note that it should be a unique 
        //! positive integer.  Also, IDs are given to all objects in a scene,
        //! so the same ID cannot be given to a shape and a connector for 
        //! example.
        //!
        ConnRef(Router *router, const unsigned int id = 0);
        //! @brief Constructs a connector with endpoints specified.
        //!
        //! @param[in]  router  The router scene to place the connector into.
        //! @param[in]  id      A unique positive integer ID for the connector.
        //! @param[in]  src     The source endpoint of the connector.
        //! @param[in]  dst     The destination endpoint of the connector.
        //!
        //! If an ID is not specified, then one will be assigned to the shape.
        //! If assigning an ID yourself, note that it should be a unique 
        //! positive integer.  Also, IDs are given to all objects in a scene,
        //! so the same ID cannot be given to a shape and a connector for 
        //! example.
        //!
        ConnRef(Router *router, const ConnEnd& src, const ConnEnd& dst,
                const unsigned int id = 0);
        //! @brief  Destuctor.
        ~ConnRef();
        
        //! @brief  Sets both new source and destination endpoints for this 
        //!         connector.
        //!
        //! @param[in]  srcPoint  New source endpoint for the connector.
        //! @param[in]  dstPoint  New destination endpoint for the connector.
        void setEndpoints(const ConnEnd& srcPoint, const ConnEnd& dstPoint);
        //! @brief  Sets just a new source endpoint for this connector.
        //!
        //! @param[in]  srcPoint  New source endpoint for the connector.
        void setSourceEndpoint(const ConnEnd& srcPoint);
        //! @brief  Sets just a new destination endpoint for this connector.
        //!
        //! @param[in]  dstPoint  New destination endpoint for the connector.
        void setDestEndpoint(const ConnEnd& dstPoint);
        //! @brief   Returns the ID of this connector.
        //! @returns The ID of the connector. 
        unsigned int id(void) const;
        //! @brief   Returns a pointer to the router scene this connector is in.
        //! @returns A pointer to the router scene for this connector.
        Router *router(void) const;

        //! @brief   Returns an indication of whether this connector has a 
        //!          new route and thus needs to be repainted.
        //!
        //! If the connector has been rerouted and need repainting, the  
        //! route() method can be called to get a reference to the new route.
        //!
        //! @returns Returns true if the connector requires repainting, or 
        //!          false if it does not.
        bool needsRepaint(void) const;
        
        //! @brief   Returns a reference to the current route for the connector.
        //!
        //! This is a "raw" version of the route, where each line segment in
        //! the route may be made up of multiple collinear line segments.  It
        //! will also not have post-processing (like curved corners) applied
        //! to it.  The simplified route for display can be obtained by calling
        //! displayRoute().
        //!
        //! @returns The PolyLine route for the connector.
        //! @note    You can obtain a modified version of this poly-line 
        //!          route with curved corners added by calling 
        //!          PolyLine::curvedPolyline().
        const PolyLine& route(void) const;
        
        //! @brief   Returns a reference to the current display version of the
        //!          route for the connector.
        //! 
        //! The display version of a route has been simplified to collapse all
        //! collinear line segments into single segments.  It may also have 
        //! post-processing applied to the route, such as curved corners or
        //! nudging.
        //! 
        //! @returns The PolyLine display route for the connector.
        PolyLine& displayRoute(void);
        
        //! @brief   Sets a callback function that will called to indicate that
        //!          the connector needs rerouting.
        //!
        //! The cb function will be called when shapes are added to, removed 
        //! from or moved about on the page.  The pointer ptr will be passed 
        //! as an argument to the callback function.
        //!
        //! @param[in]  cb   A pointer to the callback function.
        //! @param[in]  ptr  A generic pointer that will be passed to the 
        //!                  callback function.
        void setCallback(void (*cb)(void *), void *ptr);
        //! @brief   Returns the type of routing performed for this connector.
        //! @return  The type of routing performed.
        //!
        ConnType routingType(void) const;
        //! @brief       Sets the type of routing to be performed for this 
        //!              connector.
        //! 
        //! If a call to this method changes the current type of routing 
        //! being used for the connector, then it will get rerouted during
        //! the next processTransaction() call, or immediately if 
        //! transactions are not being used.
        //!
        //! @param type  The type of routing to be performed.
        //!
        void setRoutingType(ConnType type);

       

        // @brief   Returns the source endpoint vertex in the visibility graph.
        // @returns The source endpoint vertex.
        VertInf *src(void);
        // @brief   Returns the destination endpoint vertex in the 
        //          visibility graph.
        // @returns The destination endpoint vertex.
        VertInf *dst(void);
        

        void set_route(const PolyLine& route);
        void calcRouteDist(void);
        void setEndPointId(const unsigned int type, const unsigned int id);
        unsigned int getSrcShapeId(void);
        unsigned int getDstShapeId(void);
        void makeActive(void);
        void makeInactive(void);
        VertInf *start(void);
        void removeFromGraph(void);
        bool isInitialised(void);
        void makePathInvalid(void);
        void setHateCrossings(bool value);
        bool doesHateCrossings(void);
        void setEndpoint(const unsigned int type, const ConnEnd& connEnd);
        bool setEndpoint(const unsigned int type, const VertID& pointID, 
                Point *pointSuggestion = NULL);
    
    private:
        friend class Router;

        PolyLine& routeRef(void);
        void freeRoutes(void);
        void performCallback(void);
        bool generatePath(void);
        bool generatePath(Point p0, Point p1);
        void unInitialise(void);
        void updateEndPoint(const unsigned int type, const ConnEnd& connEnd);
        void common_updateEndPoint(const unsigned int type, const ConnEnd& connEnd);
        Router *_router;
        unsigned int _id;
        ConnType _type;
        unsigned int _srcId, _dstId;
        bool _orthogonal;
        bool _needs_reroute_flag;
        bool _false_path;
        bool _needs_repaint;
        bool _active;
        PolyLine _route;
        Polygon _display_route;
        double _route_dist;
        ConnRefList::iterator _pos;
        VertInf *_srcVert;
        VertInf *_dstVert;
        VertInf *_startVert;
        bool _initialised;
        void (*_callback)(void *);
        void *_connector;
        bool _hateCrossings;
};


class PointRep;
typedef std::set<PointRep *> PointRepSet;
typedef std::list<PointRep *> PointRepList;

class PointRep
{
    public:
        PointRep(Point *p, const ConnRef *c)
            : point(p),
              conn(c)

        {
        }
        bool follow_inner(PointRep *target);

        Point *point;
        const ConnRef *conn;
        // inner_set: Set of pointers to the PointReps 'inner' of 
        // this one, at this corner.
        PointRepSet inner_set;
};


typedef std::pair<Point *, ConnRef *> PtConnPtrPair;

class PtOrder
{
    public:
        PtOrder()
        {
        }
        ~PtOrder();
        bool addPoints(const int dim, PtConnPtrPair innerArg, 
                PtConnPtrPair outerArg, bool swapped);
        void sort(const int dim);
        int positionFor(const ConnRef *conn, const size_t dim) const;

        // One for each dimension.
        PointRepList connList[2];
};

typedef std::map<Avoid::Point,PtOrder> PtOrderMap;
typedef std::set<Avoid::Point> PointSet;


const unsigned int CROSSING_NONE = 0;
const unsigned int CROSSING_TOUCHES = 1;
const unsigned int CROSSING_SHARES_PATH = 2;
const unsigned int CROSSING_SHARES_PATH_AT_END = 4;
const unsigned int CROSSING_SHARES_FIXED_SEGMENT = 8;


typedef std::pair<int, unsigned int> CrossingsInfoPair;

extern CrossingsInfoPair countRealCrossings( Avoid::Polygon& poly, 
        bool polyIsConn, Avoid::Polygon& conn, size_t cIndex, 
        bool checkForBranchingSegments, const bool finalSegment = false, 
        PointSet *crossingPoints = NULL, PtOrderMap *pointOrders = NULL, 
        ConnRef *polyConnRef = NULL, ConnRef *connConnRef = NULL);
extern void splitBranchingSegments(Avoid::Polygon& poly, bool polyIsConn,
        Avoid::Polygon& conn, const double tolerance = 0);
extern bool validateBendPoint(VertInf *aInf, VertInf *bInf, VertInf *cInf);

}


#endif


