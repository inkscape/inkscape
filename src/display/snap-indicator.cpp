/** \file
 * Provides a class that shows a temporary indicator on the canvas of where the snap was, and what kind of snap
 *
 * Authors:
 *   Johan Engelen
 *   Diederik van Lierop
 *
 * Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Diederik van Lierop 2008 <mail@diedenrezi.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/snap-indicator.h"

#include "desktop.h"
#include "desktop-handles.h"
#include "sp-namedview.h"
#include "display/sodipodi-ctrl.h"
#include "knot.h"

namespace Inkscape {
namespace Display {

SnapIndicator::SnapIndicator(SPDesktop * desktop)
    :   _tempitem(NULL),
        _desktop(desktop)
{
}

SnapIndicator::~SnapIndicator()
{
    // remove item that might be present
    remove_snappoint();
}

void
SnapIndicator::set_new_snappoint(Inkscape::SnappedPoint const p)
{
    remove_snappoint();
    
    g_assert(_desktop != NULL);
    
    /* Commented out for now, because this might hide any snapping bug!
    if (!p.getSnapped()) {
       return; // If we haven't snapped, then it is of no use to draw a snapindicator
    }
    */
    
    SPNamedView *nv = sp_desktop_namedview(_desktop);
    
    if (nv->snapindicator) {
        // TODO add many different kinds of snap indicator :-)
        // For this we should use p->getTarget() to find out what has snapped 
        // and adjust the shape of the snapindicator accordingly, e.g. a cross
        // when snapping to an intersection, a circle when snapping to a node, etc. 
        SPCanvasItem * canvasitem = sp_canvas_item_new( sp_desktop_tempgroup (_desktop),
                                                        SP_TYPE_CTRL,
                                                        "anchor", GTK_ANCHOR_CENTER,
                                                        "size", 10.0,
                                                        "stroked", TRUE,
                                                        "stroke_color", 0xf000f0ff,
                                                        "mode", SP_KNOT_MODE_XOR,
                                                        "shape", SP_KNOT_SHAPE_CROSS,
                                                        NULL );        
        
        SP_CTRL(canvasitem)->moveto(p.getPoint());
        _tempitem = _desktop->add_temporary_canvasitem(canvasitem, 1000); // TODO add preference for snap indicator timeout
    }
}

void
SnapIndicator::remove_snappoint()
{
    if (_tempitem) {
        _desktop->remove_temporary_canvasitem(_tempitem);
        _tempitem = NULL;
    }
}


} //namespace Display
} /* namespace Inkscape */

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
