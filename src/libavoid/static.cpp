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

#include <cassert>
#include "libavoid/connector.h"
#include "libavoid/visibility.h"

namespace Avoid {


// This should only be used for the static algorithm.
//
// XXX: If to set up the vis graph for incremental it would need 
//      the shapeRef ppinters in obs.
//
void CreateVisGraph(Polygn **obs, int n_obs)
{
    for (int poly_i = 0; poly_i < n_obs; poly_i++)
    {
        uint id = obs[poly_i]->id;
        
        new ShapeRef(id, *(obs[poly_i]));
    }
    computeCompleteVis();
}


void DestroyVisGraph(void)
{
    ShapeRefList::iterator sFinish = shapeRefs.end();
    ShapeRefList::iterator sCurr;
    
    while ((sCurr = shapeRefs.begin()) != sFinish)
    {
        ShapeRef *shape = (*sCurr);

        shape->removeFromGraph();
        delete shape;
    }
    
    ConnRefList::iterator cFinish = connRefs.end();
    ConnRefList::iterator cCurr;
    
    while ((cCurr = connRefs.begin())!= cFinish)
    {
        ConnRef *conn = (*cCurr);

        conn->removeFromGraph();
        conn->unInitialise();
    }

    assert(vertices.connsBegin() == NULL);
}


}

