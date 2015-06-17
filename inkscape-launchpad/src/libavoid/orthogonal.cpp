/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2009  Monash University
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


#include <cstdlib>
#include <cfloat>
#include <cmath>
#include <set>
#include <list>
#include <algorithm>

#include "libavoid/router.h"
#include "libavoid/geomtypes.h"
#include "libavoid/shape.h"
#include "libavoid/orthogonal.h"
#include "libavoid/connector.h"
#include "libavoid/vpsc.h"
#include "libavoid/assertions.h"

#ifdef LIBAVOID_SDL
  #include <SDL_gfxPrimitives.h>
#endif


namespace Avoid {


static const double CHANNEL_MAX = 100000000;

static const size_t XDIM = 0;
static const size_t YDIM = 1;


class ShiftSegment
{
    public:
        // For shiftable segments.
        ShiftSegment(ConnRef *conn, const size_t low, const size_t high,
                bool isSBend, const size_t dim, double minLim, double maxLim)
            : connRef(conn),
              indexLow(low),
              indexHigh(high),
              sBend(isSBend),
              fixed(false),
              dimension(dim),
              variable(NULL),
              minSpaceLimit(minLim),
              maxSpaceLimit(maxLim)
        {
        }

        // For fixed segments.
        ShiftSegment(ConnRef *conn, const size_t low, const size_t high,
                const size_t dim)
            : connRef(conn),
              indexLow(low),
              indexHigh(high),
              sBend(false),
              fixed(true),
              dimension(dim),
              variable(NULL)
        {
            // This has no space to shift.
            minSpaceLimit = lowPoint()[dim];
            maxSpaceLimit = lowPoint()[dim];
        }

        Point& lowPoint(void)
        {
            return connRef->displayRoute().ps[indexLow];
        }

        Point& highPoint(void)
        {
            return connRef->displayRoute().ps[indexHigh];
        }

        const Point& lowPoint(void) const
        {
            return connRef->displayRoute().ps[indexLow];
        }

        const Point& highPoint(void) const
        {
            return connRef->displayRoute().ps[indexHigh];
        }

        int fixedOrder(bool& isFixed) const
        {
            if (fixed)
            {
                isFixed = true;
                return 0;
            }
            if (lowC())
            {
                return 1;
            }
            else if (highC())
            {
                return -1;
            }
            return 0;
        }

        int order(void) const
        {
            if (lowC())
            {
                return -1;
            }
            else if (highC())
            {
                return 1;
            }
            return 0;
        }

        bool operator<(const ShiftSegment& rhs) const
        {
            const Point& lowPt = lowPoint();
            const Point& rhsLowPt = rhs.lowPoint();

            if (lowPt[dimension] != rhsLowPt[dimension])
            {
                return lowPt[dimension] < rhsLowPt[dimension];
            }
            return this < &rhs;
        }

        // This counts segments that are colliear and share an endpoint as
        // overlapping.  This allows them to be nudged apart where possible.
        bool overlapsWith(const ShiftSegment& rhs, const size_t dim) const
        {
            size_t altDim = (dim + 1) % 2;
            const Point& lowPt = lowPoint();
            const Point& highPt = highPoint();
            const Point& rhsLowPt = rhs.lowPoint();
            const Point& rhsHighPt = rhs.highPoint();
            if ( (lowPt[altDim] <= rhsHighPt[altDim]) &&
                    (rhsLowPt[altDim] <= highPt[altDim]))
            {
                if ( (minSpaceLimit <= rhs.maxSpaceLimit) &&
                        (rhs.minSpaceLimit <= maxSpaceLimit))
                {
                    return true;
                }
            }
            return false;
        }

        ConnRef *connRef;
        size_t indexLow;
        size_t indexHigh;
        bool sBend;
        bool fixed;
        size_t dimension;
        Variable *variable;
        double minSpaceLimit;
        double maxSpaceLimit;
    private:
        bool lowC(void) const
        {
            // This is true if this is a cBend and its adjoining points
            // are at lower positions.
            if (!sBend && !fixed && (minSpaceLimit == lowPoint()[dimension]))
            {
                return true;
            }
            return false;
        }

        bool highC(void) const
        {
            // This is true if this is a cBend and its adjoining points
            // are at higher positions.
            if (!sBend && !fixed && (maxSpaceLimit == lowPoint()[dimension]))
            {
                return true;
            }
            return false;
        }
};
typedef std::list<ShiftSegment> ShiftSegmentList;

struct Node;
struct CmpNodePos { bool operator()(const Node* u, const Node* v) const; };

typedef std::set<Node*,CmpNodePos> NodeSet;
struct Node
{
    ShapeRef *v;
    VertInf *c;
    ShiftSegment *ss;
    double pos;
    double min[2], max[2];
    Node *firstAbove, *firstBelow;
    NodeSet::iterator iter;

