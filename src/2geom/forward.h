/**
 * \file
 * \brief  Contains forward declarations of 2geom types
 *//*
 * Authors:
 *  Johan Engelen <goejendaagh@zonnet.nl>
 *  Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2008-2010 Authors
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

#ifndef LIB2GEOM_SEEN_FORWARD_H
#define LIB2GEOM_SEEN_FORWARD_H

namespace Geom {

// primitives
typedef double Coord;
typedef int IntCoord;
class Point;
class IntPoint;
class Line;
class Ray;
template <typename> class GenericInterval;
template <typename> class GenericOptInterval;
class Interval;
class OptInterval;
typedef GenericInterval<IntCoord> IntInterval;
typedef GenericOptInterval<IntCoord> OptIntInterval;
template <typename> class GenericRect;
template <typename> class GenericOptRect;
class Rect;
class OptRect;
typedef GenericRect<IntCoord> IntRect;
typedef GenericOptRect<IntCoord> OptIntRect;

// fragments
class Linear;
class Bezier;
class SBasis;
class Poly;

// shapes
class Circle;
class Ellipse;
class ConvexHull;

// curves
class Curve;
class SBasisCurve;
class BezierCurve;
template <unsigned degree> class BezierCurveN;
typedef BezierCurveN<1> LineSegment;
typedef BezierCurveN<2> QuadraticBezier;
typedef BezierCurveN<3> CubicBezier;
class EllipticalArc;

// paths and path sequences
class Path;
class PathVector;
struct PathTime;
class PathInterval;
struct PathVectorTime;

// errors
class Exception;
class LogicalError;
class RangeError;
class NotImplemented;
class InvariantsViolation;
class NotInvertible;
class ContinuityError;

// transforms
class Affine;
class Translate;
class Rotate;
class Scale;
class HShear;
class VShear;
class Zoom;

// templates
template <typename> class D2;
template <typename> class Piecewise;

// misc
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
