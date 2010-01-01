/** \file
 * Provides a class that shows a temporary indicator on the canvas of where the snap was, and what kind of snap
 *
 * Authors:
 *   Johan Engelen
 *   Diederik van Lierop
 *
 * Copyright (C) Johan Engelen 2009 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Diederik van Lierop 2009 <mail@diedenrezi.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/snap-indicator.h"

#include "desktop.h"
#include "desktop-handles.h"
#include "display/sodipodi-ctrl.h"
#include "display/canvas-text.h"
#include "knot.h"
#include "preferences.h"
#include <glibmm/i18n.h>

namespace Inkscape {
namespace Display {

SnapIndicator::SnapIndicator(SPDesktop * desktop)
    :   _snaptarget(NULL),
        _snaptarget_tooltip(NULL),
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
    remove_snaptarget(); //only display one snaptarget at a time

    g_assert(_desktop != NULL);

    if (!p.getSnapped()) {
        g_warning("No snapping took place, so no snap target will be displayed");
        return; // If we haven't snapped, then it is of no use to draw a snapindicator
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool value = prefs->getBool("/options/snapindicator/value", true);

    if (value) {
        // TRANSLATORS: undefined target for snapping
        gchar *target_name = _("UNDEFINED");
        switch (p.getTarget()) {
            case SNAPTARGET_UNDEFINED:
                target_name = _("UNDEFINED");
                break;
            case SNAPTARGET_GRID:
                target_name = _("grid line");
                break;
            case SNAPTARGET_GRID_INTERSECTION:
                target_name = _("grid intersection");
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
            case SNAPTARGET_PATH_INTERSECTION:
                target_name = _("path intersection");
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
            case SNAPTARGET_HANDLE:
                target_name = _("handle");
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
            case SNAPTARGET_CONVEX_HULL_CORNER:
                target_name = _("convex hull corner");
                break;
            case SNAPTARGET_ELLIPSE_QUADRANT_POINT:
                target_name = _("quadrant point");
                break;
            case SNAPTARGET_CENTER:
                target_name = _("center");
                break;
            case SNAPTARGET_CORNER:
                target_name = _("corner");
                break;
            case SNAPTARGET_TEXT_BASELINE:
                target_name = _("text baseline");
                break;
            case SNAPTARGET_CONSTRAINED_ANGLE:
                target_name = _("constrained angle");
                break;
            default:
                g_warning("Snap target has not yet been defined!");
                break;
        }

        gchar *source_name = _("UNDEFINED");
        switch (p.getSource()) {
            case SNAPSOURCE_UNDEFINED:
                source_name = _("UNDEFINED");
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
            case SNAPSOURCE_HANDLE:
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
            case SNAPSOURCE_CENTER:
                source_name = _("Center");
                break;
            case SNAPSOURCE_CORNER:
                source_name = _("Corner");
                break;
            case SNAPSOURCE_TEXT_BASELINE:
                source_name = _("Text baseline");
                break;
            default:
                g_warning("Snap source has not yet been defined!");
                break;
        }
        //std::cout << "Snapped " << source_name << " to " << target_name << std::endl;

        remove_snapsource(); // Don't set both the source and target indicators, as these will overlap

        // Display the snap indicator (i.e. the cross)
        SPCanvasItem * canvasitem = NULL;
        if (p.getTarget() == SNAPTARGET_NODE_SMOOTH || p.getTarget() == SNAPTARGET_NODE_CUSP) {
            canvasitem = sp_canvas_item_new(sp_desktop_tempgroup (_desktop),
                                            SP_TYPE_CTRL,
                                            "anchor", GTK_ANCHOR_CENTER,
                                            "size", 10.0,
                                            "stroked", TRUE,
                                            "stroke_color", 0xf000f0ff,
                                            "mode", SP_KNOT_MODE_XOR,
                                            "shape", SP_KNOT_SHAPE_DIAMOND,
                                            NULL );
        } else {
            canvasitem = sp_canvas_item_new(sp_desktop_tempgroup (_desktop),
                                            SP_TYPE_CTRL,
                                            "anchor", GTK_ANCHOR_CENTER,
                                            "size", 10.0,
                                            "stroked", TRUE,
                                            "stroke_color", 0xf000f0ff,
                                            "mode", SP_KNOT_MODE_XOR,
                                            "shape", SP_KNOT_SHAPE_CROSS,
                                            NULL );
        }

        const int timeout_val = 1200; // TODO add preference for snap indicator timeout?

        SP_CTRL(canvasitem)->moveto(p.getPoint());
        _snaptarget = _desktop->add_temporary_canvasitem(canvasitem, timeout_val);

        gchar *tooltip_str = g_strconcat(source_name, _(" to "), target_name, NULL);
        Geom::Point tooltip_pos = p.getPoint() + _desktop->w2d(Geom::Point(15, -15));

        SPCanvasItem *canvas_tooltip = sp_canvastext_new(sp_desktop_tempgroup(_desktop), _desktop, tooltip_pos, tooltip_str);
        g_free(tooltip_str);

        sp_canvastext_set_anchor((SPCanvasText* )canvas_tooltip, -1, 1);
        _snaptarget_tooltip = _desktop->add_temporary_canvasitem(canvas_tooltip, timeout_val);
    }
}

void
SnapIndicator::remove_snaptarget()
{
    if (_snaptarget) {
        _desktop->remove_temporary_canvasitem(_snaptarget);
        _snaptarget = NULL;
    }

    if (_snaptarget_tooltip) {
        _desktop->remove_temporary_canvasitem(_snaptarget_tooltip);
        _snaptarget_tooltip = NULL;
    }

}

void
SnapIndicator::set_new_snapsource(std::pair<Geom::Point, int> const p)
{
    remove_snapsource();

    g_assert(_desktop != NULL);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool value = prefs->getBool("/options/snapindicator/value", true);

    if (value) {
        SPCanvasItem * canvasitem = sp_canvas_item_new( sp_desktop_tempgroup (_desktop),
                                                        SP_TYPE_CTRL,
                                                        "anchor", GTK_ANCHOR_CENTER,
                                                        "size", 6.0,
                                                        "stroked", TRUE,
                                                        "stroke_color", 0xf000f0ff,
                                                        "mode", SP_KNOT_MODE_XOR,
                                                        "shape", SP_KNOT_SHAPE_CIRCLE,
                                                        NULL );

        SP_CTRL(canvasitem)->moveto(p.first);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
