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

#ifndef LIBDEPIXELIZE_TRACER_SPLINES_H
#define LIBDEPIXELIZE_TRACER_SPLINES_H

#include <2geom/pathvector.h>
#include <glib.h>

namespace Tracer {

template<typename T, bool adjust_splines = true>
class SimplifiedVoronoi;

template<typename T>
class HomogeneousSplines;

class Splines
{
public:
    struct Path
    {
        /**
         * It may be benefited from C++11 move references.
         */
        Geom::PathVector pathVector;

        guint8 rgba[4];
    };

    typedef std::vector<Path>::iterator iterator;
    typedef std::vector<Path>::const_iterator const_iterator;

    Splines() /* = default */ {}

    template<typename T, bool adjust_splines>
    Splines(const SimplifiedVoronoi<T, adjust_splines> &simplifiedVoronoi);

    /**
     * There are two levels of optimization. The first level only removes
     * redundant points of colinear points. The second level uses the
     * Kopf-Lischinski optimization. The first level is always enabled.
     * The second level is enabled using \p optimize.
     */
    template<typename T>
    Splines(const HomogeneousSplines<T> &homogeneousSplines, bool optimize,
            int nthreads);

    // Iterators
    iterator begin()
    {
        return _paths.begin();
    }

    const_iterator begin() const
    {
        return _paths.begin();
    }

    iterator end()
    {
        return _paths.end();
    }

    const_iterator end() const
    {
        return _paths.end();
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
    std::vector<Path> _paths;
    int _width;
    int _height;
};

} // namespace Tracer

#endif // LIBDEPIXELIZE_TRACER_SPLINES_H

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
