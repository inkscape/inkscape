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

//! @file    shape.h
//! @brief   Contains the interface for the ShapeRef class.


#ifndef AVOID_SHAPE_H
#define AVOID_SHAPE_H

#include "libavoid/geometry.h"
#include <list>


namespace Avoid {

class VertInf;
class Router;
class ShapeRef;
typedef std::list<ShapeRef *> ShapeRefList;


//! @brief   The ShapeRef class represents a shape object.
//!
//! Shapes are obstacles that connectors must be routed around.  They can be 
//! placed into a Router scene and can be repositioned or resized (via
//! Router::moveShape()).
//! 
//! Usually, it is expected that you would create a ShapeRef for each shape 
//! in your diagram and keep that reference in your own shape class.
//!
class ShapeRef
{
    public:
        //! @brief  Shape reference constructor.
        //!
        //! Creates a shape obect reference, but does not yet place it into the
        //! Router scene.
        //!
        //! The poly argument will usually be the boundary of the shape in your 
        //! application with additional buffer of several pixels on each side.
        //! Specifying such a buffer results in connectors leaving a small 
        //! amount of space around shapes, rather than touching them on the 
        //! corners or edges.
        //!
        //! If an ID is not specified, then one will be assigned to the shape.
        //! If assigning an ID yourself, note that it should be a unique 
        //! positive integer.  Also, IDs are given to all objects in a scene,
        //! so the same ID cannot be given to a shape and a connector for 
        //! example.
        //!
        //! @param[in]  router  The router scene to place the shape into.
        //! @param[in]  poly    A Polygon representing the boundary of the 
        //!                     shape.
        //! @param[in]  id      A unique positive integer ID for the shape.  
        ShapeRef(Router *router, Polygon& poly, const unsigned int id = 0);
        //! @brief  Shape reference destructor.
        //!
        //! This will call Router::removeShape() for this shape, if this has
        //! not already be called.
        ~ShapeRef();
        
        //! @brief   Returns the ID of this shape.
        //! @returns The ID of the shape. 
        unsigned int id(void) const;
        //! @brief   Returns a reference to the polygon boundary of this shape.
        //! @returns A reference to the polygon boundary of the shape.
        const Polygon& polygon(void) const;
        //! @brief   Returns a pointer to the router scene this shape is in.
        //! @returns A pointer to the router scene for this shape.
        Router *router(void) const;
        
        void setNewPoly(const Polygon& poly);
        VertInf *firstVert(void);
        VertInf *lastVert(void);
        void boundingBox(BBox& bbox);

        void makeActive(void);
        void makeInactive(void);
        bool isActive(void) const;

        void removeFromGraph(void);
        void markForMove(void);
        void clearMoveMark(void);

        VertInf *getPointVertex(const Point& point);

    private:
        Router *_router;
        unsigned int _id;
        Polygon _poly;
        bool _active;
        bool _inMoveList;
        ShapeRefList::iterator _pos;
        VertInf *_firstVert;
        VertInf *_lastVert;
};


}


#endif


