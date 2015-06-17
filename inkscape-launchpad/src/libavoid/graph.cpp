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


#include <cmath>

#include "libavoid/debug.h"
#include "libavoid/graph.h"
#include "libavoid/connector.h"
#include "libavoid/geometry.h"
#include "libavoid/timer.h"
#include "libavoid/vertices.h"
#include "libavoid/router.h"
#include "libavoid/assertions.h"


using std::pair;

namespace Avoid {


EdgeInf::EdgeInf(VertInf *v1, VertInf *v2, const bool orthogonal)
    : lstPrev(NULL),
      lstNext(NULL),
      _blocker(0),
      _router(NULL),
      _added(false),
      _visible(false),
      _orthogonal(orthogonal),
      _v1(v1),
      _v2(v2),
      _dist(-1)
{
    // Not passed NULL values.
    COLA_ASSERT(v1 && v2);

    // We are in the same instance
    COLA_ASSERT(_v1->_router == _v2->_router);
    _router = _v1->_router;

    _conns.clear();
}


EdgeInf::~EdgeInf()
{
    if (_added)
    {
        makeInactive();
    }
}


// Gives an order value between 0 and 3 for the point c, given the last
// segment was from a to b.  Returns the following value:
//    0 : Point c is directly backwards from point b.
//    1 : Point c is a left-hand 90 degree turn.
//    2 : Point c is a right-hand 90 degree turn.
//    3 : Point c is straight ahead (collinear).
//
static inline int orthogTurnOrder(const Point& a, const Point& b, 
        const Point& c)
{
    // We should only be calling this with orthogonal points, 
    COLA_ASSERT((c.x == b.x) || (c.y == b.y));
    COLA_ASSERT((a.x == b.x) || (a.y == b.y));

    int direction = vecDir(a, b, c);

    if (direction > 0)
    {
        // Counterclockwise := left
        return 1;
    }
    else if (direction < 0)
    {
        // Clockwise := right
        return 2;
    }

    if (b.x == c.x)
    {
        if ( ((a.y < b.y) && (c.y < b.y)) || 
             ((a.y > b.y) && (c.y > b.y)) ) 
        {
            // Behind.
            return 0;
        }
    }
    else
    {
        if ( ((a.x < b.x) && (c.x < b.x)) || 
             ((a.x > b.x) && (c.x > b.x)) ) 
        {
            // Behind.
            return 0;
        }
    }

    // Ahead.
    return 3;
}


// Returns a less than operation for a set exploration order for orthogonal
// searching.  Forward, then left, then right.  Or if there is no previous 
// point, then the order is north, east, south, then west.
// Note: This method assumes the two Edges that share a common point.
bool EdgeInf::rotationLessThan(const VertInf *lastV, const EdgeInf *rhs) const
{
	assert(_v1 == rhs->_v1 || _v1 == rhs->_v2 || _v2 == rhs->_v1 || _v2 == rhs->_v2 );

	if ((_v1 == rhs->_v1) && (_v2 == rhs->_v2))
    {
        // Effectively the same visibility edge, so they are equal.
        return false;
    }
    VertInf *lhsV = NULL, *rhsV = NULL, *commonV = NULL;
    
    // Determine common Point and the comparison point on the left- and
    // the right-hand-side.
    if (_v1 == rhs->_v1)
    {
        commonV = _v1;
        lhsV = _v2;
        rhsV = rhs->_v2;
    }
    else if (_v1 == rhs->_v2)
    {
        commonV = _v1;
        lhsV = _v2;
        rhsV = rhs->_v1;
    }
    else if (_v2 == rhs->_v1)
    {
        commonV = _v2;
        lhsV = _v1;
        rhsV = rhs->_v2;
    }
    else if (_v2 == rhs->_v2)
    {
        commonV = _v2;
        lhsV = _v1;
        rhsV = rhs->_v1;
    }

    const Point& lhsPt = lhsV->point;
    const Point& rhsPt = rhsV->point;
    const Point& commonPt = commonV->point;
    
    // If no lastPt, use one directly to the left;
    Point lastPt = (lastV) ? lastV->point : Point(commonPt.x - 10,  commonPt.y);

    int lhsVal = orthogTurnOrder(lastPt, commonPt, lhsPt);
    int rhsVal = orthogTurnOrder(lastPt, commonPt, rhsPt);

    return lhsVal < rhsVal;
}


void EdgeInf::makeActive(void)
{
    COLA_ASSERT(_added == false);

    if (_orthogonal)
    {
        COLA_ASSERT(_visible);
        _router->visOrthogGraph.addEdge(this);
        _pos1 = _v1->orthogVisList.insert(_v1->orthogVisList.begin(), this);
        _v1->orthogVisListSize++;
        _pos2 = _v2->orthogVisList.insert(_v2->orthogVisList.begin(), this);
        _v2->orthogVisListSize++;
    }
    else
    {
        if (_visible)
        {
            _router->visGraph.addEdge(this);
            _pos1 = _v1->visList.insert(_v1->visList.begin(), this);
            _v1->visListSize++;
            _pos2 = _v2->visList.insert(_v2->visList.begin(), this);
            _v2->visListSize++;
        }
        else // if (invisible)
        {
            _router->invisGraph.addEdge(this);
            _pos1 = _v1->invisList.insert(_v1->invisList.begin(), this);
            _v1->invisListSize++;
            _pos2 = _v2->invisList.insert(_v2->invisList.begin(), this);
            _v2->invisListSize++;
        }
    }
    _added = true;
}


void EdgeInf::makeInactive(void)
{
    COLA_ASSERT(_added == true);

    if (_orthogonal)
    {
        COLA_ASSERT(_visible);
        _router->visOrthogGraph.removeEdge(this);
        _v1->orthogVisList.erase(_pos1);
        _v1->orthogVisListSize--;
        _v2->orthogVisList.erase(_pos2);
        _v2->orthogVisListSize--;
    }
    else
    {
        if (_visible)
        {
            _router->visGraph.removeEdge(this);
            _v1->visList.erase(_pos1);
            _v1->visListSize--;
            _v2->visList.erase(_pos2);
            _v2->visListSize--;
        }
        else // if (invisible)
        {
            _router->invisGraph.removeEdge(this);
            _v1->invisList.erase(_pos1);
            _v1->invisListSize--;
            _v2->invisList.erase(_pos2);
            _v2->invisListSize--;
        }
    }
    _blocker = 0;
    _conns.clear();
    _added = false;
}


void EdgeInf::setDist(double dist)
{
    //COLA_ASSERT(dist != 0);

    if (_added && !_visible)
    {
        makeInactive();
        COLA_ASSERT(!_added);
    }
    if (!_added)
    {
        _visible = true;
        makeActive();
    }
    _dist = dist;
    _blocker = 0;
}


bool EdgeInf::added(void)
{
    return _added;
}


void EdgeInf::alertConns(void)
{
    FlagList::iterator finish = _conns.end();
    for (FlagList::iterator i = _conns.begin(); i != finish; ++i)
    {
        *(*i) = true;
    }
    _conns.clear();
}


void EdgeInf::addConn(bool *flag)
{
    _conns.push_back(flag);
}


void EdgeInf::addCycleBlocker(void)
{
    // Needs to be in invisibility graph.
    addBlocker(-1);
}


void EdgeInf::addBlocker(int b)
{
    COLA_ASSERT(_router->InvisibilityGrph);

    if (_added && _visible)
    {
        makeInactive();
        COLA_ASSERT(!_added);
    }
    if (!_added)
    {
        _visible = false;
        makeActive();
    }
    _dist = 0;
    _blocker = b;
}


pair<VertID, VertID> EdgeInf::ids(void)
{
    return std::make_pair(_v1->id, _v2->id);
}


pair<Point, Point> EdgeInf::points(void)
{
    return std::make_pair(_v1->point, _v2->point);
}


void EdgeInf::db_print(void)
{
    db_printf("Edge(");
    _v1->id.db_print();
    db_printf(",");
    _v2->id.db_print();
    db_printf(")\n");
}


void EdgeInf::checkVis(void)
{
    if (_added && !_visible)
    {
        db_printf("\tChecking visibility for existing invisibility edge..."
                "\n\t\t");
        db_print();
    }
    else if (_added && _visible)
    {
        db_printf("\tChecking visibility for existing visibility edge..."
                "\n\t\t");
        db_print();
    }

    int blocker = 0;
    bool cone1 = true;
    bool cone2 = true;

    VertInf *i = _v1;
    VertInf *j = _v2;
    const VertID& iID = i->id;
    const VertID& jID = j->id;
    const Point& iPoint = i->point;
    const Point& jPoint = j->point;

    _router->st_checked_edges++;

    if (iID.isShape)
    {
        cone1 = inValidRegion(_router->IgnoreRegions, i->shPrev->point,
                iPoint, i->shNext->point, jPoint);
    }
    else if (_router->IgnoreRegions == false)
    {
        // If Ignoring regions then this case is already caught by 
        // the invalid regions, so only check it when not ignoring
        // regions.
        ShapeSet& ss = _router->contains[iID];

        if ((jID.isShape) && (ss.find(jID.objID) != ss.end()))
        {
            db_printf("1: Edge of bounding shape\n");
            // Don't even check this edge, it should be zero,
            // since a point in a shape can't see it's corners
            cone1 = false;
        }
    }

    if (cone1)
    {
        // If outside the first cone, don't even bother checking.
        if (jID.isShape)
        {
            cone2 = inValidRegion(_router->IgnoreRegions, j->shPrev->point,
                    jPoint, j->shNext->point, iPoint);
        }
        else if (_router->IgnoreRegions == false)
        {
            // If Ignoring regions then this case is already caught by 
            // the invalid regions, so only check it when not ignoring
            // regions.
            ShapeSet& ss = _router->contains[jID];

            if ((iID.isShape) && (ss.find(iID.objID) != ss.end()))
            {
                db_printf("2: Edge of bounding shape\n");
                // Don't even check this edge, it should be zero,
                // since a point in a shape can't see it's corners
                cone2 = false;
            }
        }
    }

    if (cone1 && cone2 && ((blocker = firstBlocker()) == 0))
    {

        // if i and j see each other, add edge
        db_printf("\tSetting visibility edge... \n\t\t");
        db_print();

        double d = euclideanDist(iPoint, jPoint);

        setDist(d);

    }
    else if (_router->InvisibilityGrph)
    {
#if 0
        db_printf("%d, %d, %d\n", cone1, cone2, blocker);
        db_printf("\t(%d, %d)--(%d, %d)\n", (int) iInfo.point.x,
                (int) iInfo.point.y, (int) jInfo.point.x,
                (int) jInfo.point.y);
#endif

        // if i and j can't see each other, add blank edge
        db_printf("\tSetting invisibility edge... \n\t\t");
        db_print();
        addBlocker(blocker);
    }
}


int EdgeInf::firstBlocker(void)
{
    ShapeSet ss = ShapeSet();

    Point& pti = _v1->point;
    Point& ptj = _v2->point;
    VertID& iID = _v1->id;
    VertID& jID = _v2->id;

    ContainsMap &contains = _router->contains;
    if (!(iID.isShape))
    {
        ss.insert(contains[iID].begin(), contains[iID].end());
    }
    if (!(jID.isShape))
    {
        ss.insert(contains[jID].begin(), contains[jID].end());
    }

    VertInf *last = _router->vertices.end();
    unsigned int lastId = 0;
    bool seenIntersectionAtEndpoint = false;
    for (VertInf *k = _router->vertices.shapesBegin(); k != last; )
    {
        VertID kID = k->id;
        if (k->id == dummyOrthogID)
        {
            // Don't include orthogonal dummy vertices.
            k = k->lstNext;
            continue;
        }
        if (kID.objID != lastId)
        {
            if ((ss.find(kID.objID) != ss.end()))
            {
                unsigned int shapeID = kID.objID;
                db_printf("Endpoint is inside shape %u so ignore shape "
                        "edges.\n", kID.objID);
                // One of the endpoints is inside this shape so ignore it.
                while ((k != last) && (k->id.objID == shapeID))
                {
                    // And skip the other vertices from this shape.
                    k = k->lstNext;
                }
                continue;
            }
            seenIntersectionAtEndpoint = false;
            lastId = kID.objID;
        }
        Point& kPoint = k->point;
        Point& kPrevPoint = k->shPrev->point;
        if (segmentShapeIntersect(pti, ptj, kPrevPoint, kPoint, 
                    seenIntersectionAtEndpoint))
        {
            ss.clear();
            return kID.objID;
        }
        k = k->lstNext;
    }
    ss.clear();
    return 0;
}


bool EdgeInf::isBetween(VertInf *i, VertInf *j)
{
    if ( ((i == _v1) && (j == _v2)) ||
         ((i == _v2) && (j == _v1)) )
    {
        return true;
    }
    return false;
}


