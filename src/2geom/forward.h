/*
 * forward - this file contains forward declarations of 2geom types
 *
 * Authors:
 *  Johan Engelen <goejendaagh@zonnet.nl>
 * 
 * Copyright 2008  authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#ifndef SEEN_GEOM_FORWARD_H
#define SEEN_GEOM_FORWARD_H

#include <vector>   // include this dependency so PathVector can be defined more explicitly

namespace Geom {

template <unsigned> class BezierCurve;
template<> class BezierCurve<0>;
typedef BezierCurve<2> QuadraticBezier;
typedef BezierCurve<1> LineSegment;
typedef BezierCurve<3> CubicBezier;

typedef double Coord;
class Point;

class Exception;
class LogicalError;
class RangeError;
class NotImplemented;
class InvariantsViolation;
class NotInvertible;
class ContinuityError;

class Interval;
class Linear;
class Hat;
class Tri;

class Matrix;
class Translate;
class Rotate;
class Scale;

class Curve;
class Path;
typedef std::vector<Path> PathVector;

template <class> class D2;
template <typename> class Piecewise;
class SBasis;
class SBasisCurve;

typedef D2<Interval> Rect;

class Shape;
class Region;

class SVGEllipticalArc;
class SVGPathSink;
template <typename> class SVGPathGenerator;

}

#endif // SEEN_GEOM_FORWARD_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
