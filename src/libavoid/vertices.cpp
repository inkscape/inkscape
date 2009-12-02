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


#include <iostream>
#include <cstdlib>

#include "libavoid/vertices.h"
#include "libavoid/geometry.h"
#include "libavoid/graph.h"  // For alertConns
#include "libavoid/debug.h"
#include "libavoid/router.h"
#include "libavoid/assertions.h"

using std::ostream;


namespace Avoid {


VertID::VertID()
{
}


VertID::VertID(unsigned int id, bool s, int n)
    : objID(id)
    , isShape(s)
    , vn(n)
{
}


VertID::VertID(const VertID& other)
    : objID(other.objID)
    , isShape(other.isShape)
    , vn(other.vn)
{
}


VertID& VertID::operator= (const VertID& rhs)
{
    // Gracefully handle self assignment
    //if (this == &rhs) return *this;

    objID = rhs.objID;
    isShape = rhs.isShape;
    vn = rhs.vn;

    return *this;
}


bool VertID::operator==(const VertID& rhs) const
{
    if ((objID != rhs.objID) || (vn != rhs.vn))
    {
        return false;
    }
    // XXX RubberBand search breaks this:
    // COLA_ASSERT(isShape == rhs.isShape);
    return true;
}


bool VertID::operator!=(const VertID& rhs) const
{
    if ((objID != rhs.objID) || (vn != rhs.vn))
    {
        return true;
    }
    COLA_ASSERT(isShape == rhs.isShape);
    return false;
}


bool VertID::operator<(const VertID& rhs) const
{
    if ((objID < rhs.objID) ||
            ((objID == rhs.objID) && (vn < rhs.vn)))
    {
        return true;
    }
    return false;
}


VertID VertID::operator+(const int& rhs) const
{
    return VertID(objID, isShape, vn + rhs);
}


VertID VertID::operator-(const int& rhs) const
{
    return VertID(objID, isShape, vn - rhs);
}


VertID& VertID::operator++(int)
{
    vn += 1;
    return *this;
}


void VertID::print(FILE *file) const
{
    fprintf(file, "[%u,%d]", objID, vn);
}

void VertID::db_print(void) const
{
    db_printf("[%u,%d]", objID, vn);
}


const unsigned short VertID::src = 1;
const unsigned short VertID::tar = 2;


ostream& operator<<(ostream& os, const VertID& vID)
{
    return os << '[' << vID.objID << ',' << vID.vn << ']';
}



VertInf::VertInf(Router *router, const VertID& vid, const Point& vpoint, 
        const bool addToRouter)
    : _router(router),
      id(vid),
      point(vpoint),
      lstPrev(NULL),
      lstNext(NULL),
      shPrev(NULL),
      shNext(NULL),
      visListSize(0),
      orthogVisListSize(0),
      invisListSize(0),
      pathNext(NULL),
      visDirections(ConnDirNone)
{
    point.id = vid.objID;
    point.vn = vid.vn;

    if (addToRouter)
    {
        _router->vertices.addVertex(this);
    }
}


VertInf::~VertInf()
{
}


void VertInf::Reset(const VertID& vid, const Point& vpoint)
{
    id = vid;
    point = vpoint;
    point.id = id.objID;
    point.vn = id.vn;
}


void VertInf::Reset(const Point& vpoint)
{
    point = vpoint;
    point.id = id.objID;
    point.vn = id.vn;
}


// Returns true if this vertex is not involved in any (in)visibility graphs.
bool VertInf::orphaned(void)
{
    return (visList.empty() && invisList.empty() && orthogVisList.empty());
}


void VertInf::removeFromGraph(const bool isConnVert)
{
    if (isConnVert)
    {
        COLA_ASSERT(!(id.isShape));
    }

    // For each vertex.
    EdgeInfList::const_iterator finish = visList.end();
    EdgeInfList::const_iterator edge;
    while ((edge = visList.begin()) != finish)
    {
        // Remove each visibility edge
        (*edge)->alertConns();
        delete (*edge);
    }

    finish = orthogVisList.end();
    while ((edge = orthogVisList.begin()) != finish)
    {
        // Remove each orthogonal visibility edge.
        (*edge)->alertConns();
        delete (*edge);
    }

    finish = invisList.end();
    while ((edge = invisList.begin()) != finish)
    {
        // Remove each invisibility edge
        delete (*edge);
    }
}


bool directVis(VertInf *src, VertInf *dst)
{
    ShapeSet ss = ShapeSet();

    Point& p = src->point;
    Point& q = dst->point;

    VertID& pID = src->id;
    VertID& qID = dst->id;

    // We better be part of the same instance of libavoid.
    Router *router = src->_router;
    COLA_ASSERT(router == dst->_router);

    ContainsMap& contains = router->contains;
    if (!(pID.isShape))
    {
        ss.insert(contains[pID].begin(), contains[pID].end());
    }
    if (!(qID.isShape))
    {
        ss.insert(contains[qID].begin(), contains[qID].end());
    }

    // The "beginning" should be the first shape vertex, rather
    // than an endpoint, which are also stored in "vertices".
    VertInf *endVert = router->vertices.end();
    for (VertInf *k = router->vertices.shapesBegin(); k != endVert;
            k = k->lstNext)
    {
        if ((ss.find(k->id.objID) == ss.end()))
        {
            if (segmentIntersect(p, q, k->point, k->shNext->point))
            {
                return false;
            }
        }
    }
    return true;
}


VertInfList::VertInfList()
    : _firstShapeVert(NULL),
      _firstConnVert(NULL),
      _lastShapeVert(NULL),
      _lastConnVert(NULL),
      _shapeVertices(0),
      _connVertices(0)
{
}


#define checkVertInfListConditions() \
        do { \
            COLA_ASSERT((!_firstConnVert && (_connVertices == 0)) || \
                    ((_firstConnVert->lstPrev == NULL) && (_connVertices > 0))); \
            COLA_ASSERT((!_firstShapeVert && (_shapeVertices == 0)) || \
                    ((_firstShapeVert->lstPrev == NULL) && (_shapeVertices > 0))); \
            COLA_ASSERT(!_lastShapeVert || (_lastShapeVert->lstNext == NULL)); \
            COLA_ASSERT(!_lastConnVert || (_lastConnVert->lstNext == _firstShapeVert)); \
            COLA_ASSERT((!_firstConnVert && !_lastConnVert) || \
                    (_firstConnVert &&  _lastConnVert) ); \
            COLA_ASSERT((!_firstShapeVert && !_lastShapeVert) || \
                    (_firstShapeVert &&  _lastShapeVert) ); \
            COLA_ASSERT(!_firstShapeVert || _firstShapeVert->id.isShape); \
            COLA_ASSERT(!_lastShapeVert || _lastShapeVert->id.isShape); \
            COLA_ASSERT(!_firstConnVert || !(_firstConnVert->id.isShape)); \
            COLA_ASSERT(!_lastConnVert || !(_lastConnVert->id.isShape)); \
        } while(0)


void VertInfList::addVertex(VertInf *vert)
{
    checkVertInfListConditions();
    COLA_ASSERT(vert->lstPrev == NULL);
    COLA_ASSERT(vert->lstNext == NULL);

    if (!(vert->id.isShape))
    {
        // A Connector vertex
        if (_firstConnVert)
        {
            // Join with previous front
            vert->lstNext = _firstConnVert;
            _firstConnVert->lstPrev = vert;

            // Make front
            _firstConnVert = vert;
        }
        else
        {
            // Make front and back
            _firstConnVert = vert;
            _lastConnVert = vert;

            // Link to front of shapes list
            vert->lstNext = _firstShapeVert;
        }
        _connVertices++;
    }
    else // if (vert->id.shape > 0)
    {
        // A Shape vertex
        if (_lastShapeVert)
        {
            // Join with previous back
            vert->lstPrev = _lastShapeVert;
            _lastShapeVert->lstNext = vert;

            // Make back
            _lastShapeVert = vert;
        }
        else
        {
            // Make first and last
            _firstShapeVert = vert;
            _lastShapeVert = vert;

            // Join with conns list
            if (_lastConnVert)
            {
                assert (_lastConnVert->lstNext == NULL);

                _lastConnVert->lstNext = vert;
            }
        }
        _shapeVertices++;
    }
    checkVertInfListConditions();
}


// Removes a vertex from the list and returns a pointer to the vertex
// following the removed one.
VertInf *VertInfList::removeVertex(VertInf *vert)
{
    if (vert == NULL)
    {
        return NULL;
    }
    // Conditions for correct data structure
    checkVertInfListConditions();
    
    VertInf *following = vert->lstNext;

    if (!(vert->id.isShape))
    {
        // A Connector vertex
        if (vert == _firstConnVert)
        {

            if (vert == _lastConnVert)
            {
                _firstConnVert = NULL;
                _lastConnVert = NULL;
            }
            else
            {
                // Set new first
                _firstConnVert = _firstConnVert->lstNext;

                if (_firstConnVert)
                {
                    // Set previous
                    _firstConnVert->lstPrev = NULL;
                }
            }
        }
        else if (vert == _lastConnVert)
        {
            // Set new last
            _lastConnVert = _lastConnVert->lstPrev;

            // Make last point to shapes list
            _lastConnVert->lstNext = _firstShapeVert;
        }
        else
        {
            vert->lstNext->lstPrev = vert->lstPrev;
            vert->lstPrev->lstNext = vert->lstNext;
        }
        _connVertices--;
    }
    else // if (vert->id.shape > 0)
    {
        // A Shape vertex
        if (vert == _lastShapeVert)
        {
            // Set new last
            _lastShapeVert = _lastShapeVert->lstPrev;

            if (vert == _firstShapeVert)
            {
                _firstShapeVert = NULL;
                if (_lastConnVert)
                {
                    _lastConnVert->lstNext = NULL;
                }
            }

            if (_lastShapeVert)
            {
                _lastShapeVert->lstNext = NULL;
            }
        }
        else if (vert == _firstShapeVert)
        {
            // Set new first
            _firstShapeVert = _firstShapeVert->lstNext;

            // Correct the last conn vertex
            if (_lastConnVert)
            {
                _lastConnVert->lstNext = _firstShapeVert;
            }

            if (_firstShapeVert)
            {
                _firstShapeVert->lstPrev = NULL;
            }
        }
        else
        {
            vert->lstNext->lstPrev = vert->lstPrev;
            vert->lstPrev->lstNext = vert->lstNext;
        }
        _shapeVertices--;
    }
    vert->lstPrev = NULL;
    vert->lstNext = NULL;

    checkVertInfListConditions();

    return following;
}


VertInf *VertInfList::getVertexByID(const VertID& id)
{
    VertID searchID = id;
    if (searchID.vn == kUnassignedVertexNumber)
    {
        unsigned int topbit = ((unsigned int) 1) << 31;
        if (searchID.objID & topbit)
        {
            searchID.objID = searchID.objID & ~topbit;
            searchID.vn = VertID::src;
        }
        else
        {
            searchID.vn = VertID::tar;
        }
    }
    VertInf *last = end();
    for (VertInf *curr = connsBegin(); curr != last; curr = curr->lstNext)
    {
        if (curr->id == searchID)
        {
            return curr;
        }
    }
    return NULL;
}


VertInf *VertInfList::getVertexByPos(const Point& p)
{
    VertInf *last = end();
    for (VertInf *curr = shapesBegin(); curr != last; curr = curr->lstNext)
    {
        if (curr->point == p)
        {
            return curr;
        }
    }
    return NULL;
}


VertInf *VertInfList::shapesBegin(void)
{
    return _firstShapeVert;
}


VertInf *VertInfList::connsBegin(void)
{
    if (_firstConnVert)
    {
        return _firstConnVert;
    }
    // No connector vertices
    return _firstShapeVert;
}


VertInf *VertInfList::end(void)
{
    return NULL;
}


unsigned int VertInfList::connsSize(void) const
{
    return _connVertices;
}


unsigned int VertInfList::shapesSize(void) const
{
    return _shapeVertices;
}


}


