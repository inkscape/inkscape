/*
 * Inkscape::SVG::PathString - builder for SVG path strings
 *
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

#include "svg/path-string.h"
#include "svg/stringstream.h"
#include "svg/svg.h"
#include "prefs-utils.h"
#include <algorithm>

Inkscape::SVG::PathString::PathString() :
    allow_relative_coordinates(0 != prefs_get_int_attribute("options.svgoutput", "allowrelativecoordinates", 1)),
    force_repeat_commands(0 != prefs_get_int_attribute("options.svgoutput", "forcerepeatcommands", 0))
{}

void Inkscape::SVG::PathString::_appendOp(char abs_op, char rel_op) {
    bool abs_op_repeated = _abs_state.prevop == abs_op && !force_repeat_commands;
    bool rel_op_repeated = _rel_state.prevop == rel_op && !force_repeat_commands;
    unsigned int const abs_added_size = abs_op_repeated ? 0 : 2;
    unsigned int const rel_added_size = rel_op_repeated ? 0 : 2;
    if ( false && _rel_state.str.size()+2 < _abs_state.str.size()+abs_added_size && allow_relative_coordinates ) {
        // Copy rel to abs
        _abs_state = _rel_state;
        _abs_state.switches++;
        abs_op_repeated = false;
        // We do not have to copy abs to rel:
        //   _rel_state.str.size()+2 < _abs_state.str.size()+abs_added_size
        //   _rel_state.str.size()+rel_added_size < _abs_state.str.size()+2
        //   _abs_state.str.size()+2 > _rel_state.str.size()+rel_added_size
    } else if ( false && _abs_state.str.size()+2 < _rel_state.str.size()+rel_added_size ) {
        // Copy abs to rel
        _rel_state = _abs_state;
        _abs_state.switches++;
        rel_op_repeated = false;
    }
    if ( !abs_op_repeated ) _abs_state.appendOp(abs_op);
    if ( !rel_op_repeated ) _rel_state.appendOp(rel_op);
}

void Inkscape::SVG::PathString::State::append(NR::Coord v) {
    SVGOStringStream os;
    os << ' ' << v;
    str.append(os.str());
}

void Inkscape::SVG::PathString::State::append(NR::Point p) {
    SVGOStringStream os;
    os << ' ' << p[NR::X] << ',' << p[NR::Y];
    str.append(os.str());
}

void Inkscape::SVG::PathString::State::append(NR::Coord v, NR::Coord &rv) {
    SVGOStringStream os;
    os << v;
    str += ' ';
    str += os.str();
    double c;
    sp_svg_number_read_d(os.str().c_str(), &c);
    rv = c;
}

void Inkscape::SVG::PathString::State::append(NR::Point p, NR::Point &rp) {
    SVGOStringStream osx, osy;
    osx << p[NR::X];
    osy << p[NR::Y];
    str += ' ';
    str += osx.str();
    str += ',';
    str += osy.str();
    double x, y;
    sp_svg_number_read_d(osx.str().c_str(), &x);
    sp_svg_number_read_d(osy.str().c_str(), &y);
    rp[NR::X] = x;
    rp[NR::Y] = y;
}

// NOTE: The following two appendRelative methods will not be exact if the total number of digits needed
// to represent the difference exceeds the precision of a double. This is not very likely though, and if
// it does happen the imprecise value is not likely to be chosen (because it will probably be a lot longer
// than the absolute value).

void Inkscape::SVG::PathString::State::appendRelative(NR::Coord v, NR::Coord r) {
    SVGOStringStream os;
    int precision = (int)os.precision();
    int digitsBegin = (int)floor(log10(fabs(v-r))); // Position of first digit of difference
    int digitsEnd   = (int)floor(log10(std::min(fabs(v),fabs(r)))) - precision; // Position just beyond the last significant digit of the smallest (in absolute sense) number
    os << ' ';
    if (r == 0) {
        os.precision(precision);
        os << v;
    } else if (v == 0) {
        os.precision(precision);
        os << -r;
    } else if (digitsBegin>digitsEnd) {
        os.precision(digitsBegin-digitsEnd);
        os << (v-r);
    } else {
        // This assumes the input numbers are already rounded to 'precision' digits
        os << '0';
    }
    str.append(os.str());
}

void Inkscape::SVG::PathString::State::appendRelative(NR::Point p, NR::Point r) {
    SVGOStringStream os;
    int precision = (int)os.precision();
    int digitsBeginX = (int)floor(log10(fabs(p[NR::X]-r[NR::X]))); // Position of first digit of difference
    int digitsEndX   = (int)floor(log10(std::min(fabs(p[NR::X]),fabs(r[NR::X])))) - precision; // Position just beyond the last significant digit of the smallest (in absolute sense) number
    int digitsBeginY = (int)floor(log10(fabs(p[NR::Y]-r[NR::Y]))); // Position of first digit of difference
    int digitsEndY   = (int)floor(log10(std::min(fabs(p[NR::Y]),fabs(r[NR::Y])))) - precision; // Position just beyond the last significant digit of the smallest (in absolute sense) number
    os << ' ';
    if (r[NR::X] == 0) {
        os.precision(precision);
        os << p[NR::X];
    } else if (p[NR::X] == 0) {
        os.precision(precision);
        os << -r[NR::X];
    } else if (digitsBeginX>digitsEndX) {
        os.precision(digitsBeginX-digitsEndX);
        os << (p[NR::X]-r[NR::X]);
    } else {
        // This assumes the input numbers are already rounded to 'precision' digits
        os << '0';
    }
    os << ',';
    if (r[NR::Y] == 0) {
        os.precision(precision);
        os << p[NR::Y];
    } else if (p[NR::Y] == 0) {
        os.precision(precision);
        os << -r[NR::Y];
    } else if (digitsBeginY>digitsEndY) {
        os.precision(digitsBeginY-digitsEndY);
        os << (p[NR::Y]-r[NR::Y]);
    } else {
        // This assumes the input numbers are already rounded to 'precision' digits
        os << '0';
    }
    str.append(os.str());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