    Node(ShapeRef *v, const double p)
        : v(v),
          c(NULL),
          ss(NULL),
          pos(p),
          firstAbove(NULL),
          firstBelow(NULL)
    {
        //COLA_ASSERT(r->width()<1e40);
        v->polygon().getBoundingRect(&min[0], &min[1], &max[0], &max[1]);
    }
    Node(VertInf *c, const double p)
        : v(NULL),
          c(c),
          ss(NULL),
          pos(p),
          firstAbove(NULL),
          firstBelow(NULL)
    {
        min[0] = max[0] = c->point.x;
        min[1] = max[1] = c->point.y;
    }
    Node(ShiftSegment *ss, const double p)
        : v(NULL),
          c(NULL),
          ss(ss),
          pos(p),
          firstAbove(NULL),
          firstBelow(NULL)
    {
        // These values shouldn't ever be used, so they don't matter.
        min[0] = max[0] = min[1] = max[1] = 0;
    }
    ~Node()
    {
    }
    // Find the first Node above in the scanline that is a shape edge,
    // and does not have an open or close event at this position (meaning
    // it is just about to be removed).
    double firstObstacleAbove(size_t dim)
    {
        Node *curr = firstAbove;
        while (curr && (curr->ss || (curr->max[dim] > pos)))
        {
            curr = curr->firstAbove;
        }

        if (curr)
        {
            return curr->max[dim];
        }
        return -DBL_MAX;
    }
    // Find the first Node below in the scanline that is a shape edge,
    // and does not have an open or close event at this position (meaning
    // it is just about to be removed).
    double firstObstacleBelow(size_t dim)
    {
        Node *curr = firstBelow;
        while (curr && (curr->ss || (curr->min[dim] < pos)))
        {
            curr = curr->firstBelow;
        }

        if (curr)
        {
            return curr->min[dim];
        }
        return DBL_MAX;
    }
    // Mark all connector segments above in the scanline as being able
    // to see to this shape edge.
    void markShiftSegmentsAbove(size_t dim)
    {
        Node *curr = firstAbove;
        while (curr && (curr->ss || (curr->pos > min[dim])))
        {
            if (curr->ss && (curr->pos <= min[dim]))
            {
                curr->ss->maxSpaceLimit =
                        std::min(min[dim], curr->ss->maxSpaceLimit);
            }
            curr = curr->firstAbove;
        }
    }
    // Mark all connector segments below in the scanline as being able
    // to see to this shape edge.
    void markShiftSegmentsBelow(size_t dim)
    {
        Node *curr = firstBelow;
        while (curr && (curr->ss || (curr->pos < max[dim])))
        {
            if (curr->ss && (curr->pos >= max[dim]))
            {
                curr->ss->minSpaceLimit =
                        std::max(max[dim], curr->ss->minSpaceLimit);
            }
            curr = curr->firstBelow;
        }
    }
    bool findFirstPointAboveAndBelow(const size_t dim, double& firstAbovePos,
            double& firstBelowPos, double& lastAbovePos, double& lastBelowPos)
    {
        bool clearVisibility = true;
        firstAbovePos = -DBL_MAX;
        firstBelowPos = DBL_MAX;
        // We start looking left from the right side of the shape,
        // and vice versa.
        lastAbovePos = max[dim];
        lastBelowPos = min[dim];

        // Find the first blocking edge above this point.  Don't count the
        // edges as we are travelling out of shapes we are inside, but then
        // mark clearVisibility as false.
        Node *curr = firstAbove;
        while (curr && (curr->max[dim] > min[dim]))
        {
            lastAbovePos = std::min(curr->min[dim], lastAbovePos);
            if ((curr->max[dim] >= min[dim]) && (curr->max[dim] <= max[dim]))
            {
                lastAbovePos = std::min(curr->max[dim], lastAbovePos);
            }
            lastBelowPos = std::max(curr->max[dim], lastBelowPos);
            clearVisibility = false;
            curr = curr->firstAbove;
        }
        if (curr)
        {
            firstAbovePos = curr->max[dim];
        }
        while (curr)
        {
            // There might be a larger shape after this one in the ordering.
            if (curr->max[dim] < min[dim])
            {
                firstAbovePos = std::max(curr->max[dim], firstAbovePos);
            }
            curr = curr->firstAbove;
        }

        // Find the first blocking edge below this point.  Don't count the
        // edges as we are travelling out of shapes we are inside, but then
        // mark clearVisibility as false.
        curr = firstBelow;
        while (curr && (curr->min[dim] < max[dim]))
        {
            lastBelowPos = std::max(curr->max[dim], lastBelowPos);
            if ((curr->min[dim] >= min[dim]) && (curr->min[dim] <= max[dim]))
            {
                lastBelowPos = std::max(curr->min[dim], lastBelowPos);
            }
            lastAbovePos = std::min(curr->min[dim], lastAbovePos);
            clearVisibility = false;
            curr = curr->firstBelow;
        }
        if (curr)
        {
            firstBelowPos = curr->min[dim];
        }
        while (curr)
        {
            // There might be a larger shape after this one in the ordering.
            if (curr->min[dim] > max[dim])
            {
                firstBelowPos = std::min(curr->min[dim], firstBelowPos);
            }
            curr = curr->firstBelow;
        }

        return clearVisibility;
    }
    double firstPointAbove(size_t dim)
    {
        Node *curr = firstAbove;
        while (curr && (curr->max[dim] >= pos))
        {
            curr = curr->firstAbove;
        }

        if (curr)
        {
            return curr->max[dim];
        }
        return -DBL_MAX;
    }
    double firstPointBelow(size_t dim)
    {
        Node *curr = firstBelow;
        while (curr && (curr->min[dim] <= pos))
        {
            curr = curr->firstBelow;
        }

        if (curr)
        {
            return curr->min[dim];
        }
        return DBL_MAX;
    }
    // This is a bit inefficient, but we won't need to do it once we have
    // connection points.
    bool isInsideShape(size_t dimension)
    {
        for (Node *curr = firstBelow; curr; curr = curr->firstBelow)
        {
            if ((curr->min[dimension] < pos) && (pos < curr->max[dimension]))
            {
                return true;
            }
        }
        for (Node *curr = firstAbove; curr; curr = curr->firstAbove)
        {
            if ((curr->min[dimension] < pos) && (pos < curr->max[dimension]))
            {
                return true;
            }
        }
        return false;
    }
};


bool CmpNodePos::operator() (const Node* u, const Node* v) const
{
    if (u->pos != v->pos)
    {
        return u->pos < v->pos;
    }

    // Use the pointers to the base objects to differentiate them.
    void *up = (u->v) ? (void *) u->v :
            ((u->c) ? (void *) u->c : (void *) u->ss);
    void *vp = (v->v) ? (void *) v->v :
            ((v->c) ? (void *) v->c : (void *) v->ss);
    return up < vp;
}


// Note: Open must come first.
typedef enum {
    Open = 1,
    SegOpen = 2,
    ConnPoint = 3,
    SegClose = 4,
    Close = 5
} EventType;


struct Event
{
    Event(EventType t, Node *v, double p)
        : type(t),
          v(v),
          pos(p)
    {};
    EventType type;
    Node *v;
    double pos;
};

Event **events;


// Used for quicksort.  Must return <0, 0, or >0.
static int compare_events(const void *a, const void *b)
{
	Event *ea = *(Event**) a;
	Event *eb = *(Event**) b;
    if (ea->pos != eb->pos)
    {
        return (ea->pos < eb->pos) ? -1 : 1;
    }
    if (ea->type != eb->type)
    {
        return ea->type - eb->type;
    }
    COLA_ASSERT(ea->v != eb->v);
    return ea->v - eb->v;
}


// Returns a bitfield of the direction of visibility (in this dimension)
// made up of ConnDirDown (for visibility towards lower position values)
// and ConnDirUp (for visibility towards higher position values).
//
static ConnDirFlags getPosVertInfDirection(VertInf *v, size_t dim)
{
    if (dim == XDIM) // X-dimension
    {
        unsigned int dirs = v->visDirections & (ConnDirLeft | ConnDirRight);
        if (dirs == (ConnDirLeft | ConnDirRight))
        {
            return (ConnDirDown | ConnDirUp);
        }
        else if (dirs == ConnDirLeft)
        {
            return ConnDirDown;
        }
        else if (dirs == ConnDirRight)
        {
            return ConnDirUp;
        }
    }
    else if (dim == YDIM) // Y-dimension
    {
        unsigned int dirs = v->visDirections & (ConnDirDown | ConnDirUp);
        if (dirs == (ConnDirDown | ConnDirUp))
        {
            return (ConnDirDown | ConnDirUp);
        }
        else if (dirs == ConnDirDown)
        {
            // For libavoid the Y-axis points downwards, so in terms of
            // smaller or larger position values, Down is Up and vice versa.
            return ConnDirUp;
        }
        else if (dirs == ConnDirUp)
        {
            // For libavoid the Y-axis points downwards, so in terms of
            // smaller or larger position values, Down is Up and vice versa.
            return ConnDirDown;
        }
    }

    // Can occur for ConnDirNone visibility.
    return ConnDirNone;
}


struct PosVertInf
{
    PosVertInf(double p, VertInf *vI, ConnDirFlags d = ConnDirNone)
        : pos(p),
          vert(vI),
          dir(d)
    {
    }

    bool operator<(const PosVertInf& rhs) const
    {
        if (pos != rhs.pos)
        {
            return pos < rhs.pos;
        }
        return vert < rhs.vert;
    }

    double pos;
    VertInf *vert;

    // A bitfield marking the direction of visibility (in this dimension)
    // made up of ConnDirDown (for visibility towards lower position values)
    // and ConnDirUp (for visibility towards higher position values).
    //
    ConnDirFlags dir;
};


struct CmpVertInf
{
    bool operator()(const VertInf* u, const VertInf* v) const
    {
        // Comparator for VertSet, an ordered set of VertInf pointers.
        // It is assumed vertical sets of points will all have the same
        // x position and horizontal sets all share a y position, so this
        // method can be used to sort both these sets.
        COLA_ASSERT((u->point.x == v->point.x) || (u->point.y == v->point.y));
        if (u->point.x != v->point.x)
        {
            return u->point.x < v->point.x;
        }
        else if (u->point.y != v->point.y)
        {
            return u->point.y < v->point.y;
        }
        return u < v;
    }
};


typedef std::set<VertInf *, CmpVertInf> VertSet;

// A set of points to break the line segment,
// along with vertices for these points.
typedef std::set<PosVertInf> BreakpointSet;

// Temporary structure used to store the possible horizontal visibility
// lines arising from the vertical sweep.
class LineSegment
{
public:
    LineSegment(const double& b, const double& f, const double& p,
                bool /*ss*/ = false, VertInf *bvi = NULL, VertInf *fvi = NULL)
        : begin(b),
          finish(f),
          pos(p),
          shapeSide(false)
    {
        COLA_ASSERT(begin < finish);

        if (bvi)
        {
            vertInfs.insert(bvi);
        }
        if (fvi)
        {
            vertInfs.insert(fvi);
        }
    }

    LineSegment(const double& bf, const double& p, VertInf *bfvi = NULL)
        : begin(bf),
          finish(bf),
          pos(p),
          shapeSide(false)
    {
        if (bfvi)
        {
            vertInfs.insert(bfvi);
        }
    }

    // Order by begin, pos, finish.
    bool operator<(const LineSegment& rhs) const
    {
        if (begin != rhs.begin)
        {
            return begin < rhs.begin;
        }
        if (pos != rhs.pos)
        {
            return pos < rhs.pos;
        }
        if (finish != rhs.finish)
        {
            return finish < rhs.finish;
        }
        COLA_ASSERT(shapeSide == rhs.shapeSide);
        return false;
    }

    bool overlaps(const LineSegment& rhs) const
    {
        if ((begin == rhs.begin) && (pos == rhs.pos) &&
                (finish == rhs.finish))
        {
            // Lines are exactly equal.
            return true;
        }

        if (pos == rhs.pos)
        {
            if (((begin >= rhs.begin) && (begin <= rhs.finish)) ||
                ((rhs.begin >= begin) && (rhs.begin <= finish)) )
            {
                // They are colinear and overlap by some amount.
                return true;
            }
        }
        return false;
    }

