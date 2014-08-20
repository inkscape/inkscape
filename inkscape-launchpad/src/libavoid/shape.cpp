/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2008  Monash University
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


#include "libavoid/shape.h"
#include "libavoid/graph.h"  // For alertConns
#include "libavoid/vertices.h"
#include "libavoid/router.h"
#include "libavoid/debug.h"
#include "libavoid/assertions.h"


namespace Avoid {


ShapeRef::ShapeRef(Router *router, Polygon& ply, const unsigned int id)
    : _router(router)
    , _poly(ply)
    , _active(false)
    , _inMoveList(false)
    , _firstVert(NULL)
    , _lastVert(NULL)
{
    _id = router->assignId(id);

    bool isShape = true;
    VertID i = VertID(_id, isShape, 0);
    
    const bool addToRouterNow = false;
    VertInf *last = NULL;
    VertInf *node = NULL;
    for (size_t pt_i = 0; pt_i < _poly.size(); ++pt_i)
    {
        node = new VertInf(_router, i, _poly.ps[pt_i], addToRouterNow);

        if (!_firstVert)
        {
            _firstVert = node;
        }
        else
        {
            node->shPrev = last;
            last->shNext = node;
            //node->lstPrev = last;
            //last->lstNext = node;
        }
        
        last = node;
        i++;
    }
    _lastVert = node;
    
    _lastVert->shNext = _firstVert;
    _firstVert->shPrev = _lastVert;
}


ShapeRef::~ShapeRef()
{
    COLA_ASSERT(!_router->shapeInQueuedActionList(this));

    if (_active)
    {
        // Destroying a shape without calling removeShape(), so do it now.
        _router->removeShape(this);
        _router->processTransaction();
    }

    COLA_ASSERT(_firstVert != NULL);
    
    VertInf *it = _firstVert;
    do
    {
        VertInf *tmp = it;
        it = it->shNext;

        delete tmp;
    }
    while (it != _firstVert);
    _firstVert = _lastVert = NULL;
}


void ShapeRef::setNewPoly(const Polygon& poly)
{
    COLA_ASSERT(_firstVert != NULL);
    COLA_ASSERT(_poly.size() == poly.size());
    
    VertInf *curr = _firstVert;
    for (size_t pt_i = 0; pt_i < _poly.size(); ++pt_i)
    {
        COLA_ASSERT(curr->visListSize == 0);
        COLA_ASSERT(curr->invisListSize == 0);

        // Reset with the new polygon point.
        curr->Reset(poly.ps[pt_i]);
        curr->pathNext = NULL;
        
        curr = curr->shNext;
    }
    COLA_ASSERT(curr == _firstVert);
        
    _poly = poly;
}


void ShapeRef::makeActive(void)
{
    COLA_ASSERT(!_active);
    
    // Add to shapeRefs list.
    _pos = _router->shapeRefs.insert(_router->shapeRefs.begin(), this);

    // Add points to vertex list.
    VertInf *it = _firstVert;
    do
    {
        VertInf *tmp = it;
        it = it->shNext;

        _router->vertices.addVertex(tmp);
    }
    while (it != _firstVert);
    
    _active = true;
}


void ShapeRef::makeInactive(void)
{
    COLA_ASSERT(_active);
    
    // Remove from shapeRefs list.
    _router->shapeRefs.erase(_pos);

    // Remove points from vertex list.
    VertInf *it = _firstVert;
    do
    {
        VertInf *tmp = it;
        it = it->shNext;

        _router->vertices.removeVertex(tmp);
    }
    while (it != _firstVert);
    
    _active = false;
}


bool ShapeRef::isActive(void) const
{
    return _active;
}


VertInf *ShapeRef::firstVert(void)
{
    return _firstVert;
}


VertInf *ShapeRef::lastVert(void)
{
    return _lastVert;
}


unsigned int ShapeRef::id(void) const
{
    return _id;
}


const Polygon& ShapeRef::polygon(void) const
{
    return _poly;
}


Router *ShapeRef::router(void) const
{
    return _router;
}


void ShapeRef::boundingBox(BBox& bbox)
{
    COLA_ASSERT(!_poly.empty());

    bbox.a = bbox.b = _poly.ps[0];
    Point& a = bbox.a;
    Point& b = bbox.b;

    for (size_t i = 1; i < _poly.size(); ++i)
    {
        const Point& p = _poly.ps[i];

        a.x = std::min(p.x, a.x);
        a.y = std::min(p.y, a.y);
        b.x = std::max(p.x, b.x);
        b.y = std::max(p.y, b.y);
    }
}


void ShapeRef::removeFromGraph(void)
{
    for (VertInf *iter = firstVert(); iter != lastVert()->lstNext; )
    {
        VertInf *tmp = iter;
        iter = iter->lstNext;
        
        // For each vertex.
        EdgeInfList& visList = tmp->visList;
        EdgeInfList::const_iterator finish = visList.end();
        EdgeInfList::const_iterator edge;
        while ((edge = visList.begin()) != finish)
        {
            // Remove each visibility edge
            (*edge)->alertConns();
            delete (*edge);
        }

        EdgeInfList& invisList = tmp->invisList;
        finish = invisList.end();
        while ((edge = invisList.begin()) != finish)
        {
            // Remove each invisibility edge
            delete (*edge);
        }

        EdgeInfList& orthogList = tmp->orthogVisList;
        finish = orthogList.end();
        while ((edge = orthogList.begin()) != finish)
        {
            // Remove each orthogonal visibility edge
            (*edge)->alertConns();
            delete (*edge);
        }
    }
}


void ShapeRef::markForMove(void)
{
    if (!_inMoveList)
    {
        _inMoveList = true;
    }
    else
    {
        db_printf("WARNING: two moves queued for same shape prior to rerouting."
                "\n         This is not safe.\n");
    }
}


void ShapeRef::clearMoveMark(void)
{
    _inMoveList = false;
}


VertInf *ShapeRef::getPointVertex(const Point& point)
{
    VertInf *curr = _firstVert;
    do
    {
        if (curr->point == point)
        {
            return curr;
        }
        curr = curr->shNext;
    }
    while (curr != _firstVert);

    return NULL;
}


}


