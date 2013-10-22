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

#ifndef LIBDEPIXELIZE_TRACER_POINT_H
#define LIBDEPIXELIZE_TRACER_POINT_H

namespace Tracer {

template<class T>
struct Point
{
    Point() : smooth(false), visible(true) {}
    Point(T x, T y) : smooth(false), visible(true), x(x), y(y) {}
    Point(T x, T y, bool smooth) : smooth(smooth), visible(true), x(x), y(y) {}

    Point operator+(const Point &rhs) const
    {
        return Point(x + rhs.x, y + rhs.y);
    }

    Point operator/(T foo) const
    {
        return Point(x / foo, y / foo);
    }

    Point invisible() const
    {
        Point p = *this;
        p.visible = false;
        return p;
    }

    bool smooth;

    /**
     * By default, all points are visible, but the poor amount of information
     * that B-Splines (libdepixelize-specific) allows us to represent forces us
     * to create additional points. But... these additional points don't need to
     * be visible.
     */
    bool visible;

    T x, y;
};

template<class T>
Point<T> midpoint(const Point<T> &a, const Point<T> &b)
{
    return Point<T>((a.x + b.x) / 2, (a.y + b.y) / 2);
}

template<class T>
bool operator==(const Point<T> &lhs, const Point<T> &rhs)
{
    return
        /*
         * Will make a better job identifying which points can be eliminated by
         * cells union.
         */
#ifndef LIBDEPIXELIZE_IS_VERY_WELL_TESTED
        lhs.smooth == rhs.smooth &&
#endif // LIBDEPIXELIZE_IS_VERY_WELL_TESTED
        lhs.x == rhs.x && lhs.y == rhs.y;
}

template<class T>
bool weakly_equal(const Point<T> &a, const Point<T> &b)
{
    return a.x == b.x && a.y == b.y;
}

template<class T>
Geom::Point to_geom_point(Point<T> p)
{
    return Geom::Point(p.x, p.y);
}

} // namespace Tracer

#endif // LIBDEPIXELIZE_TRACER_POINT_H

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
