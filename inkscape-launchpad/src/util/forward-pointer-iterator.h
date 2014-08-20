/*
 * Inkscape::Util::ForwardPointerIterator - wraps a simple pointer
 *                                          with various strategies
 *                                          to determine sequence
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_FORWARD_POINTER_ITERATOR_H
#define SEEN_INKSCAPE_UTIL_FORWARD_POINTER_ITERATOR_H

#include <iterator>
#include <cstddef>
#include "util/reference.h"

namespace Inkscape {

namespace Util {

template <typename BaseType, typename Strategy>
class ForwardPointerIterator;

template <typename BaseType, typename Strategy>
class ForwardPointerIterator<BaseType const, Strategy> {
public:
    typedef std::forward_iterator_tag iterator_category;
    typedef typename Traits::Reference<BaseType const>::LValue value_type;
    typedef std::ptrdiff_t difference_type;
    typedef typename Traits::Reference<BaseType const>::LValue reference;
    typedef typename Traits::Reference<BaseType const>::RValue const_reference;
    typedef typename Traits::Reference<BaseType const>::Pointer pointer;

    typedef ForwardPointerIterator<BaseType const, Strategy> Self;

    ForwardPointerIterator() : _p(NULL) {}
    ForwardPointerIterator(pointer p) : _p(p) {}

    operator pointer() const { return _p; }
    reference operator*() const { return *_p; }
    pointer operator->() const { return _p; }

    bool operator==(Self const &other) const {
        return _p == other._p;
    }
    bool operator!=(Self const &other) const {
        return _p != other._p;
    }

    Self &operator++() {
        _p = Strategy::next(_p);
        return *this;
    }
    Self operator++(int) {
        Self old(*this);
        operator++();
        return old;
    }

    operator bool() const { return _p != NULL; }

private:
    pointer _p;
};

template <typename BaseType, typename Strategy>
class ForwardPointerIterator
: public ForwardPointerIterator<BaseType const, Strategy>
{
public:
    typedef typename Traits::Reference<BaseType>::LValue value_type;
    typedef typename Traits::Reference<BaseType>::LValue reference;
    typedef typename Traits::Reference<BaseType>::RValue const_reference;
    typedef typename Traits::Reference<BaseType>::Pointer pointer;

    typedef ForwardPointerIterator<BaseType const, Strategy> Ancestor;
    typedef ForwardPointerIterator<BaseType, Strategy> Self;

    ForwardPointerIterator() : Ancestor() {}
    ForwardPointerIterator(pointer p) : Ancestor(p) {}

    operator pointer() const {
        return const_cast<pointer>(Ancestor::operator->());
    }
    reference operator*() const {
        return const_cast<reference>(Ancestor::operator*());
    }
    pointer operator->() const {
        return const_cast<pointer>(Ancestor::operator->());
    }

    Self &operator++() {
        Ancestor::operator++();
        return *this;
    }
    Self operator++(int) {
        Self old(*this);
        operator++();
        return old;
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
