/** @file
 * Wrapper for Boehm GC.
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "inkgc/gc-core.h"
#include <stdexcept>
#include <cstring>
#include <string>
#include <glib.h>
#include <sigc++/functors/ptr_fun.h>
#include <glibmm/main.h>
#include <cstddef>

namespace Inkscape {
namespace GC {

namespace {

void display_warning(char *msg, GC_word arg) {
    g_warning(msg, arg);
}

void do_init() {
    GC_no_dls = 1;
    GC_all_interior_pointers = 1;
    GC_finalize_on_demand = 0;

    GC_INIT();

    GC_set_warn_proc(&display_warning);
}

void *debug_malloc(std::size_t size) {
    return GC_debug_malloc(size, GC_EXTRAS);
}

void *debug_malloc_atomic(std::size_t size) {
    return GC_debug_malloc_atomic(size, GC_EXTRAS);
}

void *debug_malloc_uncollectable(std::size_t size) {
    return GC_debug_malloc_uncollectable(size, GC_EXTRAS);
}

void *debug_malloc_atomic_uncollectable(std::size_t size) {
    return GC_debug_malloc_uncollectable(size, GC_EXTRAS);
}

std::ptrdiff_t compute_debug_base_fixup() {
    char *base=reinterpret_cast<char *>(GC_debug_malloc(1, GC_EXTRAS));
    char *real_base=reinterpret_cast<char *>(GC_base(base));
    GC_debug_free(base);
    return base - real_base;
}

inline std::ptrdiff_t const &debug_base_fixup() {
    static std::ptrdiff_t fixup=compute_debug_base_fixup();
    return fixup;
}

void *debug_base(void *ptr) {
    char *base=reinterpret_cast<char *>(GC_base(ptr));
    return base + debug_base_fixup();
}

int debug_general_register_disappearing_link(void **p_ptr, void const *base) {
    char const *real_base = reinterpret_cast<char const *>(base) - debug_base_fixup();
#if (GC_MAJOR_VERSION >= 7 && GC_MINOR_VERSION >= 4)
    return GC_general_register_disappearing_link(p_ptr, real_base);
#else // compatibility with older Boehm GC versions
    return GC_general_register_disappearing_link(p_ptr, const_cast<char *>(real_base));
#endif
}

void dummy_do_init() {}

void *dummy_base(void *) { return NULL; }

void dummy_register_finalizer(void *, CleanupFunc, void *,
                                      CleanupFunc *old_func, void **old_data)
{
    if (old_func) {
        *old_func = NULL;
    }
    if (old_data) {
        *old_data = NULL;
    }
}

int dummy_general_register_disappearing_link(void **, void const *) { return false; }

int dummy_unregister_disappearing_link(void **/*link*/) { return false; }

std::size_t dummy_get_heap_size() { return 0; }

std::size_t dummy_get_free_bytes() { return 0; }

void dummy_gcollect() {}

void dummy_enable() {}

void dummy_disable() {}

Ops enabled_ops = {
    &do_init,
    &GC_malloc,
    &GC_malloc_atomic,
    &GC_malloc_uncollectable,
    &GC_malloc_atomic_uncollectable,
    &GC_base,
    &GC_register_finalizer_ignore_self,
#if (GC_MAJOR_VERSION >= 7 && GC_MINOR_VERSION >= 4)
     &GC_general_register_disappearing_link,
#else // compatibility with older Boehm GC versions
    (int (*)(void**, const void*))(&GC_general_register_disappearing_link),
#endif
    &GC_unregister_disappearing_link,
    &GC_get_heap_size,
    &GC_get_free_bytes,
    &GC_gcollect,
    &GC_enable,
    &GC_disable,
    &GC_free
};

Ops debug_ops = {
    &do_init,
    &debug_malloc,
    &debug_malloc_atomic,
    &debug_malloc_uncollectable,
    &debug_malloc_atomic_uncollectable,
    &debug_base,
    &GC_debug_register_finalizer_ignore_self,
    &debug_general_register_disappearing_link,
    &GC_unregister_disappearing_link,
    &GC_get_heap_size,
    &GC_get_free_bytes,
    &GC_gcollect,
    &GC_enable,
    &GC_disable,
    &GC_debug_free
};

Ops disabled_ops = {
    &dummy_do_init,
    &std::malloc,
    &std::malloc,
    &std::malloc,
    &std::malloc,
    &dummy_base,
    &dummy_register_finalizer,
    &dummy_general_register_disappearing_link,
    &dummy_unregister_disappearing_link,
    &dummy_get_heap_size,
    &dummy_get_free_bytes,
    &dummy_gcollect,
    &dummy_enable,
    &dummy_disable,
    &std::free
};

class InvalidGCModeError : public std::runtime_error {
public:
    InvalidGCModeError(const char *mode)
    : runtime_error(std::string("Unknown GC mode \"") + mode + "\"")
    {}
};

Ops const &get_ops() throw (InvalidGCModeError) {
    char *mode_string=std::getenv("_INKSCAPE_GC");
    if (mode_string) {
        if (!std::strcmp(mode_string, "enable")) {
            return enabled_ops;
        } else if (!std::strcmp(mode_string, "debug")) {
            return debug_ops;
        } else if (!std::strcmp(mode_string, "disable")) {
            return disabled_ops;
        } else {
            throw InvalidGCModeError(mode_string);
        }
    } else {
        return enabled_ops;
    }
}

void die_because_not_initialized() {
    g_error("Attempt to use GC allocator before call to Inkscape::GC::init()");
}

void *stub_malloc(std::size_t) {
    die_because_not_initialized();
    return NULL;
}

void *stub_base(void *) {
    die_because_not_initialized();
    return NULL;
}

void stub_register_finalizer_ignore_self(void *, CleanupFunc, void *,
                                                 CleanupFunc *, void **)
{
    die_because_not_initialized();
}

int stub_general_register_disappearing_link(void **, void const *) {
    die_because_not_initialized();
    return 0;
}

int stub_unregister_disappearing_link(void **) {
    die_because_not_initialized();
    return 0;
}

std::size_t stub_get_heap_size() {
    die_because_not_initialized();
    return 0;
}

std::size_t stub_get_free_bytes() {
    die_because_not_initialized();
    return 0;
}

void stub_gcollect() {
    die_because_not_initialized();
}

void stub_enable() {
    die_because_not_initialized();
}

void stub_disable() {
    die_because_not_initialized();
}

void stub_free(void *) {
    die_because_not_initialized();
}

}

Ops Core::_ops = {
    NULL,
    &stub_malloc,
    &stub_malloc,
    &stub_malloc,
    &stub_malloc,
    &stub_base,
    &stub_register_finalizer_ignore_self,
    &stub_general_register_disappearing_link,
    &stub_unregister_disappearing_link,
    &stub_get_heap_size,
    &stub_get_free_bytes,
    &stub_gcollect,
    &stub_enable,
    &stub_disable,
    &stub_free
};

void Core::init() {
    try {
        _ops = get_ops();
    } catch (InvalidGCModeError &e) {
        g_warning("%s; enabling normal collection", e.what());
        _ops = enabled_ops;
    }

    _ops.do_init();
}


namespace {

bool collection_requested=false;
bool collection_task() {
    Core::gcollect();
    Core::gcollect();
    collection_requested=false;
    return false;
}

}

void request_early_collection() {
    if (!collection_requested) {
        collection_requested=true;
        Glib::signal_idle().connect(sigc::ptr_fun(&collection_task));
    }
}

}
}

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
