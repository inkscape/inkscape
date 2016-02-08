/** @file
 * @brief Path sink which writes an SVG-compatible command string
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2014 Authors
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

#include <cmath>
#include <iomanip>
#include <2geom/coord.h>
#include <2geom/svg-path-writer.h>
#include <glib.h>

namespace Geom {

static inline bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

SVGPathWriter::SVGPathWriter()
    : _epsilon(0)
    , _precision(-1)
    , _optimize(false)
    , _use_shorthands(true)
    , _command(0)
{
    // always use C locale for number formatting
    _ns.imbue(std::locale::classic());
    _ns.unsetf(std::ios::floatfield);
}

void SVGPathWriter::moveTo(Point const &p)
{
    _setCommand('M');
    _current_pars.push_back(p[X]);
    _current_pars.push_back(p[Y]);

    _current = _subpath_start = _quad_tangent = _cubic_tangent = p;
    if (!_optimize) {
        flush();
    }
}

void SVGPathWriter::lineTo(Point const &p)
{
    // The weird setting of _current is to avoid drift with many almost-aligned segments
    // The additional conditions ensure that the smaller dimension is rounded to zero
    bool written = false;
    if (_use_shorthands) {
        Point r = _current - p;
        if (are_near(p[X], _current[X], _epsilon) && std::abs(r[X]) < std::abs(r[Y])) {
            // emit vlineto
            _setCommand('V');
            _current_pars.push_back(p[Y]);
            _current[Y] = p[Y];
            written = true;
        } else if (are_near(p[Y], _current[Y], _epsilon) && std::abs(r[Y]) < std::abs(r[X])) {
            // emit hlineto
            _setCommand('H');
            _current_pars.push_back(p[X]);
            _current[X] = p[X];
            written = true;
        }
    }

    if (!written) {
        // emit normal lineto
        if (_command != 'M' && _command != 'L') {
            _setCommand('L');
        }
        _current_pars.push_back(p[X]);
        _current_pars.push_back(p[Y]);
        _current = p;
    }

    _cubic_tangent = _quad_tangent = _current;
    if (!_optimize) {
        flush();
    }
}

void SVGPathWriter::quadTo(Point const &c, Point const &p)
{
    bool shorthand = _use_shorthands && are_near(c, _quad_tangent, _epsilon);

    _setCommand(shorthand ? 'T' : 'Q');
    if (!shorthand) {
        _current_pars.push_back(c[X]);
        _current_pars.push_back(c[Y]);
    }
    _current_pars.push_back(p[X]);
    _current_pars.push_back(p[Y]);

    _current = _cubic_tangent = p;
    _quad_tangent = p + (p - c);
    if (!_optimize) {
        flush();
    }
}

void SVGPathWriter::curveTo(Point const &p1, Point const &p2, Point const &p3)
{
    bool shorthand = _use_shorthands && are_near(p1, _cubic_tangent, _epsilon);

    _setCommand(shorthand ? 'S' : 'C');
    if (!shorthand) {
        _current_pars.push_back(p1[X]);
        _current_pars.push_back(p1[Y]);
    }
    _current_pars.push_back(p2[X]);
    _current_pars.push_back(p2[Y]);
    _current_pars.push_back(p3[X]);
    _current_pars.push_back(p3[Y]);

    _current = _quad_tangent = p3;
    _cubic_tangent = p3 + (p3 - p2);
    if (!_optimize) {
        flush();
    }
}

void SVGPathWriter::arcTo(double rx, double ry, double angle,
                          bool large_arc, bool sweep, Point const &p)
{
    _setCommand('A');
    _current_pars.push_back(rx);
    _current_pars.push_back(ry);
    _current_pars.push_back(deg_from_rad(angle));
    _current_pars.push_back(large_arc ? 1. : 0.);
    _current_pars.push_back(sweep ? 1. : 0.);
    _current_pars.push_back(p[X]);
    _current_pars.push_back(p[Y]);

    _current = _quad_tangent = _cubic_tangent = p;
    if (!_optimize) {
        flush();
    }
}

void SVGPathWriter::closePath()
{
    flush();
    if (_optimize) {
        _s << "z";
    } else {
        _s << " z";
    }
    _current = _quad_tangent = _cubic_tangent = _subpath_start;
}

void SVGPathWriter::flush()
{
    if (_command == 0 || _current_pars.empty()) return;

    if (_optimize) {
        _s << _command;
    } else {
        if (_s.tellp() != 0) {
            _s << ' ';
        }
        _s << _command;
    }

    char lastchar = _command;
    bool contained_dot = false;

    for (unsigned i = 0; i < _current_pars.size(); ++i) {
        // TODO: optimize the use of absolute / relative coords
        std::string cs = _formatCoord(_current_pars[i]);

        // Separator handling logic.
        // Floating point values can end with a digit or dot
        // and start with a digit, a plus or minus sign, or a dot.
        // The following cases require a separator:
        // * digit-digit
        // * digit-dot (only if the previous number didn't contain a dot)
        // * dot-digit
        if (_optimize) {
            // C++11: change to front()
            char firstchar = cs[0];
            if (is_digit(lastchar)) {
                if (is_digit(firstchar)) {
                    _s << " ";
                } else if (firstchar == '.' && !contained_dot) {
                    _s << " ";
                }
            } else if (lastchar == '.' && is_digit(firstchar)) {
                _s << " ";
            }
            _s << cs;

            // C++11: change to back()
            lastchar = cs[cs.length()-1];
            contained_dot = cs.find('.') != std::string::npos;
        } else {
            _s << " " << cs;
        }
    }
    _current_pars.clear();
    _command = 0;
}

void SVGPathWriter::clear()
{
    _s.clear();
    _s.str("");
    _ns.clear();
    _ns.str("");
    _command = 0;
    _current_pars.clear();
    _current = Point(0,0);
    _subpath_start = Point(0,0);
}

void SVGPathWriter::setPrecision(int prec)
{
    _precision = prec;
    if (prec < 0) {
        _epsilon = 0;
    } else {
        _epsilon = std::pow(10., -prec);
        _ns << std::setprecision(_precision);
    }
}

void SVGPathWriter::_setCommand(char cmd)
{
    if (_command != 0 && _command != cmd) {
        flush();
    }
    _command = cmd;
}

std::string SVGPathWriter::_formatCoord(Coord par)
{
    std::string ret;
    if (_precision < 0) {
        ret = format_coord_shortest(par);
    } else {
        _ns << par;
        ret = _ns.str();
        _ns.clear();
        _ns.str("");
    }
    return ret;
}


std::string write_svg_path(PathVector const &pv, int prec, bool optimize, bool shorthands)
{
    SVGPathWriter writer;
    writer.setPrecision(prec);
    writer.setOptimize(optimize);
    writer.setUseShorthands(shorthands);

    writer.feed(pv);
    return writer.str();
}

} // namespace Geom

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
