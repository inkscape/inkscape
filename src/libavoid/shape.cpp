/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2004-2005  Michael Wybrow <mjwybrow@users.sourceforge.net>
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


#include "libavoid/graph.h"  // For alertConns
#include "libavoid/polyutil.h"


namespace Avoid {


ShapeRefList shapeRefs;


ShapeRef::ShapeRef(unsigned int id, Polygn& ply)
    : _id(id)
    , _poly(copyPoly(ply))
    , _active(false)
    , _firstVert(NULL)
    , _lastVert(NULL)
{
    bool isShape = true;
    VertID i = VertID(id, isShape, 0);
    
    VertInf *last = NULL;
    VertInf *node = NULL;
    for (int pt_i = 0; pt_i < _poly.pn; pt_i++)
    {
        node = new VertInf(i, _poly.ps[pt_i]);

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
        vertices.addVertex(node);
        
        last = node;
        i++;
        // Increase total vertices count ++;
    }
    _lastVert = node;
    
    _lastVert->shNext = _firstVert;
    _firstVert->shPrev = _lastVert;
    
    // Increase total shape count ++;
    makeActive();
}


ShapeRef::~ShapeRef()
{
    assert(_firstVert != NULL);
    
    VertInf *it = _firstVert;
    do
    {
        VertInf *tmp = it;
        it = it->shNext;

        // XXX: This could possibly be done less
        //      safely but faster, all at once.
        vertices.removeVertex(tmp);
        delete tmp;
    }
    while (it != _firstVert);
    _firstVert = _lastVert = NULL;

    freePoly(_poly);
    
    makeInactive();
}


void ShapeRef::makeActive(void)
{
    assert(!_active);
    
    // Add to connRefs list.
    _pos = shapeRefs.insert(shapeRefs.begin(), this);
    _active = true;
}


void ShapeRef::makeInactive(void)
{
    assert(_active);
    
    // Remove from connRefs list.
    shapeRefs.erase(_pos);
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


}