    void mergeVertInfs(const LineSegment& segment)
    {
        begin = std::min(begin, segment.begin);
        finish = std::max(finish, segment.finish);
        vertInfs.insert(segment.vertInfs.begin(), segment.vertInfs.end());
    }

    VertInf *beginVertInf(void) const
    {
        if (vertInfs.empty())
        {
            return NULL;
        }
        return *vertInfs.begin();
    }
    VertInf *finishVertInf(void) const
    {
        if (vertInfs.empty())
        {
            return NULL;
        }
        return *vertInfs.rbegin();
    }

    VertInf *commitPositionX(Router *router, double posX)
    {
        VertInf *found = NULL;
        for (VertSet::iterator v = vertInfs.begin();
                v != vertInfs.end(); ++v)
        {
            if ((*v)->point.x == posX)
            {
                found = *v;
                break;
            }
        }
        if (!found)
        {
            found = new VertInf(router, dummyOrthogID, Point(posX, pos));
            vertInfs.insert(found);
        }
        return found;
    }
    // Set begin endpoint vertex if none has been assigned.
    void commitBegin(Router *router, VertInf *vert = NULL)
    {
        if (vert)
        {
            vertInfs.insert(vert);
        }

        if (vertInfs.empty() ||
                ((*vertInfs.begin())->point.x != begin))
        {
            vertInfs.insert(new
                    VertInf(router, dummyOrthogID, Point(begin, pos)));
        }
    }

    // Set begin endpoint vertex if none has been assigned.
    void commitFinish(Router *router, VertInf *vert = NULL)
    {
        if (vert)
        {
            vertInfs.insert(vert);
        }

        if (vertInfs.empty() ||
                ((*vertInfs.rbegin())->point.x != finish))
        {
            vertInfs.insert(new
                    VertInf(router, dummyOrthogID, Point(finish, pos)));
        }
    }

    // Converts a section of the points list to a set of breakPoints.
    // Returns the first of the intersection points occuring at finishPos.
    VertSet::iterator addSegmentsUpTo(Router */*router*/, double finishPos)
    {
        VertSet::iterator firstIntersectionPt = vertInfs.end();
        for (VertSet::iterator vert = vertInfs.begin();
                vert != vertInfs.end(); ++vert)
        {
            if ((*vert)->point.x > finishPos)
            {
                // We're done.
                break;
            }

            breakPoints.insert(PosVertInf((*vert)->point.x, (*vert),
                        getPosVertInfDirection(*vert, XDIM)));

            if ((firstIntersectionPt == vertInfs.end()) &&
                    ((*vert)->point.x == finishPos))
            {
                firstIntersectionPt = vert;
            }
        }
        // Returns the first of the intersection points at finishPos.
        return firstIntersectionPt;
    }

    // Add visibility edge(s) for this segment.  There may be multiple if
    // one of the endpoints is shared by multiple connector endpoints.
    void addEdgeHorizontal(Router *router)
    {
        commitBegin(router);
        commitFinish(router);

        addSegmentsUpTo(router, finish);
    }

    // Add visibility edge(s) for this segment up until an intersection.
    // Then, move the segment beginning to the intersection point, so we
    // later only consider the remainder of the segment.
    // There may be multiple segments added to the graph if the beginning
    // endpoint of the segment is shared by multiple connector endpoints.
    VertSet addEdgeHorizontalTillIntersection(Router *router,
            LineSegment& vertLine)
    {
        VertSet intersectionSet;

        commitBegin(router);

        // Does a vertex already exist for this point.
        commitPositionX(router, vertLine.pos);

        // Generate segments and set end iterator to the first point
        // at the intersection position.
        VertSet::iterator restBegin = addSegmentsUpTo(router, vertLine.pos);

        // Add the intersections points to intersectionSet.
        VertSet::iterator restEnd = restBegin;
        while ((restEnd != vertInfs.end()) &&
                (*restEnd)->point.x == vertLine.pos)
        {
            ++restEnd;
        }
        intersectionSet.insert(restBegin, restEnd);

        // Adjust segment to remove processed portion.
        begin = vertLine.pos;
        vertInfs.erase(vertInfs.begin(), restBegin);

        return intersectionSet;
    }

    // Insert vertical breakpoints.
    void insertBreakpointsBegin(Router *router, LineSegment& vertLine)
    {
        VertInf *vert = NULL;
        if (pos == vertLine.begin && vertLine.beginVertInf())
        {
            vert = vertLine.beginVertInf();
        }
        else if (pos == vertLine.finish && vertLine.finishVertInf())
        {
            vert = vertLine.finishVertInf();
        }
        commitBegin(router, vert);

        for (VertSet::iterator v = vertInfs.begin();
                v != vertInfs.end(); ++v)
        {
            if ((*v)->point.x == begin)
            {
                vertLine.breakPoints.insert(PosVertInf(pos, *v,
                        getPosVertInfDirection(*v, YDIM)));
            }
        }
    }

    // Insert vertical breakpoints.
    void insertBreakpointsFinish(Router *router, LineSegment& vertLine)
    {
        VertInf *vert = NULL;
        if (pos == vertLine.begin && vertLine.beginVertInf())
        {
            vert = vertLine.beginVertInf();
        }
        else if (pos == vertLine.finish && vertLine.finishVertInf())
        {
            vert = vertLine.finishVertInf();
        }
        commitFinish(router, vert);

        for (VertSet::iterator v = vertInfs.begin();
                v != vertInfs.end(); ++v)
        {
            if ((*v)->point.x == finish)
            {
                vertLine.breakPoints.insert(PosVertInf(pos, *v,
                        getPosVertInfDirection(*v, YDIM)));
            }
        }
    }
    void generateVisibilityEdgesFromBreakpointSet(Router *router, size_t dim)
    {
        if ((breakPoints.begin())->pos != begin)
        {
            if (!beginVertInf())
            {
                Point point(pos, pos);
                point[dim] = begin;
                // Add begin point if it didn't intersect another line.
                VertInf *vert = new VertInf(router, dummyOrthogID, point);
                breakPoints.insert(PosVertInf(begin, vert));
            }
        }
        if ((breakPoints.rbegin())->pos != finish)
        {
            if (!finishVertInf())
            {
                Point point(pos, pos);
                point[dim] = finish;
                // Add finish point if it didn't intersect another line.
                VertInf *vert = new VertInf(router, dummyOrthogID, point);
                breakPoints.insert(PosVertInf(finish, vert));
            }
        }

        const bool orthogonal = true;
        BreakpointSet::iterator vert, last;
        for (vert = last = breakPoints.begin(); vert != breakPoints.end();)
        {
            BreakpointSet::iterator firstPrev = last;
            while (last->vert->point[dim] != vert->vert->point[dim])
            {
                COLA_ASSERT(vert != last);
                // Assert points are not at the same position.
                COLA_ASSERT(vert->vert->point != last->vert->point);

                if ( !(vert->vert->id.isShape || last->vert->id.isShape))
                {
                    // Here we have a pair of two endpoints that are both
                    // connector endpoints and both are inside a shape.

                    // Give vert visibility back to the first non-connector
                    // endpoint vertex (i.e., the side of the shape).
                    BreakpointSet::iterator side = last;
                    while (!side->vert->id.isShape)
                    {
                        if (side == breakPoints.begin())
                        {
                            break;
                        }
                        --side;
                    }
                    bool canSeeDown = (vert->dir & ConnDirDown);
                    if (canSeeDown && side->vert->id.isShape)
                    {
                        EdgeInf *edge = new
                                EdgeInf(side->vert, vert->vert, orthogonal);
                        edge->setDist(vert->vert->point[dim] -
                                side->vert->point[dim]);
                    }

                    // Give last visibility back to the first non-connector
                    // endpoint vertex (i.e., the side of the shape).
                    side = vert;
                    while ((side != breakPoints.end()) &&
                            !side->vert->id.isShape)
                    {
                        ++side;
                    }
                    bool canSeeUp = (last->dir & ConnDirUp);
                    if (canSeeUp && (side != breakPoints.end()))
                    {
                        EdgeInf *edge = new
                                EdgeInf(last->vert, side->vert, orthogonal);
                        edge->setDist(side->vert->point[dim] -
                                last->vert->point[dim]);
                    }
                }

                // The normal case.
                //
                // Note: It's okay to give two connector endpoints visbility
                // here since we only consider the partner endpoint as a
                // candidate while searching if it is the other endpoint of
                // the connector in question.
                //
                bool generateEdge = true;
                if (!last->vert->id.isShape && !(last->dir & ConnDirUp))
                {
                    generateEdge = false;
                }
                else if (!vert->vert->id.isShape && !(vert->dir & ConnDirDown))
                {
                    generateEdge = false;
                }
                if (generateEdge)
                {
                    EdgeInf *edge =
                            new EdgeInf(last->vert, vert->vert, orthogonal);
                    edge->setDist(vert->vert->point[dim] -
                            last->vert->point[dim]);
                }

                ++last;
            }

            ++vert;

            if ((vert != breakPoints.end()) &&
                    (last->vert->point[dim] == vert->vert->point[dim]))
            {
                // Still looking at same pair, just reset prev number pointer.
                last = firstPrev;
            }
            else
            {
                // vert has moved to the beginning of a number number group.
                // Last is now in the right place, so do nothing.
            }
        }
    }

