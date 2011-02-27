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

//! @file  geomtypes.h
//! @brief Contains the interface for various geometry types and classes.


#ifndef AVOID_GEOMTYPES_H
#define AVOID_GEOMTYPES_H

#include <cstddef>
#include <vector>
#include <utility>


namespace Avoid
{


//! @brief  The Point class defines a point in the plane.
//!
//! Points consist of an x and y value.  They may also have an ID and vertex
//! number associated with them.
//!
class Point
{
    public:
        //! @brief  Default constructor.
        //!
        Point();
        //! @brief  Standard constructor.
        //!
        //! @param[in]  xv  The x position of the point.
        //! @param[in]  yv  The y position of the point.
        //!
        Point(const double xv, const double yv);

        //! @brief  Comparison operator. Returns true if at same position.
        //!
        //! @param[in]  rhs  The point to compare with this one.
        //! @return          The result of the comparison.
        //!
        bool operator==(const Point& rhs) const;
        //! @brief  Comparison operator. Returns true if at different positions.
        //!
        //! @param[in]  rhs  The point to compare with this one.
        //! @return          The result of the comparison.
        //!
        bool operator!=(const Point& rhs) const;
        //! @brief  Comparison operator. Returns true if less-then rhs point.
        //!
        //! @note  This operator is not particularly useful, but is defined 
        //!        to allow std::set<Point>.
        //!
        //! @param[in]  rhs  The point to compare with this one.
        //! @return          The result of the comparison.
        //!
        bool operator<(const Point& rhs) const;

        //! @brief  Returns the x or y value of the point, given the dimension.
        //!
        //! @param[in]  dimension  The dimension:  0 for x, 1 for y.
        //! @return                The component of the point in that dimension.
        double& operator[](const unsigned int dimension);
        const double& operator[](const unsigned int dimension) const;
        
        //! The x position.
        double x;
        //! The y position.
        double y;
        //! The ID associated with this point.
        unsigned int id;
        //! The vertex number associated with this point.
        unsigned short vn;

};


//! Constant value representing an unassigned vertex number.
//!
static const unsigned short kUnassignedVertexNumber = 8;


//! @brief  A vector, represented by the Point class.
//!
typedef Point Vector;


//! @brief  A common interface used by the Polygon classes.
//!
class PolygonInterface
{
    public:
        //! @brief  Constructor.
        PolygonInterface() { }
        //! @brief  Destructor.
        virtual ~PolygonInterface() { }
        //! @brief  Resets this to the empty polygon.
        virtual void clear(void) = 0;
        //! @brief  Returns true if this polygon is empty.
        virtual bool empty(void) const = 0;
        //! @brief  Returns the number of points in this polygon.
        virtual size_t size(void) const = 0;
        //! @brief  Returns the ID value associated with this polygon.
        virtual int id(void) const = 0;
        //! @brief  Returns a specific point in the polygon.
        //! @param[in]  index  The array index of the point to be returned.
        virtual const Point& at(size_t index) const = 0;
        //! @brief  Returns the bounding rectangle that contains this polygon.
        //!
        //! If a NULL pointer is passed for any of the arguments, then that
        //! value is ignored and not returned.
        //!
        //! @param[out]  minX  The left hand side of the bounding box.
        //! @param[out]  minY  The top of the bounding box.
        //! @param[out]  maxX  The right hand side of the bounding box.
        //! @param[out]  maxY  The bottom of the bounding box.
        void getBoundingRect(double *minX, double *minY,
                double *maxX, double *maxY) const;
};


//! @brief  A line between two points. 
//!
class Edge
{
    public:
        //! The first point.
        Point a;
        //! The second point.
        Point b;
};


//! @brief  A bounding box, represented with an Edge between top-left and
//!         bottom-right corners.
//!
typedef Edge BBox;


class Router;
class ReferencingPolygon;


//! @brief  A dynamic Polygon, to which points can be easily added and removed.
//!
//! @note The Rectangle class can be used as an easy way of constructing a
//!       square or rectangular polygon.
//!
class Polygon : public PolygonInterface
{
    public:
        //! @brief  Constructs an empty polygon (with zero points). 
        Polygon();
        //! @brief  Constructs a new polygon with n points.
        //! 
        //! A rectangle would be comprised of four point.  An n segment 
        //! PolyLine (represented as a Polygon) would be comprised of n+1
        //! points.  Whether a particular Polygon is closed or not, depends
        //! on whether it is a Polygon or Polyline.  Shape polygons are always
        //! considered to be closed, meaning the last point joins back to the
        //! first point.
        //!
        //! @param[in]  n  Number of points in the polygon.
        //!
        Polygon(const int n);
        //! @brief  Constructs a new polygon from an existing Polygon.
        //!
        //! @param[in]  poly  An existing polygon to copy the new polygon from.
        //!
        Polygon(const PolygonInterface& poly);
        //! @brief  Resets this to the empty polygon.
        void clear(void);
        //! @brief  Returns true if this polygon is empty.
        bool empty(void) const;
        //! @brief  Returns the number of points in this polygon.
        size_t size(void) const;
        //! @brief  Returns the ID value associated with this polygon.
        int id(void) const;
        //! @brief  Returns a specific point in the polygon.
        //! @param[in]  index  The array index of the point to be returned.
        const Point& at(size_t index) const;
        //! @brief  Returns a simplified Polyline, where all collinear line
        //!         segments have been collapsed down into single line 
        //!         segments.
        //!
        //! @return A new polyline with a simplified representation.
        //!
        Polygon simplify(void) const;
        //! @brief  Returns a curved approximation of this multi-segment 
        //!         PolyLine, with the corners replaced by smooth Bezier 
        //!         curves.
        //!
        //! This function does not do any further obstacle avoidance with the
        //! curves produced.  Hence, you would usually specify a curve_amount
        //! in similar size to the space buffer around obstacles in the scene.
        //! This way the curves will cut the corners around shapes but still
        //! run within this buffer space.
        //!
        //! @param  curve_amount  Describes the distance along the end of each 
        //!                       line segment to turn into a curve.
        //! @param  closed        Describes whether the Polygon should be 
        //!                       treated as closed.  Defaults to false.
        //! @return A new polyline (polygon) representing the curved path.
        //!         Its points represent endpoints of line segments and 
        //!         Bezier spline control points.  The Polygon::ts vector for
        //!         this returned polygon is populated with a character for 
        //!         each point describing its type.
        //! @sa     ts
        Polygon curvedPolyline(const double curve_amount, 
                const bool closed = false) const;
        //! @brief  Translates the polygon position by a relative amount.
        //!
        //! @param[in]  xDist  Distance to move polygon in the x dimension.
        //! @param[in]  yDist  Distance to move polygon in the y dimension.
        void translate(const double xDist, const double yDist);

