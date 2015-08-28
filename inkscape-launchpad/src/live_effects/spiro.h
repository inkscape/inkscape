/*
Copyright (C) 2007-2012 Authors

Authors: Raph Levien
         Johan Engelen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/

#ifndef INKSCAPE_SPIRO_H
#define INKSCAPE_SPIRO_H

#include "live_effects/spiro-converters.h"
#include <2geom/forward.h>

class SPCurve;

namespace Spiro {

typedef struct {
    double x;
    double y;
    char ty;
} spiro_cp;


void spiro_run(const spiro_cp *src, int src_len, SPCurve &curve);
void spiro_run(const spiro_cp *src, int src_len, Geom::Path &path);

/* the following methods are only for expert use: */
typedef struct spiro_seg_s spiro_seg;
spiro_seg * run_spiro(const spiro_cp *src, int n);
void free_spiro(spiro_seg *s);
void spiro_to_otherpath(const spiro_seg *s, int n, ConverterBase &bc);
double get_knot_th(const spiro_seg *s, int i);


} // namespace Spiro

#endif // INKSCAPE_SPIRO_H
