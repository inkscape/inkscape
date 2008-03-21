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
#include "prefs-utils.h"

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