        //! @brief  An ID for the polygon.
        int _id;
        //! @brief  A vector of the points that make up the Polygon.
        std::vector<Point> ps;
        //! @brief  If used, denotes whether the corresponding point in ps is 
        //!         a move-to operation or a Bezier curve-to.
        //! 
        //! Each character describes the drawing operation for the 
        //! corresponding point in the ps vector.  Possible values are:
        //!  -  'M': A moveto operation, marks the first point;
        //!  -  'L': A lineto operation, is a line from the previous point to
        //!     the current point; or
        //!  -  'C': A curveto operation, three consecutive 'C' points 
        //!     (along with the previous point) describe the control points 
        //!     of a Bezier curve.
        //!  -  'Z': Closes the path (used for cluster boundaries).
        //!
        //! @note   This vector will currently only be populated for polygons 
        //!         returned by curvedPolyline().  
        std::vector<char> ts;
};


//! @brief  A multi-segment line, represented with the Polygon class.
//!
typedef Polygon PolyLine;


//! @brief  A Polygon which just references its points from other Polygons.
//!          
//! This type of Polygon is used to accurately represent cluster boundaries 
//! made up from the corner points of shapes.
//!
class ReferencingPolygon : public PolygonInterface
{
    public:
        ReferencingPolygon();
        ReferencingPolygon(const Polygon& poly, const Router *router);
        void clear(void);
        bool empty(void) const;
        size_t size(void) const;
        int id(void) const;
        const Point& at(size_t index) const;

        int _id;
        std::vector<std::pair<const Polygon *, unsigned short> > ps;
};


//! @brief  A Rectangle, a simpler way to define the polygon for square or
//!         rectangular shapes.
//!
class Rectangle : public Polygon
{
    public:
        //! @brief  Constructs a rectangular polygon given two opposing 
        //!         corner points.
        //!
        //! @param[in]  topLeft      The first corner point of the rectangle.
        //! @param[in]  bottomRight  The opposing corner point of the rectangle.
        //!
        Rectangle(const Point& topLeft, const Point& bottomRight);
        
        //! @brief  Constructs a rectangular polygon given the centre, width
        //!         and height.
        //!
        //! @param[in]  centre  The centre of the rectangle, specified as 
        //!                     a point.
        //! @param[in]  width   The width of the rectangle.
        //! @param[in]  height  The height of the rectangle.
        //!
        Rectangle(const Point& centre, const double width, const double height);
};


}

#endif
