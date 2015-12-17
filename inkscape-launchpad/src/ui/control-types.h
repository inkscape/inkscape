#ifndef SEEN_UI_CONTROL_TYPES_H
#define SEEN_UI_CONTROL_TYPES_H

/*
 * Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2012 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

namespace Inkscape
{

// Rough initial set. Most likely needs refinement.
enum ControlType {
    CTRL_TYPE_UNKNOWN,
    CTRL_TYPE_ADJ_HANDLE,
    CTRL_TYPE_ANCHOR,
    CTRL_TYPE_POINT,
    CTRL_TYPE_ROTATE,
    CTRL_TYPE_SIZER,
    CTRL_TYPE_SHAPER,
    CTRL_TYPE_LINE,
    CTRL_TYPE_NODE_AUTO,
    CTRL_TYPE_NODE_CUSP,
    CTRL_TYPE_NODE_SMOOTH,
    CTRL_TYPE_NODE_SYMETRICAL,
    CTRL_TYPE_INVISIPOINT
};

/**
 * Flags for internal representation/tracking.
 */
enum ControlFlags {
    CTRL_FLAG_NORMAL = 0,
    CTRL_FLAG_ACTIVE = 1 << 0,
    CTRL_FLAG_PRELIGHT = 1 << 1,
    CTRL_FLAG_SELECTED = 1 << 2,
};

} // namespace Inkscape

#endif // SEEN_UI_CONTROL_TYPES_H

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
