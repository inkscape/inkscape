#ifndef SEEN_MODIFIER_FNS_H
#define SEEN_MODIFIER_FNS_H

/** \file 
 * Functions on GdkEventKey.state that test modifier keys.
 * 
 * The MOD__SHIFT macro in macros.h is equivalent to mod_shift(event-\>key.state).
 */

/*
 * Hereby placed in public domain.
 */

#include <gdk/gdktypes.h>
#include <glib/gtypes.h>

inline bool
mod_shift(guint const state)
{
    return state & GDK_SHIFT_MASK;
}

inline bool
mod_ctrl(guint const state)
{
    return state & GDK_CONTROL_MASK;
}

inline bool
mod_alt(guint const state)
{
    return state & GDK_MOD1_MASK;
}

inline bool
mod_shift_only(guint const state)
{
    return (state & GDK_SHIFT_MASK) && !(state & GDK_CONTROL_MASK) && !(state & GDK_MOD1_MASK);
}

inline bool
mod_ctrl_only(guint const state)
{
    return !(state & GDK_SHIFT_MASK) && (state & GDK_CONTROL_MASK) && !(state & GDK_MOD1_MASK);
}

inline bool
mod_alt_only(guint const state)
{
    return !(state & GDK_SHIFT_MASK) && !(state & GDK_CONTROL_MASK) && (state & GDK_MOD1_MASK);
}

#endif /* !SEEN_MODIFIER_FNS_H */

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
