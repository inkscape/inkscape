/** \file
 * Inkscape::Lifecycle - automatic object lifecycle management
 *
 * Copyright 2008 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_LIFECYCLE_H
#define SEEN_INKSCAPE_LIFECYCLE_H

#include <glib/gtypes.h>
#include <glib/gatomic.h>

namespace Inkscape {
namespace Lifecycle {

class BaseManagedPtr;

// Abstract base class for managed objects
class AbstractManaged {
public:
    AbstractManaged() {}
    virtual ~AbstractManaged() {}
protected:
    virtual void managedAddReference() const=0;
    virtual void managedRemoveReference() const=0;
    friend class BaseManagedPtr;
private:
    AbstractManaged(AbstractManaged &); // no copy
    void operator=(AbstractManaged &); // no assign
};

// Base class for managed pointers; not intended for direct use
class BaseManagedPtr {
protected:
    BaseManagedPtr() : ptr(NULL) {}
    BaseManagedPtr(AbstractManaged const *managed) : ptr(NULL) {
        set(managed);
    }
    BaseManagedPtr(BaseManagedPtr const &other) : ptr(NULL) {
        set(other.ptr);
    }

    AbstractManaged const *get() const { return ptr; }
    void set(AbstractManaged const *managed) {
        if (managed) {
            ptr->managedAddReference();
        }
        if (ptr) {
            ptr->managedRemoveReference();
        }
        ptr = managed;
    }

private:
    AbstractManaged const *ptr;
};

// Type-safe managed pointer
template <typename T>
class ManagedPtr : public BaseManagedPtr {
public:
    ManagedPtr() : BaseManagedPtr() {}
    ManagedPtr(T *managed) : BaseManagedPtr(managed) {}
    ManagedPtr(ManagedPtr const &other) : BaseManagedPtr(other) {}

    operator T *() const { return get_recast(); }
    T &operator*() const { return *get_recast(); }
    T *operator->() const { return get_recast(); }
    ManagedPtr &operator=(T *managed) {
        set(managed);
        return *this;
    }
    ManagedPtr &operator=(ManagedPtr const &other) {
        set(other.get());
        return *this;
    }

private:
    T *get_recast() const {
        return const_cast<T *>(static_cast<T const *>(get()));
    }
};

// Wrapper for a simple pointer emulating the ManagedPtr interface
template <typename T>
class DumbPtr {
public:
    DumbPtr() : ptr(NULL) {}
    DumbPtr(T *p) : ptr(p) {}
    DumbPtr(DumbPtr const &other) : ptr(other.ptr) {}

    operator T *() const { return ptr; }
    T &operator*() const { return *ptr; }
    T *operator->() const { return ptr; }
    DumbPtr &operator=(T *p) {
        ptr = p;
        return *this;
    }
    DumbPtr &operator=(DumbPtr const &other) {
        ptr = other.ptr;
        return *this;
    }

private:
    T *ptr;
};

// Dummy implementation of AbstractManaged to ease migration
class DummyManaged : public virtual AbstractManaged {
protected:
    virtual void managedAddReference() const {}
    virtual void managedRemoveReference() const {}
};

// Straightforward refcounting implementation of AbstractManaged
class SimpleManaged : public virtual AbstractManaged {
protected:
    SimpleManaged() : refcount(0) {}
    virtual ~SimpleManaged() {}

    virtual void managedAddReference() const {
        g_atomic_int_inc(&refcount);
    }
    virtual void managedRemoveReference() const {
        if (g_atomic_int_dec_and_test(&refcount)) {
            const_cast<SimpleManaged *>(this)->managedDispose();
        }
    }
    virtual void managedDispose() {
        delete this;
    }

private:
    mutable volatile gint refcount;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
