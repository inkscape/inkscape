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

#ifndef LIBDEPIXELIZE_TRACER_SIMPLIFIEDVORONOI_H
#define LIBDEPIXELIZE_TRACER_SIMPLIFIEDVORONOI_H

#include "pixelgraph.h"
#include "colorspace.h"
#include "point.h"
#include "branchless.h"

namespace Tracer {

template<typename T, bool adjust_splines>
class SimplifiedVoronoi
{
public:
    /**
     * The "smooth" attribute of each vertex is only accurate if edge is
     * visible. This decision was made because invisible edges will disappear
     * during polygon-union, the next phase of Kopf-Lischinski.
     */
    struct Cell
    {
        // There may not exist more than 8 vertices per cell and a
        // "small vector optimization" could improve the performance
        // by avoiding memory fragmentation. Serious testing is needed.

        // The vertices are filled in clockwise order
        std::vector< Point<T> > vertices;
        guint8 rgba[4];
    };

    typedef typename std::vector<Cell>::iterator iterator;
    typedef typename std::vector<Cell>::const_iterator const_iterator;
    typedef typename std::vector<Cell>::reverse_iterator reverse_iterator;

    typedef typename std::vector<Cell>::const_reverse_iterator
    const_reverse_iterator;

    /*
      It will work correctly if no crossing-edges are present.
     */
    SimplifiedVoronoi(const PixelGraph &graph);

    // Iterators
    iterator begin()
    {
        return _cells.begin();
    }

    const_iterator begin() const
    {
        return _cells.begin();
    }

    iterator end()
    {
        return _cells.end();
    }

    const_iterator end() const
    {
        return _cells.end();
    }

    reverse_iterator rbegin()
    {
        return _cells.rbegin();
    }

    const_reverse_iterator rbegin() const
    {
        return _cells.rbegin();
    }

    reverse_iterator rend()
    {
        return _cells.rend();
    }

    const_reverse_iterator rend() const
    {
        return _cells.rend();
    }

