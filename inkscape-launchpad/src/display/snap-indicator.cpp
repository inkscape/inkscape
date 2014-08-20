/*
 * Provides a class that shows a temporary indicator on the canvas of where the snap was, and what kind of snap
 *
 * Authors:
 *   Johan Engelen
 *   Diederik van Lierop
 *
 * Copyright (C) Johan Engelen 2009 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Diederik van Lierop 2010 - 2012 <mail@diedenrezi.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/snap-indicator.h"

#include "desktop.h"
#include "desktop-handles.h"
#include "display/sodipodi-ctrl.h"
#include "display/sodipodi-ctrlrect.h"
#include "display/canvas-text.h"
#include "display/sp-canvas-util.h"
#include "knot.h"
#include "preferences.h"
#include <glibmm/i18n.h>
#include "tools-switch.h"
#include "enums.h"

namespace Inkscape {
namespace Display {

SnapIndicator::SnapIndicator(SPDesktop * desktop)
    :   _snaptarget(NULL),
        _snaptarget_tooltip(NULL),
        _snaptarget_bbox(NULL),
        _snapsource(NULL),
        _snaptarget_is_presnap(false),
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
SnapIndicator::set_new_snaptarget(Inkscape::SnappedPoint const &p, bool pre_snap)
{
    remove_snaptarget(); //only display one snaptarget at a time

    g_assert(_desktop != NULL);

    if (!p.getSnapped()) {
        return; // If we haven't snapped, then it is of no use to draw a snapindicator
    }

    if (p.getTarget() == SNAPTARGET_CONSTRAINT) {
        // This is not a real snap, although moving along the constraint did affect the mouse pointer's position.
        // Maybe we should only show a snap indicator when the user explicitly asked for a constraint by pressing ctrl?
        // We should not show a snap indicator when stretching a selection box, which is also constrained. That would be
        // too much information.
        return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool value = prefs->getBool("/options/snapindicator/value", true);

    if (value) {
        // TRANSLATORS: undefined target for snapping
        gchar *target_name = _("UNDEFINED");
        switch (p.getTarget()) {
            case SNAPTARGET_UNDEFINED:
                target_name = _("UNDEFINED");
                g_warning("Snap target has not been specified");
                break;
            case SNAPTARGET_GRID:
                target_name = _("grid line");
                break;
            case SNAPTARGET_GRID_INTERSECTION:
                target_name = _("grid intersection");
                break;
            case SNAPTARGET_GRID_PERPENDICULAR:
                target_name = _("grid line (perpendicular)");
                break;
            case SNAPTARGET_GUIDE:
                target_name = _("guide");
                break;
            case SNAPTARGET_GUIDE_INTERSECTION:
                target_name = _("guide intersection");
                break;
            case SNAPTARGET_GUIDE_ORIGIN:
                target_name = _("guide origin");
                break;
            case SNAPTARGET_GUIDE_PERPENDICULAR:
                target_name = _("guide (perpendicular)");
                break;
            case SNAPTARGET_GRID_GUIDE_INTERSECTION:
                target_name = _("grid-guide intersection");
                break;
            case SNAPTARGET_NODE_CUSP:
                target_name = _("cusp node");
                break;
            case SNAPTARGET_NODE_SMOOTH:
                target_name = _("smooth node");
                break;
            case SNAPTARGET_PATH:
                target_name = _("path");
                break;
            case SNAPTARGET_PATH_PERPENDICULAR:
                target_name = _("path (perpendicular)");
                break;
            case SNAPTARGET_PATH_TANGENTIAL:
                target_name = _("path (tangential)");
                break;
            case SNAPTARGET_PATH_INTERSECTION:
                target_name = _("path intersection");
                break;
            case SNAPTARGET_PATH_GUIDE_INTERSECTION:
                target_name = _("guide-path intersection");
                break;
            case SNAPTARGET_PATH_CLIP:
                target_name = _("clip-path");
                break;
            case SNAPTARGET_PATH_MASK:
                target_name = _("mask-path");
                break;
            case SNAPTARGET_BBOX_CORNER:
                target_name = _("bounding box corner");
                break;
            case SNAPTARGET_BBOX_EDGE:
                target_name = _("bounding box side");
                break;
            case SNAPTARGET_PAGE_BORDER:
                target_name = _("page border");
                break;
            case SNAPTARGET_LINE_MIDPOINT:
                target_name = _("line midpoint");
                break;
            case SNAPTARGET_OBJECT_MIDPOINT:
                target_name = _("object midpoint");
                break;
            case SNAPTARGET_ROTATION_CENTER:
                target_name = _("object rotation center");
                break;
            case SNAPTARGET_BBOX_EDGE_MIDPOINT:
                target_name = _("bounding box side midpoint");
                break;
            case SNAPTARGET_BBOX_MIDPOINT:
                target_name = _("bounding box midpoint");
                break;
            case SNAPTARGET_PAGE_CORNER:
                target_name = _("page corner");
                break;
            case SNAPTARGET_ELLIPSE_QUADRANT_POINT:
                target_name = _("quadrant point");
                break;
            case SNAPTARGET_RECT_CORNER:
            case SNAPTARGET_IMG_CORNER:
                target_name = _("corner");
                break;
            case SNAPTARGET_TEXT_ANCHOR:
                target_name = _("text anchor");
                break;
            case SNAPTARGET_TEXT_BASELINE:
                target_name = _("text baseline");
                break;
            case SNAPTARGET_CONSTRAINED_ANGLE:
                target_name = _("constrained angle");
                break;
            case SNAPTARGET_CONSTRAINT:
                target_name = _("constraint");
                break;
            default:
                g_warning("Snap target not in SnapTargetType enum");
                break;
        }

        gchar *source_name = _("UNDEFINED");
        switch (p.getSource()) {
            case SNAPSOURCE_UNDEFINED:
                source_name = _("UNDEFINED");
                g_warning("Snap source has not been specified");
                break;
            case SNAPSOURCE_BBOX_CORNER:
                source_name = _("Bounding box corner");
                break;
            case SNAPSOURCE_BBOX_MIDPOINT:
                source_name = _("Bounding box midpoint");
                break;
            case SNAPSOURCE_BBOX_EDGE_MIDPOINT:
                source_name = _("Bounding box side midpoint");
                break;
            case SNAPSOURCE_NODE_SMOOTH:
                source_name = _("Smooth node");
                break;
            case SNAPSOURCE_NODE_CUSP:
                source_name = _("Cusp node");
                break;
            case SNAPSOURCE_LINE_MIDPOINT:
                source_name = _("Line midpoint");
                break;
            case SNAPSOURCE_OBJECT_MIDPOINT:
                source_name = _("Object midpoint");
                break;
            case SNAPSOURCE_ROTATION_CENTER:
                source_name = _("Object rotation center");
                break;
            case SNAPSOURCE_NODE_HANDLE:
            case SNAPSOURCE_OTHER_HANDLE:
                source_name = _("Handle");
                break;
            case SNAPSOURCE_PATH_INTERSECTION:
                source_name = _("Path intersection");
                break;
            case SNAPSOURCE_GUIDE:
                source_name = _("Guide");
                break;
            case SNAPSOURCE_GUIDE_ORIGIN:
                source_name = _("Guide origin");
                break;
            case SNAPSOURCE_CONVEX_HULL_CORNER:
                source_name = _("Convex hull corner");
                break;
            case SNAPSOURCE_ELLIPSE_QUADRANT_POINT:
                source_name = _("Quadrant point");
                break;
            case SNAPSOURCE_RECT_CORNER:
            case SNAPSOURCE_IMG_CORNER:
                source_name = _("Corner");
                break;
            case SNAPSOURCE_TEXT_ANCHOR:
                source_name = _("Text anchor");
                break;
            case SNAPSOURCE_GRID_PITCH:
                source_name = _("Multiple of grid spacing");
                break;
            default:
                g_warning("Snap source not in SnapSourceType enum");
                break;
        }
        //std::cout << "Snapped " << source_name << " to " << target_name << std::endl;

        remove_snapsource(); // Don't set both the source and target indicators, as these will overlap

        // Display the snap indicator (i.e. the cross)
        SPCanvasItem * canvasitem = NULL;
        canvasitem = sp_canvas_item_new(sp_desktop_tempgroup (_desktop),
                                        SP_TYPE_CTRL,
                                        "anchor", SP_ANCHOR_CENTER,
                                        "size", 10.0,
                                        "stroked", TRUE,
                                        "stroke_color", pre_snap ? 0x7f7f7fff : 0xff0000ff,
                                        "mode", SP_KNOT_MODE_XOR,
                                        "shape", SP_KNOT_SHAPE_CROSS,
                                        NULL );

        const int timeout_val = 1200; // TODO add preference for snap indicator timeout?

        SP_CTRL(canvasitem)->moveto(p.getPoint());
        _snaptarget = _desktop->add_temporary_canvasitem(canvasitem, timeout_val);
        _snaptarget_is_presnap = pre_snap;

        // Display the tooltip, which reveals the type of snap source and the type of snap target
        gchar *tooltip_str = NULL;
        if ( (p.getSource() != SNAPSOURCE_GRID_PITCH) && (p.getTarget() != SNAPTARGET_UNDEFINED) ) {
            tooltip_str = g_strconcat(source_name, _(" to "), target_name, NULL);
        } else if (p.getSource() != SNAPSOURCE_UNDEFINED) {
            tooltip_str = g_strdup(source_name);
        }
        double fontsize = prefs->getInt("/tools/measure/fontsize");

        if (tooltip_str) {
            Geom::Point tooltip_pos = p.getPoint();
            if (tools_isactive(_desktop, TOOLS_MEASURE)) {
                // Make sure that the snap tooltips do not overlap the ones from the measure tool
                tooltip_pos += _desktop->w2d(Geom::Point(0, -3*fontsize));
            } else {
                tooltip_pos += _desktop->w2d(Geom::Point(0, -2*fontsize));
            }

            SPCanvasItem *canvas_tooltip = sp_canvastext_new(sp_desktop_tempgroup(_desktop), _desktop, tooltip_pos, tooltip_str);
            sp_canvastext_set_fontsize(SP_CANVASTEXT(canvas_tooltip), fontsize);
            SP_CANVASTEXT(canvas_tooltip)->rgba = 0xffffffff;
            SP_CANVASTEXT(canvas_tooltip)->outline = false;
            SP_CANVASTEXT(canvas_tooltip)->background = true;
            if (pre_snap) {
                SP_CANVASTEXT(canvas_tooltip)->rgba_background = 0x33337f40;
            } else {
                SP_CANVASTEXT(canvas_tooltip)->rgba_background = 0x33337f7f;
            }
            SP_CANVASTEXT(canvas_tooltip)->anchor_position = TEXT_ANCHOR_CENTER;
            g_free(tooltip_str);

            _snaptarget_tooltip = _desktop->add_temporary_canvasitem(canvas_tooltip, timeout_val);
        }

        // Display the bounding box, if we snapped to one
        Geom::OptRect const bbox = p.getTargetBBox();
        if (bbox) {
            SPCanvasItem* box = sp_canvas_item_new(sp_desktop_tempgroup (_desktop),
                                                     SP_TYPE_CTRLRECT,
                                                     NULL);

            SP_CTRLRECT(box)->setRectangle(*bbox);
            SP_CTRLRECT(box)->setColor(pre_snap ? 0x7f7f7fff : 0xff0000ff, 0, 0);
            SP_CTRLRECT(box)->setDashed(true);
            sp_canvas_item_move_to_z(box, 0);
            _snaptarget_bbox = _desktop->add_temporary_canvasitem(box, timeout_val);
        }
    }
}

void
SnapIndicator::remove_snaptarget(bool only_if_presnap)
{
    if (only_if_presnap && !_snaptarget_is_presnap) {
        return;
    }

    if (_snaptarget) {
        _desktop->remove_temporary_canvasitem(_snaptarget);
        _snaptarget = NULL;
        _snaptarget_is_presnap = false;
    }

    if (_snaptarget_tooltip) {
        _desktop->remove_temporary_canvasitem(_snaptarget_tooltip);
        _snaptarget_tooltip = NULL;
    }

    if (_snaptarget_bbox) {
        _desktop->remove_temporary_canvasitem(_snaptarget_bbox);
        _snaptarget_bbox = NULL;
    }

}

void
SnapIndicator::set_new_snapsource(Inkscape::SnapCandidatePoint const &p)
{
    remove_snapsource();

    g_assert(_desktop != NULL); // If this fails, then likely setup() has not been called on the snap manager (see snap.cpp -> setup())

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool value = prefs->getBool("/options/snapindicator/value", true);

    if (value) {
        SPCanvasItem * canvasitem = sp_canvas_item_new( sp_desktop_tempgroup (_desktop),
                                                        SP_TYPE_CTRL,
                                                        "anchor", SP_ANCHOR_CENTER,
                                                        "size", 6.0,
                                                        "stroked", TRUE,
                                                        "stroke_color", 0xff0000ff,
                                                        "mode", SP_KNOT_MODE_XOR,
                                                        "shape", SP_KNOT_SHAPE_CIRCLE,
                                                        NULL );

        SP_CTRL(canvasitem)->moveto(p.getPoint());
        _snapsource = _desktop->add_temporary_canvasitem(canvasitem, 1000);
    }
}

void
SnapIndicator::set_new_debugging_point(Geom::Point const &p)
{
    g_assert(_desktop != NULL);
    SPCanvasItem * canvasitem = sp_canvas_item_new( sp_desktop_tempgroup (_desktop),
                                                    SP_TYPE_CTRL,
                                                    "anchor", SP_ANCHOR_CENTER,
                                                    "size", 10.0,
                                                    "fill_color", 0x00ff00ff,
                                                    "stroked", FALSE,
                                                    "mode", SP_KNOT_MODE_XOR,
                                                    "shape", SP_KNOT_SHAPE_DIAMOND,
                                                    NULL );

    SP_CTRL(canvasitem)->moveto(p);
    _debugging_points.push_back(_desktop->add_temporary_canvasitem(canvasitem, 5000));

}

void
SnapIndicator::remove_snapsource()
{
    if (_snapsource) {
        _desktop->remove_temporary_canvasitem(_snapsource);
        _snapsource = NULL;
    }
}

void
SnapIndicator::remove_debugging_points()
{
    for (std::list<TemporaryItem *>::const_iterator i = _debugging_points.begin(); i != _debugging_points.end(); ++i) {
        _desktop->remove_temporary_canvasitem(*i);
    }
    _debugging_points.clear();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
