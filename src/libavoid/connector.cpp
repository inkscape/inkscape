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

#include "libavoid/connector.h"
#include "libavoid/graph.h"
#include "libavoid/makepath.h"
#include "libavoid/visibility.h"
#include "libavoid/debug.h"


namespace Avoid {

    
ConnRefList connRefs;


ConnRef::ConnRef(const unsigned int id)
    : _id(id)
    , _srcId(0)
    , _dstId(0)
    , _needs_reroute_flag(true)
    , _false_path(false)
    , _active(false)
    , _route_dist(0)
    , _srcVert(NULL)
    , _dstVert(NULL)
    , _initialised(false)
    , _callback(NULL)
    , _connector(NULL)
{
    // TODO: Store endpoints and details.
    _route.pn = 0;
    _route.ps = NULL;
}


ConnRef::ConnRef(const unsigned int id, const Point& src, const Point& dst)
    : _id(id)
    , _srcId(0)
    , _dstId(0)
    , _needs_reroute_flag(true)
    , _false_path(false)
    , _active(false)
    , _route_dist(0)
    , _srcVert(NULL)
    , _dstVert(NULL)
    , _initialised(false)
    , _callback(NULL)
    , _connector(NULL)
{
    _route.pn = 0;
    _route.ps = NULL;

    if (IncludeEndpoints)
    {
        bool isShape = false;
        _srcVert = new VertInf(VertID(id, isShape, 1), src);
        _dstVert = new VertInf(VertID(id, isShape, 2), dst);
        vertices.addVertex(_srcVert);
        vertices.addVertex(_dstVert);
        makeActive();
        _initialised = true;
    }
}


ConnRef::~ConnRef()
{
    freeRoute();

    if (_srcVert)
    {
        vertices.removeVertex(_srcVert);
        delete _srcVert;
        _srcVert = NULL;
    }

    if (_dstVert)
    {
        vertices.removeVertex(_dstVert);
        delete _dstVert;
        _dstVert = NULL;
    }

    if (_active)
    {
        makeInactive();
    }
}

void ConnRef::updateEndPoint(const unsigned int type, const Point& point)
{
    assert((type == (unsigned int) VertID::src) ||
           (type == (unsigned int) VertID::tar));
    //assert(IncludeEndpoints);

    if (!_initialised)
    {
        makeActive();
        _initialised = true;
    }
    
    VertInf *altered = NULL;
    VertInf *partner = NULL;
    bool isShape = false;

    if (type == (unsigned int) VertID::src)
    {
        if (_srcVert)
        {
            _srcVert->Reset(point);
        }
        else
        {
            _srcVert = new VertInf(VertID(_id, isShape, type), point);
            vertices.addVertex(_srcVert);
        }
        
        altered = _srcVert;
        partner = _dstVert;
    }
    else // if (type == (unsigned int) VertID::dst)
    {
        if (_dstVert)
        {
            _dstVert->Reset(point);
        }
        else
        {
            _dstVert = new VertInf(VertID(_id, isShape, type), point);
            vertices.addVertex(_dstVert);
        }
        
        altered = _dstVert;
        partner = _srcVert;
    }

    bool knownNew = false;
    vertexVisibility(altered, partner, knownNew, true);
}


void ConnRef::setEndPointId(const unsigned int type, const unsigned int id)
{
    if (type == (unsigned int) VertID::src)
    {
        _srcId = id;
    }
    else  // if (type == (unsigned int) VertID::dst)
    {
        _dstId = id;
    }
}


void ConnRef::makeActive(void)
{
    assert(!_active);
    
    // Add to connRefs list.
    _pos = connRefs.insert(connRefs.begin(), this);
    _active = true;
}


void ConnRef::makeInactive(void)
{
    assert(_active);
    
    // Remove from connRefs list.
    connRefs.erase(_pos);
    _active = false;
}


void ConnRef::freeRoute(void)
{
    if (_route.ps)
    {
        _route.pn = 0;
        std::free(_route.ps);
        _route.ps = NULL;
    }
}
    

PolyLine& ConnRef::route(void)
{
    return _route;
}


void ConnRef::calcRouteDist(void)
{
    _route_dist = 0;
    for (int i = 1; i < _route.pn; i++)
    {
        _route_dist += dist(_route.ps[i], _route.ps[i - 1]);
    }
}


bool ConnRef::needsReroute(void)
{
    return (_false_path || _needs_reroute_flag);
}


void ConnRef::moveRoute(const int& diff_x, const int& diff_y)
{
    for (int i = 0; i < _route.pn; i++)
    {
        _route.ps[i].x += diff_x;
        _route.ps[i].y += diff_y;
    }
}


void ConnRef::lateSetup(const Point& src, const Point& dst)
{
    assert(!_initialised);

    bool isShape = false;
    _srcVert = new VertInf(VertID(_id, isShape, 1), src);
    _dstVert = new VertInf(VertID(_id, isShape, 2), dst);
    vertices.addVertex(_srcVert);
    vertices.addVertex(_dstVert);
    makeActive();
    _initialised = true;
}


VertInf *ConnRef::src(void)
{
    return _srcVert;
}

    
VertInf *ConnRef::dst(void)
{
    return _dstVert;
}


bool ConnRef::isInitialised(void)
{
    return _initialised;
}


void ConnRef::unInitialise(void)
{
    vertices.removeVertex(_srcVert);
    vertices.removeVertex(_dstVert);
    makeInactive();
    _initialised = false;
}


void ConnRef::removeFromGraph(void)
{
    for (VertInf *iter = _srcVert; iter != NULL; )
    {
        VertInf *tmp = iter;
        iter = (iter == _srcVert) ? _dstVert : NULL;
        
        // For each vertex.
        EdgeInfList& visList = tmp->visList;
        EdgeInfList::iterator finish = visList.end();
        EdgeInfList::iterator edge;
        while ((edge = visList.begin()) != finish)
        {
            // Remove each visibility edge
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


void ConnRef::setCallback(void (*cb)(void *), void *ptr)
{
    _callback = cb;
    _connector = ptr;
}


void ConnRef::handleInvalid(void)
{
    if (_false_path || _needs_reroute_flag) {
        if (_callback) {
            _callback(_connector);
        }
    }
}


void ConnRef::makePathInvalid(void)
{
    _needs_reroute_flag = true;
}


int ConnRef::generatePath(Point p0, Point p1)
{
    if (!_false_path && !_needs_reroute_flag) {
        // This connector is up to date.
        return (int) false;
    }

    _false_path = false;
    _needs_reroute_flag = false;

    VertInf *src = _srcVert;
    VertInf *tar = _dstVert;

    if (!IncludeEndpoints)
    {
        lateSetup(p0, p1);
        
        // Update as they have just been set by lateSetup.
        src = _srcVert;
        tar = _dstVert;
   
        bool knownNew = true;
        vertexVisibility(src, tar, knownNew);
        vertexVisibility(tar, src, knownNew);
    }

    bool *flag = &(_needs_reroute_flag);
    
    makePath(this, flag);
    
    bool result = true;
    
    int pathlen = 1;
    for (VertInf *i = tar; i != src; i = i->pathNext)
    {
        pathlen++;
        if (i == NULL)
        {
            db_printf("Warning: Path not found...\n");
            pathlen = 2;
            tar->pathNext = src;
            if (InvisibilityGrph)
            {
                // TODO:  Could we know this edge already?
                EdgeInf *edge = EdgeInf::existingEdge(src, tar);
                assert(edge != NULL);
                edge->addCycleBlocker();
            }
            result = false;
            break;
        }
        if (pathlen > 100)
        {
            fprintf(stderr, "ERROR: Should never be here...\n");
            exit(1);
        }
    }
    Point *path = (Point *) malloc(pathlen * sizeof(Point));

    int j = pathlen - 1;
    for (VertInf *i = tar; i != src; i = i->pathNext)
    {
        if (InvisibilityGrph)
        {
            // TODO: Again, we could know this edge without searching.
            EdgeInf *edge = EdgeInf::existingEdge(i, i->pathNext);
            edge->addConn(flag);
        }
        else
        {
            _false_path = true;
        }
        path[j--] = i->point;
    }
    path[0] = src->point;


    // Would clear visibility for endpoints here if required.

    PolyLine& output_route = route();
    output_route.pn = pathlen;
    output_route.ps = path;
   
    return (int) result;
}


//============================================================================

const unsigned int ConnRef::runningTo = 1;
const unsigned int ConnRef::runningFrom = 2;
const unsigned int ConnRef::runningToAndFrom =
        ConnRef::runningTo | ConnRef::runningFrom;


void attachedToShape(IntList &conns, const unsigned int shapeId,
        const unsigned int type)
{
    ConnRefList::iterator fin = connRefs.end();
    for (ConnRefList::iterator i = connRefs.begin(); i != fin; ++i) {
        if ((type & ConnRef::runningTo) && ((*i)->_dstId == shapeId)) {
            conns.push_back((*i)->_srcId);
        }
        else if ((type & ConnRef::runningFrom) && ((*i)->_srcId == shapeId)) {
            conns.push_back((*i)->_dstId);
        }
    }
}


    // It's intended this function is called after shape movement has 
    // happened to alert connectors that they need to be rerouted.
void callbackAllInvalidConnectors(void)
{
    ConnRefList::iterator fin = connRefs.end();
    for (ConnRefList::iterator i = connRefs.begin(); i != fin; ++i) {
        (*i)->handleInvalid();
    }
}


}


