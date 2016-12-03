#ifndef SEEN_KNOT_ENUMS_H
#define SEEN_KNOT_ENUMS_H

/**
 * @file
 * Some enums used by SPKnot and by related types \& functions. 
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

typedef enum {
    SP_KNOT_SHAPE_SQUARE,
    SP_KNOT_SHAPE_DIAMOND,
    SP_KNOT_SHAPE_CIRCLE,
    SP_KNOT_SHAPE_TRIANGLE,
    SP_KNOT_SHAPE_CROSS,
    SP_KNOT_SHAPE_BITMAP,
    SP_KNOT_SHAPE_IMAGE
} SPKnotShapeType;

typedef enum {
    SP_KNOT_MODE_COLOR,
    SP_KNOT_MODE_XOR
} SPKnotModeType;

typedef enum {
    SP_KNOT_STATE_NORMAL,
    SP_KNOT_STATE_MOUSEOVER,
    SP_KNOT_STATE_DRAGGING,
    SP_KNOT_STATE_HIDDEN
} SPKnotStateType;

#define SP_KNOT_VISIBLE_STATES 3

enum {
    SP_KNOT_VISIBLE = 1 << 0,
    SP_KNOT_MOUSEOVER = 1 << 1,
    SP_KNOT_DRAGGING = 1 << 2,
    SP_KNOT_GRABBED = 1 << 3
};


#endif /* !SEEN_KNOT_ENUMS_H */

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
