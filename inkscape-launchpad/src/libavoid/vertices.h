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


#ifndef AVOID_VERTICES_H
#define AVOID_VERTICES_H

#include <list>
#include <set>
#include <map>
#include <iostream>
#include <cstdio>

#include "libavoid/geomtypes.h"

namespace Avoid {

class EdgeInf;
class Router;

typedef std::list<EdgeInf *> EdgeInfList;

typedef unsigned int ConnDirFlags;


class VertID
{
    public:
        unsigned int objID;
        bool isShape;
        unsigned short vn;

        static const unsigned short src;
        static const unsigned short tar;

        VertID();
        VertID(unsigned int id, bool s, int n);
        VertID(const VertID& other);
        VertID& operator= (const VertID& rhs);
        bool operator==(const VertID& rhs) const;
        bool operator!=(const VertID& rhs) const;
        bool operator<(const VertID& rhs) const;
        VertID operator+(const int& rhs) const;
        VertID operator-(const int& rhs) const;
        VertID& operator++(int);
        void print(FILE *file = stdout) const;
        void db_print(void) const;
        friend std::ostream& operator<<(std::ostream& os, const VertID& vID);
};


// An ID given to all dummy vertices inserted to allow creation of the
// orthogonal visibility graph since the vertices in the orthogonal graph 
// mostly do not correspond to shape corners or connector endpoints.
//
static const VertID dummyOrthogID(0, true, 0);


class VertInf
{
    public:
        VertInf(Router *router, const VertID& vid, const Point& vpoint,
                const bool addToRouter = true);
        ~VertInf();
        void Reset(const VertID& vid, const Point& vpoint);
        void Reset(const Point& vpoint);
        void removeFromGraph(const bool isConnVert = true);
        bool orphaned(void);

        Router *_router;
        VertID id;
        Point  point;
        VertInf *lstPrev;
        VertInf *lstNext;
        VertInf *shPrev;
        VertInf *shNext;
        EdgeInfList visList;
        unsigned int visListSize;
        EdgeInfList orthogVisList;
        unsigned int orthogVisListSize;
        EdgeInfList invisList;
        unsigned int invisListSize;
        VertInf *pathNext;
        ConnDirFlags visDirections;
};


bool directVis(VertInf *src, VertInf *dst);


// A linked list of all the vertices in the router instance.  All the 
// connector endpoints are listed first, then all the shape vertices.
// Dunnny vertices inserted for orthogonal routing are classed as shape
// vertices but have VertID(0, 0).
//
class VertInfList
{
    public:
        VertInfList();
        void addVertex(VertInf *vert);
        VertInf *removeVertex(VertInf *vert);
        VertInf *getVertexByID(const VertID& id);
        VertInf *getVertexByPos(const Point& p);
        VertInf *shapesBegin(void);
        VertInf *connsBegin(void);
        VertInf *end(void);
        unsigned int connsSize(void) const;
        unsigned int shapesSize(void) const;
        void stats(FILE *fp = stderr)
        {
            fprintf(fp, "Conns %u, shapes %u\n", _connVertices, 
                    _shapeVertices);
        }
    private:
        VertInf *_firstShapeVert;
        VertInf *_firstConnVert;
        VertInf *_lastShapeVert;
        VertInf *_lastConnVert;
        unsigned int _shapeVertices;
        unsigned int _connVertices;
};


typedef std::set<unsigned int> ShapeSet;
typedef std::map<VertID, ShapeSet> ContainsMap;


}


#endif


