#ifndef SEEN_SP_SELTRANS_HANDLES_H
#define SEEN_SP_SELTRANS_HANDLES_H

/*
 * Seltrans knots
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/forward.h>
#include <gdk/gdk.h>
#include "enums.h"

namespace Inkscape
{
  class SelTrans;
}

enum SPSelTransType {
    HANDLE_STRETCH,
    HANDLE_SCALE,
    HANDLE_SKEW,
    HANDLE_ROTATE,
    HANDLE_CENTER
};

struct SPSelTransTypeInfo {
        gchar const *tip;
};
// One per handle type in order
extern SPSelTransTypeInfo const handtypes[5];

struct SPSelTransHandle;

struct SPSelTransHandle {
        SPSelTransType type;
	SPAnchorType anchor;
	GdkCursorType cursor;
	guint control;
	gdouble x, y;
};
// These are 4 * each handle type + 1 for center
int const NUMHANDS = 17;
extern SPSelTransHandle const hands[17];

#endif // SEEN_SP_SELTRANS_HANDLES_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :


