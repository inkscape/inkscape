/*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 * * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_GC_ANCHORED_H
#define SEEN_INKSCAPE_GC_ANCHORED_H

#include "inkgc/gc-managed.h"

namespace Inkscape {

namespace GC {

/**
 * A base class for anchored objects.  
 *
 * Objects are managed by our mark-and-sweep collector, but are anchored 
 * against garbage collection so long as their reference count is nonzero.
 *
 * Object and member destructors will not be called on destruction
 * unless a subclass also inherits from Inkscape::GC::Finalized.
 *
 * New instances of anchored objects should be created using the C++ new
 * operator.  Under normal circumstances they should not be created on
 * the stack.
 *
 * A newly created anchored object begins with a refcount of one, and
 * will not be collected unless the refcount is zero.
 *
 * NOTE: If you create an object yourself, it is already anchored for
 *       you.  You do not need to anchor it a second time.
 *
 * Note that a cycle involving an anchored object (with nonzero refcount)
 * cannot be collected.  To avoid this, don't increment refcounts for
 * pointers between two GC-managed objects.
 *
 * @see Inkscape::GC::Managed
 * @see Inkscape::GC::Finalized
 * @see Inkscape::GC::anchor
 * @see Inkscape::GC::release
 */

class Anchored {
public:
    void anchor() const;
    void release() const;

    // for debugging
    unsigned _anchored_refcount() const {
        return ( _anchor ? _anchor->refcount : 0 );
    }

protected:
    Anchored() : _anchor(NULL) { anchor(); } // initial refcount of one
    virtual ~Anchored() {}

private:
    struct Anchor : public Managed<SCANNED, MANUAL> {
        Anchor() : refcount(0),base(NULL) {}
        Anchor(Anchored const *obj) : refcount(0) {
            base = Core::base(const_cast<Anchored *>(obj));
        }
        int refcount;
        void *base;
    };

    mutable Anchor *_anchor;

    Anchor *_new_anchor() const;
    void _free_anchor(Anchor *anchor) const;

    Anchored(Anchored const &); // no copy
    void operator=(Anchored const &); // no assign
};

/**
 * @brief Increments the reference count of a anchored object.
 *
 * This function template generates functions which take
 * a reference to a anchored object of a given type, increment
 * that object's reference count, and return a reference to the
 * object of the same type as the function's parameter.
 *
 * @param m a reference to a anchored object
 *
 * @return the reference to the object
 */
template <typename R>
static R &anchor(R &r) {
    static_cast<Anchored const &>(const_cast<R const &>(r)).anchor();
    return r;
}

/**
 * @brief Increments the reference count of a anchored object.
 *
 * This function template generates functions which take
 * a pointer to a anchored object of a given type, increment
 * that object's reference count, and return a pointer to the
 * object of the same type as the function's parameter.
 *
 * @param m a pointer to anchored object
 *
 * @return the pointer to the object
 */
template <typename R>
static R *anchor(R *r) {
    static_cast<Anchored const *>(const_cast<R const *>(r))->anchor();
    return r;
}

/**
 * @brief Decrements the reference count of a anchored object.
 *
 * This function template generates functions which take
 * a reference to a anchored object of a given type, increment
 * that object's reference count, and return a reference to the
 * object of the same type as the function's parameter.
 *
 * The return value is safe to use since the object, even if
 * its refcount has reached zero, will not actually be collected
 * until there are no references to it in local variables or
 * parameters.
 *
 * @param m a reference to a anchored object
 *
 * @return the reference to the object
 */
template <typename R>
static R &release(R &r) {
    static_cast<Anchored const &>(const_cast<R const &>(r)).release();
    return r;
}

/**
 * @brief Decrements the reference count of a anchored object.
 *
 * This function template generates functions which take
 * a pointer to a anchored object of a given type, increment
 * that object's reference count, and return a pointer to the
 * object of the same type as the function's parameter.
 *
 * The return value is safe to use since the object, even if
 * its refcount has reached zero, will not actually be collected
 * until there are no references to it in local variables or
 * parameters.
 *
 * @param m a pointer to a anchored object
 *
 * @return the pointer to the object
 */
template <typename R>
static R *release(R *r) {
    static_cast<Anchored const *>(const_cast<R const *>(r))->release();
    return r;
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
