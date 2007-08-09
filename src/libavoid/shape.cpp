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

#include <cassert>

#include "libavoid/shape.h"
#include "libavoid/graph.h"  // For alertConns
#include "libavoid/vertices.h"
#include "libavoid/polyutil.h"
#include "libavoid/router.h"


namespace Avoid {


ShapeRef::ShapeRef(Router *router, unsigned int id, Polygn& ply)
    : _router(router)
    , _id(id)
    , _poly(copyPoly(ply))
    , _active(false)
    , _inMoveList(false)
    , _firstVert(NULL)
    , _lastVert(NULL)
{
    bool isShape = true;
    VertID i = VertID(id, isShape, 0);
    
    VertInf *last = NULL;
    VertInf *node = NULL;
    for (int pt_i = 0; pt_i < _poly.pn; pt_i++)
    {
        node = new VertInf(_router, i, _poly.ps[pt_i]);

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
    
    makeActive();
}


ShapeRef::~ShapeRef()
{
    assert(_firstVert != NULL);
    
    makeInactive();

    VertInf *it = _firstVert;
    do
    {
        VertInf *tmp = it;
        it = it->shNext;

        delete tmp;
    }
    while (it != _firstVert);
    _firstVert = _lastVert = NULL;

    freePoly(_poly);
}


void ShapeRef::setNewPoly(Polygn& poly)
{
    assert(_firstVert != NULL);
    assert(_poly.pn == poly.pn);
    
    VertInf *curr = _firstVert;
    for (int pt_i = 0; pt_i < _poly.pn; pt_i++)
    {
        assert(curr->visListSize == 0);
        assert(curr->invisListSize == 0);

        // Reset with the new polygon point.
        curr->Reset(poly.ps[pt_i]);
        curr->pathNext = NULL;
        curr->pathDist = 0;
        
        curr = curr->shNext;
    }
    assert(curr == _firstVert);
        
    freePoly(_poly);
    _poly = copyPoly(poly);
}


void ShapeRef::makeActive(void)
{
    assert(!_active);
    
    // Add to connRefs list.
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
    assert(_active);
    
    // Remove from connRefs list.
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
    

VertInf *ShapeRef::firstVert(void)
{
    return _firstVert;
}


VertInf *ShapeRef::lastVert(void)
{
    return _lastVert;
}


unsigned int ShapeRef::id(void)
{
    return _id;
}


Polygn ShapeRef::poly(void)
{
    return _poly;
}


Router *ShapeRef::router(void)
{
    return _router;
}


void ShapeRef::boundingBox(BBox& bbox)
{
    assert(_poly.pn > 0);

    bbox.a = bbox.b = _poly.ps[0];
    Point& a = bbox.a;
    Point& b = bbox.b;

    for (int i = 1; i < _poly.pn; ++i)
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
        EdgeInfList::iterator finish = visList.end();
        EdgeInfList::iterator edge;
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
        fprintf(stderr, "WARNING: two moves queued for same shape prior to "
                "rerouting.\n         This is not safe.\n");
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


