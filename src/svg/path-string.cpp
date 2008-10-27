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
#include "preferences.h"
#include <algorithm>

// 1<=numericprecision<=16, doubles are only accurate upto (slightly less than) 16 digits (and less than one digit doesn't make sense)
// Please note that these constants are used to allocate sufficient space to hold serialized numbers
static int const minprec = 1;
static int const maxprec = 16;

int Inkscape::SVG::PathString::numericprecision;
int Inkscape::SVG::PathString::minimumexponent;

Inkscape::SVG::PathString::PathString() :
    allow_relative_coordinates(Inkscape::Preferences::get()->getBool("/options/svgoutput/allowrelativecoordinates", true)),
    force_repeat_commands(Inkscape::Preferences::get()->getBool("/options/svgoutput/forcerepeatcommands"))
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    numericprecision = std::max<int>(minprec,std::min<int>(maxprec, prefs->getInt("/options/svgoutput/numericprecision", 8)));
    minimumexponent = prefs->getInt("/options/svgoutput/minimumexponent", -8);
}

void Inkscape::SVG::PathString::_appendOp(char abs_op, char rel_op) {
    bool abs_op_repeated = _abs_state.prevop == abs_op && !force_repeat_commands;
    bool rel_op_repeated = _rel_state.prevop == rel_op && !force_repeat_commands;
    unsigned int const abs_added_size = abs_op_repeated ? 0 : 2;
    unsigned int const rel_added_size = rel_op_repeated ? 0 : 2;
    if ( _rel_state.str.size()+2 < _abs_state.str.size()+abs_added_size && allow_relative_coordinates ) {
        // Store common prefix
        commonbase += _rel_state.str;
        _rel_state.str.clear();
        // Copy rel to abs
        _abs_state = _rel_state;
        _abs_state.switches++;
        abs_op_repeated = false;
        // We do not have to copy abs to rel:
        //   _rel_state.str.size()+2 < _abs_state.str.size()+abs_added_size
        //   _rel_state.str.size()+rel_added_size < _abs_state.str.size()+2
        //   _abs_state.str.size()+2 > _rel_state.str.size()+rel_added_size
    } else if ( _abs_state.str.size()+2 < _rel_state.str.size()+rel_added_size ) {
        // Store common prefix
        commonbase += _abs_state.str;
        _abs_state.str.clear();
        // Copy abs to rel
        _rel_state = _abs_state;
        _abs_state.switches++;
        rel_op_repeated = false;
    }
    if ( !abs_op_repeated ) _abs_state.appendOp(abs_op);
    if ( !rel_op_repeated ) _rel_state.appendOp(rel_op);
}

void Inkscape::SVG::PathString::State::append(NR::Coord v) {
    str += ' ';
    appendNumber(v);
}

void Inkscape::SVG::PathString::State::append(NR::Point p) {
    str += ' ';
    appendNumber(p[NR::X]);
    str += ',';
    appendNumber(p[NR::Y]);
}

void Inkscape::SVG::PathString::State::append(NR::Coord v, NR::Coord& rv) {
    str += ' ';
    appendNumber(v, rv);
}

void Inkscape::SVG::PathString::State::append(NR::Point p, NR::Point &rp) {
    str += ' ';
    appendNumber(p[NR::X], rp[NR::X]);
    str += ',';
    appendNumber(p[NR::Y], rp[NR::Y]);
}

// NOTE: The following appendRelativeCoord function will not be exact if the total number of digits needed
// to represent the difference exceeds the precision of a double. This is not very likely though, and if
// it does happen the imprecise value is not likely to be chosen (because it will probably be a lot longer
// than the absolute value).

// NOTE: This assumes v and r are already rounded (this includes flushing to zero if they are < 10^minexp)
void Inkscape::SVG::PathString::State::appendRelativeCoord(NR::Coord v, NR::Coord r) {
    int const minexp = minimumexponent-numericprecision+1;
    int const digitsEnd = (int)floor(log10(std::min(fabs(v),fabs(r)))) - numericprecision; // Position just beyond the last significant digit of the smallest (in absolute sense) number
    double const roundeddiff = floor((v-r)*pow(10.,-digitsEnd-1)+.5);
    int const numDigits = (int)floor(log10(fabs(roundeddiff)))+1; // Number of digits in roundeddiff
    if (r == 0) {
        appendNumber(v, numericprecision, minexp);
    } else if (v == 0) {
        appendNumber(-r, numericprecision, minexp);
    } else if (numDigits>0) {
        appendNumber(v-r, numDigits, minexp);
    } else {
        // This assumes the input numbers are already rounded to 'precision' digits
        str += '0';
    }
}

void Inkscape::SVG::PathString::State::appendRelative(NR::Point p, NR::Point r) {
    str += ' ';
    appendRelativeCoord(p[NR::X], r[NR::X]);
    str += ',';
    appendRelativeCoord(p[NR::Y], r[NR::Y]);
}

void Inkscape::SVG::PathString::State::appendRelative(NR::Coord v, NR::Coord r) {
    str += ' ';
    appendRelativeCoord(v, r);
}

void Inkscape::SVG::PathString::State::appendNumber(double v, int precision, int minexp) {
    size_t const reserve = precision+1+1+1+1+3; // Just large enough to hold the maximum number of digits plus a sign, a period, the letter 'e', another sign and three digits for the exponent
    size_t const oldsize = str.size();
    str.append(reserve, (char)0);
    char* begin_of_num = const_cast<char*>(str.data()+oldsize); // Slightly evil, I know (but std::string should be storing its data in one big block of memory, so...)
    size_t added = sp_svg_number_write_de(begin_of_num, v, precision, minexp);
    str.resize(oldsize+added); // remove any trailing characters
}

void Inkscape::SVG::PathString::State::appendNumber(double v, double &rv, int precision, int minexp) {
    size_t const oldsize = str.size();
    appendNumber(v, precision, minexp);
    char* begin_of_num = const_cast<char*>(str.data()+oldsize); // Slightly evil, I know (but std::string should be storing its data in one big block of memory, so...)
    sp_svg_number_read_d(begin_of_num, &rv);
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
