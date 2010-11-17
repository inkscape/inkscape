/*
 * Inkscape::Util::ListContainer - encapsulates lists as STL containers,
 *                                 providing fast appending
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2005 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_LIST_CONTAINER_H
#define SEEN_INKSCAPE_UTIL_LIST_CONTAINER_H

#include <limits>
#include "util/list.h"

namespace Inkscape {

namespace Util {

template <typename T>
class ListContainer {
public:
    /* default constructible */
    ListContainer() {}

    /* assignable */
    ListContainer(ListContainer const &other) {
        *this = other;
    }
    ListContainer &operator=(ListContainer const &other) {
        clear();
        for ( const_iterator iter = other.begin() ; iter ; ++iter ) {
            push_back(*iter);
        }
        return *this;
    }
    void swap(ListContainer<T> &other) {
        std::swap(other._head, _head);
        std::swap(other._tail, _tail);
    }

    /* equality comparable */
    bool operator==(ListContainer const &other) const {
        const_iterator iter = _head;
        const_iterator other_iter = other._head;
        while ( iter && other_iter ) {
            if (!( *iter == *other_iter )) {
                return false;
            }
            ++iter;
            ++other_iter;
        }
        return !iter && !other_iter;
    }
    bool operator!=(ListContainer const &other) const {
        return !operator==(other);
    }

    /* lessthan comparable */
    bool operator<(ListContainer const &other) const {
        const_iterator iter = _head;
        const_iterator other_iter = other._head;
        while ( iter && other_iter ) {
            if ( *iter < *other_iter ) {
                return true;
            } else if ( *other_iter < *iter ) {
                return false;
            }
            ++iter;
            ++other_iter;
        }
        return bool(other_iter);
    }
    bool operator>=(ListContainer const &other) const {
        return !operator<(other);
    }

    /* container */
    typedef typename List<T>::value_type value_type;
    typedef List<T> iterator;
    typedef List<T const> const_iterator;
    typedef typename List<T>::reference reference;
    typedef typename List<T>::const_reference const_reference;
    typedef typename List<T>::pointer pointer;
    typedef typename List<T>::difference_type difference_type;
    typedef std::size_t size_type;

    iterator begin() { return _head; }
    const_iterator begin() const { return _head; }
    iterator end() { return iterator(); }
    const_iterator end() const { return const_iterator(); }
    size_type size() const {
        size_type size=0;
        for ( iterator iter = _head ; iter ; ++iter ) {
            size++;
        }
        return size;
    }
    size_type max_size() const {
        return std::numeric_limits<std::size_t>::max();
    }
    bool empty() const { return !_head; }

    /* sequence */
    ListContainer(size_type count, const_reference value) {
        for ( ; count ; --count ) {
            push_back(value);
        }
    }
    ListContainer(size_type count) {
        value_type default_value;
        for ( ; count ; --count ) {
            push_back(default_value);
        }
    }
    template <typename ForwardIterator>
    ListContainer(ForwardIterator i, ForwardIterator j) {
        for ( ; i != j ; ++i ) {
            push_back(*i);
        }
    }

    reference front() { return *_head; }
    const_reference front() const { return *_head; }

    iterator insert(const_iterator position, const_reference value) {
        if (position) {
            if ( position != _head ) {
                MutableList<T> added(value);
                MutableList<T> before=_before(position);
                set_rest(added, rest(before));
                set_rest(before, added);
                return added;
            } else {
                push_front(value);
                return _head;
            }
        } else {
            push_back(value);
            return _tail;
        }
    }
    void insert(const_iterator position, size_type count, const_reference value)
    {
        _insert_from_temp(position, ListContainer(count, value));
    }
    template <typename ForwardIterator>
    void insert(const_iterator position, ForwardIterator i, ForwardIterator j) {
        _insert_from_temp(position, ListContainer(i, j));
    }
    void erase(const_iterator position) {
        erase(position, rest(position));
    }
    void erase(const_iterator i, const_iterator j) {
        if ( i == _head ) {
            _head = static_cast<MutableList<T> &>(j);
            if ( !j || !rest(j) ) {
                _tail = _head;
            }
        } else {
            MutableList<T> before=_before(i);
            if (j) {
                set_rest(before, static_cast<MutableList<T> &>(j));
            } else {
                set_rest(before, MutableList<T>());
                _tail = before;
            }
        }
    }
    void clear() {
        _head = _tail = MutableList<T>();
    }
    void resize(size_type size, const_reference fill) {
        MutableList<T> before;
        MutableList<T> iter;
        for ( iter = _head ; iter && size ; ++iter ) {
            before = iter;
            size--;
        }
        if (size) {
            ListContainer temp(size, fill);
            if (empty()) {
                _head = temp._head;
                _tail = temp._tail;
            } else {
                set_rest(_tail, temp._head);
                _tail = temp._tail;
            }
        } else if (iter) {
            if (before) {
                set_rest(before, MutableList<T>());
                _tail = before;
            } else {
                _head = _tail = MutableList<T>();
            }
        }
    }
    void resize(size_type size) {
        resize(size, value_type());
    }

    /* front insertion sequence */
    void push_front(const_reference value) {
        if (_head) {
            _head = cons(value, _head);
        } else {
            _head = _tail = MutableList<T>(value);
        }
    }
    void pop_front() {
        if (_head) {
            _head = rest(_head);
            if (!_head) {
                _tail = _head;
            }
        }
    }

    /* back insertion sequence */
    reference back() { return *_tail; }
    const_reference back() const { return *_tail; }
    void push_back(const_reference value) {
        if (_tail) {
            MutableList<T> added(value);
            set_rest(_tail, added);
            _tail = added;
        } else {
            _head = _tail = MutableList<T>(value);
        }
    }
    // we're not required to provide pop_back if we can't
    // implement it efficiently

    /* additional */
    MutableList<T> detatchList() {
        MutableList<T> list=_head;
        _head = _tail = MutableList<T>();
        return list;
    }
    iterator insert_after(const_iterator pos, const_reference value) {
        MutableList<T> added(value);
        if (pos) {
            MutableList<T> before=static_cast<MutableList<T> &>(pos);
            set_rest(added, rest(before));
            set_rest(before, added);
            if ( _tail == before ) {
                _tail = added;
            }
        } else {
            push_front(value);
        }
    }
    void insert_after(const_iterator position, size_type count,
                      const_reference value)
    {
        _insert_after_from_temp(position, ListContainer(count, value));
    }
    template <typename ForwardIterator>
    void insert_after(const_iterator position,
                      ForwardIterator i, ForwardIterator j)
    {
        _insert_after_from_temp(position, ListContainer(i, j));
    }
    void erase_after(const_iterator position) {
        if (!position) {
            pop_front();
        } else {
            MutableList<T> before=static_cast<MutableList<T> &>(position);
            MutableList<T> removed=rest(before);
            set_rest(before, rest(removed));
            if ( removed == _tail ) {
                _tail = before;
            }
        }
    }

