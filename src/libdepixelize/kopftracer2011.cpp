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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm.h>

#include <algorithm>
#include "kopftracer2011.h"
#include "priv/colorspace.h"
#include "priv/homogeneoussplines.h"
#include "priv/branchless.h"
#include "priv/splines-kopf2011.h"
#include "priv/iterator.h"

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
#include <glibmm/datetime.h>
#include <iostream>
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

namespace Tracer {
namespace Heuristics {

int curves(const PixelGraph &graph, PixelGraph::const_iterator a,
           PixelGraph::const_iterator b);
bool islands(PixelGraph::const_iterator a, PixelGraph::const_iterator b);

struct SparsePixels
{
    enum Diagonal {
        /**
         * From (first) the top left corner to (second) the bottom right.
         */
        MAIN_DIAGONAL      = 0,
        /**
         * From (first) the top right to (second) the bottom left.
         */
        SECONDARY_DIAGONAL = 1
    };

    typedef std::pair<PixelGraph::const_iterator, PixelGraph::const_iterator>
    Edge;
    typedef std::pair<Edge, int> EdgeWeight;

    void operator()(const PixelGraph &graph, unsigned radius);

    static bool similar_colors(PixelGraph::const_iterator n,
                               const guint8 (&a)[4], const guint8 (&b)[4]);

