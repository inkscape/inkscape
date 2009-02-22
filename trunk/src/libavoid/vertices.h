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

#ifndef AVOID_VERTICES_H
#define AVOID_VERTICES_H

#include <list>
#include <set>
#include <map>
#include <iostream>
#include "libavoid/geomtypes.h"

namespace Avoid {

class EdgeInf;
class Router;

typedef std::list<EdgeInf *> EdgeInfList;


class VertID
{
    public:
        unsigned int objID;
        bool isShape;
        int vn;

        static const int src;
        static const int tar;

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


class VertInf
{
    public:
        VertInf(Router *router, const VertID& vid, const Point& vpoint);
        void Reset(const Point& vpoint);
        void removeFromGraph(const bool isConnVert = true);

        Router *_router;
        VertID id;
        Point  point;
        VertInf *lstPrev;
        VertInf *lstNext;
        VertInf *shPrev;
        VertInf *shNext;
        EdgeInfList visList;
        unsigned int visListSize;
        EdgeInfList invisList;
        unsigned int invisListSize;
        VertInf *pathNext;
        double pathDist;
};


bool directVis(VertInf *src, VertInf *dst);


class VertInfList
{
    public:
        VertInfList();
        void addVertex(VertInf *vert);
        void removeVertex(VertInf *vert);
        VertInf *shapesBegin(void);
        VertInf *connsBegin(void);
        VertInf *end(void);
        void stats(void)
        {
            printf("Conns %d, shapes %d\n", _connVertices, _shapeVertices);
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


