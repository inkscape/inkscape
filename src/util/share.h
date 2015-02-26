/*
 * Inkscape::Util::ptr_shared<T> - like T const *, but stronger
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2006 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_SHARE_H
#define SEEN_INKSCAPE_UTIL_SHARE_H

#include "inkgc/gc-core.h"
#include <cstring>
#include <cstddef>

namespace Inkscape {
namespace Util {

template <typename T>
class ptr_shared {
public:
    ptr_shared() : _obj(NULL) {}

    template <typename T1>
    ptr_shared(ptr_shared<T1> const &other) : _obj(other._obj) {}

    T const *pointer() const { return _obj; }

    template <typename T1>
    operator T1 const *() const { return _obj; }

    operator bool() const { return _obj; }

    T const &operator*() const { return *_obj; }
    T const *operator->() const { return _obj; }
    T const &operator[](int i) const { return _obj[i]; }

    ptr_shared<T> operator+(int i) const {
        return share_unsafe(_obj+i);
    }
    ptr_shared<T> operator-(int i) const {
        return share_unsafe(_obj-i);
    }

    ptr_shared<T> &operator+=(int i) const {
        _obj += i;
        return *this;
    }
    ptr_shared<T> &operator-=(int i) const {
        _obj -= i;
        return *this;
    }

    template <typename T1>
    std::ptrdiff_t operator-(ptr_shared<T1> const &other) {
        return _obj - other._obj;
    }

    template <typename T1>
    ptr_shared<T> &operator=(ptr_shared<T1> const &other) {
        _obj = other._obj;
        return *this;
    }

    template <typename T1>
    bool operator==(ptr_shared<T1> const &other) const {
        return _obj == other._obj;
    }

    template <typename T1>
    bool operator!=(ptr_shared<T1> const &other) const {
        return _obj != other._obj;
    }

    template <typename T1>
    bool operator>(ptr_shared<T1> const &other) const {
        return _obj > other._obj;
    }

    template <typename T1>
    bool operator<(ptr_shared<T1> const &other) const {
        return _obj < other._obj;
    }

    static ptr_shared<T> share_unsafe(T const *obj) {
        return ptr_shared<T>(obj);
    }

protected:
    explicit ptr_shared(T const *obj) : _obj(obj) {}

private:
    T const *_obj;
};

template <typename T>
inline ptr_shared<T> share(T const *obj) {
    return share_unsafe(obj ? new T(*obj) : NULL);
}

ptr_shared<char> share_string(char const *string);
ptr_shared<char> share_string(char const *string, std::size_t length);

template <typename T>
inline ptr_shared<T> reshare(T const *obj) {
    return ptr_shared<T>::share_unsafe(obj);
}

template <typename T>
inline ptr_shared<T> share_unsafe(T const *obj) {
    return ptr_shared<T>::share_unsafe(obj);
}

inline ptr_shared<char> share_static_string(char const *string) {
    return share_unsafe(string);
}

template <typename T1, typename T2>
inline ptr_shared<T1> static_cast_shared(ptr_shared<T2> const &ref) {
    return reshare(static_cast<T1 const *>(ref.pointer()));
}

template <typename T1, typename T2>
inline ptr_shared<T1> dynamic_cast_shared(ptr_shared<T2> const &ref) {
    return reshare(dynamic_cast<T1 const *>(ref.pointer()));
}

template <typename T1, typename T2>
inline ptr_shared<T1> reinterpret_cast_shared(ptr_shared<T2> const &ref) {
    return reshare(reinterpret_cast<T1 const *>(ref.pointer()));
}

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