    double begin;
    double finish;
    double pos;
    bool shapeSide;

    VertSet vertInfs;
    BreakpointSet breakPoints;
private:
	// MSVC wants to generate the assignment operator and the default
	// constructor, but fails.  Therefore we declare them private and
	// don't implement them.
    LineSegment & operator=(LineSegment const &);
    LineSegment();
};

typedef std::list<LineSegment> SegmentList;

class SegmentListWrapper
{
    public:
        LineSegment *insert(LineSegment segment)
        {
            SegmentList::iterator found = _list.end();
            for (SegmentList::iterator curr = _list.begin();
                    curr != _list.end(); ++curr)
            {
                if (curr->overlaps(segment))
                {
                    if (found != _list.end())
                    {
                        // This is not the first segment that overlaps,
                        // so we need to merge and then delete an existing
                        // segment.
                        curr->mergeVertInfs(*found);
                        _list.erase(found);
                        found = curr;
                    }
                    else
                    {
                        // This is the first overlapping segment, so just
                        // merge the new segment with this one.
                        curr->mergeVertInfs(segment);
                        found = curr;
                    }
                }
            }

            if (found == _list.end())
            {
                // Add this line.
                _list.push_back(segment);
                return &(_list.back());
            }

            return &(*found);
        }
        SegmentList& list(void)
        {
            return _list;
        }
    private:
        SegmentList _list;
};


// Given a router instance and a set of possible horizontal segments, and a
// possible vertical visibility segment, compute and add edges to the
// orthogonal visibility graph for all the visibility edges.
static void intersectSegments(Router *router, SegmentList& segments,
        LineSegment& vertLine)
{
    COLA_ASSERT(vertLine.beginVertInf() == NULL);
    COLA_ASSERT(vertLine.finishVertInf() == NULL);
    for (SegmentList::iterator it = segments.begin(); it != segments.end(); )
    {
        LineSegment& horiLine = *it;

        bool inVertSegRegion = ((vertLine.begin <= horiLine.pos) &&
                                (vertLine.finish >= horiLine.pos));

        if (horiLine.finish < vertLine.pos)
        {
            // Add horizontal visibility segment.
            horiLine.addEdgeHorizontal(router);

            size_t dim = XDIM; // x-dimension
            horiLine.generateVisibilityEdgesFromBreakpointSet(router, dim);

            // We've now swept past this horizontal segment, so delete.
            it = segments.erase(it);
            continue;
        }
        else if (horiLine.begin > vertLine.pos)
        {
            // We've yet to reach this segment in the sweep, so ignore.
            ++it;
            continue;
        }
        else if (horiLine.begin == vertLine.pos)
        {
            if (inVertSegRegion)
            {
                horiLine.insertBreakpointsBegin(router, vertLine);
            }
        }
        else if (horiLine.finish == vertLine.pos)
        {
            if (inVertSegRegion)
            {
                // Add horizontal visibility segment.
                horiLine.addEdgeHorizontal(router);

                horiLine.insertBreakpointsFinish(router, vertLine);

                size_t dim = XDIM; // x-dimension
                horiLine.generateVisibilityEdgesFromBreakpointSet(router, dim);

                // And we've now finished with the segment, so delete.
                it = segments.erase(it);
                continue;
            }
        }
        else
        {
            COLA_ASSERT(horiLine.begin < vertLine.pos);
            COLA_ASSERT(horiLine.finish > vertLine.pos);

            if (inVertSegRegion)
            {
                // Add horizontal visibility segment.
                VertSet intersectionVerts =
                        horiLine.addEdgeHorizontalTillIntersection(
                            router, vertLine);

                for (VertSet::iterator v = intersectionVerts.begin();
                        v != intersectionVerts.end(); ++v)
                {
                    vertLine.breakPoints.insert(PosVertInf(horiLine.pos, *v,
                            getPosVertInfDirection(*v, YDIM)));
                }
            }
        }
        ++it;
    }

    // Split breakPoints set into visibility segments.
    size_t dimension = YDIM; // y-dimension
    vertLine.generateVisibilityEdgesFromBreakpointSet(router, dimension);
}


// Processes an event for the vertical sweep used for computing the static
// orthogonal visibility graph.  This adds possible visibility sgments to
// the segments list.
// The first pass is adding the event to the scanline, the second is for
// processing the event and the third for removing it from the scanline.
static void processEventVert(Router *router, NodeSet& scanline,
        SegmentListWrapper& segments, Event *e, unsigned int pass)
{
    Node *v = e->v;

    if ( ((pass == 1) && (e->type == Open)) ||
         ((pass == 2) && (e->type == ConnPoint)) )
    {
        std::pair<NodeSet::iterator, bool> result = scanline.insert(v);
        v->iter = result.first;
        COLA_ASSERT(result.second);

        NodeSet::iterator it = v->iter;
        // Work out neighbours
        if (it != scanline.begin())
        {
            Node *u = *(--it);
            v->firstAbove = u;
            u->firstBelow = v;
        }
        it = v->iter;
        if (++it != scanline.end())
        {
            Node *u = *it;
            v->firstBelow = u;
            u->firstAbove = v;
        }
    }

    if (pass == 2)
    {
        if ((e->type == Open) || (e->type == Close))
        {
            // Shape edge positions.
            double minShape = v->min[0];
            double maxShape = v->max[0];
            // As far as we can see.
            double minLimit, maxLimit;
            double minLimitMax, maxLimitMin;
            v->findFirstPointAboveAndBelow(0, minLimit, maxLimit,
                    minLimitMax, maxLimitMin);

            // Only difference between Open and Close is whether the line
            // segments are at the top or bottom of the shape.  Decide here.
            double lineY = (e->type == Open) ? v->min[1] : v->max[1];

            if (minLimitMax >= maxLimitMin)
            {
                // Insert possible visibility segments.
                VertInf *vI1 = new VertInf(router, dummyOrthogID,
                            Point(minShape, lineY));
                VertInf *vI2 = new VertInf(router, dummyOrthogID,
                            Point(maxShape, lineY));

                // There are no overlapping shapes, so give full visibility.
                if (minLimit < minShape)
                {
                    segments.insert(LineSegment(minLimit, minShape, lineY,
                                true, NULL, vI1));
                }
                segments.insert(LineSegment(minShape, maxShape, lineY,
                            true, vI1, vI2));
                if (maxShape < maxLimit)
                {
                    segments.insert(LineSegment(maxShape, maxLimit, lineY,
                                true, vI2, NULL));
                }
            }
            else
            {
                if ((minLimitMax > minLimit) && (minLimitMax >= minShape))
                {
                    segments.insert(LineSegment(minLimit, minLimitMax, lineY,
                                true, NULL, NULL));
                }
                if ((maxLimitMin < maxLimit) && (maxLimitMin <= maxShape))
                {
                    segments.insert(LineSegment(maxLimitMin, maxLimit, lineY,
                                true, NULL, NULL));
                }
            }
        }
        else if (e->type == ConnPoint)
        {
            // Connection point.
            VertInf *centreVert = e->v->c;
            Point& cp = centreVert->point;

            // As far as we can see.
            double minLimit = v->firstPointAbove(0);
            double maxLimit = v->firstPointBelow(0);
            bool inShape = v->isInsideShape(0);

            LineSegment *line1 = NULL, *line2 = NULL;
            if (!inShape || (centreVert->visDirections & ConnDirLeft))
            {
                line1 = segments.insert(LineSegment(minLimit, cp.x, e->pos,
                        true, NULL, centreVert));
            }
            if (!inShape || (centreVert->visDirections & ConnDirRight))
            {
                line2 = segments.insert(LineSegment(cp.x, maxLimit, e->pos,
                        true, centreVert, NULL));
            }
            if (!line1 && !line2)
            {
                // Add a point segment for the centre point.
                segments.insert(LineSegment(cp.x, e->pos, centreVert));
            }

            if (!inShape)
            {
                // This is not contained within a shape so add a normal
                // visibility graph point here too (since paths won't route
                // *through* connector endpoint vertices).
                if (line1 || line2)
                {
                    VertInf *cent = new VertInf(router, dummyOrthogID, cp);
                    if (line1)
                    {
                        line1->vertInfs.insert(cent);
                    }
                    if (line2)
                    {
                        line2->vertInfs.insert(cent);
                    }
                }
            }
        }
    }

    if ( ((pass == 3) && (e->type == Close)) ||
         ((pass == 2) && (e->type == ConnPoint)) )
    {
        // Clean up neighbour pointers.
        Node *l = v->firstAbove, *r = v->firstBelow;
        if (l != NULL)
        {
            l->firstBelow = v->firstBelow;
        }
        if (r != NULL)
        {
            r->firstAbove = v->firstAbove;
        }

        if (e->type == ConnPoint)
        {
            scanline.erase(v->iter);
            delete v;
        }
        else  // if (e->type == Close)
        {
            size_t result;
            result = scanline.erase(v);
            COLA_ASSERT(result == 1);
            delete v;
        }
    }
}


// Processes an event for the vertical sweep used for computing the static
// orthogonal visibility graph.  This adds possible visibility sgments to
// the segments list.
// The first pass is adding the event to the scanline, the second is for
// processing the event and the third for removing it from the scanline.
static void processEventHori(Router */*router*/, NodeSet& scanline,
        SegmentListWrapper& segments, Event *e, unsigned int pass)
{
    Node *v = e->v;

    if ( ((pass == 1) && (e->type == Open)) ||
         ((pass == 2) && (e->type == ConnPoint)) )
    {
        std::pair<NodeSet::iterator, bool> result = scanline.insert(v);
        v->iter = result.first;
        COLA_ASSERT(result.second);

        NodeSet::iterator it = v->iter;
        // Work out neighbours
        if (it != scanline.begin())
        {
            Node *u = *(--it);
            v->firstAbove = u;
            u->firstBelow = v;
        }
        it = v->iter;
        if (++it != scanline.end())
        {
            Node *u = *it;
            v->firstBelow = u;
            u->firstAbove = v;
        }
    }

    if (pass == 2)
    {
        if ((e->type == Open) || (e->type == Close))
        {
            // Shape edge positions.
            double minShape = v->min[1];
            double maxShape = v->max[1];
            // As far as we can see.
            double minLimit, maxLimit;
            double minLimitMax, maxLimitMin;
            v->findFirstPointAboveAndBelow(1, minLimit, maxLimit,
                    minLimitMax, maxLimitMin);

            // Only difference between Open and Close is whether the line
            // segments are at the left or right of the shape.  Decide here.
            double lineX = (e->type == Open) ? v->min[0] : v->max[0];

            if (minLimitMax >= maxLimitMin)
            {
                LineSegment vertSeg = LineSegment(minLimit, maxLimit, lineX);
                segments.insert(vertSeg);
            }
            else
            {
                if ((minLimitMax > minLimit) && (minLimitMax >= minShape))
                {
                    LineSegment vertSeg =
                            LineSegment(minLimit, minLimitMax, lineX);
                    segments.insert(vertSeg);
                }
                if ((maxLimitMin < maxLimit) && (maxLimitMin <= maxShape))
                {
                    LineSegment vertSeg =
                            LineSegment(maxLimitMin, maxLimit, lineX);
                    segments.insert(vertSeg);
                }
            }
        }
        else if (e->type == ConnPoint)
        {
            // Connection point.
            VertInf *centreVert = e->v->c;
            Point& cp = centreVert->point;

            // As far as we can see.
            double minLimit = v->firstPointAbove(1);
            double maxLimit = v->firstPointBelow(1);
            bool inShape = v->isInsideShape(1);

            if (!inShape || (centreVert->visDirections & ConnDirUp))
            {
                segments.insert(LineSegment(minLimit, cp.y, e->pos));
            }
            if (!inShape || (centreVert->visDirections & ConnDirDown))
            {
                segments.insert(LineSegment(cp.y, maxLimit, e->pos));
            }
        }
    }

    if ( ((pass == 3) && (e->type == Close)) ||
         ((pass == 2) && (e->type == ConnPoint)) )
    {
        // Clean up neighbour pointers.
        Node *l = v->firstAbove, *r = v->firstBelow;
        if (l != NULL)
        {
            l->firstBelow = v->firstBelow;
        }
        if (r != NULL)
        {
            r->firstAbove = v->firstAbove;
        }

        if (e->type == ConnPoint)
        {
            scanline.erase(v->iter);
            delete v;
        }
        else  // if (e->type == Close)
        {
            size_t result;
            result = scanline.erase(v);
            COLA_ASSERT(result == 1);
            delete v;
        }
    }
}


extern void generateStaticOrthogonalVisGraph(Router *router)
{
    const size_t n = router->shapeRefs.size();
    const unsigned cpn = router->vertices.connsSize();
    // Set up the events for the vertical sweep.
    size_t totalEvents = (2 * n) + cpn;
    events = new Event*[totalEvents];
    unsigned ctr = 0;
    ShapeRefList::iterator shRefIt = router->shapeRefs.begin();
    for (unsigned i = 0; i < n; i++)
    {
        ShapeRef *shRef = *shRefIt;
        double minX, minY, maxX, maxY;
        shRef->polygon().getBoundingRect(&minX, &minY, &maxX, &maxY);
        double midX = minX + ((maxX - minX) / 2);
        Node *v = new Node(shRef, midX);
        events[ctr++] = new Event(Open, v, minY);
        events[ctr++] = new Event(Close, v, maxY);

        ++shRefIt;
    }
    for (VertInf *curr = router->vertices.connsBegin();
            curr && (curr != router->vertices.shapesBegin());
            curr = curr->lstNext)
    {
        Point& point = curr->point;

        Node *v = new Node(curr, point.x);
        events[ctr++] = new Event(ConnPoint, v, point.y);
    }
    qsort((Event*)events, (size_t) totalEvents, sizeof(Event*), compare_events);

    // Process the vertical sweep.
    // We do multiple passes over sections of the list so we can add relevant
    // entries to the scanline that might follow, before process them.
    SegmentListWrapper segments;
    NodeSet scanline;
    double thisPos = (totalEvents > 0) ? events[0]->pos : 0;
    unsigned int posStartIndex = 0;
    unsigned int posFinishIndex = 0;
    for (unsigned i = 0; i <= totalEvents; ++i)
    {
        // If we have finished the current scanline or all events, then we
        // process the events on the current scanline in a couple of passes.
        if ((i == totalEvents) || (events[i]->pos != thisPos))
        {
            posFinishIndex = i;
            for (int pass = 2; pass <= 3; ++pass)
            {
                for (unsigned j = posStartIndex; j < posFinishIndex; ++j)
                {
                    processEventVert(router, scanline, segments,
                            events[j], pass);
                }
            }

            if (i == totalEvents)
            {
                // We have cleaned up, so we can now break out of loop.
                break;
            }

            thisPos = events[i]->pos;
            posStartIndex = i;
        }

        // Do the first sweep event handling -- building the correct
        // structure of the scanline.
        const int pass = 1;
        processEventVert(router, scanline, segments, events[i], pass);
    }
    COLA_ASSERT(scanline.empty());
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }

