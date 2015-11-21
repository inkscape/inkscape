/*
 * Utility functions for switching tools (= contexts)
 *
 * Authors:
 *   bulia byak <bulia@dr.com>
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_TOOLS_SWITCH_H
#define SEEN_TOOLS_SWITCH_H

#if HAVE_CONFIG_H
# include "config.h"
#endif

class SPDesktop;
class SPItem;
namespace Geom {
class Point;
}


enum {
  TOOLS_INVALID,
  TOOLS_SELECT,
  TOOLS_NODES,
  TOOLS_TWEAK,
  TOOLS_SPRAY,
  TOOLS_SHAPES_RECT,
  TOOLS_SHAPES_3DBOX,
  TOOLS_SHAPES_ARC,
  TOOLS_SHAPES_STAR,
  TOOLS_SHAPES_SPIRAL,
  TOOLS_FREEHAND_PENCIL,
  TOOLS_FREEHAND_PEN,
  TOOLS_CALLIGRAPHIC,
  TOOLS_TEXT,
  TOOLS_GRADIENT,
  TOOLS_MESH,
  TOOLS_ZOOM,
  TOOLS_MEASURE,
  TOOLS_DROPPER,
  TOOLS_CONNECTOR,

#if HAVE_POTRACE
  TOOLS_PAINTBUCKET,
#endif

  TOOLS_ERASER,
  TOOLS_LPETOOL
};

int tools_isactive(SPDesktop *dt, unsigned num);
int tools_active(SPDesktop *dt);
void tools_switch(SPDesktop *dt, int num);
void tools_switch_by_item (SPDesktop *dt, SPItem *item, Geom::Point const p);

#endif // !SEEN_TOOLS_SWITCH_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
