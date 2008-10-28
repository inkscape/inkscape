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

#ifndef __2GEOM_REGION_H
#define __2GEOM_REGION_H

#include <2geom/path.h>
#include <2geom/path-intersection.h>

namespace Geom {

class Shape;

class Region {
    friend Crossings crossings(Region const &a, Region const &b);
    friend class Shape;
    friend Shape shape_boolean(bool rev, Shape const & a, Shape const & b, CrossingSet const & crs);

    Path boundary;
    mutable boost::optional<Rect> box;
    bool fill;
  public:
    Region() : fill(true) {}
    explicit Region(Path const &p) : boundary(p) { fill = path_direction(p); }
    Region(Path const &p, bool dir) : boundary(p), fill(dir) {}
    Region(Path const &p, boost::optional<Rect> const &b) : boundary(p), box(b) { fill = path_direction(p); }
    Region(Path const &p, boost::optional<Rect> const &b, bool dir) : boundary(p), box(b), fill(dir) {}
    
    unsigned size() const { return boundary.size(); }
    
    bool isFill() const { return fill; }    
    Region asFill() const { if(fill) return Region(*this); else return inverse(); } 
    Region asHole() const { if(fill) return inverse(); else return Region(*this); }
    
    operator Path() const { return boundary; }
    Rect boundsFast() const {
        if(!box) box = boost::optional<Rect>(boundary.boundsFast());
        return *box;
    }
    
    bool contains(Point const &p) const {
        if(box && !box->contains(p)) return false;
        return Geom::contains(boundary, p);
    }
    bool contains(Region const &other) const { return contains(other.boundary.initialPoint()); }
    
    bool includes(Point const &p) const {
        return logical_xor(!fill, contains(p));
    }
    
    Region inverse() const { return Region(boundary.reverse(), box, !fill); }
    
    Region operator*(Matrix const &m) const;
    
    bool invariants() const;
};

typedef std::vector<Region> Regions;

unsigned outer_index(Regions const &ps);

//assumes they're already sanitized somewhat
inline Regions regions_from_paths(std::vector<Path> const &ps) {
    Regions res;
    for(unsigned i = 0; i < ps.size(); i++)
        res.push_back(Region(ps[i]));
    return res;
}

inline std::vector<Path> paths_from_regions(Regions const &rs) {
    std::vector<Path> res;
    for(unsigned i = 0; i < rs.size(); i++)
        res.push_back(rs[i]);
    return res;
}

Regions sanitize_path(Path const &p);

Regions region_boolean(bool rev, Region const & a, Region const & b, Crossings const &cr);
Regions region_boolean(bool rev, Region const & a, Region const & b, Crossings const & cr_a, Crossings const & cr_b);

inline Regions region_boolean(bool rev, Region const & a, Region const & b) {
    return region_boolean(rev, a, b, crossings(a, b));
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
