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

#ifndef LIBDEPIXELIZE_TRACER_PIXELGRAPH_H
#define LIBDEPIXELIZE_TRACER_PIXELGRAPH_H

#include <gdkmm/pixbuf.h>
#include <vector>
#include <cassert>
#include <utility>

namespace Tracer {

class PixelGraph
{
public:
    class Node
    {
    public:
        /*
         * Hamming weight of \p adj
         */
        unsigned adjsize() const
        {
            unsigned all[8] = {
                adj.top,
                adj.topright,
                adj.right,
                adj.bottomright,
                adj.bottom,
                adj.bottomleft,
                adj.left,
                adj.topleft
            };
            return all[0] + all[1] + all[2] + all[3]
                + all[4] + all[5] + all[6] + all[7];
        }

        guint8 rgba[4];
        // Nodes pointing from this
        struct Adj
        {
            unsigned top: 1;
            unsigned topright: 1;
            unsigned right: 1;
            unsigned bottomright: 1;
            unsigned bottom: 1;
            unsigned bottomleft: 1;
            unsigned left: 1;
            unsigned topleft: 1;
        } adj;
    };

    typedef std::vector<Node>::iterator iterator;
    typedef std::vector<Node>::const_iterator const_iterator;
    typedef std::vector<Node>::reverse_iterator reverse_iterator;
    typedef std::vector<Node>::const_reverse_iterator const_reverse_iterator;

    typedef std::pair<iterator, iterator> Edge;
    typedef std::pair<Edge, Edge> EdgePair;
    typedef std::vector<EdgePair> EdgePairContainer;

    class ColumnView
    {
    public:
        ColumnView(std::vector<Node> &nodes, int width, int column) :
            _nodes(nodes), _width(width), _column(column)
        {}

        Node &operator[](int line);

    private:
        std::vector<Node> &_nodes;
        const int _width;
        const int _column;
    };

    PixelGraph(Glib::RefPtr<Gdk::Pixbuf const> pixbuf);

    void checkConsistency();

    /**
     * It'll let you access the nodes using the syntax:
     *
     *     graph[x][y]
     *
     * Where x is the column and y is the line.
     */
    ColumnView operator[](int column);

    // Iterators
    iterator begin()
    {
        return _nodes.begin();
    }

    const_iterator begin() const
    {
        return _nodes.begin();
    }

    iterator end()
    {
        return _nodes.end();
    }

    const_iterator end() const
    {
        return _nodes.end();
    }

    reverse_iterator rbegin()
    {
        return _nodes.rbegin();
    }

    const_reverse_iterator rbegin() const
    {
        return _nodes.rbegin();
    }

    reverse_iterator rend()
    {
        return _nodes.rend();
    }

    const_reverse_iterator rend() const
    {
        return _nodes.rend();
    }

    size_t size() const
    {
        return _nodes.size();
    }

    int width() const
    {
        return _width;
    }

    int height() const
    {
        return _height;
    }

    // Algorithms
    void connectAllNeighbors();
    EdgePairContainer crossingEdges();

    int toX(const_iterator n) const
    {
        return (&*n - &_nodes[0]) % _width;
    }

    int toY(const_iterator n) const
    {
        return (&*n - &_nodes[0]) / _width;
    }

    iterator nodeTop(iterator n)
    {
        return n - _width;
    }

    iterator nodeBottom(iterator n)
    {
        return n + _width;
    }

    iterator nodeLeft(iterator n)
    {
        return n - 1;
    }

    iterator nodeRight(iterator n)
    {
        return n + 1;
    }

    iterator nodeTopLeft(iterator n)
    {
        return n - _width - 1;
    }

    iterator nodeTopRight(iterator n)
    {
        return n - _width + 1;
    }

    iterator nodeBottomLeft(iterator n)
    {
        return n + _width - 1;
    }

    iterator nodeBottomRight(iterator n)
    {
        return n + _width + 1;
    }

    const_iterator nodeTop(const_iterator n) const
    {
        return n - _width;
    }

    const_iterator nodeBottom(const_iterator n) const
    {
        return n + _width;
    }

    const_iterator nodeLeft(const_iterator n) const
    {
        return n - 1;
    }

    const_iterator nodeRight(const_iterator n) const
    {
        return n + 1;
    }

    const_iterator nodeTopLeft(const_iterator n) const
    {
        return n - _width - 1;
    }

    const_iterator nodeTopRight(const_iterator n) const
    {
        return n - _width + 1;
    }

