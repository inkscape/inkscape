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
    if ( _rel_state.str.size()+2 < _abs_state.str.size()+abs_added_size && allow_relative_coordinates ) {
        // Copy rel to abs
        _abs_state = _rel_state;
        _abs_state.switches++;
        abs_op_repeated = false;
        // We do not have to copy abs to rel:
        //   _rel_state.str.size()+2 < _abs_state.str.size()+abs_added_size
        //   _rel_state.str.size()+rel_added_size < _abs_state.str.size()+2
        //   _abs_state.str.size()+2 > _rel_state.str.size()+rel_added_size
    } else if ( _abs_state.str.size()+2 < _rel_state.str.size()+rel_added_size ) {
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

static void appendCoord(Glib::ustring& str, NR::Coord v, NR::Coord &rv) {
    Inkscape::SVGOStringStream os;
    os << v;
    str += os.str();
    double c;
    sp_svg_number_read_d(os.str().c_str(), &c);
    rv = c;
    /*{
        Inkscape::SVGOStringStream ost;
        ost << rv;
        if (ost.str()!=os.str()) {
            FILE* file = fopen("pathstring log.txt","at");
            fprintf(file, "v: %g, rv: %g\n", v, rv);
            fclose(file);
        }
    }*/
}

void Inkscape::SVG::PathString::State::append(NR::Point p, NR::Point &rp) {
    str += ' ';
    appendCoord(str, p[NR::X], rp[NR::X]);
    str += ',';
    appendCoord(str, p[NR::Y], rp[NR::Y]);
}

void Inkscape::SVG::PathString::State::append(NR::Coord v, NR::Coord& rv) {
    str += ' ';
    appendCoord(str, v, rv);
}

// NOTE: The following two appendRelative methods will not be exact if the total number of digits needed
// to represent the difference exceeds the precision of a double. This is not very likely though, and if
// it does happen the imprecise value is not likely to be chosen (because it will probably be a lot longer
// than the absolute value).

static void appendRelativeCoord(Glib::ustring& str, NR::Coord v, NR::Coord r) {
    Inkscape::SVGOStringStream os;
    int precision = (int)os.precision();
    int digitsEnd   = (int)floor(log10(std::min(fabs(v),fabs(r)))) - precision; // Position just beyond the last significant digit of the smallest (in absolute sense) number
    double roundeddiff = floor((v-r)*pow(10.,-digitsEnd-1)+.5);
    int numDigits = (int)floor(log10(fabs(roundeddiff)))+1; // Number of digits in roundeddiff
    if (r == 0) {
        os.precision(precision);
        os << v;
    } else if (v == 0) {
        os.precision(precision);
        os << -r;
    } else if (numDigits>0) {
        os.precision(numDigits);
        os << (v-r);
    } else {
        // This assumes the input numbers are already rounded to 'precision' digits
        os << '0';
    }
    str.append(os.str());
    {
        /*double c;
        sp_svg_number_read_d(os.str().c_str(), &c);
        if (fabs((v-r)-c)>5.*pow(10.,digitsEnd)) {
            FILE* file = fopen("pathstring log.txt","at");
            fprintf(file, "bad, v: %.9g, r: %.9g, os: %s, c: %.12g, roundeddiff: %.12g, precision: %d, digitsEnd: %d, numDigits: %d\n", v, r, os.str().c_str(), c, roundeddiff, precision, digitsEnd, numDigits);
            fclose(file);
        }
        Inkscape::SVGOStringStream ostr1, ostr2;
        ostr1 << v;
        ostr2 << (r+c);
        if (ostr1.str() != ostr2.str()) {
            FILE* file = fopen("pathstring log.txt","at");
            fprintf(file, "bad, v: %.9g, r: %.9g, os: %s, c: %.12g, ostr1: %s, ostr2: %s, roundeddiff: %.12g\n", v, r, os.str().c_str(), c, ostr1.str().c_str(), ostr2.str().c_str(), roundeddiff);
            fclose(file);
        }*/
        /*FILE* file = fopen("pathstring log.txt","at");
        fprintf(file, "good, v: %.9g, r: %.9g, os: %s, c: %.12g, roundeddiff: %.12g, precision: %d, digitsEnd: %d, numDigits: %d\n", v, r, os.str().c_str(), c, roundeddiff, precision, digitsEnd, numDigits);
        fclose(file);*/
    }
}

void Inkscape::SVG::PathString::State::appendRelative(NR::Point p, NR::Point r) {
    str += ' ';
    appendRelativeCoord(str, p[NR::X], r[NR::X]);
    str += ',';
    appendRelativeCoord(str, p[NR::Y], r[NR::Y]);
}

void Inkscape::SVG::PathString::State::appendRelative(NR::Coord v, NR::Coord r) {
    str += ' ';
    appendRelativeCoord(str, v, r);
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
