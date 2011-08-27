#ifndef SNAPPREFERENCES_H_
#define SNAPPREFERENCES_H_

/**
 *  \file snap-preferences.cpp
 *  \brief Storing of snapping preferences
 *
 * Authors:
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2008 - 2011 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "helper/units.h"
#include "snap-enums.h"

namespace Inkscape
{

class SnapPreferences
{
public:
    SnapPreferences();
    void setTargetSnappable(Inkscape::SnapTargetType const target, bool enabled);
    bool isTargetSnappable(Inkscape::SnapTargetType const target) const;
    bool isTargetSnappable(Inkscape::SnapTargetType const target1, Inkscape::SnapTargetType const target2) const;
    bool isTargetSnappable(Inkscape::SnapTargetType const target1, Inkscape::SnapTargetType const target2, Inkscape::SnapTargetType const target3) const;
    bool isTargetSnappable(Inkscape::SnapTargetType const target1, Inkscape::SnapTargetType const target2, Inkscape::SnapTargetType const target3, Inkscape::SnapTargetType const target4) const;
    bool isTargetSnappable(Inkscape::SnapTargetType const target1, Inkscape::SnapTargetType const target2, Inkscape::SnapTargetType const target3, Inkscape::SnapTargetType const target4, Inkscape::SnapTargetType const target5) const;
    bool isSnapButtonEnabled(Inkscape::SnapTargetType const target) const;

    void setSnapModeBBox(bool enabled);
    void setSnapModeNode(bool enabled);
    void setSnapModeOthers(bool enabled);
    bool getSnapModeBBox() const;
    bool getSnapModeNode() const;
    bool getSnapModeDatums() const;
    bool getSnapModeOthers() const;
    bool getSnapModeAny() const;

    void setSnapEnabledGlobally(bool enabled) {_snap_enabled_globally = enabled;}
    bool getSnapEnabledGlobally() const {return _snap_enabled_globally;}

    void setSnapPostponedGlobally(bool postponed) {_snap_postponed_globally = postponed;}
    bool getSnapPostponedGlobally() const {return _snap_postponed_globally;}

    void setSnapFrom(Inkscape::SnapSourceType t, bool s);
    bool getSnapFrom(Inkscape::SnapSourceType t) const;

    bool getStrictSnapping() const {return _strict_snapping;}

    gdouble getGridTolerance() const {return _grid_tolerance;}
    gdouble getGuideTolerance() const {return _guide_tolerance;}
    gdouble getObjectTolerance() const {return _object_tolerance;}

    void setGridTolerance(gdouble val) {_grid_tolerance = val;}
    void setGuideTolerance(gdouble val) {_guide_tolerance = val;}
    void setObjectTolerance(gdouble val) {_object_tolerance = val;}

private:
    void _mapTargetToArrayIndex(Inkscape::SnapTargetType &target, bool &always_on, bool &group_on) const;
    int _active_snap_targets[Inkscape::SNAPTARGET_MAX_ENUM_VALUE];

    bool _snap_enabled_globally; // Toggles ALL snapping
    bool _snap_postponed_globally; // Hold all snapping temporarily when the mouse is moving fast

    SnapSourceType _snap_from; ///< bitmap of point types that we will snap from

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
