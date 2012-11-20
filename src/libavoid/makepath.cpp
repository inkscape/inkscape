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


#include <algorithm>
#include <vector>
#include <climits>
#define _USE_MATH_DEFINES
#include <cmath>

#include "libavoid/vertices.h"
#include "libavoid/makepath.h"
#include "libavoid/geometry.h"
#include "libavoid/connector.h"
#include "libavoid/graph.h"
#include "libavoid/router.h"
#include "libavoid/debug.h"
#include "libavoid/assertions.h"
#ifdef ASTAR_DEBUG
  #include <SDL_gfxPrimitives.h>
#endif

namespace Avoid {

class ANode
{
    public:
        VertInf* inf;
        double g;        // Gone
        double h;        // Heuristic
        double f;        // Formula f = g + h
        
        int prevIndex;   // Index into DONE for the previous ANode.
        int timeStamp;   // Timestamp used to determine explaration order of
                         // seemingly equal paths during orthogonal routing.

        ANode(VertInf *vinf, int time)
            : inf(vinf),
              g(0),
              h(0),
              f(0),
              prevIndex(-1),
              timeStamp(time)
        {
        }
        ANode()
            : inf(NULL),
              g(0),
              h(0),
              f(0),
              prevIndex(-1),
              timeStamp(-1)
        {
        }
};


// This returns the opposite result (>) so that when used with stl::make_heap, 
// the head node of the heap will be the smallest value, rather than the 
// largest.  This saves us from having to sort the heap (and then reorder
// it back into a heap) when getting the next node to examine.  This way we
// get better complexity -- logarithmic pushs and pops to the heap.
//
static bool operator<(const ANode &a, const ANode &b)
{
    if (a.f != b.f)
    {
        return a.f > b.f;
    }
    if (a.timeStamp != b.timeStamp)
    {
        // Tiebreaker, if two paths have equal cost, then choose the one with
        // the highest timeStamp.  This corresponds to the furthest point
        // explored along the straight-line path.  When exploring we give the
        // directions the following timeStamps; left:1, right:2 and forward:3,
        // then we always try to explore forward first.
        return a.timeStamp < b.timeStamp;
    }
    COLA_ASSERT(a.prevIndex != b.prevIndex);
    return a.prevIndex > b.prevIndex;
}


static double Dot(const Point& l, const Point& r)
{
    return (l.x * r.x) + (l.y * r.y);
}

static double CrossLength(const Point& l, const Point& r)
{
    return (l.x * r.y) - (l.y * r.x);
}


// Return the angle between the two line segments made by the
// points p1--p2 and p2--p3.  Return value is in radians.
//
static double angleBetween(const Point& p1, const Point& p2, const Point& p3)
{
    if ((p1.x == p2.x && p1.y == p2.y) || (p2.x == p3.x && p2.y == p3.y))
    {
        // If two of the points are the same, then we can't say anything
        // about the angle between.  Treat them as being collinear.
        return M_PI;
    }

    Point v1(p1.x - p2.x, p1.y - p2.y);
    Point v2(p3.x - p2.x, p3.y - p2.y);

    return fabs(atan2(CrossLength(v1, v2), Dot(v1, v2)));
}


// Construct a temporary Polygon path given several VertInf's for a connector.
//
static void constructPolygonPath(Polygon& connRoute, VertInf *inf2, 
        VertInf *inf3, std::vector<ANode>& done, int inf1Index)
{
    int routeSize = 2;
    for (int curr = inf1Index; curr >= 0; curr = done[curr].prevIndex)
    {
        routeSize += 1;
    }
    connRoute.ps.resize(routeSize);
    connRoute.ps[routeSize - 1] = inf3->point;
    connRoute.ps[routeSize - 2] = inf2->point;
    routeSize -= 3;
    for (int curr = inf1Index; curr >= 0; curr = done[curr].prevIndex)
    {
        connRoute.ps[routeSize] = done[curr].inf->point;
        routeSize -= 1;
    }
}


// Given the two points for a new segment of a path (inf2 & inf3)
// as well as the distance between these points (dist), as well as
// possibly the previous point (inf1) [from inf1--inf2], return a
// cost associated with this route.
//
static double cost(ConnRef *lineRef, const double dist, VertInf *inf2, 
        VertInf *inf3, std::vector<ANode>& done, int inf1Index)
{
    VertInf *inf1 = (inf1Index >= 0) ?  done[inf1Index].inf : NULL;
    double result = dist;
    Polygon connRoute;

    Router *router = inf2->_router;
    if (inf1 != NULL)
    {
        const double angle_penalty = router->routingPenalty(anglePenalty);
        const double segmt_penalty = router->routingPenalty(segmentPenalty);

        // This is not the first segment, so there is a bend
        // between it and the last one in the existing path.
        if ((angle_penalty > 0) || (segmt_penalty > 0))
        {
            Point p1 = inf1->point;
            Point p2 = inf2->point;
            Point p3 = inf3->point;

            double rad = M_PI - angleBetween(p1, p2, p3);

            if (rad > 0)
            {
                // Make `xval' between 0--10 then take its log so small
                // angles are not penalised as much as large ones.
                //
                double xval = rad * 10 / M_PI;
                double yval = xval * log10(xval + 1) / 10.5;
                result += (angle_penalty * yval);
                //db_printf("deg from straight: %g\tpenalty: %g\n",
                //        rad * 180 / M_PI, (angle_penalty * yval));
            }

            if (rad == M_PI)
            {
                // Needs to double back
                result += (2 * segmt_penalty);
            }
            else if (rad > 0)
            {
                // Only penalise as an extra segment if the two 
                // segments are not collinear.
                result += segmt_penalty;
            }
        }
    }

    if (!router->_inCrossingPenaltyReroutingStage)
    {
        // Return here if we ar not in the postprocessing stage 
        return result;
    }

    const double cluster_crossing_penalty = 
            router->routingPenalty(clusterCrossingPenalty);
    // XXX: Clustered routing doesn't yet work with orthogonal connectors.
    if (router->ClusteredRouting && !router->clusterRefs.empty() &&
            (cluster_crossing_penalty > 0) && 
            (lineRef->routingType() != ConnType_Orthogonal))
    {
        if (connRoute.empty())
        {
            constructPolygonPath(connRoute, inf2, inf3, done, inf1Index);
        }
        // There are clusters so do cluster routing.
        for (ClusterRefList::const_iterator cl = router->clusterRefs.begin(); 
                cl != router->clusterRefs.end(); ++cl)
        {
            ReferencingPolygon& cBoundary = (*cl)->polygon();
            COLA_ASSERT(cBoundary.ps[0] != cBoundary.ps[cBoundary.size() - 1]);
            for (size_t j = 0; j < cBoundary.size(); ++j)
            {
                // Cluster boundary points should correspond to shape 
                // vertices and hence already be in the list of vertices.
                COLA_ASSERT(router->vertices.getVertexByPos(cBoundary.at(j))!=NULL);
            }
            
            bool isConn = false;
            Polygon dynamic_c_boundary(cBoundary);
            Polygon dynamic_conn_route(connRoute);
            const bool finalSegment = (inf3 == lineRef->dst());
            CrossingsInfoPair crossings = countRealCrossings(
                    dynamic_c_boundary, isConn, dynamic_conn_route, 
                    connRoute.size() - 1, true, finalSegment);
            result += (crossings.first * cluster_crossing_penalty);
        }
    }

    const double shared_path_penalty = 
            router->routingPenalty(fixedSharedPathPenalty);
    if (shared_path_penalty > 0)
    {
        // Penalises shared paths, except if the connectors shared an endpoint.
        if (connRoute.empty())
        {
            constructPolygonPath(connRoute, inf2, inf3, done, inf1Index);
        }
        ConnRefList::const_iterator curr, finish = router->connRefs.end();
        for (curr = router->connRefs.begin(); curr != finish; ++curr)
        {
            ConnRef *connRef = *curr;

            if (connRef->id() == lineRef->id())
            {
                continue;
            }
            const Avoid::PolyLine& route2 = connRef->route();
            
            bool isConn = true;
            Polygon dynamic_route2(route2);
            Polygon dynamic_conn_route(connRoute);
            CrossingsInfoPair crossings = countRealCrossings(
                    dynamic_route2, isConn, dynamic_conn_route, 
                    connRoute.size() - 1, false, false, NULL, NULL,
                    connRef, lineRef);

            if ((crossings.second & CROSSING_SHARES_PATH) &&
                    (crossings.second & CROSSING_SHARES_FIXED_SEGMENT) &&
                    !(crossings.second & CROSSING_SHARES_PATH_AT_END))
            {
                // Penalise unecessary shared paths in the middle of
                // connectors.
                result += shared_path_penalty;
            }
        }
    }

    const double crossing_penalty = router->routingPenalty(crossingPenalty);
    if (lineRef->doesHateCrossings() && (crossing_penalty > 0))
    {
        if (connRoute.empty())
        {
            constructPolygonPath(connRoute, inf2, inf3, done, inf1Index);
        }
        ConnRefList::const_iterator curr, finish = router->connRefs.end();
        for (curr = router->connRefs.begin(); curr != finish; ++curr)
        {
            ConnRef *connRef = *curr;

            if (connRef->id() == lineRef->id())
            {
                continue;
            }
            const Avoid::PolyLine& route2 = connRef->route();
            
            bool isConn = true;
            Polygon dynamic_route2(route2);
            Polygon dynamic_conn_route(connRoute);
            CrossingsInfoPair crossings = countRealCrossings(
                    dynamic_route2, isConn, dynamic_conn_route, 
                    connRoute.size() - 1, true);
            result += (crossings.first * crossing_penalty);
        }
    }

    return result;
}


static double estimatedCost(ConnRef *lineRef, const Point *last, 
        const Point& a, const Point& b)
{
    if (lineRef->routingType() == ConnType_PolyLine)
    {
        return euclideanDist(a, b);
    }
    else // Orthogonal
    {
        // XXX: This currently just takes into account the compulsory
        //      bend but will have to be updated when port direction 
        //      information is available.
        int num_penalties = 0;
        double xmove = b.x - a.x;
        double ymove = b.y - a.y;
        if (!last)
        {
            // Just two points.
            if ((xmove != 0) && (ymove != 0))
            {
                num_penalties += 1;
            }
        }
        else
        {
            // We have three points, so we know the direction of the 
            // previous segment.
            double rad = M_PI - angleBetween(*last, a, b);
            if (rad > (M_PI / 2))            
            {
                // Target point is back in the direction of the first point,
                // so at least two bends are required.
                num_penalties += 2;
            }
            else if (rad > 0)
            {
                // To the side, so at least one bend.
                num_penalties += 1;
            }
        }
        double penalty = num_penalties * 
                lineRef->router()->routingPenalty(segmentPenalty);

        return manhattanDist(a, b) + penalty;
    }
}


class CmpVisEdgeRotation 
{
    public:
        CmpVisEdgeRotation(const VertInf* lastPt)
            : _lastPt(lastPt)
        {
        }
        bool operator() (const EdgeInf* u, const EdgeInf* v) const 
        {
            return u->rotationLessThan(_lastPt, v);
        }
    private:
        const VertInf *_lastPt;
};


// Returns the best path from src to tar using the cost function.
//
// The path is worked out using the aStar algorithm, and is encoded via
// prevIndex values for each ANode which point back to the previous ANode's
// position in the DONE vector.  At completion, this order is written into
// the pathNext links in each of the VerInfs along the path.
//
// The aStar STL code is based on public domain code available on the
// internet.
//
static void aStarPath(ConnRef *lineRef, VertInf *src, VertInf *tar, 
        VertInf *start)
{
    bool isOrthogonal = (lineRef->routingType() == ConnType_Orthogonal);

    double (*dist)(const Point& a, const Point& b) = 
        (isOrthogonal) ? manhattanDist : euclideanDist;

    std::vector<ANode> PENDING;     // STL Vectors chosen because of rapid
    std::vector<ANode> DONE;        // insertions/deletions at back,
    ANode Node, BestNode;           // Temporary Node and BestNode
    bool bNodeFound = false;        // Flag if node is found in container
    int timestamp = 1;

    if (start == NULL)
    {
        start = src;
    }

    Router *router = lineRef->router();
    if (router->RubberBandRouting && (start != src))
    {
        COLA_ASSERT(router->IgnoreRegions == true);
        
        const PolyLine& currRoute = lineRef->route();
        VertInf *last = NULL;
        int rIndx = 0;
        while (last != start)
        {
            const Point& pnt = currRoute.at(rIndx);
            bool isShape = (rIndx > 0);
            VertID vID(pnt.id, isShape, pnt.vn);

#ifdef PATHDEBUG
            db_printf("/// %d %d %d\n", pnt.id, (int) isShape, pnt.vn);
#endif
            VertInf *curr = router->vertices.getVertexByID(vID);
            COLA_ASSERT(curr != NULL);

            Node = ANode(curr, timestamp++);
            if (!last)
            {
                Node.g = 0;
                Node.h = estimatedCost(lineRef, NULL, Node.inf->point, 
                        tar->point);
                Node.f = Node.g + Node.h;
            }
            else
            {
                double edgeDist = dist(BestNode.inf->point, curr->point);

                Node.g = BestNode.g + cost(lineRef, edgeDist, BestNode.inf, 
                        Node.inf, DONE, BestNode.prevIndex);

                // Calculate the Heuristic.
                Node.h = estimatedCost(lineRef, &(BestNode.inf->point),
                        Node.inf->point, tar->point);

                // The A* formula
                Node.f = Node.g + Node.h;
                
                // Point parent to last BestNode (pushed onto DONE)
                Node.prevIndex = DONE.size() - 1;
            }

            if (curr != start)
            {
                BestNode = Node;

                DONE.push_back(BestNode);
            }
            else
            {
                PENDING.push_back(Node);
            }

            rIndx++;
            last = curr;
        }
    }
    else
    {
        // Create the start node
        Node = ANode(src, timestamp++);
        Node.g = 0;
        Node.h = estimatedCost(lineRef, NULL, Node.inf->point, tar->point);
        Node.f = Node.g + Node.h;
        // Set a null parent, so cost function knows this is the first segment.

        // Populate the PENDING container with the first location
        PENDING.push_back(Node);
    }

    tar->pathNext = NULL;

    // Create a heap from PENDING for sorting
    using std::make_heap; using std::push_heap; using std::pop_heap;
    make_heap( PENDING.begin(), PENDING.end() );

    while (!PENDING.empty())
    {
        // Set the Node with lowest f value to BESTNODE.
        // Since the ANode operator< is reversed, the head of the
        // heap is the node with the lowest f value.
        BestNode = PENDING.front();

        // Pop off the heap.  Actually this moves the
        // far left value to the far right.  The node
        // is not actually removed since the pop is to
        // the heap and not the container.
        pop_heap(PENDING.begin(), PENDING.end());
        // Remove node from right (the value we pop_heap'd)
        PENDING.pop_back();

        // Push the BestNode onto DONE
        DONE.push_back(BestNode);

        VertInf *prevInf = (BestNode.prevIndex >= 0) ?
                DONE[BestNode.prevIndex].inf : NULL;
#if 0
        db_printf("Considering... ");
        db_printf(" %g %g  ", BestNode.inf->point.x, BestNode.inf->point.y);
        BestNode.inf->id.db_print();
        db_printf(" - g: %3.1f h: %3.1f back: ", BestNode.g, BestNode.h);
        if (prevInf)
        {
            db_printf(" %g %g", prevInf->point.x, prevInf->point.y);
            //prevInf->id.db_print();
        }
        db_printf("\n");
#endif

#if defined(ASTAR_DEBUG)
        if (router->avoid_screen)
        {
            int canx = 151;
            int cany = 55;
            int radius = 5;
            ANode curr;
            for (curr = BestNode; curr.prevIndex >= 0; 
                    curr = DONE[curr.prevIndex])
            {
                filledCircleRGBA(router->avoid_screen, 
                        (int) curr.inf->point.x + canx,
                        (int) curr.inf->point.y + cany, 
                        radius, 0, 0, 255, 128);
            }
            filledCircleRGBA(router->avoid_screen, 
                    (int) BestNode.inf->point.x + canx,
                    (int) BestNode.inf->point.y + cany, 
                    radius, 255, 0, 0, 255);

            SDL_Flip(router->avoid_screen);
            //SDL_Delay(500);

            filledCircleRGBA(router->avoid_screen, 
                    (int) BestNode.inf->point.x + canx,
                    (int) BestNode.inf->point.y + cany, 
                    radius, 255, 255, 255, 255);
            filledCircleRGBA(router->avoid_screen, 
                    (int) BestNode.inf->point.x + canx,
                    (int) BestNode.inf->point.y + cany, 
                    radius, 0, 255, 0, 128);
            for (curr = BestNode; curr.prevIndex >= 0; 
                    curr = DONE[curr.prevIndex])
            {
                filledCircleRGBA(router->avoid_screen, 
                        (int) curr.inf->point.x + canx,
                        (int) curr.inf->point.y + cany, 
                        radius, 255, 255, 255, 255);
                filledCircleRGBA(router->avoid_screen, 
                        (int) curr.inf->point.x + canx,
                        (int) curr.inf->point.y + cany, 
                        radius, 0, 255, 0, 128);
            }
        }
#endif

        // If at destination, break and create path below
        if (BestNode.inf == tar)
        {
#ifdef PATHDEBUG
            db_printf("Cost: %g\n", BestNode.f);
#endif
            //bPathFound = true; // arrived at destination...
            
            // Correct all the pathNext pointers.
            ANode curr;
            int currIndex = DONE.size() - 1;
            for (curr = BestNode; curr.prevIndex > 0; 
                    curr = DONE[curr.prevIndex])
            {
                COLA_ASSERT(curr.prevIndex < currIndex);   
                curr.inf->pathNext = DONE[curr.prevIndex].inf;
                currIndex = curr.prevIndex;
            }
            // Check that we've gone through the complete path.
            COLA_ASSERT(curr.prevIndex == 0);
            // Fill in the final pathNext pointer.
            curr.inf->pathNext = DONE[curr.prevIndex].inf;

            break;
        }

        // Check adjacent points in graph
        EdgeInfList& visList = (!isOrthogonal) ?
                BestNode.inf->visList : BestNode.inf->orthogVisList;
        if (isOrthogonal)
        {
            // We would like to explore in a structured way, 
            // so sort the points in the visList...
            CmpVisEdgeRotation compare(prevInf);
            visList.sort(compare);
        }
        EdgeInfList::const_iterator finish = visList.end();
        for (EdgeInfList::const_iterator edge = visList.begin(); 
                edge != finish; ++edge)
        {
            Node = ANode((*edge)->otherVert(BestNode.inf), timestamp++);

            // Set the index to the previous ANode that we reached
            // this ANode through (the last BestNode pushed onto DONE).
            Node.prevIndex = DONE.size() - 1;

            // Only check shape verticies, or the tar endpoint.
            if (!(Node.inf->id.isShape) && (Node.inf != tar))
            {
                continue;
            }

            VertInf *prevInf = (BestNode.prevIndex >= 0) ?
                    DONE[BestNode.prevIndex].inf : NULL;

            // Don't bother looking at the segment we just arrived along.
            if (prevInf && (prevInf == Node.inf))
            {
                continue;
            }

            double edgeDist = (*edge)->getDist();

            if (edgeDist == 0)
            {
                continue;
            }

            if (!router->_orthogonalRouting &&
                  (!router->RubberBandRouting || (start == src)) && 
                  (validateBendPoint(prevInf, BestNode.inf, Node.inf) == false))
            {
                // The bendpoint is not valid, i.e., is a zigzag corner, so...
                continue;
                // For RubberBand routing we want to allow these routes and
                // unwind them later, otherwise instead or unwinding, paths
                // can go the *really* long way round.
            }

            Node.g = BestNode.g + cost(lineRef, edgeDist, BestNode.inf, 
                    Node.inf, DONE, BestNode.prevIndex);

            // Calculate the Heuristic.
            Node.h = estimatedCost(lineRef, &(BestNode.inf->point),
                    Node.inf->point, tar->point);

            // The A* formula
            Node.f = Node.g + Node.h;

#if 0
            db_printf("-- Adding: %g %g  ", Node.inf->point.x, 
                    Node.inf->point.y);
            Node.inf->id.db_print();
            db_printf(" - g: %3.1f h: %3.1f \n", Node.g, Node.h);
#endif

            bNodeFound = false;

            // Check to see if already on PENDING
            for (unsigned int i = 0; i < PENDING.size(); i++)
            {
                ANode& ati = PENDING.at(i);
                if ((Node.inf == ati.inf) &&
                        (DONE[Node.prevIndex].inf == DONE[ati.prevIndex].inf))
                {
                    // If already on PENDING
                    if (Node.g < ati.g)
                    {
                        PENDING[i] = Node;

                        make_heap( PENDING.begin(), PENDING.end() );
                    }
                    bNodeFound = true;
                    break;
                }
            }
            if (!bNodeFound ) // If Node NOT found on PENDING
            {
                // Check to see if already on DONE
                for (unsigned int i = 0; i < DONE.size(); i++)
                {
                    ANode& ati = DONE.at(i);
                    if ((Node.inf == ati.inf) && 
                            (DONE[Node.prevIndex].inf == DONE[ati.prevIndex].inf))
                    {
                        // If on DONE, Which has lower gone?
                        if (Node.g < ati.g)
                        {
                            DONE[i] = Node;
                        }
                        bNodeFound = true;
                        break;
                    }
                }
            }

            if (!bNodeFound ) // If Node NOT found on PENDING or DONE
            {
                // Push NewNode onto PENDING
                PENDING.push_back(Node);
                // Push NewNode onto heap
                push_heap( PENDING.begin(), PENDING.end() );

#if 0
                using std::cout; using std::endl;
                // Display PENDING and DONE containers (For Debugging)
                cout << "PENDING:   ";
                for (unsigned int i = 0; i < PENDING.size(); i++)
                {
                    cout << PENDING.at(i).g << "," << PENDING.at(i).h << ",";
                    cout << PENDING.at(i).inf << "," << PENDING.at(i).pp << "  ";
                }
                cout << endl;
                cout << "DONE:   ";
                for (unsigned int i = 0; i < DONE.size(); i++)
                {
                    cout << DONE.at(i).g << "," << DONE.at(i).h << ",";
                    cout << DONE.at(i).inf << "," << DONE.at(i).pp << "  ";
                }
                cout << endl << endl;
#endif
            }
        }
    }
}


// Returns the best path for the connector referred to by lineRef.
//
// The path encoded in the pathNext links in each of the VertInfs
// backwards along the path, from the tar back to the source.
//
void makePath(ConnRef *lineRef, bool *flag)
{
    bool isOrthogonal = (lineRef->routingType() == ConnType_Orthogonal);
    Router *router = lineRef->router();
    VertInf *src = lineRef->src();
    VertInf *tar = lineRef->dst();
    VertInf *start = lineRef->start();

    // TODO: Could be more efficient here.
    if (isOrthogonal)
    {
        aStarPath(lineRef, src, tar, start);
    }
    else // if (!isOrthogonal)
    {
        EdgeInf *directEdge = EdgeInf::existingEdge(src, tar);
        // If the connector hates crossings or there are clusters present,
        // then we want to examine direct paths:
        bool examineDirectPath = lineRef->doesHateCrossings() || 
                !(router->clusterRefs.empty());
        
        if ((start == src) && directEdge && (directEdge->getDist() > 0) && 
                !examineDirectPath)
        {
            tar->pathNext = src;
            directEdge->addConn(flag);
        }
        else
        {
            aStarPath(lineRef, src, tar, start);
        }
    }

#if 0
    for (VertInf *t = vertices.connsBegin(); t != vertices.end();
            t = t->lstNext)
    {
        t->id.db_print();
        db_printf(" -> ");
        t->pathNext->id.db_print();
        db_printf("\n");
    }
#endif
}


}


