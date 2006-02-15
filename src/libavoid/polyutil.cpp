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

#include <cassert>
#include <cstdlib>

#include "libavoid/polyutil.h"
#include "libavoid/geomtypes.h"
#include "libavoid/vertices.h"
#include "libavoid/shape.h"

namespace Avoid {


Polygn newPoly(int size)
{
    Polygn newpoly;

    newpoly.pn = size;
    newpoly.ps = (Point *) calloc(size, sizeof(Point));
    if (!newpoly.ps)
    {
        fprintf(stderr,
                "Error: Unable to allocate Point array in Avoid::newPoly\n");
        abort();
    }
    return newpoly;
}


Polygn copyPoly(Polygn poly)
{
    Polygn newpoly = newPoly(poly.pn);

    newpoly.id = poly.id;
    for (int i = 0; i < poly.pn; i++)
    {
        newpoly.ps[i] = poly.ps[i];
    }
    return newpoly;
}


Polygn copyPoly(ShapeRef *shape)
{
    Polygn poly = shape->poly();
    Polygn newpoly = newPoly(poly.pn);

    newpoly.id = poly.id;
    for (int i = 0; i < poly.pn; i++)
    {
        newpoly.ps[i] = poly.ps[i];
    }
    return newpoly;
}


void freePoly(Polygn& poly)
{
    std::free(poly.ps);
}


void freePtrPoly(Polygn *poly)
{
    std::free(poly->ps);
    std::free(poly);
}


}

