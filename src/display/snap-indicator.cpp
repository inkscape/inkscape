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
#include "display/sodipodi-ctrl.h"
#include "knot.h"
#include "preferences.h"

namespace Inkscape {
namespace Display {

SnapIndicator::SnapIndicator(SPDesktop * desktop)
    :   _snaptarget(NULL),
    	_snapsource(NULL),
        _desktop(desktop)
{
}

SnapIndicator::~SnapIndicator()
{
    // remove item that might be present
	remove_snaptarget();
	remove_snapsource();
}

void
SnapIndicator::set_new_snaptarget(Inkscape::SnappedPoint const p)
{
	remove_snaptarget();
    
    g_assert(_desktop != NULL);
    
    /* Commented out for now, because this might hide any snapping bug!
    if (!p.getSnapped()) {
       return; // If we haven't snapped, then it is of no use to draw a snapindicator
    }
    */
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool value = prefs->getBool("/options/snapindicator/value", true);

    if (value) {
        SPCanvasItem * canvasitem = NULL;
        switch (p.getTarget()) {
            /// @todo  add the different kinds of snapindicator visuals
            case SNAPTARGET_GRID:
            case SNAPTARGET_GRID_INTERSECTION:
            case SNAPTARGET_GUIDE:
            case SNAPTARGET_GUIDE_INTERSECTION:
            case SNAPTARGET_GRID_GUIDE_INTERSECTION:
            case SNAPTARGET_NODE:
            case SNAPTARGET_PATH:
            case SNAPTARGET_PATH_INTERSECTION:
            case SNAPTARGET_BBOX_CORNER:
            case SNAPTARGET_BBOX_EDGE:
            case SNAPTARGET_GRADIENT:
            case SNAPTARGET_UNDEFINED:
            default:
                canvasitem = sp_canvas_item_new(sp_desktop_tempgroup (_desktop),
                                                SP_TYPE_CTRL,
                                                "anchor", GTK_ANCHOR_CENTER,
                                                "size", 10.0,
                                                "stroked", TRUE,
                                                "stroke_color", 0xf000f0ff,
                                                "mode", SP_KNOT_MODE_XOR,
                                                "shape", SP_KNOT_SHAPE_CROSS,
                                                NULL );
                break;
        }

        SP_CTRL(canvasitem)->moveto(p.getPoint());
        remove_snapsource(); // Don't set both the source and target indicators, as these will overlap
        _snaptarget = _desktop->add_temporary_canvasitem(canvasitem, 1000); // TODO add preference for snap indicator timeout
    }
}

void
SnapIndicator::remove_snaptarget()
{
    if (_snaptarget) {
        _desktop->remove_temporary_canvasitem(_snaptarget);
        _snaptarget = NULL;
    }
}

void
SnapIndicator::set_new_snapsource(Geom::Point const p)
{
	remove_snapsource();
    
    g_assert(_desktop != NULL);
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool value = prefs->getBool("/options/snapindicator/value", true);
    	
    if (value) {
        SPCanvasItem * canvasitem = sp_canvas_item_new( sp_desktop_tempgroup (_desktop),
                                                        SP_TYPE_CTRL,
                                                        "anchor", GTK_ANCHOR_CENTER,
                                                        "size", 10.0,
                                                        "stroked", TRUE,
                                                        "stroke_color", 0xf000f0ff,
                                                        "mode", SP_KNOT_MODE_XOR,
                                                        "shape", SP_KNOT_SHAPE_DIAMOND,
                                                        NULL );        
        
        SP_CTRL(canvasitem)->moveto(p);
        _snapsource = _desktop->add_temporary_canvasitem(canvasitem, 1000);
    }
}

void
SnapIndicator::remove_snapsource()
{
    if (_snapsource) {
        _desktop->remove_temporary_canvasitem(_snapsource);
        _snapsource = NULL;
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
