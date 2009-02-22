/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2004-2006  Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * --------------------------------------------------------------------
 * The dijkstraPath function is based on code published and described
 * in "Algorithms in C" (Second Edition), 1990, by Robert Sedgewick.
 * --------------------------------------------------------------------
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

#include "libavoid/vertices.h"
#include "libavoid/makepath.h"
#include "libavoid/geometry.h"
#include "libavoid/connector.h"
#include "libavoid/graph.h"
#include "libavoid/router.h"
#include <algorithm>
#include <vector>
#include <climits>
#include <limits.h>
#include <math.h>

namespace Avoid {


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
    Point v1(p1.x - p2.x, p1.y - p2.y);
    Point v2(p3.x - p2.x, p3.y - p2.y);

    return fabs(atan2(CrossLength(v1, v2), Dot(v1, v2)));
}


// Given the two points for a new segment of a path (inf2 & inf3)
// as well as the distance between these points (dist), as well as
// possibly the previous point (inf1) [from inf1--inf2], return a
// cost associated with this route.
//
double cost(ConnRef *lineRef, const double dist, VertInf *inf1,
        VertInf *inf2, VertInf *inf3)
{
    double result = dist;

    Router *router = inf2->_router;
    if (inf2->pathNext != NULL)
    {
        double& angle_penalty = router->angle_penalty;
        double& segmt_penalty = router->segmt_penalty;

        // This is not the first segment, so there is a bend
        // between it and the last one in the existing path.
        if ((angle_penalty > 0) || (segmt_penalty > 0))
        {
            Point p1 = inf1->point;
            Point p2 = inf2->point;
            Point p3 = inf3->point;

            double rad = M_PI - angleBetween(p1, p2, p3);

            // Make `xval' between 0--10 then take its log so small
            // angles are not penalised as much as large ones.
            //
            double xval = rad * 10 / M_PI;
            double yval = xval * log10(xval + 1) / 10.5;
            result += (angle_penalty * yval);
            //printf("deg from straight: %g\tpenalty: %g\n",
            //        rad * 180 / M_PI, (angle_penalty * yval));

            // Don't penalise as an extra segment if there is no turn.
            if (rad > 0.0005)
            {
                result += segmt_penalty;
            }
        }
    }

    if (lineRef->doesHateCrossings() && (router->crossing_penalty > 0))
    {
        Point& a1 = inf2->point;
        Point& a2 = inf3->point;

        ConnRefList::iterator curr, finish = router->connRefs.end();
        for (curr = router->connRefs.begin(); curr != finish; ++curr)
        {
            ConnRef *connRef = *curr;

            if (connRef->id() == lineRef->id())
            {
                continue;
            }
            Avoid::PolyLine& route2 = connRef->route();
            for (int j = 1; j < route2.pn; ++j)
            {
                Avoid::Point& b1 = route2.ps[j - 1];
                Avoid::Point& b2 = route2.ps[j];
            
                if (((a1 == b1) && (a2 == b2)) ||
                    ((a2 == b1) && (a1 == b2)))
                {
                    // Route along same segment: no penalty.  We detect
                    // crossovers when we see the segments diverge.
                    continue;
                }

                if ((a2 == b2) || (a2 == b1) || (b2 == a1))
                {
                    // Each crossing that is at a vertex in the 
                    // visibility graph gets noticed four times.
                    // We ignore three of these cases.
                    // This also catches the case of a shared path,
                    // but this is one that terminates at a common
                    // endpoint, so we don't care about it.
                    continue;
                }

                if (a1 == b1)
                {
                    if (j == 1)
                    {
                        // common source point.
                        continue;
                    }
                    Avoid::Point& b0 = route2.ps[j - 2];
                    // The segments share an endpoint -- a1==b1.
                    if (a2 == b0)
                    {
                        // a2 is not a split, continue.
                        continue;
                    }
                    
                    // If here, then we know that a2 != b2
                    // And a2 and its pair in b are a split.
                    assert(a2 != b2);

                    if (inf2->pathNext == NULL)
                    {
                        continue;
                    }
                    Avoid::Point& a0 = inf1->point;

                    if ((a0 == b0) || (a0 == b2))
                    {
                        //printf("Shared path... ");
                        bool normal = (a0 == b0) ? true : false;
                        // Determine direction we have to look through
                        // the points of connector b.
                        int dir = normal ? -1 : 1;
                        
                        int traceJ = j - 1 + dir;
                        
                        int endCornerSide = Avoid::cornerSide(
                                a0, a1, a2, normal ? b2 : b0);

                        
                        VertInf *traceInf1 = inf2->pathNext;
                        VertInf *traceInf2 = inf2;
                        VertInf *traceInf3 = inf3;
                        while (traceInf1 &&
                                (traceJ >= 0) && (traceJ < route2.pn) &&
                                (traceInf1->point == route2.ps[traceJ]))
                        {
                            traceInf3 = traceInf2;
                            traceInf2 = traceInf1;
                            traceInf1 = traceInf1->pathNext;
                            traceJ += dir;
                        }
                        
                        if (!traceInf1 ||
                                (traceJ < 0) || (traceJ >= route2.pn))
                        {
                            //printf("common source or destination.\n");
                            // The connectors have a shared path, but it
                            // comes from a common source point.
                            // XXX: There might be a better way to
                            //      check this by asking the connectors
                            //      for the IDs of the attached shapes.
                            continue;
                        }
                        
                        int startCornerSide = Avoid::cornerSide(
                                traceInf1->point, traceInf2->point,
                                traceInf3->point, route2.ps[traceJ]);
                        
                        if (endCornerSide != startCornerSide)
                        {
                            //printf("crosses.\n");
                            result += router->crossing_penalty;
                        }
                        else
                        {
                            //printf("doesn't cross.\n");
                        }
                    }
                    else
                    {
                        // The connectors cross or touch at this point.
                        //printf("Cross or touch at point... ");
                    
                        int side1 = Avoid::cornerSide(a0, a1, a2, b0);
                        int side2 = Avoid::cornerSide(a0, a1, a2, b2);

                        if (side1 != side2)
                        {
                            //printf("cross.\n");
                            // The connectors cross at this point.
                            result += router->crossing_penalty;
                        }
                        else
                        {
                            //printf("touch.\n");
                            // The connectors touch at this point.
                        }
                    }
                    continue;
                }

                double xc, yc;
                int intersectResult = Avoid::segmentIntersectPoint(
                        a1, a2, b1, b2, &xc, &yc);

                if (intersectResult == Avoid::DO_INTERSECT)
                {
                    result += router->crossing_penalty;
                }
            }
        }
    }
    
    return result;
}


