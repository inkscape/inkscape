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

#include "libavoid/geometry.h"
#include "libavoid/shape.h"

#ifndef AVOID_POLYUTIL_H
#define AVOID_POLYUTIL_H

namespace Avoid {


extern Polygn newPoly(int size);
extern Polygn copyPoly(Polygn);
extern Polygn copyPoly(ShapeRef *shape);
extern void freePoly(Polygn&);
extern void freePtrPoly(Polygn *argpoly);


}

#endif
