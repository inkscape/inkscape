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

//! @file    router.h
//! @brief   Contains the interface for the Router class.


#ifndef AVOID_ROUTER_H
#define AVOID_ROUTER_H

#include <list>
#include <utility>
#include <string>

#include "libavoid/shape.h"
#include "libavoid/viscluster.h"
#include "libavoid/graph.h"
#include "libavoid/timer.h"
#include "libavoid/connector.h"

#if defined(LINEDEBUG) || defined(ASTAR_DEBUG) || defined(LIBAVOID_SDL)
    #include <SDL.h>
    #ifndef LIBAVOID_SDL
        #define LIBAVOID_SDL
    #endif
#endif


namespace Avoid {

typedef std::list<unsigned int> IntList;

class ActionInfo;
typedef std::list<ActionInfo> ActionInfoList;
class ShapeRef;

//! @brief  Flags that can be passed to the router during initialisation 
//!         to specify options.
enum RouterFlag
{
    //! @brief  This option specifies that the router should maintain the
    //!         structures necessary to allow poly-line connector routing.
    PolyLineRouting = 1,
    //! @brief  This option specifies that the router should maintain the
    //!         structures necessary to allow orthogonal connector routing.
    OrthogonalRouting = 2
};


static const unsigned int runningTo = 1;
static const unsigned int runningFrom = 2;
static const unsigned int runningToAndFrom = runningTo | runningFrom;

//! @brief  Types of penalty cases that can be used to improve the quality 
//!         of the connector routes produced.
enum PenaltyType
{
    //! @brief  This penalty is applied for each segment in the connector 
    //!         path beyond the first.  This should always normally be set
    //!         when doing orthogonal routing to prevent step-like connector
    //!         paths.
    segmentPenalty = 0,
    //! @brief  This penalty is applied in its full amount to tight acute 
    //!         bends in the connector path.  A smaller portion of the penalty
    //!         is applied for slight bends, i.e., where the bend is close to
    //!         180 degrees.  This is useful for polyline routing where there
    //!         is some evidence that tighter corners are worse for 
    //!         readability, but that slight bends might not be so bad, 
    //!         especially when smoothed by curves.
    anglePenalty,
    //! @brief  This penalty is applied whenever a connector path crosses 
    //!         another connector path.  It takes shared paths into 
    //!         consideration and the penalty is only applied if there
    //!         is an actual crossing.
    //! @note   This penalty is still experimental!  It is not recommended
    //!         for normal use.
    crossingPenalty,
    //! @brief  This penalty is applied whenever a connector path crosses 
    //!         a cluster boundary.
    //! @note   This penalty is still experimental!  It is not recommended
    //!         for normal use.
    clusterCrossingPenalty,
    //! @brief  This penalty is applied whenever a connector path shares 
    //!         some segments with an immovable portion of an existing 
    //!         connector route (such as the first or last segment of a
    //!         connector).
    //! @note   This penalty is still experimental!  It is not recommended
    //!         for normal use.
    fixedSharedPathPenalty,
    // Used for determining the size of the penalty array.  
    // This should always we the last value in the enum.
    lastPenaltyMarker
};


static const double noPenalty = 0;
static const double chooseSensiblePenalty = -1;


//! @brief   The Router class represents a libavoid router instance.
//!
//! Usually you would keep a separate Router instance for each diagram
//! or layout you have open in your application.
//
class Router {
    public:
        //! @brief  Constructor for router instance.
        //!
        //! @param[in]  flags  One or more Avoid::RouterFlag options to 
        //!                    control the behaviour of the router.
        Router(const unsigned int flags);

        //! @brief  Destructor for router instance.
        //!
        //! @note   Destroying a router instance will delete all remaining
        //!         shapes and connectors, thereby invalidating any existing
        //!         pointers to them.
        ~Router();

        ShapeRefList shapeRefs;
        ConnRefList connRefs;
        ClusterRefList clusterRefs;
        EdgeList visGraph;
        EdgeList invisGraph;
        EdgeList visOrthogGraph;
        ContainsMap contains;
        VertInfList vertices;
        ContainsMap enclosingClusters;
        