// Returns the best path from src to tar using the cost function.
//
// The path is worked out via Dijkstra's algorithm, and is encoded via
// pathNext links in each of the VerInfs along the path.
//
// Based on the code of 'matrixpfs'.
//
static void dijkstraPath(ConnRef *lineRef, VertInf *src, VertInf *tar)
{
    Router *router = src->_router;

    double unseen = (double) __INT_MAX__;

    // initialize arrays
    VertInf *finish = router->vertices.end();
    for (VertInf *t = router->vertices.connsBegin(); t != finish; t = t->lstNext)
    {
        t->pathNext = NULL;
        t->pathDist = -unseen;
    }

    VertInf *min = src;
    while (min != tar)
    {
        VertInf *k = min;
        min = NULL;

        k->pathDist *= -1;
        if (k->pathDist == unseen)
        {
            k->pathDist = 0;
        }

        EdgeInfList& visList = k->visList;
        EdgeInfList::iterator finish = visList.end();
        for (EdgeInfList::iterator edge = visList.begin(); edge != finish;
                ++edge)
        {
            VertInf *t = (*edge)->otherVert(k);
            VertID tID = t->id;

            // Only check shape verticies, or endpoints.
            if ((t->pathDist < 0) &&
                    ((tID.objID == src->id.objID) || tID.isShape))
            {
                double kt_dist = (*edge)->getDist();
                double priority = k->pathDist +
                        cost(lineRef, kt_dist, k->pathNext, k, t);

                if ((kt_dist != 0) && (t->pathDist < -priority))
                {
                    t->pathDist = -priority;
                    t->pathNext = k;
                }
                if ((min == NULL) || (t->pathDist > min->pathDist))
                {
                    min = t;
                }
            }
        }
        EdgeInfList& invisList = k->invisList;
        finish = invisList.end();
        for (EdgeInfList::iterator edge = invisList.begin(); edge != finish;
                ++edge)
        {
            VertInf *t = (*edge)->otherVert(k);
            VertID tID = t->id;

            // Only check shape verticies, or endpoints.
            if ((t->pathDist < 0) &&
                    ((tID.objID == src->id.objID) || tID.isShape > 0))
            {
                if ((min == NULL) || (t->pathDist > min->pathDist))
                {
                    min = t;
                }
            }
        }
    }
}


