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
    void setSnapMidpoints(bool enabled) {_midpoints = enabled;}
    bool getSnapIntersectionGG() const {return _intersectionGG;}
    bool getSnapIntersectionCS() const {return _intersectionCS;}
    bool getSnapSmoothNodes() const {return _smoothNodes;}
    bool getSnapMidpoints() const {return _midpoints;}

    void setIncludeItemCenter(bool enabled) {_include_item_center = enabled;}
    bool getIncludeItemCenter() const {return _include_item_center;}

    void setSnapEnabledGlobally(bool enabled) {_snap_enabled_globally = enabled;}
    bool getSnapEnabledGlobally() const {return _snap_enabled_globally;}

    void setSnapPostponedGlobally(bool postponed) {_snap_postponed_globally = postponed;}
    bool getSnapPostponedGlobally() const {return _snap_postponed_globally;}

    void setSnapFrom(PointType t, bool s);
    bool getSnapFrom(PointType t) const;

private:
    bool _include_item_center; //If true, snapping nodes will also snap the item's center
    bool _intersectionGG; //Consider snapping to intersections of grid and guides
    bool _intersectionCS; //Consider snapping to intersections of curves
    bool _smoothNodes;
    bool _midpoints;
    bool _snap_enabled_globally; // Toggles ALL snapping
    bool _snap_postponed_globally; // Hold all snapping temporarily when the mouse is moving fast
    PointType _snap_from; ///< bitmap of point types that we will snap from

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