    const_iterator nodeBottomLeft(const_iterator n) const
    {
        return n + _width - 1;
    }

    const_iterator nodeBottomRight(const_iterator n) const
    {
        return n + _width + 1;
    }

private:
    PixelGraph(const PixelGraph&);

    int _width;
    int _height;

    // The data representation follows the image data pattern from gdk-pixbuf.
    //
    // Quoting:
    // "Image data in a pixbuf is stored in memory in uncompressed, packed
    // format. Rows in the image are stored top to bottom, and in each row
    // pixels are stored from left to right. There may be padding at the end of
    // a row."
    //
    // Differently, _nodes don't put padding among rows.
    std::vector<Node> _nodes;
};

inline PixelGraph::PixelGraph(Glib::RefPtr<Gdk::Pixbuf const> pixbuf) :
    _width(pixbuf->get_width()),
    _height(pixbuf->get_height()),
    _nodes(size_t(_width) * _height)
{
    if ( !_width || !_height )
        return;

    // Initialize the graph using the pixels' color data
    guint8 *pixels = pixbuf->get_pixels();
    Node *dest = &_nodes[0];
    const int n_channels = pixbuf->get_n_channels();
    const int rowpadding = pixbuf->get_rowstride() - _width * n_channels;

    if ( n_channels == 4 ) {
        for ( int i = 0 ; i != _height ; ++i ) {
            for ( int j = 0 ; j != _width ; ++j ) {
                for ( int k = 0 ; k != 4 ; ++k )
                    dest->rgba[k] = pixels[k];
                {
                    dest->adj.top = 0;
                    dest->adj.topright = 0;
                    dest->adj.right = 0;
                    dest->adj.bottomright = 0;
                    dest->adj.bottom = 0;
                    dest->adj.bottomleft = 0;
                    dest->adj.left = 0;
                    dest->adj.topleft = 0;
                }
                pixels += n_channels;
                ++dest;
            }
            pixels += rowpadding;
        }
    } else {
        assert(n_channels == 3);
        for ( int i = 0 ; i != _height ; ++i ) {
            for ( int j = 0 ; j != _width ; ++j ) {
                for ( int k = 0 ; k != 3 ; ++k )
                    dest->rgba[k] = pixels[k];
                dest->rgba[3] = '\xFF';
                {
                    dest->adj.top = 0;
                    dest->adj.topright = 0;
                    dest->adj.right = 0;
                    dest->adj.bottomright = 0;
                    dest->adj.bottom = 0;
                    dest->adj.bottomleft = 0;
                    dest->adj.left = 0;
                    dest->adj.topleft = 0;
                }
                pixels += n_channels;
                ++dest;
            }
            pixels += rowpadding;
        }
    }
}

inline void PixelGraph::checkConsistency()
{
    PixelGraph::Node *it = &_nodes.front();
    for ( int i = 0 ; i != _height ; ++i ) {
        for ( int j = 0 ; j != _width ; ++j, ++it ) {
            if ( it->adj.top )
                assert((it - _width)->adj.bottom);
            if ( it->adj.topright )
                assert((it - _width + 1)->adj.bottomleft);
            if ( it->adj.right )
                assert((it + 1)->adj.left);
            if ( it->adj.bottomright )
                assert((it + _width + 1)->adj.topleft);
            if ( it->adj.bottom )
                assert((it + _width)->adj.top);
            if ( it->adj.bottomleft )
                assert((it + _width - 1)->adj.topright);
            if ( it->adj.left )
                assert((it - 1)->adj.right);
            if ( it->adj.topleft )
                assert((it - _width - 1)->adj.bottomright);
        }
    }
}

inline PixelGraph::ColumnView PixelGraph::operator[](int column)
{
    return ColumnView(_nodes, _width, column);
}

inline void PixelGraph::connectAllNeighbors()
{
    //  ...the "center" nodes first...
    if ( _width > 2 && _height > 2 ) {
        iterator it = nodeBottomRight(begin()); // [1][1]
        for ( int i = 1 ; i != _height - 1 ; ++i ) {
            for ( int j = 1 ; j != _width - 1 ; ++j ) {
                it->adj.top = 1;
                it->adj.topright = 1;
                it->adj.right = 1;
                it->adj.bottomright = 1;
                it->adj.bottom = 1;
                it->adj.bottomleft = 1;
                it->adj.left = 1;
                it->adj.topleft = 1;

                it = nodeRight(it);
            }
            // After the previous loop, 'it' is pointing to the last node from
            // the row.
            // Go south, then first node in the row (increment 'it' by 1)
            // Go to the second node in the line (increment 'it' by 1)
            it += 2;
        }
    }

    //  ...then the "top" nodes...
    if ( _width > 2 ) {
        Node *it = &_nodes[1];
        if ( _height > 1 ) {
            for ( int i = 1 ; i != _width - 1 ; ++i ) {
                it->adj.right = 1;
                it->adj.bottomright = 1;
                it->adj.bottom = 1;
                it->adj.bottomleft = 1;
                it->adj.left = 1;

                ++it;
            }
        } else {
            for ( int i = 1 ; i != _width - 1 ; ++i ) {
                it->adj.right = 1;
                it->adj.left = 1;

                ++it;
            }
        }
    }

    //  ...then the "bottom" nodes...
    if ( _width > 2 && _height > 1 ) {
        Node *it = &((*this)[1][_height - 1]);
        for ( int i = 1 ; i != _width - 1 ; ++i ) {
            it->adj.left = 1;
            it->adj.topleft = 1;
            it->adj.top = 1;
            it->adj.topright = 1;
            it->adj.right = 1;

            ++it;
        }
    }

    //  ...then the "left" nodes...
    if ( _height > 2 ) {
        iterator it = nodeBottom(begin()); // [0][1]
        if ( _width > 1 ) {
            for ( int i = 1 ; i != _height - 1 ; ++i ) {
                it->adj.top = 1;
                it->adj.topright = 1;
                it->adj.right = 1;
                it->adj.bottomright = 1;
                it->adj.bottom = 1;

                it = nodeBottom(it);
            }
        } else {
            for ( int i = 1 ; i != _height - 1 ; ++i ) {
                it->adj.top = 1;
                it->adj.bottom = 1;

                it = nodeBottom(it);
            }
        }
    }

    //  ...then the "right" nodes...
    if ( _height > 2 && _width > 1 ) {
        iterator it = nodeBottom(begin() + _width - 1);// [_width - 1][1]
        for ( int i = 1 ; i != _height - 1 ; ++i ) {
            it->adj.bottom = 1;
            it->adj.bottomleft = 1;
            it->adj.left = 1;
            it->adj.topleft = 1;
            it->adj.top = 1;

            it = nodeBottom(it);
        }
    }

    //  ...and the 4 corner nodes
    {
        Node *const top_left = &(*this)[0][0];

        if ( _width > 1 )
            top_left->adj.right = 1;

        if ( _width > 1 && _height > 1 )
            top_left->adj.bottomright = 1;

        if ( _height > 1 )
            top_left->adj.bottom = 1;
    }
    if ( _width > 1 ) {
        Node *const top_right = &(*this)[_width - 1][0];

        if ( _height > 1 ) {
            top_right->adj.bottom = 1;
            top_right->adj.bottomleft = 1;
        }

        top_right->adj.left = 1;
    }
    if ( _height > 1 ) {
        Node *const down_left = &(*this)[0][_height - 1];
        down_left->adj.top = 1;

        if ( _width > 1 ) {
            down_left->adj.topright = 1;
            down_left->adj.right = 1;
        }
    }
    if ( _width > 1 && _height > 1 ) {
        Node *const down_right = &(*this)[_width - 1][_height - 1];
        down_right->adj.left = 1;
        down_right->adj.topleft = 1;
        down_right->adj.top = 1;
    }
}

PixelGraph::EdgePairContainer PixelGraph::crossingEdges()
{
    EdgePairContainer ret;

    if ( width() < 2 || height() < 2 )
        return ret;

    // Iterate over the graph, 2x2 blocks at time
    PixelGraph::iterator it = begin();
    for (int i = 0 ; i != height() - 1 ; ++i, ++it ) {
        for ( int j = 0 ; j != width() - 1 ; ++j, ++it ) {
            EdgePair diagonals(
                Edge(it, nodeBottomRight(it)),
                Edge(nodeRight(it), nodeBottom(it)));

            // Check if there are crossing edges
            if ( !diagonals.first.first->adj.bottomright
                 || !diagonals.second.first->adj.bottomleft ) {
                continue;
            }

            ret.push_back(diagonals);
        }
    }

    return ret;
}

inline PixelGraph::Node &PixelGraph::ColumnView::operator[](int line)
{
    return _nodes[line * _width + _column];
}

} // namespace Tracer

#endif // LIBDEPIXELIZE_TRACER_PIXELGRAPH_H

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