    segments.list().sort();

    // Set up the events for the horizontal sweep.
    SegmentListWrapper vertSegments;
    ctr = 0;
    shRefIt = router->shapeRefs.begin();
    for (unsigned i = 0; i < n; i++)
    {
        ShapeRef *shRef = *shRefIt;
        double minX, minY, maxX, maxY;
        shRef->polygon().getBoundingRect(&minX, &minY, &maxX, &maxY);
        double midY = minY + ((maxY - minY) / 2);
        Node *v = new Node(shRef, midY);
        events[ctr++] = new Event(Open, v, minX);
        events[ctr++] = new Event(Close, v, maxX);

        ++shRefIt;
    }
    for (VertInf *curr = router->vertices.connsBegin();
            curr && (curr != router->vertices.shapesBegin());
            curr = curr->lstNext)
    {
        Point& point = curr->point;

        Node *v = new Node(curr, point.y);
        events[ctr++] = new Event(ConnPoint, v, point.x);
    }
    qsort((Event*)events, (size_t) totalEvents, sizeof(Event*), compare_events);

    // Process the horizontal sweep
    thisPos = (totalEvents > 0) ? events[0]->pos : 0;
    posStartIndex = 0;
    for (unsigned i = 0; i <= totalEvents; ++i)
    {
        // If we have finished the current scanline or all events, then we
        // process the events on the current scanline in a couple of passes.
        if ((i == totalEvents) || (events[i]->pos != thisPos))
        {
            posFinishIndex = i;
            for (int pass = 2; pass <= 3; ++pass)
            {
                for (unsigned j = posStartIndex; j < posFinishIndex; ++j)
                {
                    processEventHori(router, scanline, vertSegments,
                            events[j], pass);
                }
            }

            // Process the merged line segments.
            vertSegments.list().sort();
            for (SegmentList::iterator curr = vertSegments.list().begin();
                    curr != vertSegments.list().end(); ++curr)
            {
                intersectSegments(router, segments.list(), *curr);
            }
            vertSegments.list().clear();

            if (i == totalEvents)
            {
                // We have cleaned up, so we can now break out of loop.
                break;
            }

            thisPos = events[i]->pos;
            posStartIndex = i;
        }

        // Do the first sweep event handling -- building the correct
        // structure of the scanline.
        const int pass = 1;
        processEventHori(router, scanline, vertSegments, events[i], pass);
    }
    COLA_ASSERT(scanline.empty());
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }
    delete [] events;

    // Add portions of the horizontal line that are after the final vertical
    // position we considered.
    for (SegmentList::iterator it = segments.list().begin();
            it != segments.list().end(); )
    {
        LineSegment& horiLine = *it;

        horiLine.addEdgeHorizontal(router);

        size_t dim = XDIM; // x-dimension
        horiLine.generateVisibilityEdgesFromBreakpointSet(router, dim);

        it = segments.list().erase(it);
    }
}


