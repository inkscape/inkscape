/**
 * \file
 * \brief  parse SVG path specifications
 *
 * Copyright 2007 MenTaLguY <mental@rydia.net>
 * Copyright 2007 Aaron Spike <aaron@ekips.org>
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

#ifndef SEEN_SVG_PATH_PARSER_H
#define SEEN_SVG_PATH_PARSER_H

#include <vector>
#include <iterator>
#include <stdexcept>
#include <2geom/exception.h>
#include <2geom/point.h>
#include <2geom/path-sink.h>

namespace Geom {

void parse_svg_path(char const *str, PathSink &sink) throw(SVGPathParseError);

inline std::vector<Path> parse_svg_path(char const *str) throw(SVGPathParseError) {
    typedef std::vector<Path> Subpaths;
    typedef std::back_insert_iterator<Subpaths> Inserter;
    
    Subpaths subpaths;
    Inserter iter(subpaths);
    PathIteratorSink<Inserter> generator(iter);

    parse_svg_path(str, generator);
    return subpaths;
}

inline std::vector<Path> read_svgd_f(FILE * fi) throw(SVGPathParseError) {
    /// @bug The 10kB length limit should be removed
    char input[1024 * 10];
    fgets(input, 1024 * 10, fi);
    return parse_svg_path(input);
}

inline std::vector<Path> read_svgd(char const * name) throw(SVGPathParseError) {
    FILE* fi = fopen(name, "r");
    if(fi == NULL) throw(std::runtime_error("Error opening file"));
    std::vector<Path> out = read_svgd_f(fi);
    fclose(fi);
    return out;
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
