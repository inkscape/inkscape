/*  This file is part of the libdepixelize project
    Copyright (C) 2013 Vinícius dos Santos Oliveira <vini.ipsmaker@gmail.com>

    GNU Lesser General Public License Usage
    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License as published by the
    Free Software Foundation; either version 2.1 of the License, or (at your
    option) any later version.
    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.

    GNU General Public License Usage
    Alternatively, this library may be used under the terms of the GNU General
    Public License as published by the Free Software Foundation, either version
    2 of the License, or (at your option) any later version.
    You should have received a copy of the GNU General Public License along with
    this library.  If not, see <http://www.gnu.org/licenses/>.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
*/

#ifndef LIBDEPIXELIZE_TRACER_SPLINES_KOPF2011_H
#define LIBDEPIXELIZE_TRACER_SPLINES_KOPF2011_H

#include "../splines.h"
#include "homogeneoussplines.h"
#include "optimization-kopf2011.h"

namespace Tracer {

/**
 * Maybe the pass-by-value and then move idiom should be more efficient. But all
 * this is inlinable and we're not even in C++11 yet.
 */
template<class T>
Geom::Path worker_helper(const std::vector< Point<T> > &source1, bool optimize)
{
    typedef Geom::LineSegment Line;
    typedef Geom::QuadraticBezier Quad;
    typedef typename std::vector< Point<T> >::const_iterator iterator;

    std::vector< Point<T> > source;

    if ( optimize )
        source = Tracer::optimize(source1);
    else
        source = source1;

    iterator it = source.begin();
    Point<T> prev = source.back();
    Geom::Path ret(to_geom_point(midpoint(prev, *it)));

    for ( iterator end = source.end() ; it != end ; ++it ) {
#if LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES
        // remove redundant points
        if ( !it->visible ) {
            prev = *it;
            continue;
        }
#endif // LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES

        if ( !prev.visible ) {
            Geom::Point middle = to_geom_point(midpoint(prev, *it));
            if ( ret.finalPoint() != middle ) {
                // All generated invisible points are straight lines
                ret.appendNew<Line>(middle);
            }
        }

        Point<T> next = (it + 1 == end) ? source.front() : *(it + 1);
        Point<T> middle = midpoint(*it, next);

        if ( !it->smooth ) {
            ret.appendNew<Line>(to_geom_point(*it));
            ret.appendNew<Line>(to_geom_point(middle));
        } else {
            ret.appendNew<Quad>(to_geom_point(*it), to_geom_point(middle));
        }

        prev = *it;
    }

    return ret;
}

/**
 * It should be used by worker threads. Convert only one object.
 */
template<class T>
void worker(const typename HomogeneousSplines<T>::Polygon &source,
            Splines::Path &dest, bool optimize)
{
    //dest.pathVector.reserve(source.holes.size() + 1);

    for ( int i = 0 ; i != 4 ; ++i )
        dest.rgba[i] = source.rgba[i];

    dest.pathVector.push_back(worker_helper(source.vertices, optimize));

    for ( typename std::vector< std::vector< Point<T> > >::const_iterator
              it = source.holes.begin(), end = source.holes.end()
              ; it != end ; ++it ) {
        dest.pathVector.push_back(worker_helper(*it, optimize));
    }
}

template<typename T, bool adjust_splines>
Splines::Splines(const SimplifiedVoronoi<T, adjust_splines> &diagram) :
    _width(diagram.width()),
    _height(diagram.height())
{
    _paths.reserve(diagram.size());

    for ( typename SimplifiedVoronoi<T, adjust_splines>::const_iterator
              it = diagram.begin() , end = diagram.end() ; it != end ; ++it ) {
        Path path;

        path.pathVector
            .push_back(Geom::Path(to_geom_point(it->vertices.front())));

        for ( typename std::vector< Point<T> >::const_iterator
                  it2 = ++it->vertices.begin(), end2 = it->vertices.end()
                  ; it2 != end2 ; ++it2 ) {
            path.pathVector.back()
                .appendNew<Geom::LineSegment>(Geom::Point(it2->x, it2->y));
        }

        for ( int i = 0 ; i != 4 ; ++i )
            path.rgba[i] = it->rgba[i];

        _paths.push_back(path);
    }
}

template<class T>
Splines::Splines(const HomogeneousSplines<T> &homogeneousSplines,
                 bool optimize, int /*nthreads*/) :
    _paths(homogeneousSplines.size()),
    _width(homogeneousSplines.width()),
    _height(homogeneousSplines.height())
{
    // TODO: It should be threaded
    iterator paths_it = begin();
    for ( typename HomogeneousSplines<T>::const_iterator
              it = homogeneousSplines.begin(), end = homogeneousSplines.end()
              ; it != end ; ++it, ++paths_it ) {
        worker<T>(*it, *paths_it, optimize);
    }
}

} // namespace Tracer

#endif // LIBDEPIXELIZE_TRACER_SPLINES_KOPF2011_H

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
