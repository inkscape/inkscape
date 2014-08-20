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


#ifndef AVOID_CLUSTER_H
#define AVOID_CLUSTER_H

#include <list>

#include "libavoid/geometry.h"


namespace Avoid {

class Router;
class ClusterRef;
typedef std::list<ClusterRef *> ClusterRefList;


class ClusterRef
{
    public:
        ClusterRef(Router *router, unsigned int id, Polygon& poly);
        ~ClusterRef();
        void setNewPoly(Polygon& poly);
        unsigned int id(void);
        ReferencingPolygon& polygon(void);
        Router *router(void);
        void makeActive(void);
        void makeInactive(void);

    private:
        Router *_router;
        unsigned int _id;
        ReferencingPolygon _poly;
        bool _active;
        ClusterRefList::iterator _pos;
};


}


#endif