//============================================================================
//                           Path Adjustment code
//============================================================================




// Processes sweep events used to determine each horizontal and vertical
// line segment in a connector's channel of visibility.
// Four calls to this function are made at each position by the scanline:
//   1) Handle all Close event processing.
//   2) Remove Close event objects from the scanline.
//   3) Add Open event objects to the scanline.
//   4) Handle all Open event processing.
//
static void processShiftEvent(Router */*router*/, NodeSet& scanline,
                              ShiftSegmentList& /*segments*/, Event *e, size_t dim,
                              unsigned int pass)
{
    Node *v = e->v;

    if ( ((pass == 3) && (e->type == Open)) ||
         ((pass == 3) && (e->type == SegOpen)) )
    {
        std::pair<NodeSet::iterator, bool> result = scanline.insert(v);
        v->iter = result.first;
        COLA_ASSERT(result.second);

        NodeSet::iterator it = v->iter;
        // Work out neighbours
        if (it != scanline.begin())
        {
            Node *u = *(--it);
            v->firstAbove = u;
            u->firstBelow = v;
        }
        it = v->iter;
        if (++it != scanline.end())
        {
            Node *u = *it;
            v->firstBelow = u;
            u->firstAbove = v;
        }
    }

    if ( ((pass == 4) && (e->type == Open)) ||
         ((pass == 4) && (e->type == SegOpen)) ||
         ((pass == 1) && (e->type == SegClose)) ||
         ((pass == 1) && (e->type == Close)) )
    {
        if (v->ss)
        {
            // As far as we can see.
            double minLimit = v->firstObstacleAbove(dim);
            double maxLimit = v->firstObstacleBelow(dim);

            v->ss->minSpaceLimit =
                    std::max(minLimit, v->ss->minSpaceLimit);
            v->ss->maxSpaceLimit =
                    std::min(maxLimit, v->ss->maxSpaceLimit);
        }
        else
        {
            v->markShiftSegmentsAbove(dim);
            v->markShiftSegmentsBelow(dim);
        }
    }

    if ( ((pass == 2) && (e->type == SegClose)) ||
         ((pass == 2) && (e->type == Close)) )
    {
        // Clean up neighbour pointers.
        Node *l = v->firstAbove, *r = v->firstBelow;
        if (l != NULL)
        {
            l->firstBelow = v->firstBelow;
        }
        if (r != NULL)
        {
            r->firstAbove = v->firstAbove;
        }

        size_t result;
        result = scanline.erase(v);
        COLA_ASSERT(result == 1);
        delete v;
    }
}


static void buildOrthogonalChannelInfo(Router *router,
        const size_t dim, ShiftSegmentList& segmentList)
{
    if (router->routingPenalty(segmentPenalty) == 0)
    {
        // This code assumes the routes are pretty optimal, so we don't
        // do this adjustment if the routes have no segment penalty.
        return;
    }

    size_t altDim = (dim + 1) % 2;
    // For each connector.
    for (ConnRefList::const_iterator curr = router->connRefs.begin();
            curr != router->connRefs.end(); ++curr)
    {
        if ((*curr)->routingType() != ConnType_Orthogonal)
        {
            continue;
        }
        Polygon& displayRoute = (*curr)->displayRoute();
        // Determine all line segments that we are interested in shifting.
        // We don't consider the first or last segment of a path.
        for (size_t i = 1; i < displayRoute.size()-1; ++i)
        {
            if (displayRoute.ps[i - 1][dim] == displayRoute.ps[i][dim])
            {
                // It's a segment in the dimension we are processing,
                size_t indexLow = i - 1;
                size_t indexHigh = i;
                if (displayRoute.ps[i - 1][altDim] > displayRoute.ps[i][altDim])
                {
                    indexLow = i;
                    indexHigh = i - 1;
                }
                COLA_ASSERT(displayRoute.at(indexLow)[altDim] <
                        displayRoute.at(indexHigh)[altDim]);

                if ((i == 1) || ((i + 1) == displayRoute.size()))
                {
                    // The first and last segment of a connector can't be
                    // shifted.  We call them fixed segments.  Note: this
                    // will change if we later allow connection channels.
                    segmentList.push_back(
                            ShiftSegment(*curr, indexLow, indexHigh, dim));
                    continue;
                }

                // The segment probably has space to be shifted.
                double minLim = -CHANNEL_MAX;
                double maxLim = CHANNEL_MAX;
                bool isSBend = false;

                double prevPos = displayRoute.ps[i - 2][dim];
                double nextPos = displayRoute.ps[i + 1][dim];
                if (((prevPos < displayRoute.ps[i][dim]) &&
                     (nextPos > displayRoute.ps[i][dim]))
                     ||
                    ((prevPos > displayRoute.ps[i][dim]) &&
                     (nextPos < displayRoute.ps[i][dim])) )
                {
                    isSBend = true;

                    // Determine limits if the s-bend is not due to an
                    // obstacle.  In this case we need to limit the channel
                    // to the span of the adjoining segments to this one.
                    if ((prevPos < displayRoute.ps[i][dim]) &&
                        (nextPos > displayRoute.ps[i][dim]))
                    {
                        minLim = std::max(minLim, prevPos);
                        maxLim = std::min(maxLim, nextPos);
                    }
                    else
                    {
                        minLim = std::max(minLim, nextPos);
                        maxLim = std::min(maxLim, prevPos);
                    }
                }
                else
                {
                    // isCBend: Both adjoining segments are in the same
                    // direction.  We indicate this for later by setting
                    // the maxLim or minLim to the segment position.
                    if (prevPos < displayRoute.ps[i][dim])
                    {
                        minLim = displayRoute.ps[i][dim];
                    }
                    else
                    {
                        maxLim = displayRoute.ps[i][dim];
                    }
                }

                segmentList.push_back(ShiftSegment(*curr, indexLow,
                            indexHigh, isSBend, dim, minLim, maxLim));
            }
        }
    }
    if (segmentList.empty())
    {
        // There are no segments, so we can just return now.
        return;
    }

    // Do a sweep and shift these segments.
    const size_t n = router->shapeRefs.size();
    const size_t cpn = segmentList.size();
    // Set up the events for the sweep.
    size_t totalEvents = 2 * (n + cpn);
    events = new Event*[totalEvents];
    unsigned ctr = 0;
    ShapeRefList::iterator shRefIt = router->shapeRefs.begin();
    for (unsigned i = 0; i < n; i++)
    {
        ShapeRef *shRef = *shRefIt;
        Point min, max;
        shRef->polygon().getBoundingRect(&min.x, &min.y, &max.x, &max.y);
        double mid = min[dim] + ((max[dim] - min[dim]) / 2);
        Node *v = new Node(shRef, mid);
        events[ctr++] = new Event(Open, v, min[altDim]);
        events[ctr++] = new Event(Close, v, max[altDim]);

        ++shRefIt;
    }
    for (ShiftSegmentList::iterator curr = segmentList.begin();
            curr != segmentList.end(); ++curr)
    {
        const Point& lowPt = curr->lowPoint();
        const Point& highPt = curr->highPoint();

        COLA_ASSERT(lowPt[dim] == highPt[dim]);
        COLA_ASSERT(lowPt[altDim] < highPt[altDim]);
        Node *v = new Node(&(*curr), lowPt[dim]);
        events[ctr++] = new Event(SegOpen, v, lowPt[altDim]);
        events[ctr++] = new Event(SegClose, v, highPt[altDim]);
    }
    qsort((Event*)events, (size_t) totalEvents, sizeof(Event*), compare_events);

    // Process the sweep.
    // We do multiple passes over sections of the list so we can add relevant
    // entries to the scanline that might follow, before process them.
    NodeSet scanline;
    double thisPos = (totalEvents > 0) ? events[0]->pos : 0;
    unsigned int posStartIndex = 0;
    unsigned int posFinishIndex = 0;
    for (unsigned i = 0; i <= totalEvents; ++i)
    {
        // If we have finished the current scanline or all events, then we
        // process the events on the current scanline in a couple of passes.
        if ((i == totalEvents) || (events[i]->pos != thisPos))
        {
            posFinishIndex = i;
            for (int pass = 2; pass <= 4; ++pass)
            {
                for (unsigned j = posStartIndex; j < posFinishIndex; ++j)
                {
                    processShiftEvent(router, scanline, segmentList, events[j],
                            dim, pass);
                }
            }

            if (i == totalEvents)
            {
                // We have cleaned up, so we can now break out of loop.
                break;
            }

            thisPos = events[i]->pos;
            posStartIndex = i;
        }

        // Do the first sweep event handling -- building the correct
        // structure of the scanline.
        const int pass = 1;
        processShiftEvent(router, scanline, segmentList, events[i],
                dim, pass);
    }
    COLA_ASSERT(scanline.empty());
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }
    delete [] events;
}


