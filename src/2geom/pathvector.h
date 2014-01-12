/**
 * \file
 * \brief PathVector - std::vector containing Geom::Path.
 * This file provides a set of operations that can be performed on PathVector,
 * e.g. an affine transform.
 *//*
 * Authors:
 *  Johan Engelen <goejendaagh@zonnet.nl>
 * 
 * Copyright 2008  authors
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

#ifndef LIB2GEOM_SEEN_PATHVECTOR_H
#define LIB2GEOM_SEEN_PATHVECTOR_H

#include <2geom/forward.h>
#include <2geom/path.h>
#include <2geom/transforms.h>

namespace Geom {

typedef std::vector<Geom::Path> PathVector;

/* general path transformation: */
inline
void operator*= (PathVector & path_in, Affine const &m) {
    for(PathVector::iterator it = path_in.begin(); it != path_in.end(); ++it) {
        (*it) *= m;
    }
}
inline
PathVector operator*(PathVector const & path_in, Affine const &m) {
    PathVector ret(path_in);
    ret *= m;
    return ret;
}

/* specific path transformations: Translation: 
 * This makes it possible to make optimized implementations for Translate transforms */
inline
void operator*= (PathVector & path_in, Translate const &m) {
    for(PathVector::iterator it = path_in.begin(); it != path_in.end(); ++it) {
        (*it) *= m;
    }
}
inline
PathVector operator*(PathVector const & path_in, Translate const &m) {
    PathVector ret(path_in);
    ret *= m;
    return ret;
}

/* user friendly approach to Translate transforms: just add an offset Point to the whole path */
inline
void operator+=(PathVector &path_in, Point const &p) {
    for(PathVector::iterator it = path_in.begin(); it != path_in.end(); ++it) {
        (*it) *= Translate(p);
    }
}
inline
PathVector operator+(PathVector const &path_in, Point const &p) {
    PathVector ret(path_in);
    ret *= Translate(p);
    return ret;
}

inline
Geom::Point initialPoint(PathVector const &path_in)
{
    return path_in.front().initialPoint();
}

inline
Geom::Point finalPoint(PathVector const &path_in)
{
    return path_in.back().finalPoint();
}

PathVector reverse_paths_and_order (PathVector const & path_in);

OptRect bounds_fast( PathVector const & pv );
OptRect bounds_exact( PathVector const & pv );

struct PathVectorPosition {
    // pathvector[path_nr].pointAt(t) is the position
    unsigned int path_nr;
    double       t;
    PathVectorPosition() :
        path_nr(0),
        t(0)
        {}
    PathVectorPosition(unsigned int path_nr,
                       double       t) : path_nr(path_nr), t(t) {}
};
boost::optional<PathVectorPosition> nearestPoint(PathVector const & path_in, Point const& _point, double *distance_squared = NULL);

std::vector<PathVectorPosition> allNearestPoints(PathVector const & path_in, Point const& _point, double *distance_squared = NULL);

inline
Point pointAt(PathVector const & path_in, PathVectorPosition const &pvp) {
    return path_in[pvp.path_nr].pointAt(pvp.t);
}

} // end namespace Geom

#endif // LIB2GEOM_SEEN_PATHVECTOR_H

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
