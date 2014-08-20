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

#ifndef LIBDEPIXELIZE_TRACER_HOMOGENEOUSSPLINES_H
#define LIBDEPIXELIZE_TRACER_HOMOGENEOUSSPLINES_H

#include "simplifiedvoronoi.h"
#include "point.h"
#include <algorithm>
#include <utility>

namespace Tracer {

template<typename T>
class HomogeneousSplines
{
public:
    struct Polygon
    {
        typedef std::vector< Point<T> > Points;
        typedef typename Points::iterator points_iter;
        typedef typename Points::const_iterator const_points_iter;
        typedef typename std::vector<Points>::iterator holes_iter;
        typedef typename std::vector<Points>::const_iterator const_holes_iter;

        Polygon() {}
        Polygon(const guint8 (&rgba)[4])
        {
            for ( int i = 0 ; i != 4 ; ++i )
                this->rgba[i] = rgba[i];
        }

        std::vector< Point<T> > vertices;

        /**
         * It may be benefited from C++11 move references.
         */
        std::vector< std::vector< Point<T> > > holes;

        guint8 rgba[4];
    };

    typedef typename std::vector<Polygon>::iterator iterator;
    typedef typename std::vector<Polygon>::const_iterator const_iterator;
    typedef typename std::vector<Polygon>::size_type size_type;

    template<bool adjust_splines>
    HomogeneousSplines(const SimplifiedVoronoi<T, adjust_splines> &voronoi);

    // Iterators
    iterator begin()
    {
        return _polygons.begin();
    }

    const_iterator begin() const
    {
        return _polygons.begin();
    }

    iterator end()
    {
        return _polygons.end();
    }

    const_iterator end() const
    {
        return _polygons.end();
    }

    size_type size() const
    {
        return _polygons.size();
    }

    int width() const
    {
        return _width;
    }

    int height() const
    {
        return _height;
    }

private:
    typedef std::vector< Point<T> > Points;
    typedef typename Points::iterator points_iter;
    typedef typename Points::const_iterator points_citer;
    typedef typename Points::reverse_iterator points_riter;
    typedef typename Points::const_reverse_iterator points_criter;

    typedef std::pair<points_iter, points_iter> points_range;
    typedef std::pair<points_citer, points_citer> points_crange;

    struct CommonEdge
    {
        bool ok; //< share an edge
        Points *dst;
        const Points *src;

        // the interval is closed on both ends
        // different from [begin, end) STL style
        points_iter dst_begin, dst_end;
        points_citer src_begin, src_end;
    };

    struct SelfCommonEdge
    {
        bool ok; //< share an edge

        // Greater range. The one that should be erased from the vector.
        points_riter grt_begin, grt_end;

        // Smaller range. The one that should be used to create a new vector.
        points_riter sml_begin, sml_end;
    };

    /**
     * Return ok == true if they share an edge (more than one point).
     */
    CommonEdge _common_edge(Points &dst, const Points &src);

    /**
     * Return ok == true if they share an edge (more than one point).
     *
     *  - [dst_begin, dst_end) will contain the hole polygon
     *  - [src_begin, src_end) will contain the range to be erased
     *
     * It's required to do the search in backward order.
     */
    SelfCommonEdge _common_edge(Points &points, points_riter it);

    /*!
     * Add polygon represented by \p common_edge.src to \p common_edge.dst.
     */
    void _polygon_union(CommonEdge common_edge);

    /**
     * Weird recursive function created to solve the complex problem to fill
     * polygons holes without the need to store temporaries on the heap nor
     * changing requirements to some data type that don't invalidate iterators
     * that point before the current element (maybe I'll write some poetry about
     * the problem someday).
     */
    void _fill_holes(std::vector<Points> &holes, points_iter region_begin,
                     points_iter region_end);