    /*
     * Precondition: Must be filled according to Diagonal enum.
     */
    EdgeWeight diagonals[2];
};

} // namespace Heuristics

Splines Kopf2011::to_voronoi(const std::string &filename,
                             const Options &options)
{
    return to_voronoi(Gdk::Pixbuf::create_from_file(filename), options);
}

Splines Kopf2011::to_voronoi(const Glib::RefPtr<Gdk::Pixbuf const> &buf,
                             const Options &options)
{
#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    SimplifiedVoronoi<Precision, false> voronoi
        = _voronoi<Precision, false>(buf, options);

    Glib::DateTime profiling_info[2];
    profiling_info[0] = Glib::DateTime::create_now_utc();

    Splines ret(voronoi);

    profiling_info[1] = Glib::DateTime::create_now_utc();
    std::cerr << "Tracer::Splines construction time: "
        << profiling_info[1].difference(profiling_info[0])
        << std::endl;

    return ret;
#else // LIBDEPIXELIZE_PROFILE_KOPF2011
    return Splines(_voronoi<Precision, false>(buf, options));
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011
}

Splines Kopf2011::to_grouped_voronoi(const std::string &filename,
                             const Options &options)
{
    return to_grouped_voronoi(Gdk::Pixbuf::create_from_file(filename), options);
}

Splines Kopf2011::to_grouped_voronoi(const Glib::RefPtr<Gdk::Pixbuf const> &buf,
                             const Options &options)
{
#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    SimplifiedVoronoi<Precision, false> voronoi
        = _voronoi<Precision, false>(buf, options);

    Glib::DateTime profiling_info[2];
    profiling_info[0] = Glib::DateTime::create_now_utc();

    HomogeneousSplines<Precision> splines(voronoi);

#else // LIBDEPIXELIZE_PROFILE_KOPF2011
    HomogeneousSplines<Precision> splines(_voronoi<Precision, false>
                                          (buf, options));
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    profiling_info[1] = Glib::DateTime::create_now_utc();
    std::cerr << "Tracer::HomogeneousSplines<" << typeid(Precision).name()
              << ">(Tracer::SimplifiedVoronoi<" << typeid(Precision).name()
              << ",false>) construction time: "
              << profiling_info[1].difference(profiling_info[0])
              << std::endl;
    profiling_info[0] = Glib::DateTime::create_now_utc();
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

    for ( HomogeneousSplines<Precision>::iterator it = splines.begin(),
              end = splines.end() ; it != end ; ++it ) {
        for ( HomogeneousSplines<Precision>::Polygon::points_iter
                  it2 = it->vertices.begin(), end2 = it->vertices.end()
                  ; it2 != end2 ; ++it2 ) {
            it2->smooth = false;
        }
        for ( HomogeneousSplines<Precision>::Polygon::holes_iter
                  it2 = it->holes.begin(), end2 = it->holes.end()
                  ; it2 != end2 ; ++it2 ) {
            for ( HomogeneousSplines<Precision>::Polygon::points_iter
                      it3 = it2->begin(), end3 = it2->end()
                      ; it3 != end3 ; ++it3 ) {
                it3->smooth = false;
            }
        }
    }

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    profiling_info[1] = Glib::DateTime::create_now_utc();
    std::cerr << "Tracer::Kopf2011::to_grouped_voronoi internal work time: "
              << profiling_info[1].difference(profiling_info[0])
              << std::endl;
    profiling_info[0] = Glib::DateTime::create_now_utc();

    Splines ret(splines, false, options.nthreads);

    profiling_info[1] = Glib::DateTime::create_now_utc();
    std::cerr << "Tracer::Splines construction time: "
        << profiling_info[1].difference(profiling_info[0])
        << std::endl;

    return ret;
#else // LIBDEPIXELIZE_PROFILE_KOPF2011
    return Splines(splines, false, options.nthreads);
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011
}

Splines Kopf2011::to_splines(const std::string &filename,
                             const Options &options)
{
    return to_splines(Gdk::Pixbuf::create_from_file(filename), options);
}

Splines Kopf2011::to_splines(const Glib::RefPtr<Gdk::Pixbuf const> &buf,
                             const Options &options)
{
#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    SimplifiedVoronoi<Precision, true> voronoi
        = _voronoi<Precision, true>(buf, options);

    Glib::DateTime profiling_info[2];
    profiling_info[0] = Glib::DateTime::create_now_utc();

    HomogeneousSplines<Precision> splines(voronoi);

    profiling_info[1] = Glib::DateTime::create_now_utc();
    std::cerr << "Tracer::HomogeneousSplines<" << typeid(Precision).name()
        << "> construction time: "
        << profiling_info[1].difference(profiling_info[0])
        << std::endl;

    profiling_info[0] = Glib::DateTime::create_now_utc();

    Splines ret(splines, options.optimize, options.nthreads);

    profiling_info[1] = Glib::DateTime::create_now_utc();
    std::cerr << "Tracer::Splines construction time: "
        << profiling_info[1].difference(profiling_info[0])
        << std::endl;

    return ret;
#else // LIBDEPIXELIZE_PROFILE_KOPF2011
    HomogeneousSplines<Precision> splines(_voronoi<Precision, true>
                                          (buf, options));
    return Splines(splines, options.optimize, options.nthreads);
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011
}

template<class T, bool adjust_splines>
SimplifiedVoronoi<T, adjust_splines>
Kopf2011::_voronoi(const Glib::RefPtr<Gdk::Pixbuf const> &buf,
                   const Options &options)
{
#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    Glib::DateTime profiling_info[2];
    profiling_info[0] = Glib::DateTime::create_now_utc();
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

    PixelGraph graph(buf);

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    profiling_info[1] = Glib::DateTime::create_now_utc();
    std::cerr << "Tracer::PixelGraph creation time: "
              << profiling_info[1].difference(profiling_info[0]) << std::endl;
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

    // gdk-pixbuf2 already checks if image size is meaningful, but asserts state
    // preconditions and will be useful if gdk-pixbuf is replaced later
    assert(graph.width() > 0);
    assert(graph.height() > 0);

#ifndef NDEBUG
    graph.checkConsistency();
#endif

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    profiling_info[0] = Glib::DateTime::create_now_utc();
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

    // This step could be part of the initialization of PixelGraph
    // and decrease the necessary number of passes
    graph.connectAllNeighbors();

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    profiling_info[1] = Glib::DateTime::create_now_utc();
    std::cerr << "Tracer::PixelGraph::connectAllNeighbors() time: "
              << profiling_info[1].difference(profiling_info[0]) << std::endl;
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

#ifndef NDEBUG
    graph.checkConsistency();
#endif

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    profiling_info[0] = Glib::DateTime::create_now_utc();
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

    // This step can't be part of PixelGraph initilization without adding some
    // cache misses due to random access patterns that might be injected
    _disconnect_neighbors_with_dissimilar_colors(graph);

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    profiling_info[1] = Glib::DateTime::create_now_utc();
    std::cerr << "Tracer::Kopf2011::"
        "_disconnect_neighbors_with_dissimilar_colors(Tracer::PixelGraph) time: "
              << profiling_info[1].difference(profiling_info[0]) << std::endl;
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

#ifndef NDEBUG
    graph.checkConsistency();
#endif

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    profiling_info[0] = Glib::DateTime::create_now_utc();
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

    {
        // edges_safe and edges_unsafe must be executed in separate.
        // Otherwise, there will be colateral effects due to misassumption about
        // the data being read.
        PixelGraph::EdgePairContainer edges = graph.crossingEdges();

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    profiling_info[1] = Glib::DateTime::create_now_utc();
    std::cerr << "Tracer::PixelGraph::crossingEdges() time: "
              << profiling_info[1].difference(profiling_info[0]) << std::endl;
    profiling_info[0] = Glib::DateTime::create_now_utc();
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

        _remove_crossing_edges_safe(edges);

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
        profiling_info[1] = Glib::DateTime::create_now_utc();
        std::cerr << "Tracer::Kopf2011::_remove_crossing_edges_safe"
            "(Tracer::PixelGraph) time: "
                  << profiling_info[1].difference(profiling_info[0])
                  << std::endl;
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

#ifndef NDEBUG
        graph.checkConsistency();
#endif

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
        profiling_info[0] = Glib::DateTime::create_now_utc();
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

        _remove_crossing_edges_unsafe(graph, edges, options);
    }

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    profiling_info[1] = Glib::DateTime::create_now_utc();
    std::cerr << "Tracer::Kopf2011::_remove_crossing_edges_unsafe"
        "(Tracer::PixelGraph) time: "
              << profiling_info[1].difference(profiling_info[0]) << std::endl;
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011

#ifndef NDEBUG
    graph.checkConsistency();
#endif

    assert(graph.crossingEdges().size() == 0);

#ifdef LIBDEPIXELIZE_PROFILE_KOPF2011
    profiling_info[0] = Glib::DateTime::create_now_utc();

    SimplifiedVoronoi<T, adjust_splines> ret(graph);

    profiling_info[1] = Glib::DateTime::create_now_utc();
    std::cerr << "Tracer::SimplifiedVoronoi<" << typeid(T).name() << ','
              << (adjust_splines ? "true" : "false")
              << ">(Tracer::PixelGraph) construction time: "
              << profiling_info[1].difference(profiling_info[0]) << std::endl;

    return ret;
#else // LIBDEPIXELIZE_PROFILE_KOPF2011
    return SimplifiedVoronoi<T, adjust_splines>(graph);
#endif // LIBDEPIXELIZE_PROFILE_KOPF2011
}

// TODO: move this function (plus connectAllNeighbors) to PixelGraph constructor
inline void
Kopf2011::_disconnect_neighbors_with_dissimilar_colors(PixelGraph &graph)
{
    using colorspace::similar_colors;
    for ( PixelGraph::iterator it = graph.begin(), end = graph.end() ; it != end
              ; ++it ) {
        if ( it->adj.top )
            it->adj.top = similar_colors(it->rgba, (it - graph.width())->rgba);
        if ( it->adj.topright ) {
            it->adj.topright
                = similar_colors(it->rgba, (it - graph.width() + 1)->rgba);
        }
        if ( it->adj.right )
            it->adj.right = similar_colors(it->rgba, (it + 1)->rgba);
        if ( it->adj.bottomright ) {
            it->adj.bottomright
                = similar_colors(it->rgba, (it + graph.width() + 1)->rgba);
        }
        if ( it->adj.bottom ) {
            it->adj.bottom
                = similar_colors(it->rgba, (it + graph.width())->rgba);
        }
        if ( it->adj.bottomleft ) {
            it->adj.bottomleft
                = similar_colors(it->rgba, (it + graph.width() - 1)->rgba);
        }
        if ( it->adj.left )
            it->adj.left = similar_colors(it->rgba, (it - 1)->rgba);
        if ( it->adj.topleft ) {
            it->adj.topleft = similar_colors(it->rgba,
                                             (it - graph.width() - 1)->rgba);
        }
    }
}

/**
 * This method removes crossing edges if the 2x2 block is fully connected.
 *
 * In this case the two diagonal connections can be safely removed without
 * affecting the final result.
 */
template<class T>
void Kopf2011::_remove_crossing_edges_safe(T &container)
{
    for ( typename T::reverse_iterator it = container.rbegin(),
              end = container.rend() ; it != end ; ) {
        /* A | B
           --+--
           C | D */
        PixelGraph::iterator a = it->first.first;
        PixelGraph::iterator b = it->second.first;
        PixelGraph::iterator c = it->second.second;
        PixelGraph::iterator d = it->first.second;

        if ( !a->adj.right || !a->adj.bottom || !b->adj.bottom
             || !c->adj.right ) {
            ++it;
            continue;
        }

        // main diagonal
        a->adj.bottomright = 0;
        d->adj.topleft = 0;

        // secondary diagonal
        b->adj.bottomleft = 0;
        c->adj.topright = 0;

        // base iterator is always past one
        typename T::iterator current = --(it.base());
        ++it;
        container.erase(current);
    }
}

/**
 * This method removes crossing edges using the heuristics.
 */
template<class T>
void Kopf2011::_remove_crossing_edges_unsafe(PixelGraph &graph, T &edges,
                                             const Options &options)
{
    std::vector< std::pair<int, int> > weights(edges.size(),
                                               std::make_pair(0, 0));

    // Compute weights
    for ( typename T::size_type i = 0 ; i != edges.size() ; ++i ) {
        /* A | B
           --+--
           C | D */
        PixelGraph::iterator a = edges[i].first.first;
        PixelGraph::iterator b = edges[i].second.first;
        PixelGraph::iterator c = edges[i].second.second;
        PixelGraph::iterator d = edges[i].first.second;

        // Curves heuristic
        weights[i].first += Heuristics::curves(graph, a, d)
            * options.curvesMultiplier;
        weights[i].second += Heuristics::curves(graph, b, c)
            * options.curvesMultiplier;

        // Islands heuristic
        weights[i].first += Heuristics::islands(a, d) * options.islandsWeight;
        weights[i].second += Heuristics::islands(b, c) * options.islandsWeight;

        // Sparse pixels heuristic
        using Heuristics::SparsePixels;
        SparsePixels sparse_pixels;

        sparse_pixels.diagonals[SparsePixels::MAIN_DIAGONAL].first
            = edges[i].first;
        sparse_pixels.diagonals[SparsePixels::SECONDARY_DIAGONAL].first
            = edges[i].second;

        sparse_pixels(graph, options.sparsePixelsRadius);

        weights[i].first
            += sparse_pixels.diagonals[SparsePixels::MAIN_DIAGONAL].second
            * options.sparsePixelsMultiplier;
        weights[i].second
            += sparse_pixels.diagonals[SparsePixels::SECONDARY_DIAGONAL].second
            * options.sparsePixelsMultiplier;
    }

    // Remove edges with lower weight
    for ( typename T::size_type i = 0 ; i != edges.size() ; ++i ) {
        /* A | B
           --+--
           C | D */
        PixelGraph::iterator a = edges[i].first.first;
        PixelGraph::iterator b = edges[i].second.first;
        PixelGraph::iterator c = edges[i].second.second;
        PixelGraph::iterator d = edges[i].first.second;

        if ( weights[i].first > weights[i].second ) {
            b->adj.bottomleft = 0;
            c->adj.topright = 0;
        } else if ( weights[i].first < weights[i].second ) {
            a->adj.bottomright = 0;
            d->adj.topleft = 0;
        } else {
            a->adj.bottomright = 0;
            b->adj.bottomleft = 0;
            c->adj.topright = 0;
            d->adj.topleft = 0;
        }
    }

    edges.clear();
}

inline int Heuristics::curves(const PixelGraph &graph,
                              PixelGraph::const_iterator a,
                              PixelGraph::const_iterator b)
{
    int count = 1;
    ToPtr<PixelGraph::Node> to_ptr;
    ToIter<PixelGraph::Node> to_iter(graph.begin());

    // b -> a
    // and then a -> b
    for ( int i = 0 ; i != 2 ; ++i ) {
        PixelGraph::const_iterator it = i ? a : b;
        PixelGraph::const_iterator prev = i ? b : a;
        int local_count = 0;

        // Used to avoid inifinite loops in circular-like edges
        const PixelGraph::const_iterator initial = it;

        while ( it->adjsize() == 2 ) {
            ++local_count;

            // Iterate to next
            {
                // There are only two values that won't be zero'ed
                // and one of them has the same value of prev
                guintptr aux = (it->adj.top
                       * guintptr(to_ptr(graph.nodeTop(it))))
                    + (it->adj.topright
                       * guintptr(to_ptr(graph.nodeTopRight(it))))
                    + (it->adj.right
                       * guintptr(to_ptr(graph.nodeRight(it))))
                    + (it->adj.bottomright
                       * guintptr(to_ptr(graph.nodeBottomRight(it))))
                    + (it->adj.bottom
                       * guintptr(to_ptr(graph.nodeBottom(it))))
                    + (it->adj.bottomleft
                       * guintptr(to_ptr(graph.nodeBottomLeft(it))))
                    + (it->adj.left
                       * guintptr(to_ptr(graph.nodeLeft(it))))
                    + (it->adj.topleft
                       * guintptr(to_ptr(graph.nodeTopLeft(it))))
                    - guintptr(to_ptr(prev));
                prev = it;
                it = to_iter(reinterpret_cast<PixelGraph::Node const*>(aux));
            }

            // Break infinite loops
            if ( it == initial )
                return local_count;
        }
        count += local_count;
    }

    return count;
}

inline void Heuristics::SparsePixels::operator ()(const PixelGraph &graph,
                                                  unsigned radius)
{
    if ( !graph.width() || !graph.height() )
        return;

    // Clear weights
    for ( int i = 0 ; i != 2 ; ++i )
        diagonals[i].second = 0;

    if ( !radius )
        return;

    // Fix radius/bounds
    {
        unsigned x = graph.toX(diagonals[MAIN_DIAGONAL].first.first);
        unsigned y = graph.toY(diagonals[MAIN_DIAGONAL].first.first);
        unsigned displace = radius - 1;

        {
            unsigned minor = std::min(x, y);

            if ( displace > minor ) {
                displace = minor;
                radius = displace + 1;
            }
        }

        displace = radius;

        if ( x + displace >= unsigned(graph.width()) ) {
            displace = unsigned(graph.width()) - x - 1;
            radius = displace;
        }

        if ( y + displace >= unsigned(graph.height()) ) {
            displace = unsigned(graph.height()) - y - 1;
            radius = displace;
        }
    }

    if ( !radius )
        return;

    // Iterate over nodes and count them
    {
        PixelGraph::const_iterator it = diagonals[MAIN_DIAGONAL].first.first;
        for ( unsigned i = radius - 1 ; i ; --i )
            it = graph.nodeTopLeft(it);

        bool invert = false;
        for ( unsigned i = 0 ; i != 2 * radius ; ++i ) {
            for ( unsigned j = 0 ; j != 2 * radius ; ++j ) {
                for ( int k = 0 ; k != 2 ; ++k ) {
                    diagonals[k].second
                        += similar_colors(it, diagonals[k].first.first->rgba,
                                          diagonals[k].first.second->rgba);
                }
                it = (invert ? graph.nodeLeft(it) : graph.nodeRight(it));
            }
            it = (invert ? graph.nodeRight(it) : graph.nodeLeft(it));


            invert = !invert;
            it = graph.nodeBottom(it);
        }
    }

    int minor = std::min(diagonals[0].second, diagonals[1].second);
    for ( int i = 0 ; i != 2 ; ++i )
        diagonals[i].second -= minor;
    std::swap(diagonals[0].second, diagonals[1].second);
}

inline bool
Heuristics::SparsePixels::similar_colors(PixelGraph::const_iterator n,
                                         const guint8 (&a)[4],
                                         const guint8 (&b)[4])
{
    using colorspace::similar_colors;
    return similar_colors(n->rgba, a) || similar_colors(n->rgba, b);
}

inline bool Heuristics::islands(PixelGraph::const_iterator a,
                                PixelGraph::const_iterator b)
{
    if ( a->adjsize() == 1 || b->adjsize() == 1 )
        return true;

    return false;
}

} // namespace Tracer

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