static void simplifyOrthogonalRoutes(Router *router)
{
    // Simplify routes.
    for (ConnRefList::const_iterator curr = router->connRefs.begin();
            curr != router->connRefs.end(); ++curr)
    {
        if ((*curr)->routingType() != ConnType_Orthogonal)
        {
            continue;
        }
        (*curr)->set_route((*curr)->displayRoute().simplify());
    }
}


static void buildOrthogonalNudgingOrderInfo(Router *router,
        PtOrderMap& pointOrders)
{
    // Simplify routes.
    simplifyOrthogonalRoutes(router);

    int crossingsN = 0;

    // Do segment splitting.
    for (ConnRefList::const_iterator curr = router->connRefs.begin();
            curr != router->connRefs.end(); ++curr)
    {
        if ((*curr)->routingType() != ConnType_Orthogonal)
        {
            continue;
        }
        ConnRef *conn = *curr;

        for (ConnRefList::const_iterator curr2 = router->connRefs.begin();
                curr2 != router->connRefs.end(); ++curr2)
        {
            if ((*curr2)->routingType() != ConnType_Orthogonal)
            {
                continue;
            }
            ConnRef *conn2 = *curr2;

            if (conn == conn2)
            {
                continue;
            }

            Avoid::Polygon& route = conn->displayRoute();
            Avoid::Polygon& route2 = conn2->displayRoute();
            splitBranchingSegments(route2, true, route);
        }
    }

    for (ConnRefList::const_iterator curr = router->connRefs.begin();
            curr != router->connRefs.end(); ++curr)
    {
        if ((*curr)->routingType() != ConnType_Orthogonal)
        {
            continue;
        }
        ConnRef *conn = *curr;

        for (ConnRefList::const_iterator curr2 = curr;
                curr2 != router->connRefs.end(); ++curr2)
        {
            if ((*curr2)->routingType() != ConnType_Orthogonal)
            {
                continue;
            }
            ConnRef *conn2 = *curr2;

            if (conn == conn2)
            {
                continue;
            }

            Avoid::Polygon& route = conn->displayRoute();
            Avoid::Polygon& route2 = conn2->displayRoute();
            bool checkForBranchingSegments = false;
            int crossings = 0;
            for (size_t i = 1; i < route.size(); ++i)
            {
                const bool finalSegment = ((i + 1) == route.size());
                crossings += countRealCrossings(route2, true, route, i,
                        checkForBranchingSegments, finalSegment, NULL,
                        &pointOrders, conn2, conn).first;
            }
            if (crossings > 0)
            {
                crossingsN += crossings;
            }
        }
    }

    // Sort the point orders.
    PtOrderMap::iterator finish = pointOrders.end();
    for (PtOrderMap::iterator it = pointOrders.begin(); it != finish; ++it)
    {
        //const VertID& ptID = it->first;
        PtOrder& order = it->second;

        for (size_t dim = XDIM; dim <= YDIM; ++dim)
        {
            order.sort(dim);
        }
    }
}


class CmpLineOrder
{
    public:
        CmpLineOrder(PtOrderMap& ord, const size_t dim)
            : orders(ord),
              dimension(dim)
        {
        }
        bool operator()(const ShiftSegment& lhs, const ShiftSegment& rhs,
                bool *comparable = NULL) const
        {
            if (comparable)
            {
                *comparable = true;
            }
            Point lhsLow  = lhs.lowPoint();
            Point rhsLow  = rhs.lowPoint();
#ifndef NDEBUG
            const Point& lhsHigh = lhs.highPoint();
            const Point& rhsHigh = rhs.highPoint();
#endif
            size_t altDim = (dimension + 1) % 2;

            COLA_ASSERT(lhsLow[dimension] == lhsHigh[dimension]);
            COLA_ASSERT(rhsLow[dimension] == rhsHigh[dimension]);

            if (lhsLow[dimension] != rhsLow[dimension])
            {
                return lhsLow[dimension] < rhsLow[dimension];
            }

            // If one of these is fixed, then determine order based on
            // fixed segment, that is, order so the fixed segment doesn't
            // block movement.
            bool oneIsFixed = false;
            const int lhsFixedOrder = lhs.fixedOrder(oneIsFixed);
            const int rhsFixedOrder = rhs.fixedOrder(oneIsFixed);
            if (oneIsFixed && (lhsFixedOrder != rhsFixedOrder))
            {
                return lhsFixedOrder < rhsFixedOrder;
            }

            // C-bends that did not have a clear order with s-bends might
            // not have a good ordering here, so compare their order in
            // terms of C-bend direction and S-bends and use that if it
            // differs for the two segments.
            const int lhsOrder = lhs.order();
            const int rhsOrder = rhs.order();
            if (lhsOrder != rhsOrder)
            {
                return lhsOrder < rhsOrder;
            }

            // Need to index using the original point into the map, so find it.
            Point& unchanged = (lhsLow[altDim] > rhsLow[altDim]) ?
                    lhsLow : rhsLow;

            PtOrder& lowOrder = orders[unchanged];
            int lhsPos = lowOrder.positionFor(lhs.connRef, dimension);
            int rhsPos = lowOrder.positionFor(rhs.connRef, dimension);
            if ((lhsPos == -1) || (rhsPos == -1))
            {
                // A value for rhsPos or lhsPos mean the points are not directly
                // comparable, meaning they are at the same position but cannot
                // overlap (they are just collinear.  The relative order for
                // these segments is not important since we do not constrain
                // them against each other.
                //COLA_ASSERT(lhs.overlapsWith(rhs, dimension) == false);
                // We do need to be consistent though.
                if (comparable)
                {
                    *comparable = false;
                }
                return lhsLow[altDim] < rhsLow[altDim];
            }

            return lhsPos < rhsPos;
        }

        PtOrderMap& orders;
        const size_t dimension;
};


