/*
 * Copyright 2007 MenTaLguY <mental@rydia.net>
 * Copyright 2008 Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 * Copyright 2013 Tavmjong Bah <tavmjong@free.fr>
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

#include <2geom/point.h>
#include <cstdio>
#include <glibmm/ustring.h>
#include <string>

namespace Inkscape {

namespace SVG {

// Relative vs. absolute coordinates
enum PATHSTRING_FORMAT {
    PATHSTRING_ABSOLUTE,  // Use only absolute coordinates
    PATHSTRING_RELATIVE,  // Use only relative coordinates
    PATHSTRING_OPTIMIZE,  // Optimize for path string length
    PATHSTRING_FORMAT_SIZE
};

/**
 * Builder for SVG path strings.
 */
class PathString {
public:
    PathString();

    // default copy
    // default assign

    std::string const &string() {
        std::string const &t = tail();
        final.reserve(commonbase.size()+t.size());
        final = commonbase;
        final += tail();
        // std::cout << " final: " << final << std::endl;
        return final;
    }

    operator std::string const &() {
        return string();
    }

    operator Glib::ustring const () const {
        return commonbase + tail();
    }

    char const *c_str() {
        return string().c_str();
    }

    PathString &moveTo(Geom::Coord x, Geom::Coord y) {
        return moveTo(Geom::Point(x, y));
    }

    PathString &moveTo(Geom::Point p) {
        _appendOp('M','m');
        _appendPoint(p, true);

        _initial_point = _current_point;
        return *this;
    }

    PathString &lineTo(Geom::Coord x, Geom::Coord y) {
        return lineTo(Geom::Point(x, y));
    }

    PathString &lineTo(Geom::Point p) {
        _appendOp('L','l');
        _appendPoint(p, true);
        return *this;
    }

    PathString &horizontalLineTo(Geom::Coord x) {
        _appendOp('H','h');
        _appendX(x, true);
        return *this;
    }

    PathString &verticalLineTo(Geom::Coord y) {
        _appendOp('V','v');
        _appendY(y, true);
        return *this;
    }

    PathString &quadTo(Geom::Coord cx, Geom::Coord cy, Geom::Coord x, Geom::Coord y) {
        return quadTo(Geom::Point(cx, cy), Geom::Point(x, y));
    }

    PathString &quadTo(Geom::Point c, Geom::Point p) {
        _appendOp('Q','q');
        _appendPoint(c, false);
        _appendPoint(p, true);
        return *this;
    }

    PathString &curveTo(Geom::Coord x0, Geom::Coord y0,
                        Geom::Coord x1, Geom::Coord y1,
                        Geom::Coord x,  Geom::Coord y)
    {
        return curveTo(Geom::Point(x0, y0), Geom::Point(x1, y1), Geom::Point(x, y));
    }

    PathString &curveTo(Geom::Point c0, Geom::Point c1, Geom::Point p) {
        _appendOp('C','c');
        _appendPoint(c0, false);
        _appendPoint(c1, false);
        _appendPoint(p, true);
        return *this;
    }

    /**
     *  \param rot the angle in degrees
     */
    PathString &arcTo(Geom::Coord rx, Geom::Coord ry, Geom::Coord rot,
                      bool large_arc, bool sweep,
                      Geom::Point p)
    {
        _appendOp('A','a');
        _appendValue(Geom::Point(rx,ry));
        _appendValue(rot);
        _appendFlag(large_arc);
        _appendFlag(sweep);
        _appendPoint(p, true);
        return *this;
    }

    PathString &closePath() {

        _abs_state.appendOp('Z');
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

    void _appendValue(Geom::Coord v) {
        _abs_state.append(v);
        _rel_state.append(v);
    }

    void _appendValue(Geom::Point p) {
        _abs_state.append(p);
        _rel_state.append(p);
    }

    void _appendX(Geom::Coord x, bool sc) {
        double rx;
        _abs_state.append(x, rx);
        _rel_state.appendRelative(rx, _current_point[Geom::X]);
        if (sc) _current_point[Geom::X] = rx;
    }

    void _appendY(Geom::Coord y, bool sc) {
        double ry;
        _abs_state.append(y, ry);
        _rel_state.appendRelative(ry, _current_point[Geom::Y]);
        if (sc) _current_point[Geom::Y] = ry;
    }

    void _appendPoint(Geom::Point p, bool sc) {
        Geom::Point rp;
        _abs_state.append(p, rp);
        _rel_state.appendRelative(rp, _current_point);
        if (sc) _current_point = rp;
    }

    struct State {
        State() { prevop = 0; switches = 0; }

        void appendOp(char op) {
            if (prevop != 0) str += ' ';
            str += op;
            prevop = ( op == 'M' ? 'L' : op == 'm' ? 'l' : op );
        }

        void append(bool flag) {
            str += ' ';
            str += ( flag ? '1' : '0' );
        }

        void append(Geom::Coord v);
        void append(Geom::Point v);
        void append(Geom::Coord v, Geom::Coord& rv);
        void append(Geom::Point p, Geom::Point& rp);
        void appendRelative(Geom::Coord v, Geom::Coord r);
        void appendRelative(Geom::Point p, Geom::Point r);

        bool operator<=(const State& s) const {
            if ( str.size() < s.str.size() ) return true;
            if ( str.size() > s.str.size() ) return false;
            if ( switches < s.switches ) return true;
            if ( switches > s.switches ) return false;
            return true;
        }

        // Note: changing this to Glib::ustring might cause problems in path-string.cpp because it assumes that
        //       size() returns the size of the string in BYTES (and Glib::ustring::resize is terribly slow)
        std::string str;
        unsigned int switches;
        char prevop;

    private:
        void appendNumber(double v, int precision=numericprecision, int minexp=minimumexponent);
        void appendNumber(double v, double &rv, int precision=numericprecision, int minexp=minimumexponent);
        void appendRelativeCoord(Geom::Coord v, Geom::Coord r);
    } _abs_state, _rel_state; // State with the last operator being an absolute/relative operator

    Geom::Point _initial_point;
    Geom::Point _current_point;

    // If both states have a common prefix it is stored here.
    // Separating out the common prefix prevents repeated copying between the states
    // to cause a quadratic time complexity (in the number of characters/operators)
    std::string commonbase;
    std::string final;
    std::string const &tail() const {
        return ( (format == PATHSTRING_ABSOLUTE) ||
                 (format == PATHSTRING_OPTIMIZE && _abs_state <= _rel_state ) ?
                 _abs_state.str : _rel_state.str );
    }

    static PATHSTRING_FORMAT format;
    bool const force_repeat_commands;
    static int numericprecision;
    static int minimumexponent;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
