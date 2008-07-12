/*
 * Inkscape::SVG::PathString - builder for SVG path strings
 *
 * Copyright 2007 MenTaLguY <mental@rydia.net>
 * Copyright 2008 Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
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
#include "libnr/nr-point-ops.h"

namespace Inkscape {

namespace SVG {

class PathString {
public:
    PathString();

    // default copy
    // default assign

    Glib::ustring const &ustring() const {
        return (_abs_state <= _rel_state || !allow_relative_coordinates) ? _abs_state.str : _rel_state.str;
    }

    operator Glib::ustring const &() const {
        return ustring();
    }

    char const *c_str() const {
        return ustring().c_str();
    }

    PathString &moveTo(NR::Coord x, NR::Coord y) {
        return moveTo(NR::Point(x, y));
    }

    PathString &moveTo(NR::Point p) {
        _appendOp('M','m');
        _appendPoint(p, true);

        _initial_point = _current_point;
        return *this;
    }

    PathString &lineTo(NR::Coord x, NR::Coord y) {
        return lineTo(NR::Point(x, y));
    }

    PathString &lineTo(NR::Point p) {
        _appendOp('L','l');
        _appendPoint(p, true);
        return *this;
    }

    PathString &horizontalLineTo(NR::Coord x) {
        _appendOp('H','h');
        _appendX(x, true);
        return *this;
    }

    PathString &verticalLineTo(NR::Coord y) {
        _appendOp('V','v');
        _appendY(y, true);
        return *this;
    }

    PathString &quadTo(NR::Coord cx, NR::Coord cy, NR::Coord x, NR::Coord y) {
        return quadTo(NR::Point(cx, cy), NR::Point(x, y));
    }

    PathString &quadTo(NR::Point c, NR::Point p) {
        _appendOp('Q','q');
        _appendPoint(c, false);
        _appendPoint(p, true);
        return *this;
    }

    PathString &curveTo(NR::Coord x0, NR::Coord y0,
                        NR::Coord x1, NR::Coord y1,
                        NR::Coord x,  NR::Coord y)
    {
        return curveTo(NR::Point(x0, y0), NR::Point(x1, y1), NR::Point(x, y));
    }

    PathString &curveTo(NR::Point c0, NR::Point c1, NR::Point p) {
        _appendOp('C','c');
        _appendPoint(c0, false);
        _appendPoint(c1, false);
        _appendPoint(p, true);
        return *this;
    }

    PathString &arcTo(NR::Coord rx, NR::Coord ry, NR::Coord rot,
                      bool large_arc, bool sweep,
                      NR::Point p)
    {
        _appendOp('A','a');
        _appendValue(NR::Point(rx,ry));
        _appendValue(rot);
        _appendFlag(large_arc);
        _appendFlag(sweep);
        _appendPoint(p, true);
        return *this;
    }

    PathString &closePath() {
        _abs_state.appendOp('z');
        _rel_state.appendOp('z');
        _current_point = _initial_point;
        return *this;
    }

private:

    void _appendOp(char abs_op, char rel_op);

    void _appendFlag(bool flag) {
        _abs_state.append(flag);
        _rel_state.append(flag);
    }

    void _appendValue(NR::Coord v) {
        _abs_state.append(v);
        _rel_state.append(v);
    }

    void _appendValue(NR::Point p) {
        _abs_state.append(p);
        _rel_state.append(p);
    }

    void _appendX(NR::Coord x, bool sc) {
        double rx;
        _abs_state.append(x, rx);
        _rel_state.appendRelative(rx, _current_point[NR::X]);
        if (sc) _current_point[NR::X] = rx;
    }

    void _appendY(NR::Coord y, bool sc) {
        double ry;
        _abs_state.append(y, ry);
        _rel_state.appendRelative(ry, _current_point[NR::Y]);
        if (sc) _current_point[NR::Y] = ry;
    }

    void _appendPoint(NR::Point p, bool sc) {
        NR::Point rp;
        _abs_state.append(p, rp);
        _rel_state.appendRelative(rp, _current_point);
        if (sc) _current_point = rp;
    }

    struct State {
        State() { prevop = 0; switches = 0; }

        void appendOp(char op) {
            if (!str.empty()) str.append(1, ' ');
            str.append(1, op);
            prevop = op == 'M' ? 'L' : op == 'm' ? 'l' : op;
        }

        void append(bool flag) {
            str.append(1, ' ');
            str.append(1, ( flag ? '1' : '0' ));
        }

        void append(NR::Coord v);
        void append(NR::Point v);
        void append(NR::Coord v, NR::Coord& rv);
        void append(NR::Point p, NR::Point& rp);
        void appendRelative(NR::Coord v, NR::Coord r);
        void appendRelative(NR::Point p, NR::Point r);

        bool operator<=(const State& s) const {
            if ( str.size() < s.str.size() ) return true;
            if ( str.size() > s.str.size() ) return false;
            if ( switches < s.switches ) return true;
            if ( switches > s.switches ) return false;
            return true;
        }

        Glib::ustring str;
        unsigned int switches;
        char prevop;
    } _abs_state, _rel_state; // State with the last operator being an absolute/relative operator

    NR::Point _initial_point;
    NR::Point _current_point;

    bool const allow_relative_coordinates;
    bool const force_repeat_commands;
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
