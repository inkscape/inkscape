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
#include "libavoid/visibility.h"

namespace Avoid {


void addShape(ShapeRef *shape)
{
    uint pid = shape->id();
    Polygn poly = shape->poly();
    
    adjustContainsWithAdd(poly, pid);
    
    // o  Check all visibility edges to see if this one shape
    //    blocks them.
    newBlockingShape(&poly, pid);

    // o  Calculate visibility for the new vertices.
    if (UseLeesAlgorithm)
    {
        shapeVisSweep(shape);
    }
    else
    {
        shapeVis(shape);
    }
    callbackAllInvalidConnectors();
}


void delShape(ShapeRef *shape)
{
    uint pid = shape->id();

    // o  Remove entries related to this shape's vertices
    shape->removeFromGraph();
    
    if (SelectiveReroute)
    {
        markConnectors(shape);
    }

    adjustContainsWithDel(pid);
    
    delete shape;
    
    // o  Check all edges that were blocked by this shape.
    if (InvisibilityGrph)
    {
        checkAllBlockedEdges(pid);
    }
    else
    {
        // check all edges not in graph
        checkAllMissingEdges();
    }
    callbackAllInvalidConnectors();
}


ShapeRef *moveShape(ShapeRef *oldShape, Polygn *newPoly)
{
    uint pid = oldShape->id();
    
    // o  Remove entries related to this shape's vertices
    oldShape->removeFromGraph();
    
    if (SelectiveReroute && !(PartialFeedback && PartialTime))
    {
        markConnectors(oldShape);
    }

    adjustContainsWithDel(pid);
    
    delete oldShape;
    oldShape = NULL;

    adjustContainsWithAdd(*newPoly, pid);
    
    // o  Check all edges that were blocked by this shape.
    if (InvisibilityGrph)
    {
        checkAllBlockedEdges(pid);
    }
    else
    {
        // check all edges not in graph
        checkAllMissingEdges();
    }
    
    ShapeRef *newShape = new ShapeRef(pid, *newPoly);

    // o  Check all visibility edges to see if this one shape
    //    blocks them.
    if (!(PartialFeedback && PartialTime))
    {
        newBlockingShape(newPoly, pid);
    }

    // o  Calculate visibility for the new vertices.
    if (UseLeesAlgorithm)
    {
        shapeVisSweep(newShape);
    }
    else
    {
        shapeVis(newShape);
    }
    callbackAllInvalidConnectors();

    return newShape;
}


}

