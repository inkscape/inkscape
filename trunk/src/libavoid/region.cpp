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

#include <stdio.h>
#include <cstdlib>
#include <algorithm>
#include <cassert>
#include <cmath>

#include "libavoid/shape.h"
#include "libavoid/region.h"


namespace Avoid {

Region *centerRegion = NULL;

static const BBox screenBBox =
        {Point(-INFINITY, -INFINITY), Point(INFINITY, INFINITY)};


Region::Region()
    : _bbox(screenBBox)
    , _left(NULL), _right(NULL), _up(NULL), _down(NULL)
{
    _blocks.clear();
}


Region::Region(double x1, double y1, double x2, double y2)
    : _left(NULL), _right(NULL), _up(NULL), _down(NULL)
{
    _bbox.a.x = x1;
    _bbox.a.y = y1;
    _bbox.b.x = x2;
    _bbox.b.y = y2;

    _blocks.clear();
}


static const unsigned int R_INSIDE = 0;
static const unsigned int R_LEFT   = 1;
static const unsigned int R_LEDGE  = 2;
static const unsigned int R_RIGHT  = 4;
static const unsigned int R_REDGE  = 8;
static const unsigned int R_ABOVE  = 16;
static const unsigned int R_TEDGE  = 32;
static const unsigned int R_BELOW  = 64;
static const unsigned int R_BEDGE  = 128;

static const unsigned int R_NONE   = R_INSIDE;
static const unsigned int R_UP     = R_ABOVE;
static const unsigned int R_DOWN   = R_BELOW;
static const unsigned int R_HORI   = R_LEFT | R_RIGHT;
static const unsigned int R_VERT   = R_UP | R_DOWN;


static void printBBox(const char *label, const BBox &bbox)
{
    if (label)
    {
        printf("%s: ", label);
    }
    printf("(%.2f, %.2f)-(%.2f, %.2f)\n", bbox.a.x, bbox.a.y,
            bbox.b.x, bbox.b.y);
}


bool Region::overlapCheck(BBox& bbox, unsigned int& p)
{
    p = R_INSIDE;
    Point& a = bbox.a;
    Point& b = bbox.b;
    Point& r = _bbox.a;
    Point& s = _bbox.b;
    
    if (s.x <= a.x)
    {
        // Wholly right.
        p = R_RIGHT;
        return false;
    }
    else if (r.x >= b.x)
    {
        // Wholly left.
        p = R_LEFT;
        return false;
    }
    
    if (s.y <= a.y)
    {
        // Wholly below.
        p = R_BELOW;
        return false;
    }
    else if (r.y >= b.y)
    {
        // Wholly above.
        p = R_ABOVE;
        return false;
    }
    
    if (a.y == r.y)
    {
        // Shared top edge.
        p |= R_TEDGE;
    }
    else if (a.y < r.y)
    {
        // Need to split above.
        p |= R_ABOVE;
    }
    
    if (b.y == s.y)
    {
        // Shared bottom edge.
        p |= R_BEDGE;
    }
    else if (b.y > s.y)
    {
        // Need to split below.
        p |= R_BELOW;
    }

    if (a.x == r.x)
    {
        // Shared left edge.
        p |= R_LEDGE;
    }
    else if (a.x < r.x)
    {
        // Need to split left.
        p |= R_LEFT;
    }

    if (b.x == s.x)
    {
        // Shared right edge.
        p |= R_REDGE;
    }
    else if (b.x > s.x)
    {
        // Need to split right.
        p |= R_RIGHT;
    }
    
    return true;
}


void Region::getBBox(BBox& bb)
{
    bb.a = _bbox.a;
    bb.b = _bbox.b;
}


Region *Region::up(void)
{
    return _up;
}


Region *Region::down(void)
{
    return _down;
}


Region *Region::left(void)
{
    return _left;
}


Region *Region::right(void)
{
    return _right;
}


bool Region::isBlock(void)
{
    return !(_blocks.empty());
}


void Region::initialSplit(BBox& bbox, unsigned int pos, unsigned int& shapeId)
{
    Point& n1 = bbox.a;
    Point& n2 = bbox.b;
    Region *newR = this;
    Region *left, *right, *top, *bottom;
    
    if (pos == R_INSIDE)
    {
        split(n2.y, R_HORI);
        split(n2.x, R_VERT);
        newR = split(n1.y, R_HORI);
        newR = newR->split(n1.x, R_VERT);
        
        printf("%p - list %d add %d\n", newR,
                (int) newR->_blocks.size(), shapeId);
        newR->_blocks.push_back((int) shapeId);
        newR->_blocks.sort();
        newR->_blocks.unique();
    }
    else
    {
        Region *tar = NULL;
        tar = newR->findRegion(n1.x, R_VERT);
        if (pos & R_LEFT)
        {
            if (n1.x == tar->_bbox.a.x)
            {
                newR = tar->_right;
            }
            else if (n1.x == tar->_bbox.b.x)
            {
                newR = tar;
            }
            else
            {
                newR = tar->split(n1.x, R_VERT);
            }
        }
        else if (!(pos & R_LEDGE))
        {
            newR = tar->split(n1.x, R_VERT);
        }
        left = newR;
        
        tar = left->findRegion(n1.y, R_HORI);
        if (pos & R_ABOVE)
        {
            if (n1.y == tar->_bbox.a.y)
            {
                newR = tar->_down;
            }
            else if (n1.y == tar->_bbox.b.y)
            {
                newR = tar;
            }
            else
            {
                newR = tar->split(n1.y, R_HORI);
            }
        }
        else if (!(pos & R_TEDGE))
        {
            newR = tar->split(n1.y, R_HORI);
        }
        top = newR;
        
        right = newR;
        tar = newR->findRegion(n2.x, R_VERT);
        if (pos & R_RIGHT)
        {
        
            if (n2.x == tar->_bbox.a.x)
            {
                right = tar->_left;
            }
            else if (n2.x == tar->_bbox.b.x)
            {
                right = tar;
            }
            else
            {
                tar->split(n2.x, R_VERT);
                right = tar;
            }
        }
        else if (!(pos & R_REDGE))
        {
            tar->split(n2.x, R_VERT);
            right = tar;
        }

        bottom = right;
        tar = right->findRegion(n2.y, R_HORI);
        if (pos & R_BELOW)
        {
            if (n2.y == tar->_bbox.a.y)
            {
                bottom = tar->_up;
            }
            else if (n2.y == tar->_bbox.b.y)
            {
                bottom = tar;
            }
            else
            {
                tar->split(n2.y, R_HORI);
                bottom = tar;
            }
        }
        else if (!(pos & R_BEDGE))
        {
            tar->split(n2.y, R_HORI);
            bottom = tar;
        } 

        // top is actually top-left, and bottom is bottom-right.
        Region *curr = top, *cptr = NULL;
        while (curr->_bbox.b.y <= bottom->_bbox.b.y)
        {
            cptr = curr;
            while (cptr->_bbox.b.x <= bottom->_bbox.b.x)
            {
                printf("%p - list %d add %d\n", cptr,
                        (int) cptr->_blocks.size(), shapeId);
                cptr->_blocks.push_back((int) shapeId);
                cptr->_blocks.sort();
                cptr->_blocks.unique();

                cptr = cptr->_right;
            }

            curr = curr->_down;
        }
    }
}


// Returns the region containing the value 'pos' in the direction 'dir'.
// Thus, if looking for the x value 55, you would pass R_VERT as 'dir'.
// 'forMerge' specifies that the left or top block of a pair of regions
// with the split value of 'pos' should be returned.
Region *Region::findRegion(double pos, unsigned int dir, const bool forMerge)
{
    Region *curr = this;

    if (dir & R_VERT)
    {
        while (pos > curr->_bbox.b.x)
        {
            curr = curr->_right;
        }
        while (pos < curr->_bbox.a.x)
        {
            curr = curr->_left;
        }
        if (forMerge)
        {
            if (pos == curr->_bbox.a.x)
            {
                curr = curr->_left;
            }
            if (pos != curr->_bbox.b.x)
            {
                // 'pos' is not on the boundary.
                return NULL;
            }
        }
    }
    else if (dir & R_HORI)
    {
        while (pos > curr->_bbox.b.y)
        {
            curr = curr->_down;
        }
        while (pos < curr->_bbox.a.y)
        {
            curr = curr->_up;
        }
        if (forMerge)
        {
            if (pos == curr->_bbox.a.y)
            {
                curr = curr->_up;
            }
            if (pos != curr->_bbox.b.y)
            {
                // 'pos' is not on the boundary.
                return NULL;
            }
        }
    }
    return curr;
}


Region *Region::split(double pos, unsigned int dir)
{
    Region *newR = NULL;
    bool first = true;

    if (dir & R_VERT)
    {
        newR = splitDir(pos, R_UP, first);
        if (_down) _down->splitDir(pos, R_DOWN);
    }
    else if (dir & R_HORI)
    {
        newR = splitDir(pos, R_RIGHT, first);
        if (_left)  _left->splitDir(pos, R_LEFT);
    }
    return newR;
}


void Region::merge(unsigned int dir)
{
    bool first = true;

    if (dir & R_VERT)
    {
        mergeDir(R_UP, first);
        if (_down) _down->mergeDir(R_DOWN);
    }
    else if (dir & R_HORI)
    {
        mergeDir(R_RIGHT, first);
        if (_left)  _left->mergeDir(R_LEFT);
    }
}


void Region::mergeRegion(Region *src)
{
    assert(src != NULL);

    if (src == _left)
    {
        pairHor(src->_left, this);
        _bbox.a.x = src->_bbox.a.x;
    }
    else if (src == _right)
    {
        pairHor(this, src->_right);
        _bbox.b.x = src->_bbox.b.x;
    }
    else if (src == _up)
    {
        pairVer(src->_up, this);
        _bbox.a.y = src->_bbox.a.y;
    }
    else if (src == _down)
    {
        pairVer(this, src->_down);
        _bbox.b.y = src->_bbox.b.y;
    }
    else
    {
        fprintf(stderr, "Error: Avoid::Region::merge(): "
                "Argument not adjoining region.\n");
        abort();
    }
    mergeAttributes(src);
    printf("DEL %p\n", src);
    delete src;
}


Region *Region::splitDir(double pos, unsigned int dir, bool first)
{
    Point& o1 = _bbox.a;
    Point& o2 = _bbox.b;

    Region *newR = NULL;
    
    if (dir & R_VERT)
    {
        assert(pos > _bbox.a.x);
        assert(pos < _bbox.b.x);
        
        // Vertical recursion:
       
        // Create new block.
        Region *r  = new Region(pos, o1.y, o2.x, o2.y);
        printf("NEW %p\n", r);
        r->copyAttributes(this);

        Region *o_up    = _up;
        Region *o_down  = _down;

        // Resize old block.
        o2.x = pos;

        pairHor(r, _right);
        pairHor(this, r);
        
        if (dir & R_UP)
        {
            if (!first)  pairVer(r, _down->_right);
        
            if (o_up)    o_up->splitDir(pos, R_UP);
        }
        else if (dir & R_DOWN)
        {
            if (!first)  pairVer(_up->_right, r);
        
            if (o_down)  o_down->splitDir(pos, R_DOWN);
        }
        newR = r;
    }
    else if (dir & R_HORI)
    {
        // Vertical recursion:

        // Create new block.
        Region *b  = new Region(o1.x, pos, o2.x, o2.y);
        printf("NEW %p\n", b);
        b->copyAttributes(this);

        Region *o_left  = _left;
        Region *o_right = _right;
    
        // Resize old block.
        o2.y = pos;
        
        pairVer(b, _down);
        pairVer(this, b);

        if (dir & R_LEFT)
        {
            if (!first)  pairHor(b, _right->_down);
        
            if (o_left)  o_left->splitDir(pos, R_LEFT);
        }
        else if (dir & R_RIGHT)
        {
            if (!first)  pairHor(_left->_down, b);
        
            if (o_right) o_right->splitDir(pos, R_RIGHT);
        }
        newR = b;
    }
    return newR;
}


void Region::mergeDir(unsigned int dir, bool first)
{
    if (dir & R_VERT)
    {
        assert(_right != NULL);

        mergeRegion(_right);

        if (dir & R_UP)
        {
            if (_up)    _up->mergeDir(dir, R_UP);
        }
        else if (dir & R_DOWN)
        {
            if (_down)  _down->mergeDir(dir, R_DOWN);
        }
    }
    else if (dir & R_HORI)
    {
        assert(_down != NULL);

        mergeRegion(_down);

        if (dir & R_LEFT)
        {
            if (_left)  _left->mergeDir(dir, R_LEFT);
        }
        else if (dir & R_RIGHT)
        {
            if (_right) _right->mergeDir(dir, R_RIGHT);
        }
    }
}


void Region::copyAttributes(Region *src)
{
    _blocks = src->_blocks;
}


void Region::mergeAttributes(Region *src)
{
    _blocks.merge(src->_blocks);
    _blocks.sort();
    _blocks.unique();
}


//---------------------------
// Static member functions:


void Region::pairHor(Region *l, Region *r)
{
    if (l)  l->_right = r;
    if (r)  r->_left  = l;
}


void Region::pairVer(Region *a, Region *b)
{
    if (a)  a->_down = b;
    if (b)  b->_up   = a;
}


void Region::addShape(ShapeRef *shape)
{
    if (!centerRegion)
    {
        // Add new default region.
        centerRegion = new Region();
        printf("NEW %p\n", centerRegion);
    }
    BBox bbox;
    // Get bounding box for added shape.
    shape->boundingBox(bbox);
    printBBox("Add", bbox);
    
    unsigned int shapeId = shape->id();

    // Find starting point for overlap
    Region *curr = centerRegion;
    unsigned int pos = R_INSIDE;
    while (!(curr->overlapCheck(bbox, pos)))
    {
        if (pos & R_LEFT)
        {
            curr = curr->_left;
        }
        else if (pos & R_RIGHT)
        {
            curr = curr->_right;
        }
        else if (pos & R_ABOVE)
        {
            curr = curr->_up;
        }
        else if (pos & R_BELOW)
        {
            curr = curr->_down;
        }
    }
    
    curr->initialSplit(bbox, pos, shapeId);
    centerRegion = curr;
}


void Region::removeShape(ShapeRef *shape)
{
    const bool forMerge = true;

    BBox bbox;
    // Get bounding box for added shape.
    shape->boundingBox(bbox);
    printBBox("Remove", bbox);

    unsigned int shapeId = shape->id();
    
    Region *aboveTop = centerRegion->findRegion(bbox.a.y, R_HORI, forMerge);
    Region *aboveBottom = aboveTop->findRegion(bbox.b.y, R_HORI, forMerge);
    Region *leftOfLeft = aboveBottom->findRegion(bbox.a.x, R_VERT, forMerge);
    Region *leftOfRight = leftOfLeft->findRegion(bbox.b.x, R_VERT, forMerge);
    
    assert(aboveTop    != NULL);
    assert(aboveBottom != NULL);
    assert(leftOfLeft  != NULL);
    assert(leftOfRight != NULL);

    // Find Top Left.
    Region *topLeft = leftOfLeft->_right;
    while (topLeft->_bbox.a.y != aboveTop->_bbox.b.y)
    {
        topLeft = topLeft->_up;
    }
    
    // Find Bottom Right.
    Region *botRight = leftOfRight;
    while (botRight->_bbox.b.y != aboveBottom->_bbox.b.y)
    {
        botRight = botRight->_down;
    }
    
    // Clear blocking flag.
    Region *curr = topLeft, *cptr = NULL;
    while (curr->_bbox.b.y <= botRight->_bbox.b.y)
    {
        cptr = curr;
        while (cptr->_bbox.b.x <= botRight->_bbox.b.x)
        {
            ShapeList& blocks = cptr->_blocks;
            
            assert(std::find(blocks.begin(), blocks.end(), (int) shapeId) !=
                    blocks.end());
            printf("%p - list %d remove %d\n", cptr,
                    (int) blocks.size(), shapeId);
            cptr->_blocks.remove((int) shapeId);

            cptr = cptr->_right;
        }

        curr = curr->_down;
    }
    
    // These two are safe since they don't invalidate the pointers
    // to the regions that are inside the shape.
    if (aboveBottom->safeToMerge(R_HORI))
    {
        aboveBottom->merge(R_HORI);
    }
    if (leftOfRight->safeToMerge(R_VERT))
    {
        leftOfRight->merge(R_VERT);
    }
    
    // Be a bit more careful with these.
    double leftX = leftOfLeft->_bbox.b.x;
    if (aboveTop->safeToMerge(R_HORI))
    {
        aboveTop->merge(R_HORI);
    }
    // leftOfLeft may have been freed, so look for the new block at
    // that position.
    leftOfLeft = aboveTop->findRegion(leftX, R_VERT, forMerge);
    assert(leftOfLeft);
    if (leftOfLeft->safeToMerge(R_VERT))
    {
        leftOfLeft->merge(R_VERT);
    }
}


bool Region::safeToMerge(unsigned int dir)
{
    bool unsafe = false;

    if (dir == R_HORI)
    {
        printf("safeToMerge? y = %.3f... ", _bbox.b.y); 
        unsafe |= unsafeToMerge(R_RIGHT);
        if (_left)  unsafe |= _left->unsafeToMerge(R_LEFT);
    }
    else if (dir == R_VERT)
    {
        printf("safeToMerge? x = %.3f... ", _bbox.b.x); 
        unsafe |= unsafeToMerge(R_DOWN);
        if (_up)  unsafe |= _up->unsafeToMerge(R_UP);
    }
    printf("%s.\n", (unsafe) ? "no" : "yes");
        
    return !unsafe;
}


bool Region::unsafeToMerge(unsigned int dir)
{
    bool unsafe = false;
    
    if (dir & R_HORI)
    {
        // If they are not the same on both sides then we can merge.
        if (_blocks != _down->_blocks)
        {
            printf("\n  _blocks:\n    ");
            for (ShapeList::iterator i = _blocks.begin(); i != _blocks.end();
                    ++i)
            {
                printf("%d ", *i);
            }
            printf("\n  _down->_blocks:\n    ");
            for (ShapeList::iterator i = _down->_blocks.begin();
                    i != _down->_blocks.end(); ++i)
            {
                printf("%d ", *i);
            }
            unsafe |= true;
            printf("\n");
        }

        if ((dir == R_LEFT) && _left)
        {
            unsafe |= _left->unsafeToMerge(dir);
        }
        else if ((dir == R_RIGHT) && _right)
        {
            unsafe |= _right->unsafeToMerge(dir);
        }
    }
    else if (dir & R_VERT)
    {
        if (_blocks !=  _right->_blocks)
        {
            printf("\n  _blocks:\n    ");
            for (ShapeList::iterator i = _blocks.begin(); i != _blocks.end();
                    ++i)
            {
                printf("%d ", *i);
            }
            printf("\n  _right->_blocks:\n    ");
            for (ShapeList::iterator i = _right->_blocks.begin();
                    i != _right->_blocks.end(); ++i)
            {
                printf("%d ", *i);
            }
            unsafe |= true;
            printf("\n");
        }
        
        if ((dir == R_UP) && _up)
        {
            unsafe |= _up->unsafeToMerge(dir);
        }
        else if ((dir == R_DOWN) && _down)
        {
            unsafe |= _down->unsafeToMerge(dir);
        }
    }
    return unsafe;
        
}


Region *Region::getTopLeftRegion(void)
{
    Region *curr = centerRegion;

    while (curr && (curr->_up || curr->_left))
    {
        if (curr->_up)
        {
            curr = curr->_up;
        }
        else
        {
            curr = curr->_left;
        }
    }
    return curr;
}


}