    // Returns true if this edge is a vertical or horizontal line segment.
bool EdgeInf::isOrthogonal(void) const
{
    return ((_v1->point.x == _v2->point.x) || 
            (_v1->point.y == _v2->point.y));
}


VertInf *EdgeInf::otherVert(VertInf *vert)
{
    COLA_ASSERT((vert == _v1) || (vert == _v2));

    if (vert == _v1)
    {
        return _v2;
    }
    return _v1;
}


EdgeInf *EdgeInf::checkEdgeVisibility(VertInf *i, VertInf *j, bool knownNew)
{
    // This is for polyline routing, so check we're not 
    // considering orthogonal vertices.
    COLA_ASSERT(i->id != dummyOrthogID);
    COLA_ASSERT(j->id != dummyOrthogID);
    
    Router *router = i->_router;
    EdgeInf *edge = NULL;

    if (knownNew)
    {
        COLA_ASSERT(existingEdge(i, j) == NULL);
        edge = new EdgeInf(i, j);
    }
    else
    {
        edge = existingEdge(i, j);
        if (edge == NULL)
        {
            edge = new EdgeInf(i, j);
        }
    }
    edge->checkVis();
    if (!(edge->_added) && !(router->InvisibilityGrph))
    {
        delete edge;
        edge = NULL;
    }

    return edge;
}


