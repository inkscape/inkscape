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
    Point() {}
    Point(T x, T y) : x(x), y(y) {}
    Point(T x, T y, bool smooth) : smooth(smooth), x(x), y(y) {}

    Point operator+(const Point &rhs) const
    {
        return Point(x + rhs.x, y + rhs.y);
    }

    Point operator/(T foo) const
    {
        return Point(x / foo, y / foo);
    }

    bool smooth;

    T x, y;
};

template<class T>
inline bool operator==(const Point<T> &lhs, const Point<T> &rhs)
{
    return
#ifndef LIBDEPIXELIZE_IS_VERY_WELL_TESTED
        lhs.smooth == rhs.smooth &&
#endif // LIBDEPIXELIZE_IS_VERY_WELL_TESTED
        lhs.x == rhs.x && lhs.y == rhs.y;
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
