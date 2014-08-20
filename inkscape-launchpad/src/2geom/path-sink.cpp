/*
 * callback interface for SVG path data
 *
 * Copyright 2007 MenTaLguY <mental@rydia.net>
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
 * in the file COPYING-LGPL-2.1; if not, output to the Free Software
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
 *
 */

#include <2geom/sbasis-to-bezier.h>
#include <2geom/path-sink.h>
#include <2geom/exception.h>

namespace Geom {

void output(Curve const &curve, PathSink &sink) {
    std::vector<Point> pts;
    sbasis_to_bezier(pts, curve.toSBasis(), 2); //TODO: use something better!
    sink.curveTo(pts[0], pts[1], pts[2]);
}

void output(HLineSegment const &curve, PathSink &sink) {
    sink.hlineTo(curve.finalPoint()[X]);
}

void output(VLineSegment const &curve, PathSink &sink) {
    sink.vlineTo(curve.finalPoint()[Y]);
}

void output(LineSegment const &curve, PathSink &sink) {
    sink.lineTo(curve[1]);
}

void output(CubicBezier const &curve, PathSink &sink) {
    sink.curveTo(curve[1], curve[2], curve[3]);
}

void output(QuadraticBezier const &curve, PathSink &sink) {
    sink.quadTo(curve[1], curve[2]);
}

void output(SVGEllipticalArc const &curve, PathSink &sink) {
    sink.arcTo( curve.ray(X), curve.ray(Y), curve.rotationAngle(),
    			curve.largeArc(), curve.sweep(),
    			curve.finalPoint() );
}

template <typename T>
bool output_as(Curve const &curve, PathSink &sink) {
    T const *t = dynamic_cast<T const *>(&curve);
    if (t) {
        output(*t, sink);
        return true;
    } else {
        return false;
    }
}

void PathSink::path(Path const &path) {
    flush();
    moveTo(path.front().initialPoint());

    Path::const_iterator iter;
    for (iter = path.begin(); iter != path.end(); ++iter) {
    	output_as<HLineSegment>(*iter, *this) ||
    	output_as<VLineSegment>(*iter, *this) ||
        output_as<LineSegment>(*iter, *this) ||
        output_as<CubicBezier>(*iter, *this) ||
        output_as<QuadraticBezier>(*iter, *this) ||
        output_as<SVGEllipticalArc>(*iter, *this) ||
        output_as<Curve>(*iter, *this);
    }

    if (path.closed()) {
        closePath();
    }
    flush();
}

void PathSink::pathvector(PathVector const &pv) {
    flush();
    for (PathVector::const_iterator i = pv.begin(); i != pv.end(); ++i) {
        path(*i);
    }
}

}

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