        bool PartialTime;
        bool SimpleRouting;
        bool ClusteredRouting;

        // Poly-line routing options:
        bool IgnoreRegions;
        bool UseLeesAlgorithm;
        bool InvisibilityGrph;
       
        // General routing options:
        bool SelectiveReroute;
        
        bool PartialFeedback;
        bool RubberBandRouting;
        

        // Instrumentation:
        Timer timers;
        int st_checked_edges;
#ifdef LIBAVOID_SDL
        SDL_Surface *avoid_screen;
#endif

        //! @brief Allows setting of the behaviour of the router in regard
        //!        to transactions.  This controls whether transactions are
        //!        used to queue changes and process them effeciently at once
        //!        or they are instead processed immediately.
        //!
        //! It is more efficient to perform actions like shape movement,
        //! addition or deletion as batch tasks, and reroute the necessary
        //! connectors just once after these actions have been performed.
        //! For this reason, libavoid allows you to group such actions into
        //! "transactions" that are processed efficiently when the 
        //! processTransaction() method is called.
        //!
        //! By default, the router will process all actions as tranactions.
        //! If transactionUse() is set to false, then all actions will get 
        //! processed immediately, and cause immediate routing callbacks to 
        //! all affected connectors after each action.
        //!
        //! @param[in]  transactions  A boolean value specifying whether to
        //!                           use transactions.
        //!
        void setTransactionUse(const bool transactions);

        //! @brief Reports whether the router groups actions into transactions.
        //!
        //! @return A boolean value describing whether transactions are in use.
        //!
        //! @sa setTransactionUse
        //! @sa processTransaction
        //!
        bool transactionUse(void) const;

        //! @brief Finishes the current transaction and processes all the 
        //!        queued object changes efficiently.
        //!
        //! This method will efficiently process all moves, additions and
        //! deletions that have occurred since processTransaction() was 
        //! last called.
        //!
        //! If transactionUse() is false, then all actions will have been 
        //! processed immediately and this method will do nothing.
        //!
        //! @return A boolean value describing whether there were any actions
        //!         to process.
        //!
        //! @sa setTransactionUse
        //!
        bool processTransaction(void);
        
        //! @brief Add a shape to the router scene.
        //!
        //! This shape will be considered to be an obstacle. Calling this 
        //! method will cause connectors intersecting the added shape to
        //! be marked as needing to be rerouted.
        //!
        //! @param[in]  shape  Pointer reference to the shape being added.
        //!
        void addShape(ShapeRef *shape);

        //! @brief Remove a shape from the router scene.
        //!
        //! Connectors that could have a better (usually shorter) path after
        //! the removal of this shape will be marked as needing to be rerouted.
        //!
        //! @param[in]  shape  Pointer reference to the shape being removed.
        //!
        void removeShape(ShapeRef *shape);

        //! @brief Move or resize an existing shape within the router scene.
        //!
        //! A new polygon for the shape can be given to effectively move or 
        //! resize the shape with the scene.  Connectors that intersect the 
        //! new shape polygon, or that could have a better (usually shorter)
        //! path after the change, will be marked as needing to be rerouted.
        //!
        //! @param[in]  shape       Pointer reference to the shape being 
        //!                         moved/resized.
        //! @param[in]  newPoly     The new polygon boundary for the shape.
        //! @param[in]  first_move  This option is used for some advanced 
        //!                         (currently undocumented) behaviour and 
        //!                         it should be ignored for the moment.
        //!
        void moveShape(ShapeRef *shape, const Polygon& newPoly,
                const bool first_move = false);

        //! @brief Move an existing shape within the router scene by a relative
        //!        distance.
        //!         
        //! Connectors that intersect the shape's new position, or that could 
        //! have a better (usually shorter) path after the change, will be 
        //! marked as needing to be rerouted.
        //!
        //! @param[in]  shape       Pointer reference to the shape being moved.
        //! @param[in]  xDiff       The distance to move the shape in the 
        //!                         x dimension.
        //! @param[in]  yDiff       The distance to move the shape in the 
        //!                         y dimension.
        //!
        void moveShape(ShapeRef *shape, const double xDiff, const double yDiff);

