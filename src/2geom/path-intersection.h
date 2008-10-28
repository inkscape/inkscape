/**
 * \file
 * \brief  \todo brief description
 *
 * Authors:
 *      ? <?@?.?>
 * 
 * Copyright ?-?  authors
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

#ifndef __GEOM_PATH_INTERSECTION_H
#define __GEOM_PATH_INTERSECTION_H

#include <2geom/path.h>

#include <2geom/crossing.h>

#include <2geom/sweep.h>

namespace Geom {

int winding(Path const &path, Point p);
bool path_direction(Path const &p);

inline bool contains(Path const & p, Point i, bool evenodd = true) {
    return (evenodd ? winding(p, i) % 2 : winding(p, i)) != 0;
}

template<typename T>
Crossings curve_sweep(Path const &a, Path const &b) {
    T t;
    Crossings ret;
    std::vector<Rect> bounds_a = bounds(a), bounds_b = bounds(b);
    std::vector<std::vector<unsigned> > ixs = sweep_bounds(bounds_a, bounds_b);
    for(unsigned i = 0; i < a.size(); i++) {
        for(std::vector<unsigned>::iterator jp = ixs[i].begin(); jp != ixs[i].end(); jp++) {
            Crossings cc = t.crossings(a[i], b[*jp]);
            offset_crossings(cc, i, *jp);
            ret.insert(ret.end(), cc.begin(), cc.end());
        }
    }
    return ret;
}

struct SimpleCrosser : public Crosser<Path> {
    Crossings crossings(Curve const &a, Curve const &b);
    Crossings crossings(Path const &a, Path const &b) { return curve_sweep<SimpleCrosser>(a, b); }
    CrossingSet crossings(std::vector<Path> const &a, std::vector<Path> const &b) { return Crosser<Path>::crossings(a, b); }
};

struct MonoCrosser : public Crosser<Path> {
    Crossings crossings(Path const &a, Path const &b) { return crossings(std::vector<Path>(1,a), std::vector<Path>(1,b))[0]; }
    CrossingSet crossings(std::vector<Path> const &a, std::vector<Path> const &b);
};

typedef SimpleCrosser DefaultCrosser;

std::vector<double> path_mono_splits(Path const &p);

CrossingSet crossings_among(std::vector<Path> const & p);
Crossings self_crossings(Path const & a);

inline Crossings crossings(Curve const & a, Curve const & b) {
    DefaultCrosser c = DefaultCrosser();
    return c.crossings(a, b);
}

inline Crossings crossings(Path const & a, Path const & b) {
    DefaultCrosser c = DefaultCrosser();
    return c.crossings(a, b);
}

inline CrossingSet crossings(std::vector<Path> const & a, std::vector<Path> const & b) {
    DefaultCrosser c = DefaultCrosser();
    return c.crossings(a, b);
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
