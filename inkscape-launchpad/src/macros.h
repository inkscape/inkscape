#ifndef SEEN_MACROS_H
#define SEEN_MACROS_H

/**
 * Useful macros for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

// I'm of the opinion that this file should be removed, so I will in the future take the necessary steps to wipe it out.
// Macros are not in general bad, but these particular ones are rather ugly. Especially that sp_round one. --Liam

#ifdef SP_MACROS_SILENT
#define SP_PRINT_MATRIX(s,m)
#define SP_PRINT_TRANSFORM(s,t)
#define SP_PRINT_DRECT(s,r)
#define SP_PRINT_DRECT_WH(s,r)
#define SP_PRINT_IRECT(s,r)
#define SP_PRINT_IRECT_WH(s,r)
#else
#define SP_PRINT_MATRIX(s,m) g_print("%s (%g %g %g %g %g %g)\n", (s), (m)->c[0], (m)->c[1], (m)->c[2], (m)->c[3], (m)->c[4], (m)->c[5])
#define SP_PRINT_TRANSFORM(s,t) g_print("%s (%g %g %g %g %g %g)\n", (s), (t)[0], (t)[1], (t)[2], (t)[3], (t)[4], (t)[5])
#define SP_PRINT_DRECT(s,r) g_print("%s (%g %g %g %g)\n", (s), (r)->x0, (r)->y0, (r)->x1, (r)->y1)
#define SP_PRINT_DRECT_WH(s,r) g_print("%s (%g %g %g %g)\n", (s), (r)->x0, (r)->y0, (r)->x1 - (r)->x0, (r)->y1 - (r)->y0)
#define SP_PRINT_IRECT(s,r) g_print("%s (%d %d %d %d)\n", (s), (r)->x0, (r)->y0, (r)->x1, (r)->y1)
#define SP_PRINT_IRECT_WH(s,r) g_print("%s (%d %d %d %d)\n", (s), (r)->x0, (r)->y0, (r)->x1 - (r)->x0, (r)->y1 - (r)->y0)
#endif

#define sp_signal_disconnect_by_data(o,d) g_signal_handlers_disconnect_matched(o, G_SIGNAL_MATCH_DATA, 0, 0, 0, 0, d)

#define sp_round(v,m) (((v) < 0.0) ? ((ceil((v) / (m) - 0.5)) * (m)) : ((floor((v) / (m) + 0.5)) * (m)))

#endif

// keyboard modifiers in an event
#define MOD__SHIFT(event) ((event)->key.state & GDK_SHIFT_MASK)
#define MOD__CTRL(event) ((event)->key.state & GDK_CONTROL_MASK)
#define MOD__ALT(event) ((event)->key.state & GDK_MOD1_MASK)
#define MOD__SHIFT_ONLY(event) (((event)->key.state & GDK_SHIFT_MASK) && !((event)->key.state & GDK_CONTROL_MASK) && !((event)->key.state & GDK_MOD1_MASK))
#define MOD__CTRL_ONLY(event) (!((event)->key.state & GDK_SHIFT_MASK) && ((event)->key.state & GDK_CONTROL_MASK) && !((event)->key.state & GDK_MOD1_MASK))
#define MOD__ALT_ONLY(event) (!((event)->key.state & GDK_SHIFT_MASK) && !((event)->key.state & GDK_CONTROL_MASK) && ((event)->key.state & GDK_MOD1_MASK))

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
