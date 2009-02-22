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

#ifndef AVOID_REGION_H
#define AVOID_REGION_H

#include <list>
#include "libavoid/geomtypes.h"

namespace Avoid {

class ShapeRef;

typedef std::list<int> ShapeList;


class Region
{
    public:
        Region();
        Region(double x1, double y1, double x2, double y2);

        bool overlapCheck(BBox& bbox, unsigned int& p);
        void getBBox(BBox& bb);
        Region *up(void);
        Region *down(void);
        Region *left(void);
        Region *right(void);
        void initialSplit(BBox& bbox, unsigned int pos, unsigned int& shapeId);
        bool isBlock(void);
        
        static void pairHor(Region *l, Region *r);
        static void pairVer(Region *a, Region *b);
        static void addShape(ShapeRef *shape);
        static void removeShape(ShapeRef *shape);
        static Region *getTopLeftRegion(void);

    private:
        BBox _bbox;
        Region *_left;
        Region *_right;
        Region *_up;
        Region *_down;
        ShapeList _blocks;
        
        Region *split(double pos, unsigned int dir); 
        void merge(unsigned int dir);
        void mergeRegion(Region *src); 
        Region *findRegion(double pos, unsigned int dir,
                const bool forMerge = false);
        Region *splitDir(double pos, unsigned int dir, bool first = false); 
        void mergeDir(unsigned int dir, bool first = false); 
        void copyAttributes(Region *src);
        void mergeAttributes(Region *src);
        bool safeToMerge(unsigned int dir);
        bool unsafeToMerge(unsigned int dir);
};


}

#endif
