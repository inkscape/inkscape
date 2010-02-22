#ifndef SNAPENUMS_H_
#define SNAPENUMS_H_

/**
 * \file snap-enums.h
 * \brief enumerations of snap source types and snap target types
 *
 * Authors:
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

namespace Inkscape {

enum SnapTargetType {
    SNAPTARGET_UNDEFINED = 0,
    SNAPTARGET_GRID,
    SNAPTARGET_GRID_INTERSECTION,
    SNAPTARGET_GUIDE,
    SNAPTARGET_GUIDE_INTERSECTION,
    SNAPTARGET_GUIDE_ORIGIN,
    SNAPTARGET_GRID_GUIDE_INTERSECTION,
    SNAPTARGET_NODE_SMOOTH,
    SNAPTARGET_NODE_CUSP,
    SNAPTARGET_LINE_MIDPOINT,
    SNAPTARGET_OBJECT_MIDPOINT,
    SNAPTARGET_ROTATION_CENTER,
    SNAPTARGET_HANDLE,
    SNAPTARGET_PATH,
    SNAPTARGET_PATH_INTERSECTION,
    SNAPTARGET_BBOX_CORNER,
    SNAPTARGET_BBOX_EDGE,
    SNAPTARGET_BBOX_EDGE_MIDPOINT,
    SNAPTARGET_BBOX_MIDPOINT,
    SNAPTARGET_PAGE_BORDER,
    SNAPTARGET_PAGE_CORNER,
    SNAPTARGET_CONVEX_HULL_CORNER,
    SNAPTARGET_ELLIPSE_QUADRANT_POINT,
    SNAPTARGET_CENTER, // of ellipse
    SNAPTARGET_CORNER, // of image or of rectangle
    SNAPTARGET_TEXT_BASELINE,
    SNAPTARGET_CONSTRAINED_ANGLE,
    SNAPTARGET_CONSTRAINT
};

enum SnapSourceType {
    SNAPSOURCE_UNDEFINED = 0,
    //-------------------------------------------------------------------
    // Bbox points can be located at the edge of the stroke (for visual bboxes); they will therefore not snap
    // to nodes because these are always located at the center of the stroke
    SNAPSOURCE_BBOX_CATEGORY = 256, // will be used as a flag and must therefore be a power of two
    SNAPSOURCE_BBOX_CORNER,
    SNAPSOURCE_BBOX_MIDPOINT,
    SNAPSOURCE_BBOX_EDGE_MIDPOINT,
    //-------------------------------------------------------------------
    // For the same reason, nodes will not snap to bbox points
    SNAPSOURCE_NODE_CATEGORY = 512, // will be used as a flag and must therefore be a power of two
    SNAPSOURCE_NODE_SMOOTH,
    SNAPSOURCE_NODE_CUSP,
    SNAPSOURCE_LINE_MIDPOINT,
    SNAPSOURCE_PATH_INTERSECTION,
    SNAPSOURCE_CORNER, // of image or of rectangle
    SNAPSOURCE_CONVEX_HULL_CORNER,
    SNAPSOURCE_ELLIPSE_QUADRANT_POINT,
    SNAPSOURCE_NODE_HANDLE, // eg. nodes in the path editor, handles of stars or rectangles, etc. (tied to a stroke)
    //-------------------------------------------------------------------
    // Other points (e.g. guides, gradient knots) will snap to both bounding boxes and nodes
    SNAPSOURCE_OTHER_CATEGORY = 1024, // will be used as a flag and must therefore be a power of two
    SNAPSOURCE_OBJECT_MIDPOINT,
    SNAPSOURCE_ROTATION_CENTER,
    SNAPSOURCE_CENTER, // of ellipse
    SNAPSOURCE_GUIDE,
    SNAPSOURCE_GUIDE_ORIGIN,
    SNAPSOURCE_TEXT_BASELINE,
    SNAPSOURCE_OTHER_HANDLE // eg. the handle of a gradient of a connector (ie not being tied to a stroke)
};

}
#endif /* SNAPENUMS_H_ */