    std::vector<Polygon> _polygons;
    int _width;
    int _height;
};

template<class T>
template<bool adjust_splines>
HomogeneousSplines<T>::HomogeneousSplines(const SimplifiedVoronoi<T,
                                          adjust_splines> &voronoi) :
    _width(voronoi.width()),
    _height(voronoi.height())
{
    //if (!voronoi.size())
    //    return;
    using colorspace::same_color;

    typedef typename SimplifiedVoronoi<T, adjust_splines>::const_iterator
        voronoi_citer;

    // Identify visible edges (group polygons with the same color)
    for ( voronoi_citer cell_it = voronoi.begin(), cell_end = voronoi.end()
              ; cell_it != cell_end ; ++cell_it ) {
        bool found = false;
        for ( iterator polygon_it = _polygons.begin(),
                  polygon_end = _polygons.end()
                  ; polygon_it != polygon_end ; ++polygon_it ) {
            if ( same_color(polygon_it->rgba, cell_it->rgba) ) {
                CommonEdge common_edge = _common_edge(polygon_it->vertices,
                                                      cell_it->vertices);
                if ( common_edge.ok ) {
                    _polygon_union(common_edge);
                    found = true;

                    for ( iterator polygon2_it = polygon_it + 1
                              ; polygon2_it != polygon_end ; ++polygon2_it ) {
                        if ( same_color(polygon_it->rgba, polygon2_it->rgba) ) {
                            CommonEdge common_edge2
                                = _common_edge(polygon_it->vertices,
                                               polygon2_it->vertices);
                            if ( common_edge2.ok ) {
                                _polygon_union(common_edge2);
                                _polygons.erase(polygon2_it);
                                break;
                            }
                        }
                    }

                    break;
                }
            }
        }
        if ( !found ) {
            Polygon polygon(cell_it->rgba);
            polygon.vertices = cell_it->vertices;
            _polygons.insert(_polygons.end(), polygon);
        }
    }

    // Find polygons with holes and fix them
    // This iteration runs such complex time-consuming algorithm, but each
    // polygon has an independent result. They wouldn't even need to share/sync
    // results and the only waste would be a join at the end of the for.
    for ( typename std::vector<Polygon>::iterator it = _polygons.begin(),
              end = _polygons.end() ; it != end ; ++it ) {
        SelfCommonEdge ce = _common_edge(it->vertices, it->vertices.rbegin());
        while ( ce.ok ) {
            _fill_holes(it->holes, ce.sml_end.base(), ce.sml_begin.base());
            it->vertices.erase(ce.grt_end.base() + 1, ce.grt_begin.base());
            ce = _common_edge(it->vertices, ce.grt_end);
        }
    }
}

// it can infinite loop if points of both entities are equal,
// but this shouldn't happen if user has only access to Kopf2011 interface
template<class T>
typename HomogeneousSplines<T>::CommonEdge
HomogeneousSplines<T>::_common_edge(Points &dst, const Points &src)
{
    // It's an edge, then the points are closer together. After we find the
    // first point, there is no need for check against all points of the src
    // a second time

    const points_iter dst_begin = dst.begin();
    const points_iter dst_end = dst.end();

    const points_citer src_begin = src.begin();
    const points_citer src_end = src.end();

    for ( points_iter it = dst_begin ; it != dst_end ; ++it ) {
        points_citer src_it = std::find(src_begin, src_end, *it);

        if ( src_it == src_end )
            continue;

        points_iter dst_common_edge_begin = it;
        points_citer src_common_edge_end = src_it;

        // iterate until find the beginning of the common edge range
        while ( *dst_common_edge_begin == *src_common_edge_end ) {
            if ( dst_common_edge_begin == dst_begin )
                dst_common_edge_begin = dst_end - 1;
            else
                --dst_common_edge_begin;

            ++src_common_edge_end;
            if ( src_common_edge_end == src_end )
                src_common_edge_end = src_begin;
        }

        // fix {dst_begin, src_end} range
        ++dst_common_edge_begin;
        if ( dst_common_edge_begin == dst_end )
            dst_common_edge_begin = dst_begin;

        if ( src_common_edge_end == src_begin )
            src_common_edge_end = src_end - 1;
        else
            --src_common_edge_end;

        points_iter dst_common_edge_end = it;
        points_citer src_common_edge_begin = src_it;

        // find the end of the common edge range
        while ( *dst_common_edge_end == *src_common_edge_begin ) {
            ++dst_common_edge_end;
            if ( dst_common_edge_end == dst_end )
                dst_common_edge_end = dst_begin;

            if ( src_common_edge_begin == src_begin )
                src_common_edge_begin = src_end - 1;
            else
                --src_common_edge_begin;
        }

        // fix {dst_end, src_begin} range
        if ( dst_common_edge_end == dst_begin )
            dst_common_edge_end = dst_end - 1;
        else
            --dst_common_edge_end;

        ++src_common_edge_begin;
        if ( src_common_edge_begin == src_end )
            src_common_edge_begin = src_begin;

        CommonEdge ret;

        // if only one point in common
        if ( dst_common_edge_begin == dst_common_edge_end )
            continue;

        ret.ok = true;

        ret.dst = &dst;
        ret.dst_begin = dst_common_edge_begin;
        ret.dst_end = dst_common_edge_end;

        ret.src = &src;
        ret.src_begin = src_common_edge_begin;
        ret.src_end = src_common_edge_end;

        return ret;
    }

    CommonEdge ret;
    ret.ok = false;
    return ret;
}

template<class T>
typename HomogeneousSplines<T>::SelfCommonEdge
HomogeneousSplines<T>::_common_edge(Points &points, points_riter it)
{
    SelfCommonEdge ret;

    ret.grt_end = points.rend();

    for ( ; it != ret.grt_end ; ++it ) {
        ret.sml_end = std::find(it + 1, ret.grt_end, *it);

        if ( ret.sml_end == ret.grt_end )
            continue;

        ret.grt_begin = it;
        ret.grt_end = ret.sml_end + 1;

        ret.sml_begin = it;

        while ( *ret.sml_begin == *ret.sml_end ) {
            ++ret.sml_begin;
            --ret.sml_end;
        }

        --ret.sml_begin;
        ++ret.sml_end;
        ++ret.sml_end;

        ret.ok = true;
        return ret;
    }

    ret.ok = false;
    return ret;
}

template<class T>
void HomogeneousSplines<T>::_polygon_union(CommonEdge common_edge)
{
    Points &dst = *common_edge.dst;
    const Points &src = *common_edge.src;

    // the rotated cell must be inserted before (dst.begin() + index)
    typename Points::difference_type index;

    // first, we remove the common edge in dst
    if ( common_edge.dst_begin < common_edge.dst_end ) {
        // common edge is in the middle of dst

        index = dst.erase(common_edge.dst_begin,
                          common_edge.dst_end + 1) - dst.begin();
    } else {
        // common edge cross the end of dst

        dst.erase(common_edge.dst_begin, dst.end());
        dst.erase(dst.begin(), common_edge.dst_end);
        index = dst.end() - dst.begin();
    }

    // second, we copy src points to polygon
    if ( common_edge.src_begin < common_edge.src_end ) {
        // common edge is in the middle of src

        const typename Points::difference_type nfirstinserted
            = src.end() - common_edge.src_end;
        const typename Points::difference_type nsecondinserted
            = 1 + (common_edge.src_begin - src.begin());

        dst.reserve(dst.size() + nfirstinserted + nsecondinserted);

        dst.insert(dst.begin() + index, common_edge.src_end, src.end());

        dst.insert(dst.begin() + index + nfirstinserted,
                   src.begin(), common_edge.src_begin + 1);
    } else {
        // common edge cross the end of src

        dst.reserve(dst.size() + 1
                    + (common_edge.src_begin - common_edge.src_end));

        dst.insert(dst.begin() + index,
                   common_edge.src_end, common_edge.src_begin + 1);
    }
}

// The following piece of code is so evil that you could end up invoking an
// ancient beast if you proceed to read it, but I'll be able to explain it in
// the form of some video (text is not so representative as an image).
template<class T>
void HomogeneousSplines<T>::_fill_holes(std::vector<Points> &holes,
                                        points_iter region_begin,
                                        points_iter region_end)
{
    // the exact location might not always be back and iterators will be
    // invalidated after some insertions, then the index is required
    const typename std::vector<Points>::size_type hole_index = holes.size();
    holes.resize(hole_index + 1);

    for ( points_iter it = region_begin + 1 ; it != region_end ; ++it ) {
        points_iter res = std::find(it + 1, region_end, *it);
        if ( res == region_end )
            continue;

        holes[hole_index].insert(holes[hole_index].end(), region_begin,
                                 it);
        region_begin = res;

        do {
            ++it;
            --res;
        } while ( *it == *res );
        _fill_holes(holes, it - 1, res + 2);

        it = region_begin;
    }

    holes[hole_index].insert(holes[hole_index].end(), region_begin,
                             region_end - 1);
}

} // namespace Tracer

#endif // LIBDEPIXELIZE_TRACER_HOMOGENEOUSSPLINES_H

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
