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

#ifndef LIBDEPIXELIZE_TRACER_ITERATOR_H
#define LIBDEPIXELIZE_TRACER_ITERATOR_H

#include <vector>
#include <iterator>

namespace Tracer {

template<typename T>
const T *to_ptr(typename std::vector<T>::const_iterator it)
{
    return &*it;
}

template<typename T>
T *to_ptr(typename std::vector<T>::iterator it)
{
    return &*it;
}

template<typename T>
typename std::vector<T>::const_iterator to_iterator(T const *ptr,
                                                    typename std::vector<T>
                                                    ::const_iterator begin)
{
    typedef typename std::vector<T>::const_iterator It;
    typedef typename std::iterator_traits<It>::difference_type difference_type;
    difference_type idx = ptr - to_ptr<T>(begin);
    return begin + idx;
}

template<typename T>
typename std::vector<T>::iterator to_iterator(T *ptr,
                                              typename std::vector<T>::iterator
                                              begin)
{
    typedef typename std::vector<T>::iterator It;
    typedef typename std::iterator_traits<It>::difference_type difference_type;
    difference_type idx = ptr - to_ptr<T>(begin);
    return begin + idx;
}

template<typename T>
class ToIter
{
public:
    typedef typename std::vector<T>::const_iterator const_iterator;
    typedef typename std::vector<T>::iterator iterator;

    ToIter(const_iterator begin) :
        begin(begin)
    {}

    const_iterator operator()(T const *ptr) const
    {
        return to_iterator<T>(ptr, begin);
    }

    iterator operator()(T *ptr) const
    {
        return to_iterator<T>(ptr, begin);
    }

private:
    typename std::vector<T>::const_iterator begin;
};

template<typename T>
class ToPtr
{
public:
    typedef typename std::vector<T>::const_iterator const_iterator;
    typedef typename std::vector<T>::iterator iterator;

    const T *operator()(const_iterator it) const
    {
        return to_ptr<T>(it);
    }

    T *operator()(iterator it) const
    {
        return to_ptr<T>(it);
    }
};

} // namespace Tracer

#endif // LIBDEPIXELIZE_TRACER_ITERATOR_H

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