        //! @brief Sets a spacing distance for overlapping orthogonal 
        //!        connectors to be nudged apart.
        //!         
        //! By default, this distance is set to a value of 4.
        //!
        //! This method does not re-trigger post-processing of connectors.
        //! The new distance will be used the next time rerouting is performed.
        //!
        //! @param[in]  dist  The distance to be used for orthogonal nudging.
        //!
        void setOrthogonalNudgeDistance(const double dist);

        //! @brief   Returns the spacing distance that overlapping orthogonal
        //!          connecotrs are nudged apart.
        //!
        //! @return  The current spacing distance used for orthogonal nudging.
        //!
        double orthogonalNudgeDistance(void) const;

        //! @brief  Sets or removes penalty values that are applied during 
        //!         connector routing.
        //!
        //! By default, libavoid will produce shortest path routes between
        //! the source and destination points for each connector.  There are
        //! several penalties that can be applied during this stage to 
        //! improve the aesthetics of the routes generated.  These different
        //! penalties are specified and explained by the PenaltyType enum.
        //! 
        //! If a value of zero or Avoid::noPenalty is given then the penalty 
        //! for this case will be removed.  If no penalty argument (or a 
        //! negative value) is specified when calling this method, then a 
        //! sensible penalty value will be automatically chosen.
        //!
        //! @param[in] penType  The type of penalty, a PenaltyType.
        //! @param[in] penVal   The value to be applied for each occurance
        //!                     of the penalty case.  
        //!
        void setRoutingPenalty(const PenaltyType penType, 
                const double penVal = chooseSensiblePenalty);

        //! @brief  Returns the current penalty value for a particular 
        //!         routing penalty case.
        //!
        //! @param[in] penType  The type of penalty, a PenaltyType.
        //! @return  The penalty value for the specified penalty case.
        //!
        double routingPenalty(const PenaltyType penType) const;

        void addCluster(ClusterRef *cluster);
        void delCluster(ClusterRef *cluster);

        void attachedConns(IntList &conns, const unsigned int shapeId,
                const unsigned int type);
        void attachedShapes(IntList &shapes, const unsigned int shapeId,
                const unsigned int type);
        
        void markConnectors(ShapeRef *shape);
        void generateContains(VertInf *pt);
        void printInfo(void);
        void outputInstanceToSVG(std::string filename = std::string());
        unsigned int assignId(const unsigned int suggestedId);
        void regenerateStaticBuiltGraph(void);
        void destroyOrthogonalVisGraph(void);
        void setStaticGraphInvalidated(const bool invalidated);
        ConnType validConnType(const ConnType select = ConnType_None) const;
        bool shapeInQueuedActionList(ShapeRef *shape) const;
        double& penaltyRef(const PenaltyType penType);
        bool existsOrthogonalPathOverlap(void);
        bool existsOrthogonalTouchingCorners(void);

    private:
        friend class ConnRef;

        void modifyConnector(ConnRef *conn);
        void modifyConnector(ConnRef *conn, unsigned int type,
                const ConnEnd &connEnd);
        void removeQueuedConnectorActions(ConnRef *conn);
        void newBlockingShape(const Polygon& poly, int pid);
        void checkAllBlockedEdges(int pid);
        void checkAllMissingEdges(void);
        void adjustContainsWithAdd(const Polygon& poly, const int p_shape);
        void adjustContainsWithDel(const int p_shape);
        void adjustClustersWithAdd(const PolygonInterface& poly, 
                const int p_cluster);
        void adjustClustersWithDel(const int p_cluster);
        void rerouteAndCallbackConnectors(void);
        bool idIsUnique(const unsigned int id) const;
        void improveCrossings(void);

        ActionInfoList actionList;
        unsigned int _largestAssignedId;
        bool _consolidateActions;
        double _orthogonalNudgeDistance;
        double _routingPenalties[lastPenaltyMarker];

    public:
        // Overall modes:
        bool _polyLineRouting;
        bool _orthogonalRouting;

        bool _staticGraphInvalidated;
        bool _inCrossingPenaltyReroutingStage;
};

}



#endif
