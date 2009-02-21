/** @file
 * @brief STL iterators for Glib list types
 */
/* Inkscape::Util::GSListIterator - STL iterator for GSList
 * Inkscape::Util::GSListConstIterator - STL iterator for GSList
 * Inkscape::Util::GListIterator - STL iterator for GList
 * Inkscape::Util::GListConstIterator - STL iterator for GList
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2005 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_GLIB_LIST_ITERATORS_H
#define SEEN_INKSCAPE_GLIB_LIST_ITERATORS_H

#include <cstddef>
#include <iterator>
#include "glib/gslist.h"
#include "glib/glist.h"

namespace Inkscape {
namespace Util {

template <typename T> class GSListConstIterator;
template <typename T> class GSListIterator;
template <typename T> class GListConstIterator;
template <typename T> class GListIterator;

template <typename T>
class GSListConstIterator<T *> {
public:
    typedef std::forward_iterator_tag iterator_category;
    typedef T * const value_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type *pointer;
    typedef value_type &reference;

    GSListConstIterator(GSList const *list) : _list(list) {}
    // default copy
    // default assign
    GSList const *list() const { return _list; }
    
    reference operator*() const {
        _ref_union u;
        u._gp = _list->data;
        return *u._p;
    }

    bool operator==(GSListConstIterator const &other) {
        return other._list == _list;
    }
    bool operator!=(GSListConstIterator const &other) {
        return other._list != _list;
    }

    GSListConstIterator &operator++() {
        _list = _list->next;
        return *this;
    }
    GSListConstIterator operator++(int) {
        GSListConstIterator saved=*this;
        _list = _list->next;
        return saved;
    }

private:
    union _ref_union {gpointer _gp; pointer _p;};
    GSList const *_list;
};

template <typename T>
class GSListIterator<T *> {
public:
    typedef std::forward_iterator_tag iterator_category;
    typedef T *value_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type *pointer;
    typedef value_type &reference;
    typedef value_type const &const_reference;

    GSListIterator(GSList *list) : _list(list) {}
    // default copy
    // default assign
    operator GSListConstIterator<T *>() const { return _list; }
    GSList const *list() const { return _list; }
    GSList *list() { return _list; }
    
    const_reference operator*() const {
        return const_cast<GSListIterator<T *> *>(this)->operator*();
    }
    reference operator*() {
        _ref_union u;
        u._gp = _list->data;
        return *u._p;
    }

    bool operator==(GSListIterator const &other) {
        return other._list == _list;
    }
    bool operator!=(GSListIterator const &other) {
        return other._list != _list;
    }

    GSListIterator &operator++() {
        _list = _list->next;
        return *this;
    }
    GSListIterator operator++(int) {
        GSListIterator saved=*this;
        _list = _list->next;
        return saved;
    }

private:
    union _ref_union {gpointer _gp; pointer _p;};
    GSList *_list;
};
 
template <typename T>
class GListConstIterator<T *> {
public:
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef T * const value_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type *pointer;
    typedef value_type &reference;

    GListConstIterator(GList const *list) : _list(list) {}
    // default copy
    // default assign
    GList const *list() const { return _list; }
    
    reference operator*() const {
        _ref_union u;
        u._gp = _list->data;
        return *u._p;
    }

    bool operator==(GListConstIterator const &other) {
        return other._list == _list;
    }
    bool operator!=(GListConstIterator const &other) {
        return other._list != _list;
    }

    GListConstIterator &operator++() {
        _list = _list->next;
        return *this;
    }
    GListConstIterator operator++(int) {
        GListConstIterator saved=*this;
        _list = _list->next;
        return saved;
    }

    GListConstIterator &operator--() {
        _list = _list->prev;
        return *this;
    }
    GListConstIterator operator--(int) {
        GListConstIterator saved=*this;
        _list = _list->prev;
        return saved;
    }

private:
    union _ref_union {gpointer _gp; pointer _p;};
    GList const *_list;
};

template <typename T>
class GListIterator<T *> {
public:
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef T *value_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type *pointer;
    typedef value_type &reference;
    typedef value_type const &const_reference;

    GListIterator(GList *list) : _list(list) {}
    // default copy
    // default assign
    operator GSListConstIterator<T *>() const {
        return reinterpret_cast<GSList *>(_list);
    }
    operator GSListIterator<T *>() const {
        return reinterpret_cast<GSList *>(_list);
    }
    operator GListConstIterator<T *>() const { return _list; }
    GList const *list() const { return _list; }
    GList *list() { return _list; }
    
    const_reference operator*() const {
        return const_cast<GListIterator<T *> *>(this)->operator*();
    }
    reference operator*() {
        _ref_union u;
        u._gp = _list->data;
        return *u._p;
    }

    bool operator==(GListIterator const &other) {
        return other._list == _list;
    }
    bool operator!=(GListIterator const &other) {
        return other._list != _list;
    }

    GListIterator &operator++() {
        _list = _list->next;
        return *this;
    }
    GListIterator operator++(int) {
        GListIterator saved=*this;
        _list = _list->next;
        return saved;
    }

    GListIterator &operator--() {
        _list = _list->prev;
        return *this;
    }
    GListIterator operator--(int) {
        GListIterator saved=*this;
        _list = _list->prev;
        return saved;
    }

private:
    union _ref_union {gpointer _gp; pointer _p;};
    GList *_list;
};
 
} // namespace Util
} // Namespace Inkscape

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
