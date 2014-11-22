/** @file
 * @brief Wrapper for Boehm GC
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_GC_CORE_H
#define SEEN_INKSCAPE_GC_CORE_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <new>
#include <cstdlib>
#include <cstddef>
#ifdef HAVE_GC_GC_H
# include <gc/gc.h>
#else
# include <gc.h>
#endif

namespace Inkscape {
namespace GC {

enum ScanPolicy {
    SCANNED,
    ATOMIC
};

enum CollectionPolicy {
    AUTO,
    MANUAL
};

enum Delete {
    GC
};

typedef void (*CleanupFunc)(void *mem, void *data);

struct Ops {
    void (*do_init)();
    void *(*malloc)(std::size_t size);
    void *(*malloc_atomic)(std::size_t size);
    void *(*malloc_uncollectable)(std::size_t size);
    void *(*malloc_atomic_uncollectable)(std::size_t size);
    void *(*base)(void *ptr);
    void (*register_finalizer_ignore_self)(void *base,
                                           CleanupFunc func, void *data,
                                           CleanupFunc *old_func,
                                           void **old_data);
    int (*general_register_disappearing_link)(void **p_ptr,
                                              void const *base);
    int (*unregister_disappearing_link)(void **p_ptr);
    std::size_t (*get_heap_size)();
    std::size_t (*get_free_bytes)();
    void (*gcollect)();
    void (*enable)();
    void (*disable)();
    void (*free)(void *ptr);
};

struct Core {
public:
    static void init();
    static inline void *malloc(std::size_t size) {
        return _ops.malloc(size);
    }
    static inline void *malloc_atomic(std::size_t size) {
        return _ops.malloc_atomic(size);
    }
    static inline void *malloc_uncollectable(std::size_t size) {
        return _ops.malloc_uncollectable(size);
    }
    static inline void *malloc_atomic_uncollectable(std::size_t size) {
        return _ops.malloc_atomic_uncollectable(size);
    }
    static inline void *base(void *ptr) {
        return _ops.base(ptr);
    }
    static inline void register_finalizer_ignore_self(void *base,
                                                      CleanupFunc func,
                                                      void *data,
                                                      CleanupFunc *old_func,
                                                      void **old_data)
    {
        return _ops.register_finalizer_ignore_self(base, func, data,
                                                   old_func, old_data);
    }
    static inline int general_register_disappearing_link(void **p_ptr,
                                                         void *base)
    {
        return _ops.general_register_disappearing_link(p_ptr, base);
    }
    static inline int unregister_disappearing_link(void **p_ptr) {
        return _ops.unregister_disappearing_link(p_ptr);
    }
    static inline std::size_t get_heap_size() {
        return _ops.get_heap_size();
    }
    static inline std::size_t get_free_bytes() {
        return _ops.get_free_bytes();
    }
    static inline void gcollect() {
        _ops.gcollect();
    }
    static inline void enable() {
        _ops.enable();
    }
    static inline void disable() {
        _ops.disable();
    }
    static inline void free(void *ptr) {
        return _ops.free(ptr);
    }
private:
    static Ops _ops;
};

inline void init() {
    Core::init();
}

void request_early_collection();

}
}

inline void *operator new(std::size_t size,
                          Inkscape::GC::ScanPolicy scan,
                          Inkscape::GC::CollectionPolicy collect,
                          Inkscape::GC::CleanupFunc cleanup=NULL,
                          void *data=NULL)
throw(std::bad_alloc)
{
    using namespace Inkscape::GC;

    void *mem;
    if ( collect == AUTO ) {
        if ( scan == SCANNED ) {
            mem = Core::malloc(size);
        } else {
            mem = Core::malloc_atomic(size);
        }
    } else {
        if ( scan == SCANNED ) {
            mem = Core::malloc_uncollectable(size);
        } else {
            mem = Core::malloc_atomic_uncollectable(size);
        }
    }
    if (!mem) {
        throw std::bad_alloc();
    }
    if (cleanup) {
        Core::register_finalizer_ignore_self(mem, cleanup, data, NULL, NULL);
    }
    return mem;
}

inline void *operator new(std::size_t size,
                          Inkscape::GC::ScanPolicy scan,
                          Inkscape::GC::CleanupFunc cleanup=NULL,
                          void *data=NULL)
throw(std::bad_alloc)
{
    return operator new(size, scan, Inkscape::GC::AUTO, cleanup, data);
}

inline void *operator new[](std::size_t size,
                            Inkscape::GC::ScanPolicy scan,
                            Inkscape::GC::CollectionPolicy collect,
                            Inkscape::GC::CleanupFunc cleanup=NULL,
                            void *data=NULL)
throw(std::bad_alloc)
{
    return operator new(size, scan, collect, cleanup, data);
}

inline void *operator new[](std::size_t size,
                            Inkscape::GC::ScanPolicy scan,
                            Inkscape::GC::CleanupFunc cleanup=NULL,
                            void *data=NULL)
throw(std::bad_alloc)
{
    return operator new[](size, scan, Inkscape::GC::AUTO, cleanup, data);
}

inline void operator delete(void *mem, Inkscape::GC::Delete) {
    Inkscape::GC::Core::free(mem);
}

inline void operator delete[](void *mem, Inkscape::GC::Delete) {
    operator delete(mem, Inkscape::GC::GC);
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
