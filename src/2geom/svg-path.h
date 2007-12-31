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
 *
 */

#ifndef SEEN_SVG_PATH_H
#define SEEN_SVG_PATH_H

#include "path.h"
#include <iterator>

namespace Geom {

class SVGPathSink {
public:
    virtual void moveTo(Point p) = 0;
    virtual void lineTo(Point p) = 0;
    virtual void curveTo(Point c0, Point c1, Point p) = 0;
    virtual void quadTo(Point c, Point p) = 0;
    virtual void arcTo(double rx, double ry, double angle,
                       bool large_arc, bool sweep, Point p) = 0;
    virtual void closePath() = 0;
    virtual void finish() = 0;
    virtual ~SVGPathSink() {}
};

void output_svg_path(Path &path, SVGPathSink &sink);

template <typename OutputIterator>
class SVGPathGenerator : public SVGPathSink {
public:
    explicit SVGPathGenerator(OutputIterator out)
    : _in_path(false), _out(out) {}

    void moveTo(Point p) {
        finish();
        _path.start(p);
        _in_path = true;
    }
//TODO: what if _in_path = false?
    void lineTo(Point p) {
        _path.template appendNew<LineSegment>(p);
    }

    void quadTo(Point c, Point p) {
        _path.template appendNew<QuadraticBezier>(c, p);
    }

    void curveTo(Point c0, Point c1, Point p) {
        _path.template appendNew<CubicBezier>(c0, c1, p);
    }

    void arcTo(double rx, double ry, double angle,
               bool large_arc, bool sweep, Point p)
    {
        _path.template appendNew<SVGEllipticalArc>(rx, ry, angle,
                                                 large_arc, sweep, p);
    }

    void closePath() {
        _path.close();
        finish();
    }

    void finish() {
        if (_in_path) {
            _in_path = false;
            *_out = _path;
            _path.clear();
            _path.close(false);
        }
    }

protected:
    bool _in_path;
    OutputIterator _out;
    Path _path;
};

typedef std::back_insert_iterator<std::vector<Path> > iter;

class PathBuilder : public SVGPathGenerator<iter> {
private:
    std::vector<Path> _pathset;
public:
    PathBuilder() : SVGPathGenerator<iter>(iter(_pathset)) {}
    std::vector<Path> const &peek() const {return _pathset;}
};

}

#endif
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
