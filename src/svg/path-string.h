/*
 * Inkscape::SVG::PathString - builder for SVG path strings
 *
 * Copyright 2007 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_SVG_PATH_STRING_H
#define SEEN_INKSCAPE_SVG_PATH_STRING_H

#include <glibmm/ustring.h>
#include "libnr/nr-point.h"
#include "svg/stringstream.h"

namespace Inkscape {

namespace SVG {

class PathString {
public:
    PathString() {}

    // default copy
    // default assign

    Glib::ustring const &ustring() const {
        return _str;
    }

    operator Glib::ustring const &() const {
        return ustring();
    }

    char const *c_str() const {
        return _str.c_str();
    }

    PathString &moveTo(NR::Coord x, NR::Coord y) {
        return moveTo(NR::Point(x, y));
    }

    PathString &moveTo(NR::Point p) {
        _appendOp('M');
        _append(p);
        return *this;
    }

    PathString &lineTo(NR::Coord x, NR::Coord y) {
        return lineTo(NR::Point(x, y));
    }

    PathString &lineTo(NR::Point p) {
        _appendOp('L');
        _append(p);
        return *this;
    }

    PathString &quadTo(NR::Coord cx, NR::Coord cy, NR::Coord x, NR::Coord y) {
        return quadTo(NR::Point(cx, cy), NR::Point(x, y));
    }

    PathString &quadTo(NR::Point c, NR::Point p) {
        _appendOp('Q');
        _append(c);
        _append(p);
        return *this;
    }

    PathString &curveTo(NR::Coord x0, NR::Coord y0,
                        NR::Coord x1, NR::Coord y1,
                        NR::Coord x,  NR::Coord y)
    {
        return curveTo(NR::Point(x0, y0), NR::Point(x1, y1), NR::Point(x, y));
    }
    

    PathString &curveTo(NR::Point c0, NR::Point c1, NR::Point p) {
        _appendOp('C');
        _append(c0);
        _append(c1);
        _append(p);
        return *this;
    }

    PathString &arcTo(NR::Coord rx, NR::Coord ry, NR::Coord rot,
                      bool large_arc, bool sweep,
                      NR::Point p)
    {
        _appendOp('A');
        _append(NR::Point(rx, ry));
        _append(rot);
        _append(large_arc);
        _append(sweep);
        _append(p);
        return *this;
    }

    PathString &closePath() {
      _appendOp('z');
      return *this;
    }

private:
    void _appendOp(char op) {
        if (!_str.empty()) {
          _str.append(1, ' ');
        }
        _str.append(1, op);
    }

    void _append(bool flag) {
        _str.append(1, ' ');
        _str.append(1, ( flag ? '1' : '0' ));
    }

    void _append(NR::Coord v) {
        SVGOStringStream os;
        os << ' ' << v;
        _str.append(os.str());
    }

    void _append(NR::Point p) {
        SVGOStringStream os;
        os << ' ' << p[NR::X] << ',' << p[NR::Y];
        _str.append(os.str());
    }

    Glib::ustring _str;
};

}

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