class ANode
{
    public:
        VertInf* inf;
        double g;        // Gone
        double h;        // Heuristic
        double f;        // Formula f = g + h
        VertInf *pp;

        ANode(VertInf *vinf)
            : inf(vinf)
            , g(0)
            , h(0)
            , f(0)
            , pp(NULL)
        {
        }
        ANode()
            : inf(NULL)
            , g(0)
            , h(0)
            , f(0)
            , pp(NULL)
        {
        }
};

bool operator<(const ANode &a, const ANode &b)
{
    return a.f < b.f;
}


bool operator>(const ANode &a, const ANode &b)
{
    return a.f > b.f;
}


// Returns the best path from src to tar using the cost function.
//
// The path is worked out using the aStar algorithm, and is encoded via
// pathNext links in each of the VerInfs along the path.
//
// The aStar STL code is based on public domain code available on the
// internet.
//
static void aStarPath(ConnRef *lineRef, VertInf *src, VertInf *tar)
{
    std::vector<ANode> PENDING;     // STL Vectors chosen because of rapid
    std::vector<ANode> DONE;        // insertions/deletions at back,
    ANode Node, BestNode;           // Temporary Node and BestNode
    bool bNodeFound = false;        // Flag if node is found in container

    tar->pathNext = NULL;

    // Create the start node
    Node = ANode(src);
    Node.g = 0;
    Node.h = dist(Node.inf->point, tar->point);
    Node.f = Node.g + Node.h;
    // Set a null parent, so cost function knows this is the first segment.
    Node.pp = NULL;

    // Populate the PENDING container with the first location
    PENDING.push_back(Node);
    // Create a heap from PENDING for sorting
    using std::make_heap; using std::push_heap; using std::pop_heap;
    make_heap( PENDING.begin(), PENDING.end() );

    while (!PENDING.empty())
    {
        // Ascending sort based on overloaded operators below
        sort_heap(PENDING.begin(), PENDING.end());

        // Set the Node with lowest f value to BESTNODE
        BestNode = PENDING.front();

        // Pop off the heap.  Actually this moves the
        // far left value to the far right.  The node
        // is not actually removed since the pop is to
        // the heap and not the container.
        pop_heap(PENDING.begin(), PENDING.end());


        // Remove node from right (the value we pop_heap'd)
        PENDING.pop_back();

        // Push the BestNode onto DONE
        BestNode.inf->pathNext = BestNode.pp;
        DONE.push_back(BestNode);

#if 0
        printf("Considering... ");
        BestNode.ID->print(stdout);
        printf(" - g: %3.1f h: %3.1f f: %3.1f back: ", BestNode.g, BestNode.h,
                BestNode.f);
        BestNode.pp.print(stdout);
        printf("\n");
#endif

        // If at destination, break and create path below
        if (BestNode.inf == tar)
        {
            //bPathFound = true; // arrived at destination...
            break;
        }

        // Check adjacent points in graph
        EdgeInfList& visList = BestNode.inf->visList;
        EdgeInfList::iterator finish = visList.end();
        for (EdgeInfList::iterator edge = visList.begin(); edge != finish;
                ++edge)
        {
            Node.inf = (*edge)->otherVert(BestNode.inf);

            // Only check shape verticies, or the tar endpoint.
            if (!(Node.inf->id.isShape) && (Node.inf != tar))
            {
                continue;
            }

            double edgeDist = (*edge)->getDist();

            if (edgeDist == 0)
            {
                continue;
            }

            VertInf *prevInf = BestNode.inf->pathNext;

            Node.g = BestNode.g + cost(lineRef, edgeDist, prevInf,
                    BestNode.inf, Node.inf);

            // Calculate the Heuristic.
            Node.h = dist(Node.inf->point, tar->point);

            // The A* formula
            Node.f = Node.g + Node.h;
            // Point parent to last BestNode (pushed onto DONE)
            Node.pp = BestNode.inf;

            bNodeFound = false;

            // Check to see if already on PENDING
            for (unsigned int i = 0; i < PENDING.size(); i++)
            {
                if (Node.inf == PENDING.at(i).inf)
                {   // If already on PENDING
                    if (Node.g < PENDING.at(i).g)
                    {
                        PENDING.at(i).g = Node.g;
                        PENDING.at(i).f = Node.g + PENDING.at(i).h;
                        PENDING.at(i).pp = Node.pp;
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
                    if (Node.inf == DONE.at(i).inf)
                    {
                        // If on DONE, Which has lower gone?
                        if (Node.g < DONE.at(i).g)
                        {
                            DONE.at(i).g = Node.g;
                            DONE.at(i).f = Node.g + DONE.at(i).h;
                            DONE.at(i).pp = Node.pp;
                            DONE.at(i).inf->pathNext = Node.pp;
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
                // Re-Assert heap, or will be short by one
                make_heap( PENDING.begin(), PENDING.end() );

#if 0
                // Display PENDING and DONE containers (For Debugging)
                cout << "PENDING:   ";
                for (int i = 0; i < PENDING.size(); i++)
                {
                    cout << PENDING.at(i).x << "," << PENDING.at(i).y << ",";
                    cout << PENDING.at(i).g << "," << PENDING.at(i).h << "  ";
                }
                cout << endl;
                cout << "DONE:   ";
                for (int i = 0; i < DONE.size(); i++)
                {
                    cout << DONE.at(i).x << "," << DONE.at(i).y << ",";
                    cout << DONE.at(i).g << "," << DONE.at(i).h << "  ";
                }
                cout << endl << endl;
                int ch = _getch();
#endif
            }
        }
    }
}


// Returns the best path for the connector referred to by lineRef.
//
// The path encoded in the pathNext links in each of the VerInfs
// backwards along the path, from the tar back to the source.
//
void makePath(ConnRef *lineRef, bool *flag)
{
    Router *router = lineRef->router();
    VertInf *src = lineRef->src();
    VertInf *tar = lineRef->dst();

    // If the connector hates crossings then we want to examine direct paths:
    bool examineDirectPath = lineRef->doesHateCrossings();
    
    // TODO: Could be more efficient here.
    EdgeInf *directEdge = EdgeInf::existingEdge(src, tar);
    if (!(router->IncludeEndpoints) && directVis(src, tar))
    {
        Point p = src->point;
        Point q = tar->point;

        assert(directEdge == NULL);

        directEdge = new EdgeInf(src, tar);
        tar->pathNext = src;
        directEdge->setDist(dist(p, q));
        directEdge->addConn(flag);

        return;
    }
    else if (router->IncludeEndpoints && directEdge &&
            (directEdge->getDist() > 0) && !examineDirectPath)
    {
        tar->pathNext = src;
        directEdge->addConn(flag);
    }
    else
    {
        // Mark the path endpoints as not being able to see
        // each other.  This is true if we are here.
        if (!(router->IncludeEndpoints) && router->InvisibilityGrph)
        {
            if (!directEdge)
            {
                directEdge = new EdgeInf(src, tar);
            }
            directEdge->addBlocker(0);
        }

        if (router->UseAStarSearch)
        {
            aStarPath(lineRef, src, tar);
        }
        else
        {
            dijkstraPath(lineRef, src, tar);
        }

#if 0
        PointMap::iterator t;
        for (VertInf *t = vertices.connsBegin(); t != vertices.end();
                t = t->lstNext)
        {

            t->id.print();
            printf(" -> ");
            t->pathNext->id.print();
            printf("\n");
        }
#endif
    }
}


}


