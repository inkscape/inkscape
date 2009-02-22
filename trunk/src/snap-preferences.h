#ifndef SNAPPREFERENCES_H_
#define SNAPPREFERENCES_H_

/**
 *  \file snap-preferences.cpp
 *  \brief Storing of snapping preferences
 *
 * Authors:
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "helper/units.h"

namespace Inkscape
{

class SnapPreferences
{
public:
	SnapPreferences();

	/// Point types to snap.
    typedef int PointType;
    static const PointType SNAPPOINT_NODE;
    static const PointType SNAPPOINT_BBOX;
    static const PointType SNAPPOINT_GUIDE;

    void setSnapModeBBox(bool enabled);
    void setSnapModeNode(bool enabled);
    void setSnapModeGuide(bool enabled);
    bool getSnapModeBBox() const;
    bool getSnapModeNode() const;
    bool getSnapModeBBoxOrNodes() const;
    bool getSnapModeAny() const;
    bool getSnapModeGuide() const;

    void setSnapIntersectionGG(bool enabled) {_intersectionGG = enabled;}
    void setSnapIntersectionCS(bool enabled) {_intersectionCS = enabled;}
    void setSnapSmoothNodes(bool enabled) {_smoothNodes = enabled;}
    void setSnapLineMidpoints(bool enabled) {_line_midpoints = enabled;}
    void setSnapObjectMidpoints(bool enabled) {_object_midpoints = enabled;}
    void setSnapBBoxEdgeMidpoints(bool enabled) {_bbox_edge_midpoints = enabled;}
	void setSnapBBoxMidpoints(bool enabled) {_bbox_midpoints = enabled;}
    bool getSnapIntersectionGG() const {return _intersectionGG;}
    bool getSnapIntersectionCS() const {return _intersectionCS;}
    bool getSnapSmoothNodes() const {return _smoothNodes;}
    bool getSnapLineMidpoints() const {return _line_midpoints;}
    bool getSnapObjectMidpoints() const {return _object_midpoints;}
    bool getSnapBBoxEdgeMidpoints() const {return _bbox_edge_midpoints;}
	bool getSnapBBoxMidpoints() const {return _bbox_midpoints;}

	void setSnapToGrids(bool enabled) {_snap_to_grids = enabled;}
    bool getSnapToGrids() const {return _snap_to_grids;}

    void setSnapToGuides(bool enabled) {_snap_to_guides = enabled;}
	bool getSnapToGuides() const {return _snap_to_guides;}

    void setIncludeItemCenter(bool enabled) {_include_item_center = enabled;}
    bool getIncludeItemCenter() const {return _include_item_center;}

    void setSnapEnabledGlobally(bool enabled) {_snap_enabled_globally = enabled;}
    bool getSnapEnabledGlobally() const {return _snap_enabled_globally;}

    void setSnapPostponedGlobally(bool postponed) {_snap_postponed_globally = postponed;}
    bool getSnapPostponedGlobally() const {return _snap_postponed_globally;}

    void setSnapFrom(PointType t, bool s);
    bool getSnapFrom(PointType t) const;

    // These will only be used for the object snapper
    void setSnapToItemNode(bool s) {_snap_to_itemnode = s;}
	bool getSnapToItemNode() const {return _snap_to_itemnode;}
	void setSnapToItemPath(bool s) {_snap_to_itempath = s;}
	bool getSnapToItemPath() const {return _snap_to_itempath;}
	void setSnapToBBoxNode(bool s) {_snap_to_bboxnode = s;}
	bool getSnapToBBoxNode() const {return _snap_to_bboxnode;}
	void setSnapToBBoxPath(bool s) {_snap_to_bboxpath = s;}
	bool getSnapToBBoxPath() const {return _snap_to_bboxpath;}
	void setSnapToPageBorder(bool s) {_snap_to_page_border = s;}
	bool getSnapToPageBorder() const {return _snap_to_page_border;}
	bool getStrictSnapping() const {return _strict_snapping;}

	gdouble getGridTolerance() const {return _grid_tolerance;}
	gdouble getGuideTolerance() const {return _guide_tolerance;}
	gdouble getObjectTolerance() const {return _object_tolerance;}

	void setGridTolerance(gdouble val) {_grid_tolerance = val;}
	void setGuideTolerance(gdouble val) {_guide_tolerance = val;}
	void setObjectTolerance(gdouble val) {_object_tolerance = val;}


private:
    bool _include_item_center; //If true, snapping nodes will also snap the item's center
    bool _intersectionGG; //Consider snapping to intersections of grid and guides
    bool _intersectionCS; //Consider snapping to intersections of curves
    bool _smoothNodes;
    bool _line_midpoints;
    bool _object_midpoints; // the midpoint of shapes (e.g. a circle, rect, polygon) or of any other shape (at [h/2, w/2])
    bool _bbox_edge_midpoints;
	bool _bbox_midpoints;
	bool _snap_to_grids;
	bool _snap_to_guides;
    bool _snap_enabled_globally; // Toggles ALL snapping
    bool _snap_postponed_globally; // Hold all snapping temporarily when the mouse is moving fast
    PointType _snap_from; ///< bitmap of point types that we will snap from

    // These will only be used for the object snapper
    bool _snap_to_itemnode;
	bool _snap_to_itempath;
	bool _snap_to_bboxnode;
	bool _snap_to_bboxpath;
	bool _snap_to_page_border;
	//If enabled, then bbox corners will only snap to bboxes,
	//and nodes will only snap to nodes and paths. We will not
	//snap bbox corners to nodes, or nodes to bboxes.
	//(snapping to grids and guides is not affected by this)
	bool _strict_snapping;

	gdouble _grid_tolerance;
	gdouble _guide_tolerance;
	gdouble _object_tolerance;
};

}
#endif /*SNAPPREFERENCES_H_*/

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
