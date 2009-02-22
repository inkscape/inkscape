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

#include "libavoid/debug.h"
#include "libavoid/graph.h"
#include "libavoid/connector.h"
#include "libavoid/geometry.h"
#include "libavoid/polyutil.h"
#include "libavoid/timer.h"
#include "libavoid/vertices.h"
#include "libavoid/router.h"

#include <math.h>

using std::pair;

namespace Avoid {


EdgeInf::EdgeInf(VertInf *v1, VertInf *v2)
    : lstPrev(NULL)
    , lstNext(NULL)
    , _blocker(0)
    , _router(NULL)
    , _added(false)
    , _visible(false)
    , _v1(v1)
    , _v2(v2)
    , _dist(-1)
{
    // Not passed NULL values.
    assert(v1 && v2);

    // We are in the same instance
    assert(_v1->_router == _v2->_router);
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


void EdgeInf::makeActive(void)
{
    assert(_added == false);

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
    _added = true;
}


void EdgeInf::makeInactive(void)
{
    assert(_added == true);

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
    _blocker = 0;
    _conns.clear();
    _added = false;
}


void EdgeInf::setDist(double dist)
{
    //assert(dist != 0);

    if (_added && !_visible)
    {
        makeInactive();
    }
    if (!_added)
    {
        _visible = true;
        makeActive();
    }
    _dist = dist;
    _blocker = 0;
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
    assert(_router->InvisibilityGrph);

    if (_added && _visible)
    {
        makeInactive();
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
    else
    {
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
        else
        {
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

        double d = dist(iPoint, jPoint);

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
    for (VertInf *k = _router->vertices.shapesBegin(); k != last; )
    {
        VertID kID = k->id;
        if ((ss.find(kID.objID) != ss.end()))
        {
            unsigned int shapeID = kID.objID;
            db_printf("Endpoint is inside shape %u so ignore shape edges.\n",
                    kID.objID);
            // One of the endpoints is inside this shape so ignore it.
            while ((k != last) && (k->id.objID == shapeID))
            {
                // And skip the other vertices from this shape.
                k = k->lstNext;
            }
            continue;
        }
        Point& kPoint = k->point;
        Point& kPrevPoint = k->shPrev->point;

        if (segmentIntersect(pti, ptj, kPrevPoint, kPoint))
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


VertInf *EdgeInf::otherVert(VertInf *vert)
{
    assert((vert == _v1) || (vert == _v2));

    if (vert == _v1)
    {
        return _v2;
    }
    return _v1;
}


EdgeInf *EdgeInf::checkEdgeVisibility(VertInf *i, VertInf *j, bool knownNew)
{
    Router *router = i->_router;
    EdgeInf *edge = NULL;

    if (knownNew)
    {
        assert(existingEdge(i, j) == NULL);
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


EdgeInf *EdgeInf::existingEdge(VertInf *i, VertInf *j)
{
    VertInf *selected = NULL;

    if (i->visListSize <= j->visListSize)
    {
        selected = i;
    }
    else
    {
        selected = j;
    }

    EdgeInfList& visList = selected->visList;
    EdgeInfList::iterator finish = visList.end();
    for (EdgeInfList::iterator edge = visList.begin(); edge != finish;
            ++edge)
    {
        if ((*edge)->isBetween(i, j))
        {
            return (*edge);
        }
    }

    if (i->invisListSize <= j->invisListSize)
    {
        selected = i;
    }
    else
    {
        selected = j;
    }

    EdgeInfList& invisList = selected->invisList;
    finish = invisList.end();
    for (EdgeInfList::iterator edge = invisList.begin(); edge != finish;
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


EdgeList::EdgeList()
    : _firstEdge(NULL)
    , _lastEdge(NULL)
    , _count(0)
{
}


void EdgeList::addEdge(EdgeInf *edge)
{
    if (_firstEdge == NULL)
    {
        assert(_lastEdge == NULL);

        _lastEdge = edge;
        _firstEdge = edge;

        edge->lstPrev = NULL;
        edge->lstNext = NULL;
    }
    else
    {
        assert(_lastEdge != NULL);

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


