#ifndef SEEN_CONTEXT_FNS_H
#define SEEN_CONTEXT_FNS_H

/*
 * <description>
 *
 * Authors:
 *
 * Copyright (C) 
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gdk/gdkevents.h>
#include <2geom/forward.h>

struct SPDesktop;
struct SPItem;

const double goldenratio = 1.61803398874989484820; // golden ratio

namespace Inkscape
{

class MessageContext;
class MessageStack;

extern bool have_viable_layer(SPDesktop *desktop, MessageContext *message);
extern bool have_viable_layer(SPDesktop *desktop, MessageStack *message);
Geom::Rect snap_rectangular_box(SPDesktop const *desktop, SPItem *item,
                              Geom::Point const &pt, Geom::Point const &center, int state);
Geom::Point setup_for_drag_start(SPDesktop *desktop, SPEventContext* ec, GdkEvent *ev);

}

#endif // !SEEN_CONTEXT_FNS_H

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
