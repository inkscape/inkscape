#ifndef SNAPPREFERENCES_H_
#define SNAPPREFERENCES_H_

/*
 * Authors:
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2008 - 2011 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "snap-enums.h"

namespace Inkscape
{

/**
 * Storing of snapping preferences.
 */
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

    SnapTargetType source2target(SnapSourceType source) const;
    bool isSourceSnappable(Inkscape::SnapSourceType const source) const;

    bool isAnyDatumSnappable() const; // Needed because we cannot toggle the datum snap targets as a group
    bool isAnyCategorySnappable() const;

    void setSnapEnabledGlobally(bool enabled) {_snap_enabled_globally = enabled;}
    bool getSnapEnabledGlobally() const {return _snap_enabled_globally;}

    void setSnapPostponedGlobally(bool postponed) {_snap_postponed_globally = postponed;}
    bool getSnapPostponedGlobally() const {return _snap_postponed_globally;}

    bool getStrictSnapping() const {return _strict_snapping;}

    bool getSnapPerp() const {return _snap_perp;}
    bool getSnapTang() const {return _snap_tang;}
    void setSnapPerp(bool enabled) {_snap_perp = enabled;}
    void setSnapTang(bool enabled) {_snap_tang = enabled;}

    double getGridTolerance() const {return _grid_tolerance;}
    double getGuideTolerance() const {return _guide_tolerance;}
    double getObjectTolerance() const {return _object_tolerance;}

    void setGridTolerance(double val) {_grid_tolerance = val;}
    void setGuideTolerance(double val) {_guide_tolerance = val;}
    void setObjectTolerance(double val) {_object_tolerance = val;}

private:

    /**
     * Map snap target to array index.
     *
     * The status of each snap toggle (in the snap toolbar) is stored as a boolean value in an array. This method returns the position
     * of relevant boolean in that array, for any given type of snap target. For most snap targets, the enumerated value of that targets
     * matches the position in the array (primary snap targets). This however does not hold for snap targets which don't have their own
     * toggle button (secondary snap targets).
     *
     * PS:
     * - For snap sources, just pass the corresponding snap target instead (each snap source should have a twin snap target, but not vice versa)
     * - All parameters are passed by reference, and will be overwritten
     *
     * @param target Stores the enumerated snap target,  which can be modified to correspond to the array index of this snap target.
     * @param always_on If true, then this snap target is always active and cannot be toggled.
     * @param group_on If true, then this snap target is in a snap group that has been enabled (e.g. bbox group, nodes/paths group, or "others" group.
     */
    void _mapTargetToArrayIndex(Inkscape::SnapTargetType &target, bool &always_on, bool &group_on) const;

    int _active_snap_targets[Inkscape::SNAPTARGET_MAX_ENUM_VALUE];

    bool _snap_enabled_globally; // Toggles ALL snapping
    bool _snap_postponed_globally; // Hold all snapping temporarily when the mouse is moving fast

    //If enabled, then bbox corners will only snap to bboxes,
    //and nodes will only snap to nodes and paths. We will not
    //snap bbox corners to nodes, or nodes to bboxes.
    //(snapping to grids and guides is not affected by this)
    bool _strict_snapping;

    bool _snap_perp;
    bool _snap_tang;

    double _grid_tolerance;
    double _guide_tolerance;
    double _object_tolerance;
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
