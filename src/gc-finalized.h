/*
 * Inkscape::GC::Finalized - mixin for GC-managed objects with non-trivial
 *                           destructors
 *
 * Copyright 2004 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_GC_FINALIZED_H
#define SEEN_INKSCAPE_GC_FINALIZED_H

#include <new>
#include <cstddef>
#include "inkgc/gc-core.h"

namespace Inkscape {

namespace GC {

/* @brief A mix-in ensuring that an object's destructor will get called before
 *        the garbage collector destroys it
 *
 * Normally, the garbage collector does not call destructors before destroying
 * an object.  On construction, this "mix-in" will register a finalizer
 * function to call destructors before derived objects are destroyed.
 * 
 * This works pretty well, with the following caveats:
 *
 *   1. The garbage collector uses strictly topologically-ordered
 *      finalization; if objects with finalizers reference each other
 *      directly or indirectly, the collector will refuse to finalize (and
 *      therefor free) them.  You'll see a warning on the console if this
 *      happens.
 *
 *      The best way to limit this effect is to only make "leaf" objects
 *      (i.e. those that don't point to other finalizable objects)
 *      finalizable, and otherwise use GC::soft_ptr<> instead of a regular
 *      pointer for "backreferences" (e.g. parent pointers in a tree
 *      structure), so that those references can be cleared to break any
 *      finalization cycles.
 *
 *      @see Inkscape::GC::soft_ptr<>
 *
 *   2. Because there is no guarantee when the collector will destroy
 *      objects, it is impossible to tell in advance when the destructor
 *      will get called. It may not get called until the very end
 *      of the program, or ever.
 *
 *   3. If allocated in arrays, only the first object in the array will
 *      have its destructor called, unless you make other arrangements by
 *      registering your own finalizer instead.
 *
 *   4. Similarly, putting a finalized object as a member in another
 *      garbage collected but non-finalized object will cause the member
 *      object's destructor not to be called when the parent object is
 *      collected, unless you register the finalizer yourself (by "member"
 *      we mean an actual by-value member, not a reference or a pointer).
 */
class Finalized {
public:
    Finalized() {
        void *base=Core::base(this);
        if (base) { // only if we are managed by the collector
            CleanupFunc old_cleanup;
            void *old_data;

            // the finalization callback needs to know the value of 'this'
            // to call the destructor, but registering a real pointer to
            // ourselves would pin us forever and prevent us from being
            // finalized; instead we use an offset-from-base-address

            Core::register_finalizer_ignore_self(base, _invoke_dtor,
                                                       _offset(base, this),
                                                       &old_cleanup, &old_data);

            if (old_cleanup) {
                // If there was already a finalizer registered for our
                // base address, there are two main possibilities:
                //
                // 1. one of our members is also a GC::Finalized and had
                //    already registered a finalizer -- keep ours, since
                //    it will call that member's destructor, too
                //
                // 2. someone else registered a finalizer and we will have
                //    to trust that they will call the destructor -- put
                //    the existing finalizer back
                //
                // It's also possible that a member's constructor was called
                // after ours (e.g. via placement new).  Don't do that.

                if ( old_cleanup != _invoke_dtor ) {
                    Core::register_finalizer_ignore_self(base,
                                                         old_cleanup, old_data,
                                                         NULL, NULL);
                }
            }
        }
    }

    virtual ~Finalized() {
        // make sure the destructor won't get invoked twice
        Core::register_finalizer_ignore_self(Core::base(this),
                                             NULL, NULL, NULL, NULL);
    }

private:
    /// invoke the destructor for an object given a base and offset pair
    static void _invoke_dtor(void *base, void *offset);

    /// turn 'this' pointer into an offset-from-base-address (stored as void *)
    static void *_offset(void *base, Finalized *self) {
        return reinterpret_cast<void *>(
            reinterpret_cast<char *>(self) - reinterpret_cast<char *>(base)
        );
    }
    /// reconstitute 'this' given an offset-from-base-address in a void *
    static Finalized *_unoffset(void *base, void *offset) {
        return reinterpret_cast<Finalized *>(
            reinterpret_cast<char *>(base) +
            reinterpret_cast<std::ptrdiff_t>(offset)
        );
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
