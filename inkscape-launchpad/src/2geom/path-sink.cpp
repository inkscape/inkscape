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
#include <2geom/circle.h>
#include <2geom/ellipse.h>

namespace Geom {

void PathSink::feed(Curve const &c, bool moveto_initial)
{
    c.feed(*this, moveto_initial);
}

void PathSink::feed(Path const &path) {
    flush();
    moveTo(path.front().initialPoint());

    // never output the closing segment to the sink
    Path::const_iterator iter = path.begin(), last = path.end_open();
    for (; iter != last; ++iter) {
        iter->feed(*this, false);
    }
    if (path.closed()) {
        closePath();
    }
    flush();
}

void PathSink::feed(PathVector const &pv) {
    for (PathVector::const_iterator i = pv.begin(); i != pv.end(); ++i) {
        feed(*i);
    }
}

void PathSink::feed(Rect const &r) {
    moveTo(r.corner(0));
    lineTo(r.corner(1));
    lineTo(r.corner(2));
    lineTo(r.corner(3));
    closePath();
}

void PathSink::feed(Circle const &e) {
    Coord r = e.radius();
    Point c = e.center();
    Point a = c + Point(0, +r);
    Point b = c + Point(0, -r);

    moveTo(a);
    arcTo(r, r, 0, false, false, b);
    arcTo(r, r, 0, false, false, a);
    closePath();
}

void PathSink::feed(Ellipse const &e) {
    Point s = e.pointAt(0);
    moveTo(s);
    arcTo(e.ray(X), e.ray(Y), e.rotationAngle(), false, false, e.pointAt(M_PI));
    arcTo(e.ray(X), e.ray(Y), e.rotationAngle(), false, false, s);
    closePath();
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