    // XXX: This function is ineffecient, and shouldn't even really be
    //      required.
EdgeInf *EdgeInf::existingEdge(VertInf *i, VertInf *j)
{
    VertInf *selected = NULL;

    // Look through poly-line visibility edges.
    selected = (i->visListSize <= j->visListSize) ? i : j;
    EdgeInfList& visList = selected->visList;
    EdgeInfList::const_iterator finish = visList.end();
    for (EdgeInfList::const_iterator edge = visList.begin(); edge != finish;
            ++edge)
    {
        if ((*edge)->isBetween(i, j))
        {
            return (*edge);
        }
    }

    // Look through orthogonal visbility edges.
    selected = (i->orthogVisListSize <= j->orthogVisListSize) ? i : j;
    EdgeInfList& orthogVisList = selected->orthogVisList;
    finish = orthogVisList.end();
    for (EdgeInfList::const_iterator edge = orthogVisList.begin(); 
            edge != finish; ++edge)
    {
        if ((*edge)->isBetween(i, j))
        {
            return (*edge);
        }
    }

    // Look through poly-line invisbility edges.
    selected = (i->invisListSize <= j->invisListSize) ? i : j;
    EdgeInfList& invisList = selected->invisList;
    finish = invisList.end();
    for (EdgeInfList::const_iterator edge = invisList.begin(); edge != finish;
            ++edge)
    {
        if ((*edge)->isBetween(i, j))
        {
            return (*edge);
        }
    }

    return NULL;
}


//===========================================================================


EdgeList::EdgeList(bool orthogonal)
    : _orthogonal(orthogonal),
      _firstEdge(NULL),
      _lastEdge(NULL),
      _count(0)
{
}


EdgeList::~EdgeList()
{
    clear();
}


void EdgeList::clear(void)
{
    while (_firstEdge)
    {
        delete _firstEdge;
    }
    COLA_ASSERT(_count == 0);
    _lastEdge = NULL;
}


int EdgeList::size(void) const
{
    return _count;
}


void EdgeList::addEdge(EdgeInf *edge)
{
    COLA_ASSERT(!_orthogonal || edge->isOrthogonal());
    
    if (_firstEdge == NULL)
    {
        COLA_ASSERT(_lastEdge == NULL);

        _lastEdge = edge;
        _firstEdge = edge;

        edge->lstPrev = NULL;
        edge->lstNext = NULL;
    }
    else
    {
        COLA_ASSERT(_lastEdge != NULL);

        _lastEdge->lstNext = edge;
        edge->lstPrev = _lastEdge;

        _lastEdge = edge;

        edge->lstNext = NULL;
    }
    _count++;
}


void EdgeList::removeEdge(EdgeInf *edge)
{
    if (edge->lstPrev)
    {
        edge->lstPrev->lstNext = edge->lstNext;
    }
    if (edge->lstNext)
    {
        edge->lstNext->lstPrev = edge->lstPrev;
    }
    if (edge == _lastEdge)
    {
        _lastEdge = edge->lstPrev;
        if (edge == _firstEdge)
        {
            _firstEdge = NULL;
        }
    }
    else if (edge == _firstEdge)
    {
        _firstEdge = edge->lstNext;
    }


    edge->lstPrev = NULL;
    edge->lstNext = NULL;

    _count--;
}


EdgeInf *EdgeList::begin(void)
{
    return _firstEdge;
}


EdgeInf *EdgeList::end(void)
{
    return NULL;
}


}