    size_t size() const
    {
        return _cells.size();
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
#ifdef LIBDEPIXELIZE_VERY_TYPE_SAFE
    typedef void (*PointTransform)(Point<T> &p, T dx, T dy);
    typedef bool (*NodeTransform)(PixelGraph::const_iterator);
#endif // LIBDEPIXELIZE_VERY_TYPE_SAFE

    /**
     * Output is translated by -.5 in each axis. This function fixes this error.
     */
    static Point<T> _adjust(Point<T> p)
    {
        return Point<T>(p.x + .5, p.y + .5);
    }

    /**
     * Output is translated by -.5 in each axis. This function fixes this error.
     */
    static Point<T> _adjust(Point<T> p, bool smooth)
    {
        return Point<T>(p.x + .5, p.y + .5, smooth);
    }

    void _complexTopLeft(const PixelGraph &graph,
                         PixelGraph::const_iterator graph_it,
                         Cell *const cells_it, int x, int y);
    void _complexTopRight(const PixelGraph &graph,
                          PixelGraph::const_iterator graph_it,
                          Cell *const cells_it, int x, int y);
    void _complexBottomRight(const PixelGraph &graph,
                             PixelGraph::const_iterator graph_it,
                             Cell *const cells_it, int x, int y);
    void _complexBottomLeft(const PixelGraph &graph,
                            PixelGraph::const_iterator graph_it,
                            Cell *const cells_it, int x, int y);

    static void _complexTopLeftTransform(Point<T> &p, T dx, T dy);
    static void _complexTopRightTransform(Point<T> &p, T dx, T dy);
    static void _complexBottomRightTransform(Point<T> &p, T dx, T dy);
    static void _complexBottomLeftTransform(Point<T> &p, T dx, T dy);

    static bool _complexTopLeftTransformTop(PixelGraph::const_iterator graph_it);
    static bool _complexTopLeftTransformTopRight(PixelGraph::const_iterator graph_it);
    static bool _complexTopLeftTransformRight(PixelGraph::const_iterator graph_it);
    static bool _complexTopLeftTransformBottomRight(PixelGraph::const_iterator graph_it);
    static bool _complexTopLeftTransformBottom(PixelGraph::const_iterator graph_it);
    static bool _complexTopLeftTransformBottomLeft(PixelGraph::const_iterator graph_it);
    static bool _complexTopLeftTransformLeft(PixelGraph::const_iterator graph_it);
    static bool _complexTopLeftTransformTopLeft(PixelGraph::const_iterator graph_it);

    static bool _complexTopRightTransformTop(PixelGraph::const_iterator graph_it);
    static bool _complexTopRightTransformTopRight(PixelGraph::const_iterator graph_it);
    static bool _complexTopRightTransformRight(PixelGraph::const_iterator graph_it);
    static bool _complexTopRightTransformBottomRight(PixelGraph::const_iterator graph_it);
    static bool _complexTopRightTransformBottom(PixelGraph::const_iterator graph_it);
    static bool _complexTopRightTransformBottomLeft(PixelGraph::const_iterator graph_it);
    static bool _complexTopRightTransformLeft(PixelGraph::const_iterator graph_it);
    static bool _complexTopRightTransformTopLeft(PixelGraph::const_iterator graph_it);

    static bool _complexBottomRightTransformTop(PixelGraph::const_iterator graph_it);
    static bool _complexBottomRightTransformTopRight(PixelGraph::const_iterator graph_it);
    static bool _complexBottomRightTransformRight(PixelGraph::const_iterator graph_it);
    static bool _complexBottomRightTransformBottomRight(PixelGraph::const_iterator graph_it);
    static bool _complexBottomRightTransformBottom(PixelGraph::const_iterator graph_it);
    static bool _complexBottomRightTransformBottomLeft(PixelGraph::const_iterator graph_it);
    static bool _complexBottomRightTransformLeft(PixelGraph::const_iterator graph_it);
    static bool _complexBottomRightTransformTopLeft(PixelGraph::const_iterator graph_it);

    static bool _complexBottomLeftTransformTop(PixelGraph::const_iterator graph_it);
    static bool _complexBottomLeftTransformTopRight(PixelGraph::const_iterator graph_it);
    static bool _complexBottomLeftTransformRight(PixelGraph::const_iterator graph_it);
    static bool _complexBottomLeftTransformBottomRight(PixelGraph::const_iterator graph_it);
    static bool _complexBottomLeftTransformBottom(PixelGraph::const_iterator graph_it);
    static bool _complexBottomLeftTransformBottomLeft(PixelGraph::const_iterator graph_it);
    static bool _complexBottomLeftTransformLeft(PixelGraph::const_iterator graph_it);
    static bool _complexBottomLeftTransformTopLeft(PixelGraph::const_iterator graph_it);

    /*
     * The memory layout assumed goes like this (with a_it being the current
     * iterated element):
     *
     *   (a_it) | (b_it)
     *   -------+-------
     *   (c_it) | (d_it)
     *
     * If you want to use it with another directions (topleft, topright, ...)
     * **DO NOT**  invert x or y axis, because the insertion order will go mad.
     *
     * The idea behind this abstraction is to rotate the iterators, then the
     * insertion order will be preserved.
     *
     * The initial value of all nodes that will be inserted is {x, y}. All
     * changes to this node **MUST** occur through \p transform or _adjust.
     *
     * Some maintainers may like this function because they will handle a
     * code base 4 times smaller and bugs will be MUCH MUCH difficult to hide.
     *
     * Some maintainers may go mad because the level extra level of
     * abstraction.
     *
     * "All problems in computer science can be solved by another level of
     *  indirection, except for the problem of too many layers of indirection."
     *         -- David J. Wheeler
     */
#ifndef LIBDEPIXELIZE_VERY_TYPE_SAFE
    template<class PointTransform, class NodeTransform>
#endif // LIBDEPIXELIZE_VERY_TYPE_SAFE
    void _genericComplexBottomRight(PixelGraph::const_iterator a_it,
                                    PixelGraph::const_iterator b_it,
                                    PixelGraph::const_iterator c_it,
                                    PixelGraph::const_iterator d_it,
                                    Cell *const cells_it, int x, int y,
                                    PointTransform transform,
                                    NodeTransform top,
                                    NodeTransform topright,
                                    NodeTransform right,
                                    NodeTransform bottomright,
                                    NodeTransform bottom,
                                    NodeTransform bottomleft,
                                    NodeTransform left,
                                    NodeTransform topleft);

    int _width;
    int _height;
    std::vector<Cell> _cells;
};

template<class T, bool adjust_splines>
SimplifiedVoronoi<T, adjust_splines>
::SimplifiedVoronoi(const PixelGraph &graph) :
    _width(graph.width()),
    _height(graph.height()),
    _cells(graph.size())
{
    if (!graph.size())
        return;

    /*
     * The insertion order of cells is not a big deal. Here I just follow
     * the order of PixelGraph arrangement.
     */

    // ...the "center" cells first...
    if ( _width > 2 && _height > 2 ) {
        PixelGraph::const_iterator graph_it = graph.begin() + _width + 1;
        Cell *cells_it = &_cells.front() + _width + 1;

        for ( int i = 1 ; i != _height - 1 ; ++i ) {
            for ( int j = 1 ; j != _width - 1 ; ++j, ++graph_it, ++cells_it ) {
                for ( int k = 0 ; k != 4 ; ++k )
                    cells_it->rgba[k] = graph_it->rgba[k];
                // Top-left
                _complexTopLeft(graph, graph_it, cells_it, j, i);

                // Top-right
                _complexTopRight(graph, graph_it, cells_it, j, i);

                // Bottom-right
                _complexBottomRight(graph, graph_it, cells_it, j, i);

                // Bottom-left
                _complexBottomLeft(graph, graph_it, cells_it, j, i);
            }
            // After the previous loop, 'it' is pointing to the last cell from
            // the row.
            // Go south, then first node in the row (increment 'it' by 1)
            // Go to the second node in the line (increment 'it' by 1)
            graph_it += 2;
            cells_it += 2;
        }
    }

    //  ...then the "top" cells...
    if ( _width > 2 ) {
        PixelGraph::const_iterator graph_it = graph.begin() + 1;
        Cell *cells_it = &_cells.front() + 1;

        if ( _height > 1 ) {
            for ( int i = 1 ; i != _width - 1 ; ++i, ++graph_it, ++cells_it ) {
                for ( int j = 0 ; j != 4 ; ++j )
                    cells_it->rgba[j] = graph_it->rgba[j];

                // Top-left
                cells_it->vertices.push_back(Point<T>(i, 0, false));

                // Top-right
                cells_it->vertices.push_back(Point<T>(i + 1, 0, false));

                // Bottom-right
                _complexBottomRight(graph, graph_it, cells_it, i, 0);

                // Bottom-left
                _complexBottomLeft(graph, graph_it, cells_it, i, 0);
            }
        } else {
            for ( int i = 1 ; i != _width - 1 ; ++i, ++graph_it, ++cells_it ) {
                for ( int j = 0 ; j != 4 ; ++j )
                    cells_it->rgba[j] = graph_it->rgba[j];

                // Top-left
                cells_it->vertices.push_back(Point<T>(i, 0, false));

                // Top-right
                cells_it->vertices.push_back(Point<T>(i + 1, 0, false));

                // Bottom-right
                cells_it->vertices.push_back(Point<T>(i + 1, 1, false));

                // Bottom-left
                cells_it->vertices.push_back(Point<T>(i, 1, false));
            }
        }
    }

    //  ...then the "bottom" cells...
    if ( _width > 2 && _height > 1 ) {
        // Node *it = &((*this)[1][_height - 1]);
        PixelGraph::const_iterator graph_it
            = graph.begin() + (_height - 1) * _width + 1;
        Cell *cells_it = &_cells.front() + (_height - 1) * _width + 1;

        for ( int i = 1 ; i != _width - 1 ; ++i, ++graph_it, ++cells_it ) {
            for ( int j = 0 ; j != 4 ; ++j )
                cells_it->rgba[j] = graph_it->rgba[j];

            // Top-left
            _complexTopLeft(graph, graph_it, cells_it, i, _height - 1);

            // Top-right
            _complexTopRight(graph, graph_it, cells_it, i, _height - 1);

            // Bottom-right
            cells_it->vertices.push_back(Point<T>(i + 1, _height, false));

            // Bottom-left
            cells_it->vertices.push_back(Point<T>(i, _height, false));
        }
    }

    //  ...then the "left" cells...
    if ( _height > 2 ) {
        PixelGraph::const_iterator graph_it = graph.begin() + _width;
        Cell *cells_it = &_cells.front() + _width;

        if ( _width > 1 ) {
            for ( int i = 1 ; i != _height - 1 ; ++i) {
                for ( int j = 0 ; j != 4 ; ++j )
                    cells_it->rgba[j] = graph_it->rgba[j];

                // Top-left
                cells_it->vertices.push_back(Point<T>(0, i, false));

                // Top-right
                _complexTopRight(graph, graph_it, cells_it, 0, i);

                // Bottom-right
                _complexBottomRight(graph, graph_it, cells_it, 0, i);

                // Bottom-left
                cells_it->vertices.push_back(Point<T>(0, i + 1, false));

                graph_it += _width;
                cells_it += _width;
            }
        } else {
            for ( int i = 1 ; i != _height - 1 ; ++i) {
                for ( int j = 0 ; j != 4 ; ++j )
                    cells_it->rgba[j] = graph_it->rgba[j];

                // Top-left
                cells_it->vertices.push_back(Point<T>(0, i, false));

                // Top-right
                cells_it->vertices.push_back(Point<T>(1, i, false));

                // Bottom-right
                cells_it->vertices.push_back(Point<T>(1, i, false));

                // Bottom-left
                cells_it->vertices.push_back(Point<T>(0, i + 1, false));

                graph_it += _width;
                cells_it += _width;
            }
        }
    }

    // ...then the "right" cells...
    if ( _height > 2 && _width > 1 ) {
        PixelGraph::const_iterator graph_it = graph.begin() + 2 * _width - 1;
        Cell *cells_it = &_cells.front() + 2 * _width - 1;

        for ( int i = 1 ; i != _height - 1 ; ++i ) {
            for ( int j = 0 ; j != 4 ; ++j )
                cells_it->rgba[j] = graph_it->rgba[j];

            // Top-left
            _complexTopLeft(graph, graph_it, cells_it, _width - 1, i);

            // Top-right
            cells_it->vertices.push_back(Point<T>(_width, i, false));

            // Bottom-right
            cells_it->vertices.push_back(Point<T>(_width, i + 1, false));

            // Bottom-left
            _complexBottomLeft(graph, graph_it, cells_it, _width - 1, i);

            graph_it += _width;
            cells_it += _width;
        }
    }

    //  ...and the 4 corner nodes
    // top-left
    {
        PixelGraph::const_iterator graph_it = graph.begin();
        Cell *cells_it = &_cells.front();

        for ( int i = 0 ; i != 4 ; ++i )
            cells_it->rgba[i] = graph_it->rgba[i];

        // Top-left
        cells_it->vertices.push_back(Point<T>(0, 0, false));

        // Top-right
        cells_it->vertices.push_back(Point<T>(1, 0, false));

        // Bottom-right
        if ( _width > 1 && _height > 1 )
            _complexBottomRight(graph, graph_it, cells_it, 0, 0);
        else
            cells_it->vertices.push_back(Point<T>(1, 1, false));

        // Bottom-left
        cells_it->vertices.push_back(Point<T>(0, 1, false));
    }

    // top-right
    if ( _width > 1 ) {
        PixelGraph::const_iterator graph_it = graph.begin() + _width - 1;
        Cell *cells_it = &_cells.front() + _width - 1;

        for ( int i = 0 ; i != 4 ; ++i )
            cells_it->rgba[i] = graph_it->rgba[i];

        // Top-left
        cells_it->vertices.push_back(Point<T>(_width - 1, 0, false));

        // Top-right
        cells_it->vertices.push_back(Point<T>(_width, 0, false));

        // Bottom-right
        cells_it->vertices.push_back(Point<T>(_width, 1, false));

        // Bottom-left
        if ( _height > 1 )
            _complexBottomLeft(graph, graph_it, cells_it, _width - 1, 0);
        else
            cells_it->vertices.push_back(Point<T>(_width - 1, 1, false));
    }

    // bottom-left
    if ( _height > 1 ) {
        PixelGraph::const_iterator graph_it
            = graph.begin() + (_height - 1) * _width;
        Cell *cells_it = &_cells.front() + (_height - 1) * _width;

        for ( int i = 0 ; i != 4 ; ++i )
            cells_it->rgba[i] = graph_it->rgba[i];

        // Top-left
        cells_it->vertices.push_back(Point<T>(0, _height - 1, false));

        // Top-right
        if ( _width > 1)
            _complexTopRight(graph, graph_it, cells_it, 0, _height - 1);
        else
            cells_it->vertices.push_back(Point<T>(1, _height - 1, false));

        // Bottom-right
        cells_it->vertices.push_back(Point<T>(1, _height, false));

        // Bottom-left
        cells_it->vertices.push_back(Point<T>(0, _height, false));
    }

    // bottom-right
    if ( _width > 1 && _height > 1 ) {
        PixelGraph::const_iterator graph_it = --graph.end();
        Cell *cells_it = &_cells.back();

        for ( int i = 0 ; i != 4 ; ++i )
            cells_it->rgba[i] = graph_it->rgba[i];

        // Top-left
        _complexTopLeft(graph, graph_it, cells_it, _width - 1, _height - 1);

        // Top-right
        cells_it->vertices.push_back(Point<T>(_width, _height - 1, false));

        // Bottom-right
        cells_it->vertices.push_back(Point<T>(_width, _height, false));

        // Bottom-left
        cells_it->vertices.push_back(Point<T>(_width - 1, _height, false));
    }
}

template<class T, bool adjust_splines> void
SimplifiedVoronoi<T, adjust_splines>
::_complexTopLeft(const PixelGraph &graph,
                  PixelGraph::const_iterator graph_it, Cell *const cells_it,
                  int x, int y)
{
    _genericComplexBottomRight(graph_it,
                               graph.nodeLeft(graph_it),
                               graph.nodeTop(graph_it),
                               graph.nodeTopLeft(graph_it),
                               cells_it, x, y,
                               &SimplifiedVoronoi::_complexTopLeftTransform,
                               &SimplifiedVoronoi::_complexTopLeftTransformTop,
                               &SimplifiedVoronoi::_complexTopLeftTransformTopRight,
                               &SimplifiedVoronoi::_complexTopLeftTransformRight,
                               &SimplifiedVoronoi::_complexTopLeftTransformBottomRight,
                               &SimplifiedVoronoi::_complexTopLeftTransformBottom,
                               &SimplifiedVoronoi::_complexTopLeftTransformBottomLeft,
                               &SimplifiedVoronoi::_complexTopLeftTransformLeft,
                               &SimplifiedVoronoi::_complexTopLeftTransformTopLeft);
}

template<class T, bool adjust_splines> void
SimplifiedVoronoi<T, adjust_splines>
::_complexTopRight(const PixelGraph &graph,
                   PixelGraph::const_iterator graph_it, Cell *const cells_it,
                   int x, int y)
{
    _genericComplexBottomRight(graph_it,
                               graph.nodeTop(graph_it),
                               graph.nodeRight(graph_it),
                               graph.nodeTopRight(graph_it),
                               cells_it, x, y,
                               &SimplifiedVoronoi::_complexTopRightTransform,
                               &SimplifiedVoronoi::_complexTopRightTransformTop,
                               &SimplifiedVoronoi::_complexTopRightTransformTopRight,
                               &SimplifiedVoronoi::_complexTopRightTransformRight,
                               &SimplifiedVoronoi::_complexTopRightTransformBottomRight,
                               &SimplifiedVoronoi::_complexTopRightTransformBottom,
                               &SimplifiedVoronoi::_complexTopRightTransformBottomLeft,
                               &SimplifiedVoronoi::_complexTopRightTransformLeft,
                               &SimplifiedVoronoi::_complexTopRightTransformTopLeft);
}

template<class T, bool adjust_splines> void
SimplifiedVoronoi<T, adjust_splines>
::_complexBottomRight(const PixelGraph &graph,
                      PixelGraph::const_iterator graph_it, Cell *const cells_it,
                      int x, int y)
{
    _genericComplexBottomRight(graph_it,
                               graph.nodeRight(graph_it),
                               graph.nodeBottom(graph_it),
                               graph.nodeBottomRight(graph_it),
                               cells_it, x, y,
                               &SimplifiedVoronoi::_complexBottomRightTransform,
                               &SimplifiedVoronoi::_complexBottomRightTransformTop,
                               &SimplifiedVoronoi::_complexBottomRightTransformTopRight,
                               &SimplifiedVoronoi::_complexBottomRightTransformRight,
                               &SimplifiedVoronoi::_complexBottomRightTransformBottomRight,
                               &SimplifiedVoronoi::_complexBottomRightTransformBottom,
                               &SimplifiedVoronoi::_complexBottomRightTransformBottomLeft,
                               &SimplifiedVoronoi::_complexBottomRightTransformLeft,
                               &SimplifiedVoronoi::_complexBottomRightTransformTopLeft);
}

template<class T, bool adjust_splines> void
SimplifiedVoronoi<T, adjust_splines>
::_complexBottomLeft(const PixelGraph &graph,
                     PixelGraph::const_iterator graph_it, Cell *const cells_it,
                     int x, int y)
{
    _genericComplexBottomRight(graph_it,
                               graph.nodeBottom(graph_it),
                               graph.nodeLeft(graph_it),
                               graph.nodeBottomLeft(graph_it),
                               cells_it, x, y,
                               &SimplifiedVoronoi::_complexBottomLeftTransform,
                               &SimplifiedVoronoi::_complexBottomLeftTransformTop,
                               &SimplifiedVoronoi::_complexBottomLeftTransformTopRight,
                               &SimplifiedVoronoi::_complexBottomLeftTransformRight,
                               &SimplifiedVoronoi::_complexBottomLeftTransformBottomRight,
                               &SimplifiedVoronoi::_complexBottomLeftTransformBottom,
                               &SimplifiedVoronoi::_complexBottomLeftTransformBottomLeft,
                               &SimplifiedVoronoi::_complexBottomLeftTransformLeft,
                               &SimplifiedVoronoi::_complexBottomLeftTransformTopLeft);
}

template<class T, bool adjust_splines> void
SimplifiedVoronoi<T, adjust_splines>
::_complexTopLeftTransform(Point<T> &p, T dx, T dy)
{
    p.x -= dx;
    p.y -= dy;
}

template<class T, bool adjust_splines> void
SimplifiedVoronoi<T, adjust_splines>
::_complexTopRightTransform(Point<T> &p, T dx, T dy)
{
    p.x += dy;
    p.y -= dx;
}

template<class T, bool adjust_splines> void
SimplifiedVoronoi<T, adjust_splines>
::_complexBottomRightTransform(Point<T> &p, T dx, T dy)
{
    p.x += dx;
    p.y += dy;
}

template<class T, bool adjust_splines> void
SimplifiedVoronoi<T, adjust_splines>
::_complexBottomLeftTransform(Point<T> &p, T dx, T dy)
{
    p.x -= dy;
    p.y += dx;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopLeftTransformTop(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.bottom;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopLeftTransformTopRight(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.bottomleft;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopLeftTransformRight(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.left;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopLeftTransformBottomRight(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.topleft;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopLeftTransformBottom(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.top;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopLeftTransformBottomLeft(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.topright;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopLeftTransformLeft(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.right;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopLeftTransformTopLeft(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.bottomright;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopRightTransformTop(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.left;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopRightTransformTopRight(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.topleft;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopRightTransformRight(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.top;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopRightTransformBottomRight(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.topright;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopRightTransformBottom(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.right;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopRightTransformBottomLeft(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.bottomright;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopRightTransformLeft(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.bottom;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexTopRightTransformTopLeft(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.bottomleft;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomRightTransformTop(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.top;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomRightTransformTopRight(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.topright;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomRightTransformRight(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.right;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomRightTransformBottomRight(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.bottomright;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomRightTransformBottom(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.bottom;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomRightTransformBottomLeft(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.bottomleft;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomRightTransformLeft(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.left;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomRightTransformTopLeft(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.topleft;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomLeftTransformTop(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.right;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomLeftTransformTopRight(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.bottomright;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomLeftTransformRight(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.bottom;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomLeftTransformBottomRight(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.bottomleft;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomLeftTransformBottom(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.left;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomLeftTransformBottomLeft(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.topleft;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomLeftTransformLeft(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.top;
}

template<class T, bool adjust_splines>
bool SimplifiedVoronoi<T, adjust_splines>
::_complexBottomLeftTransformTopLeft(PixelGraph::const_iterator graph_it)
{
    return graph_it->adj.topright;
}

template<class T, bool adjust_splines>
#ifndef LIBDEPIXELIZE_VERY_TYPE_SAFE
template<class PointTransform, class NodeTransform>
#endif // LIBDEPIXELIZE_VERY_TYPE_SAFE
void
SimplifiedVoronoi<T, adjust_splines>
::_genericComplexBottomRight(PixelGraph::const_iterator a_it,
                             PixelGraph::const_iterator b_it,
                             PixelGraph::const_iterator c_it,
                             PixelGraph::const_iterator d_it,
                             Cell *const cells_it, int x, int y,
                             PointTransform transform,
                             NodeTransform,
                             NodeTransform topright,
                             NodeTransform right,
                             NodeTransform bottomright,
                             NodeTransform bottom,
                             NodeTransform bottomleft,
                             NodeTransform,
                             NodeTransform topleft)
{
    using colorspace::contour_edge;
    using colorspace::same_color;

    const Point<T> initial(x, y);

    /*
     * The insertion order of points within the cell is very important. You must
     * follow current practice: Clockwise order.
     */

    if ( bottomright(a_it) ) {
        // this and bottom-right are connected

        bool smooth[2] = {
            ( same_color(a_it->rgba, d_it->rgba)
              || same_color(a_it->rgba, b_it->rgba)
              || same_color(b_it->rgba, d_it->rgba) ),
            ( same_color(a_it->rgba, d_it->rgba)
              || same_color(a_it->rgba, c_it->rgba)
              || same_color(c_it->rgba, d_it->rgba) )
        };

        Point<T> borderMid = initial;
        {
            transform(borderMid, 1, 1);
            borderMid = midpoint(initial, borderMid);
        }

        Point<T> vertices[2] = {initial, initial};
        {
            transform(vertices[0], 1, 0);
            vertices[0] = _adjust(midpoint(borderMid, vertices[0]), smooth[0]);

            transform(vertices[1], 0, 1);
            vertices[1] = _adjust(midpoint(borderMid, vertices[1]), smooth[1]);
        }

        if ( !smooth[0] && adjust_splines ) {
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_1ST_IS_INVISIBLE
           cells_it->vertices.push_back(vertices[0].invisible());
#else
           cells_it->vertices.push_back(vertices[0]);
#endif
            {
                Point<T> another = vertices[0];
                transform(another,
                          - ( 0.1875
                              - ( topright(a_it) - topleft(b_it) ) * 0.1875 ),
                          // y
                          - ( 0.5625
                              - ( topright(a_it) + topleft(b_it) ) * 0.1875 ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_2ND_IS_INVISIBLE
                cells_it->vertices.push_back(another.invisible());
#else
                cells_it->vertices.push_back(another);
#endif
            }
            {
                Point<T> another = vertices[0];
                transform(another,
                          - ( 0.0625
                              - ( topright(a_it) - topleft(b_it) ) * 0.0625 ),
                          // y
                          - ( 0.1875
                              - ( topright(a_it) + topleft(b_it) ) * 0.0625) );
                another.smooth = true;
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_3RD_IS_INVISIBLE
                cells_it->vertices.push_back(another.invisible());
#else
                cells_it->vertices.push_back(another);
#endif
            }
            {
                Point<T> another = vertices[0];
                transform(another,
                          0.1875
                          - ( bottomright(b_it) + topright(d_it) ) * 0.0625,
                          // y
                          0.0625
                          + ( bottomright(b_it) - topright(d_it) ) * 0.0625);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_4TH_IS_INVISIBLE
                cells_it->vertices.push_back(another.invisible());
#else
                cells_it->vertices.push_back(another);
#endif
            }
            {
                transform(vertices[0],
                          0.0625
                          + ( topright(a_it) - topright(d_it) - topleft(b_it)
                              - bottomright(b_it) ) * 0.03125,
                          // y
                          - ( 0.0625
                              + ( topright(d_it) - topright(a_it)
                                  - topleft(b_it) - bottomright(b_it) )
                              * 0.03125 ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_5TH_IS_INVISIBLE
                vertices[0].visible = false;
#endif
            }
        }

        cells_it->vertices.push_back(vertices[0]);

        if ( !smooth[1] && adjust_splines ) {
            {
                Point<T> another = vertices[1];
                transform(another,
                          - ( 0.0625
                              + ( bottomleft(d_it) - bottomleft(a_it)
                                  - topleft(c_it) - bottomright(c_it) )
                              * 0.03125 ),
                          // y
                          0.0625
                          + ( bottomleft(a_it) - bottomleft(d_it)
                              - topleft(c_it) - bottomright(c_it) ) * 0.03125);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_1ST_IS_INVISIBLE
                cells_it->vertices.push_back(another.invisible());
#else
                cells_it->vertices.push_back(another);
#endif
            }
            {
                Point<T> another = vertices[1];
                transform(another,
                          0.0625
                          + ( bottomright(c_it) - bottomleft(d_it) ) * 0.0625,
                          // y
                          0.1875
                          - ( bottomright(c_it) + bottomleft(d_it) ) * 0.0625);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_2ND_IS_INVISIBLE
                cells_it->vertices.push_back(another.invisible());
#else
                cells_it->vertices.push_back(another);
#endif
            }
            {
                Point<T> another = vertices[1];
                transform(another,
                          - ( 0.1875
                              - ( bottomleft(a_it) + topleft(c_it) )
                              * 0.0625 ),
                          // y
                          - ( 0.0625
                              - ( bottomleft(a_it) - topleft(c_it) )
                              * 0.0625 ));
                another.smooth = true;
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_3RD_IS_INVISIBLE
                cells_it->vertices.push_back(another.invisible());
#else
                cells_it->vertices.push_back(another);
#endif
            }
            {
                Point<T> another = vertices[1];
                transform(another,
                          - ( 0.5625
                              - ( bottomleft(a_it) + topleft(c_it) )
                              * 0.1875 ),
                          // y
                          - ( 0.1875
                              - ( bottomleft(a_it) - topleft(c_it) )
                              * 0.1875 ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_4TH_IS_INVISIBLE
                cells_it->vertices.push_back(another.invisible());
#else
                cells_it->vertices.push_back(another);
#endif
            }
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_5TH_IS_INVISIBLE
            vertices[1].visible = false;
#endif
        }

        cells_it->vertices.push_back(vertices[1]);
    } else if ( bottomleft(b_it) ) {
        // right and bottom are connected

        Point<T> vertex = initial;
        transform(vertex, 1, 1);
        vertex = _adjust(midpoint(midpoint(initial, vertex), initial), true);
        cells_it->vertices.push_back(vertex);
    } else {
        // Connections don't affect the shape of this squared-like
        // pixel

        Point<T> vertex = initial;
        transform(vertex, 1, 1);
        vertex = _adjust(midpoint(initial, vertex));

        // compute smoothness
        if ( right(a_it) && adjust_splines ) {
            // this and right are connected

            if ( !right(c_it) && !( bottom(a_it) && bottom(b_it) ) ) {
                // bottom and bottom-right are disconnected

                bool foreign_is_contour = contour_edge(c_it->rgba, d_it->rgba);
                bool twin_is_contour = contour_edge(b_it->rgba, d_it->rgba);
                bool another_is_contour = contour_edge(a_it->rgba, c_it->rgba);

                if ( another_is_contour + twin_is_contour
                     + foreign_is_contour == 2 ) {
                    vertex.smooth = !foreign_is_contour;

                    if ( !vertex.smooth ) {
                        if ( another_is_contour ) {
                            {
                                Point<T> another = vertex;
                                T amount = 0.125
                                    - ( ( bottomright(c_it) + topleft(c_it) )
                                        * 0.03125 );
                                transform(another, - amount, amount);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_1ST_IS_INVISIBLE
                                cells_it->vertices.push_back(another.invisible());
#else
                                cells_it->vertices.push_back(another);
#endif
                            }
                            {
                                Point<T> another = vertex;
                                T amount = 0.0625 * bottomright(c_it);
                                transform(another, amount, 0.25 - amount);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_2ND_IS_INVISIBLE
                                cells_it->vertices.push_back(another.invisible());
#else
                                cells_it->vertices.push_back(another);
#endif
                            }
                            {
                                Point<T> another = vertex;
                                T amount = 0.0625 * topleft(c_it);
                                transform(another, - ( 0.25 - amount ),
                                          - amount);
                                another.smooth = true;
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_3RD_IS_INVISIBLE
                                cells_it->vertices.push_back(another.invisible());
#else
                                cells_it->vertices.push_back(another);
#endif
                            }
                            {
                                Point<T> another = vertex;
                                T amount = 0.1875 * topleft(c_it);
                                transform(another, - ( 0.75 - amount ),
                                          - amount);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_4TH_IS_INVISIBLE
                                cells_it->vertices.push_back(another.invisible());
#else
                                cells_it->vertices.push_back(another);
#endif
                            }
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_5TH_IS_INVISIBLE
                            vertex.visible = false;
#endif
                        } else if ( twin_is_contour ) {
                            T amount = 0.125
                                - ( ( bottomleft(d_it) + topright(d_it) )
                                    * 0.03125 );
                            transform(vertex, amount, amount);
                        }
                    } else if ( !same_color(a_it->rgba, b_it->rgba) ) {
                        vertex.smooth = false;
                        // This is the same code of the if ( special )
                        // I REALLY NEED lambdas to improve this code without
                        // creating yet another interface that takes a million
                        // of function parameters and keep code locality
                        {
                            Point<T> another = vertex;
                            T amount = 0.03125;
                            transform(another,
                                      amount
                                      * ( topleft(c_it) - topright(d_it)
                                          + bottomleft(a_it) - bottomright(b_it) ),
                                      // y
                                      - amount
                                      * ( topleft(c_it) + topright(d_it)
                                          - bottomleft(a_it) - bottomright(b_it) ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_1ST_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
                        {
                            Point<T> another = vertex;
                            T amount = 0.0625;
                            transform(another,
                                      0.25 - amount
                                      * ( topright(d_it) + bottomright(b_it) ),
                                      // y
                                      - amount
                                      * ( topright(d_it) - bottomright(b_it) ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_2ND_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
                        {
                            Point<T> another = vertex;
                            T amount = 0.0625;
                            transform(another,
                                      - ( 0.25 - amount
                                          * ( topleft(c_it) + bottomleft(a_it) ) ),
                                      // y
                                      - amount
                                      * ( topleft(c_it) - bottomleft(a_it) ));
                            another.smooth = true;
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_3RD_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
                        {
                            Point<T> another = vertex;
                            T amount = 0.1875;
                            transform(another,
                                      - ( 0.75 - amount
                                          * ( topleft(c_it) + bottomleft(a_it) ) ),
                                      // y
                                      -  amount
                                      * ( topleft(c_it) - bottomleft(a_it) ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_4TH_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_5TH_IS_INVISIBLE
                        vertex.visible = false;
#endif
                    }
                } else {
                    // {this, right} is the pair with the angle
                    // closest to 180 degrees
                    vertex.smooth = same_color(a_it->rgba, b_it->rgba);
                }
            } else {
                // there might be 2-color, then vertex.smooth = true

                // or it might be 1-color and doesn't matter,
                // because the current node will disappear
                vertex.smooth
                    = same_color(a_it->rgba, b_it->rgba)
                    + same_color(a_it->rgba, c_it->rgba)
                    + same_color(d_it->rgba, b_it->rgba)
                    + same_color(d_it->rgba, c_it->rgba) == 2;
            }
        } else if ( bottom(a_it) && adjust_splines ) {
            // this and bottom are connected

            if ( !bottom(b_it) && !( right(a_it) && right(c_it) ) ) {
                // right and bottom-right are disconnected

                bool foreign_is_contour = contour_edge(b_it->rgba, d_it->rgba);
                bool twin_is_contour = contour_edge(c_it->rgba, d_it->rgba);
                bool another_is_contour = contour_edge(a_it->rgba, b_it->rgba);

                if ( another_is_contour + twin_is_contour
                     + foreign_is_contour == 2 ) {
                    vertex.smooth = !foreign_is_contour;

                    if ( !vertex.smooth ) {
                        if ( another_is_contour ) {
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_1ST_IS_INVISIBLE
                            cells_it->vertices.push_back(vertex.invisible());
#else
                            cells_it->vertices.push_back(vertex);
#endif
                            {
                                Point<T> another = vertex;
                                T amount = 0.1875 * topleft(b_it);
                                transform(another, - amount,
                                          - ( 0.75 - amount ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_2ND_IS_INVISIBLE
                                cells_it->vertices.push_back(another.invisible());
#else
                                cells_it->vertices.push_back(another);
#endif
                            }
                            {
                                Point<T> another = vertex;
                                T amount = 0.0625 * topleft(b_it);
                                transform(another, - amount,
                                          - ( 0.25 - amount ));
                                another.smooth = true;
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_3RD_IS_INVISIBLE
                                cells_it->vertices.push_back(another.invisible());
#else
                                cells_it->vertices.push_back(another);
#endif
                            }
                            {
                                Point<T> another = vertex;
                                T amount = 0.0625 * bottomright(b_it);
                                transform(another, 0.25 - amount, amount);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_4TH_IS_INVISIBLE
                                cells_it->vertices.push_back(another.invisible());
#else
                                cells_it->vertices.push_back(another);
#endif
                            }
                            {
                                T amount = 0.125
                                    - (bottomright(b_it) + topleft(b_it))
                                    * 0.03125;
                                transform(vertex, amount, - amount);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_5TH_IS_INVISIBLE
                                vertex.visible = false;
#endif
                            }
                        } else if ( twin_is_contour ) {
                            T amount = 0.125
                                - ( ( topright(d_it) + bottomleft(d_it) )
                                    * 0.03125 );
                            transform(vertex, amount, amount);
                        }
                    } else if ( !same_color(a_it->rgba, c_it->rgba) ) {
                        vertex.smooth = false;
                        // This is the same code of the if ( special )
                        // I REALLY NEED lambdas to improve this code without
                        // creating yet another interface that takes a million
                        // of function parameters and keep code locality
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_1ST_IS_INVISIBLE
                        cells_it->vertices.push_back(vertex.invisible());
#else
                        cells_it->vertices.push_back(vertex);
#endif
                        {
                            Point<T> another = vertex;
                            T amount = 0.1875;
                            transform(another,
                                      - ( topleft(b_it) - topright(a_it) ) * amount,
                                      // y
                                      - ( 0.75
                                          - ( topleft(b_it) + topright(a_it) )
                                          * amount ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_2ND_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
                        {
                            Point<T> another = vertex;
                            T amount = 0.0625;
                            transform(another,
                                      - ( topleft(b_it) - topright(a_it) ) * amount,
                                      // y
                                      - ( 0.25
                                          - ( topleft(b_it) + topright(a_it) )
                                          * amount ));
                            another.smooth = true;
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_3RD_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
                        {
                            Point<T> another = vertex;
                            T amount = 0.0625;
                            transform(another, - amount
                                      * ( bottomleft(d_it) - bottomright(c_it) ),
                                      // y
                                      0.25 - amount
                                      * ( bottomleft(d_it) + bottomright(c_it) ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_4TH_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
                        {
                            transform(vertex,
                                      - ( topleft(b_it) + bottomleft(d_it)
                                          - topright(a_it) - bottomright(c_it) )
                                      * 0.03125,
                                      // y
                                      ( topleft(b_it) - bottomleft(d_it)
                                        + topright(a_it) - bottomright(c_it) )
                                      * 0.03125);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_5TH_IS_INVISIBLE
                            vertex.visible = false;
#endif
                        }
                    }
                } else {
                    // {this, bottom} is the pair with the angle
                    // closest to 180 degrees
                    vertex.smooth = same_color(a_it->rgba, c_it->rgba);
                }
            } else {
                // there might be 2-color, then vertex.smooth = true

                // or it might be 1-color and doesn't matter,
                // because the current node will disappear
                vertex.smooth
                    = same_color(a_it->rgba, c_it->rgba)
                    + same_color(a_it->rgba, b_it->rgba)
                    + same_color(d_it->rgba, b_it->rgba)
                    + same_color(d_it->rgba, c_it->rgba) == 2;
            }
        } else if ( bottom(b_it) && adjust_splines ) {
            // right and bottom-right are connected

            bool special = false;

            bool foreign_is_contour = contour_edge(c_it->rgba, d_it->rgba);

            // the neighbor similar in 90° feature
            bool similar_neighbor_is_contour
                = contour_edge(a_it->rgba, c_it->rgba);

            if ( contour_edge(a_it->rgba, b_it->rgba)
                 + similar_neighbor_is_contour
                 + foreign_is_contour == 2 ) {
                vertex.smooth = !foreign_is_contour;

                if ( !vertex.smooth ) {
                    if ( similar_neighbor_is_contour ) {
                        {
                            Point<T> another = vertex;
                            T amount = 0.125
                                - ( topleft(c_it) + bottomright(c_it) )
                                * 0.03125;
                            transform(another, - amount, amount);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_1ST_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
                        {
                            Point<T> another = vertex;
                            T amount = 0.0625 * bottomright(c_it);
                            transform(another, amount, 0.25 - amount);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_2ND_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
                        {
                            Point<T> another = vertex;
                            T amount = 0.0625 * topleft(c_it);
                            transform(another, - ( 0.25 - amount ), - amount);
                            another.smooth = true;
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_3RD_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
                        {
                            Point<T> another = vertex;
                            T amount = 0.1875 * topleft(c_it);
                            transform(another, - ( 0.75 - amount ), - amount);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_4TH_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_5TH_IS_INVISIBLE
                        vertex.visible = false;
#endif
                    } else {
                        special = true;
                    }
                }
            } else {
                // {right, bottom-right} is the pair with the
                // angle closest to 180 degrees
                vertex.smooth = false;

                special = true;
            }

            if ( special ) {
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_1ST_IS_INVISIBLE
                cells_it->vertices.push_back(vertex.invisible());
#else
                cells_it->vertices.push_back(vertex);
#endif
                {
                    Point<T> another = vertex;
                    T amount = 0.1875;
                    transform(another,
                              - ( topleft(b_it) - topright(a_it) ) * amount,
                              // y
                              - ( 0.75
                                  - ( topleft(b_it) + topright(a_it) )
                                  * amount ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_2ND_IS_INVISIBLE
                    cells_it->vertices.push_back(another.invisible());
#else
                    cells_it->vertices.push_back(another);
#endif
                }
                {
                    Point<T> another = vertex;
                    T amount = 0.0625;
                    transform(another,
                              - ( topleft(b_it) - topright(a_it) ) * amount,
                              // y
                              - ( 0.25
                                  - ( topleft(b_it) + topright(a_it) )
                                  * amount ));
                    another.smooth = true;
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_3RD_IS_INVISIBLE
                    cells_it->vertices.push_back(another.invisible());
#else
                    cells_it->vertices.push_back(another);
#endif
                }
                {
                    Point<T> another = vertex;
                    T amount = 0.0625;
                    transform(another, - amount
                              * ( bottomleft(d_it) - bottomright(c_it) ),
                              // y
                              0.25 - amount
                              * ( bottomleft(d_it) + bottomright(c_it) ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_4TH_IS_INVISIBLE
                    cells_it->vertices.push_back(another.invisible());
#else
                    cells_it->vertices.push_back(another);
#endif
                }
                {
                    transform(vertex,
                              - ( topleft(b_it) + bottomleft(d_it)
                                  - topright(a_it) - bottomright(c_it) )
                              * 0.03125,
                              // y
                              ( topleft(b_it) - bottomleft(d_it)
                                + topright(a_it) - bottomright(c_it) )
                              * 0.03125);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_5TH_IS_INVISIBLE
                    vertex.visible = false;
#endif
                }
            }
        } else if ( right(c_it) && adjust_splines ) {
            // bottom and bottom-right are connected

            bool special = false;

            bool foreign_is_contour = contour_edge(b_it->rgba, d_it->rgba);

            // the neighbor similar in 90° feature
            bool similar_neighbor_is_contour
                = contour_edge(a_it->rgba, b_it->rgba);

            if ( contour_edge(a_it->rgba, c_it->rgba)
                 + similar_neighbor_is_contour
                 + foreign_is_contour == 2 ) {
                vertex.smooth = !foreign_is_contour;

                if ( !vertex.smooth ) {
                    if ( similar_neighbor_is_contour ) {
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_1ST_IS_INVISIBLE
                        cells_it->vertices.push_back(vertex.invisible());
#else
                        cells_it->vertices.push_back(vertex);
#endif
                        {
                            Point<T> another = vertex;
                            T amount = 0.1875 * topleft(b_it);
                            transform(another, - amount, - ( 0.75 - amount ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_2ND_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
                        {
                            Point<T> another = vertex;
                            T amount = 0.0625 * topleft(b_it);
                            transform(another, - amount, - ( 0.25 - amount ));
                            another.smooth = true;
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_3RD_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
                        {
                            Point<T> another = vertex;
                            T amount = 0.0625 * bottomright(b_it);
                            transform(another, 0.25 - amount, amount);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_4TH_IS_INVISIBLE
                            cells_it->vertices.push_back(another.invisible());
#else
                            cells_it->vertices.push_back(another);
#endif
                        }
                        {
                            T amount = 0.125
                                - 0.03125 * (topleft(b_it) + bottomright(b_it));
                            transform(vertex, amount, - amount);
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_5TH_IS_INVISIBLE
                            vertex.visible = false;
#endif
                        }
                    } else {
                        special = true;
                    }
                }
            } else {
                // {bottom, bottom-right} is the pair with the
                // angle closest to 180 degrees
                vertex.smooth = false;

                special = true;
            }

            if ( special ) {
                {
                    Point<T> another = vertex;
                    T amount = 0.03125;
                    transform(another,
                              amount
                              * ( topleft(c_it) - topright(d_it)
                                  + bottomleft(a_it) - bottomright(b_it) ),
                              // y
                              - amount
                              * ( topleft(c_it) + topright(d_it)
                                  - bottomleft(a_it) - bottomright(b_it) ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_1ST_IS_INVISIBLE
                    cells_it->vertices.push_back(another.invisible());
#else
                    cells_it->vertices.push_back(another);
#endif
                }
                {
                    Point<T> another = vertex;
                    T amount = 0.0625;
                    transform(another,
                              0.25 - amount
                              * ( topright(d_it) + bottomright(b_it) ),
                              // y
                              - amount
                              * ( topright(d_it) - bottomright(b_it) ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_2ND_IS_INVISIBLE
                    cells_it->vertices.push_back(another.invisible());
#else
                    cells_it->vertices.push_back(another);
#endif
                }
                {
                    Point<T> another = vertex;
                    T amount = 0.0625;
                    transform(another,
                              - ( 0.25 - amount
                                  * ( topleft(c_it) + bottomleft(a_it) ) ),
                              // y
                              - amount
                              * ( topleft(c_it) - bottomleft(a_it) ));
                    another.smooth = true;
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_3RD_IS_INVISIBLE
                    cells_it->vertices.push_back(another.invisible());
#else
                    cells_it->vertices.push_back(another);
#endif
                }
                {
                    Point<T> another = vertex;
                    T amount = 0.1875;
                    transform(another,
                              - ( 0.75 - amount
                                  * ( topleft(c_it) + bottomleft(a_it) ) ),
                              // y
                              -  amount
                              * ( topleft(c_it) - bottomleft(a_it) ));
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_4TH_IS_INVISIBLE
                    cells_it->vertices.push_back(another.invisible());
#else
                    cells_it->vertices.push_back(another);
#endif
                }
#ifdef LIBDEPIXELIZE_ENABLE_EXPERIMENTAL_FEATURES_5TH_IS_INVISIBLE
                vertex.visible = false;
#endif
            }
        } else {
            // there is a 4-color pattern, where the current node
            // won't be smooth
            vertex.smooth = false;
        }

        cells_it->vertices.push_back(vertex);
    }
}

} // namespace Tracer

#endif // LIBDEPIXELIZE_TRACER_SIMPLIFIEDVORONOI_H

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