// We can use the normaal sort algorithm for lists since it is not possible
// to comapre all elements, but there will be an ordering defined between
// most of the elements.  Hence we order these, using insertion sort, and
// the case of them not being able to be compared is handled by not setting
// up any constraints between such segments when doing the nudging.
//
static ShiftSegmentList linesort(ShiftSegmentList origList,
        CmpLineOrder& comparison)
{
    ShiftSegmentList resultList;

    while (!origList.empty())
    {
        // Get and remove the first element from the origList.
        ShiftSegment segment = origList.front();
        origList.pop_front();

        // Find the insertion point in the resultList.
        ShiftSegmentList::iterator curr;
        for (curr = resultList.begin(); curr != resultList.end(); ++curr)
        {
            bool comparable = false;
            bool lessThan = comparison(segment, *curr, &comparable);

            if (comparable && lessThan)
            {
                // If it is comparable and lessThan, then we have found the
                // insertion point.
                break;
            }
        }

        // Insert the element into the reultList at the required point.
        resultList.insert(curr, segment);
    }

    return resultList;
}


typedef std::list<ShiftSegment *> ShiftSegmentPtrList;


static void nudgeOrthogonalRoutes(Router *router, size_t dimension,
        PtOrderMap& pointOrders, ShiftSegmentList& segmentList)
{
    // Do the actual nudging.
    ShiftSegmentList currentRegion;
    while (!segmentList.empty())
    {
        // Take a reference segment
        ShiftSegment& currentSegment = segmentList.front();
        // Then, find the segments that overlap this one.
        currentRegion.clear();
        currentRegion.push_back(currentSegment);
        segmentList.erase(segmentList.begin());
        for (ShiftSegmentList::iterator curr = segmentList.begin();
                curr != segmentList.end(); )
        {
            bool overlaps = false;
            for (ShiftSegmentList::iterator curr2 = currentRegion.begin();
                    curr2 != currentRegion.end(); ++curr2)
            {
                if (curr->overlapsWith(*curr2, dimension))
                {
                    overlaps = true;
                    break;
                }
            }
            if (overlaps)
            {
                currentRegion.push_back(*curr);
                segmentList.erase(curr);
                // Consider segments from the beginning, since we mave have
                // since passed segments that overlap with the new set.
                curr = segmentList.begin();
            }
            else
            {
                ++curr;
            }
        }
        CmpLineOrder lineSortComp(pointOrders, dimension);
        currentRegion = linesort(currentRegion, lineSortComp);

        if (currentRegion.size() == 1)
        {
            // Save creating the solver instance if there is just one
            // immovable segment.
            if (!currentRegion.front().sBend)
            {
                continue;
            }
        }

        // Process these segments.
        Variables vs;
        Constraints cs;
        ShiftSegmentPtrList prevVars;
        // IDs:
        const int freeID    = 0;
        const int fixedID   = 1;
        // Weights:
        double freeWeight   = 0.00001;
        double strongWeight = 0.001;
        double fixedWeight  = 100000;
        //printf("-------------------------------------------------------\n");
        //printf("Nudge -- size: %d\n", (int) currentRegion.size());
        for (ShiftSegmentList::iterator currSegment = currentRegion.begin();
                currSegment != currentRegion.end(); ++currSegment)
        {
            Point& lowPt = currSegment->lowPoint();

            // Create a solver variable for the position of this segment.
            int varID = freeID;
            double idealPos = lowPt[dimension];
            double weight = freeWeight;
            if (currSegment->sBend)
            {
                COLA_ASSERT(currSegment->minSpaceLimit > -CHANNEL_MAX);
                COLA_ASSERT(currSegment->maxSpaceLimit < CHANNEL_MAX);

                // For s-bends, take the middle as ideal.
                idealPos = currSegment->minSpaceLimit +
                        ((currSegment->maxSpaceLimit -
                          currSegment->minSpaceLimit) / 2);
            }
            else if (currSegment->fixed)
            {
                // Fixed segments shouldn't get moved.
                weight = fixedWeight;
                varID = fixedID;
            }
            else
            {
                // Set a higher weight for c-bends to stop them sometimes
                // getting pushed out into channels by more-free connectors
                // to the "inner" side of them.
                weight = strongWeight;
            }
            currSegment->variable = new Variable(varID, idealPos, weight);
            vs.push_back(currSegment->variable);
            size_t index = vs.size() - 1;
            //printf("line  %.15f  pos: %g   min: %g  max: %g\n",
            //        lowPt[dimension], idealPos, currSegment->minSpaceLimit,
            //        currSegment->maxSpaceLimit);

            // Constrain position in relation to previously seen segments,
            // if necessary (i.e. when they could overlap).
            for (ShiftSegmentPtrList::iterator prevVarIt = prevVars.begin();
                    prevVarIt != prevVars.end(); )
            {
                ShiftSegment *prevSeg = *prevVarIt;
                Variable *prevVar = prevSeg->variable;

                if (currSegment->overlapsWith(*prevSeg, dimension) &&
                        (!(currSegment->fixed) || !(prevSeg->fixed)))
                {
                    // If there is a previous segment to the left that
                    // could overlap this in the shift direction, then
                    // constrain the two segments to be separated.
                    // Though don't add the constraint if both the
                    // segments are fixed in place.
                    cs.push_back(new Constraint(prevVar, vs[index],
                            router->orthogonalNudgeDistance()));
                    prevVarIt = prevVars.erase(prevVarIt);
                }
                else
                {
                    ++prevVarIt;
                }
            }

            if (!currSegment->fixed)
            {
                // If this segment sees a channel boundary to its left,
                // then constrain its placement as such.
                if (currSegment->minSpaceLimit > -CHANNEL_MAX)
                {
                    vs.push_back(new Variable(fixedID,
                                currSegment->minSpaceLimit, fixedWeight));
                    cs.push_back(new Constraint(vs[vs.size() - 1], vs[index],
                                0.0));
                }

                // If this segment sees a channel boundary to its right,
                // then constrain its placement as such.
                if (currSegment->maxSpaceLimit < CHANNEL_MAX)
                {
                    vs.push_back(new Variable(fixedID,
                                currSegment->maxSpaceLimit, fixedWeight));
                    cs.push_back(new Constraint(vs[index], vs[vs.size() - 1],
                                0.0));
                }
            }
            prevVars.push_back(&(*currSegment));
        }
#if 0
        for(unsigned i=0;i<vs.size();i++) {
            printf("-vs[%d]=%f\n",i,vs[i]->desiredPosition);
        }
#endif
        IncSolver f(vs,cs);
        f.solve();
        bool satisfied = true;
        for (size_t i = 0; i < vs.size(); ++i)
        {
            if (vs[i]->id == fixedID)
            {
                if (fabs(vs[i]->finalPosition - vs[i]->desiredPosition) > 0.01)
                {
                    satisfied = false;
                    break;
                }
            }
        }
        if (satisfied)
        {
            for (ShiftSegmentList::iterator currSegment = currentRegion.begin();
                    currSegment != currentRegion.end(); ++currSegment)
            {
                Point& lowPt = currSegment->lowPoint();
                Point& highPt = currSegment->highPoint();
                double newPos = currSegment->variable->finalPosition;
                //printf("Pos: %X, %g\n", (int) currSegment->connRef, newPos);
                lowPt[dimension] = newPos;
                highPt[dimension] = newPos;
            }
        }
#if 0
        for(unsigned i=0;i<vs.size();i++) {
            printf("+vs[%d]=%f\n",i,vs[i]->finalPosition);
        }
#endif
        for_each(vs.begin(),vs.end(),delete_object());
        for_each(cs.begin(),cs.end(),delete_object());
    }
}


extern void improveOrthogonalRoutes(Router *router)
{
    router->timers.Register(tmOrthogNudge, timerStart);
    for (size_t dimension = 0; dimension < 2; ++dimension)
    {
        // Build nudging info.
        // XXX: We need to build the point orders separately in each
        //      dimension since things move.  There is probably a more
        //      efficient way to do this.
        PtOrderMap pointOrders;
        buildOrthogonalNudgingOrderInfo(router, pointOrders);

        // Simplify routes.
        simplifyOrthogonalRoutes(router);

        // Do the centring and nudging.
        ShiftSegmentList segLists;
        buildOrthogonalChannelInfo(router, dimension, segLists);
        nudgeOrthogonalRoutes(router, dimension, pointOrders, segLists);
    }
    router->timers.Stop();
}


}