private:
    MutableList<T> _head;
    MutableList<T> _tail;

    MutableList<T> _before(const_iterator position) {
        for ( MutableList<T> iter = _head ; iter ; ++iter ) {
            if ( rest(iter) == position ) {
                return iter;
            }
        }
        return MutableList<T>();
    }
    void _insert_from_temp(const_iterator pos, ListContainer const &temp) {
        if (temp.empty()) {
            return;
        }
        if (empty()) { /* if empty, just take the whole thing */
            _head = temp._head;
            _tail = temp._tail;
        } else if (pos) {
            if ( pos == _head ) { /* prepend */
                set_rest(temp._tail, _head);
                _head = temp._head;
            } else { /* insert somewhere in the middle */
                MutableList<T> before=_before(pos);
                set_rest(temp._tail, static_cast<MutableList<T> &>(pos));
                set_rest(before, temp._head);
            }
        } else { /* append */
            set_rest(_tail, temp._head);
            _tail = temp._tail;
        }
    }
    void _insert_after_from_temp(const_iterator pos,
                                 ListContainer const &temp)
    {
        if (temp.empty()) {
            return;
        }
        if (empty()) { /* if empty, just take the whole thing */
            _head = temp._head;
            _tail = temp._tail;
        } else if (pos) {
            if ( pos == _tail ) { /* append */
                set_rest(_tail, temp._head);
                _tail = temp._tail;
            } else { /* insert somewhere in the middle */
                MutableList<T> before=static_cast<MutableList<T> &>(pos);
                set_rest(temp._tail, rest(before));
                set_rest(before, temp._head);
            }
        } else { /* prepend */
            set_rest(temp._tail, _head);
            _head = temp._head;
        }
    }
};

}

}

#endif
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
