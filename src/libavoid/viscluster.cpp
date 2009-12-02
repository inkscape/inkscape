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


#include "libavoid/viscluster.h"
#include "libavoid/router.h"
#include "libavoid/assertions.h"


namespace Avoid {


ClusterRef::ClusterRef(Router *router, unsigned int id, Polygon& ply)
    : _router(router)
    , _poly(ply, router)
    , _active(false)
{
    _id = router->assignId(id);
}


ClusterRef::~ClusterRef()
{
}


void ClusterRef::makeActive(void)
{
    COLA_ASSERT(!_active);
    
    // Add to connRefs list.
    _pos = _router->clusterRefs.insert(_router->clusterRefs.begin(), this);

    _active = true;
}


void ClusterRef::makeInactive(void)
{
    COLA_ASSERT(_active);
    
    // Remove from connRefs list.
    _router->clusterRefs.erase(_pos);

    _active = false;
}
    

void ClusterRef::setNewPoly(Polygon& poly)
{
    _poly = ReferencingPolygon(poly, _router);
}


unsigned int ClusterRef::id(void)
{
    return _id;
}


ReferencingPolygon& ClusterRef::polygon(void)
{
    return _poly;
}


Router *ClusterRef::router(void)
{
    return _router;
}


}


