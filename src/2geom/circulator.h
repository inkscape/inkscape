/** @file
 * @brief Circular iterator adapter
 *//*
 * Copyright 2006 MenTaLguY <mental@rydia.net>
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
 *
 */

#ifndef LIB2GEOM_SEEN_CIRCULATOR_H
#define LIB2GEOM_SEEN_CIRCULATOR_H

#include <iterator>

namespace Geom {

/** @brief Circular iterator adapter
 * This iterator adapter will loop indefinitely over a set of values
 * from a random access container. */
template <typename Iterator>
class Circulator {
public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    typedef typename std::iterator_traits<Iterator>::difference_type difference_type;
    typedef typename std::iterator_traits<Iterator>::pointer pointer;
    typedef typename std::iterator_traits<Iterator>::reference reference;

    Circulator(Iterator const &first,
               Iterator const &last,
               Iterator const &pos)
    : _first(first), _last(last), _pos(pos)
    {
        match_random_access(iterator_category(first));
    }

    reference operator*() const {
        return *_pos;
    }
    pointer operator->() const {
        return &*_pos;
    }
    
    Circulator &operator++() {
        if ( _first == _last ) return *this;
        ++_pos;
        if ( _pos == _last ) _pos = _first;
        return *this;
    }
    Circulator operator++(int) {
        Circulator saved=*this;
        ++(*this);
        return saved;
    }

    Circulator &operator--() {
        if ( _pos == _first ) _pos = _last;
        --_pos;
        return *this;
    }
    Circulator operator--(int) {
        Circulator saved=*this;
        --(*this);
        return saved;
    }

    Circulator &operator+=(int n) {
        _pos = _offset(n);
        return *this;
    }
    Circulator operator+(int n) const {
        return Circulator(_first, _last, _offset(n));
    }
    Circulator &operator-=(int n) {
        _pos = _offset(-n);
        return *this;
    }
    Circulator operator-(int n) const {
        return Circulator(_first, _last, _offset(-n));
    }

    difference_type operator-(Circulator const &other) {
        return _pos - other._pos;
    }

    reference operator[](int n) const {
        return *_offset(n);
    }

private:
    void match_random_access(iterator_category) {}

    Iterator _offset(int n) {
        difference_type range=( _last - _first );
        difference_type offset=( _pos - _first + n );

        if ( offset < 0 ) {
            // modulus not well-defined for negative numbers in C++
            offset += ( ( -offset / range ) + 1 ) * range;
        } else if ( offset >= range ) {
            offset %= range;
        }
        return _first + offset;
    }

    Iterator _first;
    Iterator _last;
    Iterator _pos;
};

}

template <typename T>
Geom::Circulator<T> operator+(int n, Geom::Circulator<T> const &c) {
    return c + n;
}

#endif // LIB2GEOM_SEEN_CIRCULATOR_H

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
