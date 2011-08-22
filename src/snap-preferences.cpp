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

#include "inkscape.h"
#include "snap-preferences.h"
#include <glib.h> // g_assert()

Inkscape::SnapPreferences::SnapPreferences() :
    _snap_enabled_globally(true),
    _snap_postponed_globally(false),
    _strict_snapping(true)
{
    // Check for enough space to hold all snap target toggles in the "others" group; see the comments in snap-enums.h
    g_assert(SNAPTARGET_MAX_ENUM_VALUE - SNAPTARGET_OTHERS_CATEGORY < SNAPTARGET_BBOX_CATEGORY);
    // Check for powers of two; see the comments in snap-enums.h
    g_assert((SNAPTARGET_BBOX_CATEGORY != 0) && !(SNAPTARGET_BBOX_CATEGORY & (SNAPTARGET_BBOX_CATEGORY - 1)));
    g_assert((SNAPTARGET_NODE_CATEGORY != 0) && !(SNAPTARGET_NODE_CATEGORY & (SNAPTARGET_NODE_CATEGORY - 1)));
    g_assert((SNAPTARGET_OTHERS_CATEGORY != 0) && !(SNAPTARGET_OTHERS_CATEGORY & (SNAPTARGET_OTHERS_CATEGORY - 1)));

    setSnapFrom(SnapSourceType(SNAPSOURCE_BBOX_CATEGORY | SNAPSOURCE_NODE_CATEGORY | SNAPSOURCE_OTHERS_CATEGORY), true); //Snap any point. In v0.45 and earlier, this was controlled in the preferences tab
    for (int n = 0; n < Inkscape::SNAPTARGET_MAX_ENUM_VALUE; n++) {
        _active_snap_targets[n] = -1;
    }
}

/*
 *  The snappers have too many parameters to adjust individually. Therefore only
 *  three snapping modes are presented to the user: snapping bounding box corners (to
 *  other bounding boxes, grids or guides), and/or snapping nodes (to other nodes,
 *  paths, grids or guides), and or snapping to/from others (e.g. grids, guide, text, etc)
 *  To select either of these three modes (or all), use the
 *  methods defined below: setSnapModeBBox(), setSnapModeNode(), or setSnapModeOthers()
 *
 * */


void Inkscape::SnapPreferences::setSnapModeBBox(bool enabled)
{
    if (enabled) {
        _snap_from = SnapSourceType(_snap_from | Inkscape::SNAPSOURCE_BBOX_CATEGORY);
    } else {
        _snap_from = SnapSourceType(_snap_from & ~Inkscape::SNAPSOURCE_BBOX_CATEGORY);
    }
    setTargetSnappable(SNAPTARGET_BBOX_CATEGORY, enabled);
}

bool Inkscape::SnapPreferences::getSnapModeBBox() const
{
    return (_snap_from & Inkscape::SNAPSOURCE_BBOX_CATEGORY);
}

void Inkscape::SnapPreferences::setSnapModeNode(bool enabled)
{
    if (enabled) {
        _snap_from = SnapSourceType(_snap_from | Inkscape::SNAPSOURCE_NODE_CATEGORY);
    } else {
        _snap_from = SnapSourceType(_snap_from & ~Inkscape::SNAPSOURCE_NODE_CATEGORY);
    }
    setTargetSnappable(SNAPTARGET_NODE_CATEGORY, enabled);
}

bool Inkscape::SnapPreferences::getSnapModeNode() const
{
    return (_snap_from & Inkscape::SNAPSOURCE_NODE_CATEGORY);
}

void Inkscape::SnapPreferences::setSnapModeOthers(bool enabled)
{
    if (enabled) {
        _snap_from = SnapSourceType(_snap_from | Inkscape::SNAPSOURCE_OTHERS_CATEGORY);
    } else {
        _snap_from = SnapSourceType(_snap_from & ~Inkscape::SNAPSOURCE_OTHERS_CATEGORY);
    }
    setTargetSnappable(SNAPTARGET_OTHERS_CATEGORY, enabled);
}

bool Inkscape::SnapPreferences::getSnapModeOthers() const
{
    return (_snap_from & Inkscape::SNAPSOURCE_OTHERS_CATEGORY);
}

bool Inkscape::SnapPreferences::getSnapModeAny() const
{
    return (_snap_from != 0);
}

/**
 *  Turn on/off snapping of specific point types.
 *  \param t Point type.
 *  \param s true to snap to this point type, otherwise false;
 */
void Inkscape::SnapPreferences::setSnapFrom(Inkscape::SnapSourceType t, bool s)
{
    if (s) {
        _snap_from = SnapSourceType(_snap_from | t);
    } else {
        _snap_from = SnapSourceType(_snap_from & ~t);
    }
}

/**
 *  \param t Point type.
 *  \return true if snapper will snap this type of point, otherwise false.
 */
bool Inkscape::SnapPreferences::getSnapFrom(Inkscape::SnapSourceType t) const
{
    return (_snap_from & t);
}

void Inkscape::SnapPreferences::_mapTargetToArrayIndex(Inkscape::SnapTargetType &target, bool &always_on, bool &group_on) const
{
    if (target & SNAPTARGET_BBOX_CATEGORY) {
        group_on = getSnapModeBBox(); // Only if the group with bbox sources/targets has been enabled, then we might snap to any of the bbox targets
    } else if (target & SNAPTARGET_NODE_CATEGORY) {
        group_on = getSnapModeNode(); // Only if the group with path/node sources/targets has been enabled, then we might snap to any of the nodes/paths
        if (target == SNAPTARGET_RECT_CORNER || target == SNAPTARGET_ELLIPSE_QUADRANT_POINT) { // Don't have their own button; on when the group is on
            target = SNAPTARGET_NODE_CATEGORY;
        }
    } else if (target & SNAPTARGET_OTHERS_CATEGORY) {
        // Only if the group with "other" snap sources/targets has been enabled, then we might snap to any of those targets
        // ... but this doesn't hold for the page border, grids, and guides
        group_on = getSnapModeOthers();
        switch (target) {
            // Some snap targets don't have their own toggle. These targets are called "secondary targets". We will re-map
            // them to their cousin which does have a toggle, and which is called a "primary target"
            case SNAPTARGET_GRID_INTERSECTION:
                group_on = true; // cannot be disabled as part of a disabled group;
                target = SNAPTARGET_GRID;
                break;
            case SNAPTARGET_GUIDE_INTERSECTION:
            case SNAPTARGET_GUIDE_ORIGIN:
                group_on = true; // cannot be disabled as part of a disabled group;
                target = SNAPTARGET_GUIDE;
                break;
            case SNAPTARGET_PAGE_CORNER:
                group_on = true; // cannot be disabled as part of a disabled group;
                target = SNAPTARGET_PAGE_BORDER;
                break;
            case SNAPTARGET_TEXT_ANCHOR:
                target = SNAPTARGET_TEXT_BASELINE;
                break;

            case SNAPTARGET_IMG_CORNER: // Doesn't have its own button, on if the group is on
                target = SNAPTARGET_OTHERS_CATEGORY;
                break;
            // Some snap targets cannot be toggled at all, and are therefore always enabled
            case SNAPTARGET_GRID_GUIDE_INTERSECTION:
            case SNAPTARGET_CONSTRAINED_ANGLE:
            case SNAPTARGET_CONSTRAINT:
                always_on = true; // Doesn't have it's own button
                break;
            case SNAPTARGET_GRID:
            case SNAPTARGET_GUIDE:
            case SNAPTARGET_PAGE_BORDER:
                group_on = true; // cannot be disabled as part of a disabled group;
                break;

            // These are only listed for completeness
            case SNAPTARGET_OBJECT_MIDPOINT:
            case SNAPTARGET_ROTATION_CENTER:
            case SNAPTARGET_TEXT_BASELINE:
                break;

            case SNAPTARGET_BBOX_CATEGORY:
            case SNAPTARGET_NODE_CATEGORY:
            case SNAPTARGET_OTHERS_CATEGORY:
                break;
            default:
                g_warning("Snap-preferences warning: Undefined snap target (#%i)", target);
                break;
        }
    } else if (target == SNAPTARGET_UNDEFINED ) {
        g_warning("Snap-preferences warning: Undefined snaptarget (#%i)", target);
    }
}

void Inkscape::SnapPreferences::setTargetSnappable(Inkscape::SnapTargetType const target, bool enabled)
{
    bool always_on = false;
    bool group_on = false;
    Inkscape::SnapTargetType index = target;

    _mapTargetToArrayIndex(index, always_on, group_on);

    if (always_on) {
        // Catch coding errors
        g_warning("Snap-preferences warning: Trying to enable/disable a snap target (#%i) that's always on by definition", index);
    } else {
        if (index == target) { // I.e. if it has not been re-mapped, then we have a primary target at hand
            _active_snap_targets[index] = enabled;
        } else { // If it has been re-mapped though, then this target does not have its own toggle button and should therefore not be set
            g_warning("Snap-preferences warning: Trying to enable/disable a secondary snap target (#%i); only primary targets can be set", index);
        }
    }
}

bool Inkscape::SnapPreferences::isTargetSnappable(Inkscape::SnapTargetType const target) const
{
    bool always_on = false;
    bool group_on = false;
    Inkscape::SnapTargetType index = target;

    _mapTargetToArrayIndex(index, always_on, group_on);

    if (group_on) {
        if (always_on) {
            return true;
        } else {
            if (_active_snap_targets[index] == -1) {
                // Catch coding errors
                g_warning("Snap-preferences warning: Using an uninitialized snap target setting (#%i)", index);
            }
            return _active_snap_targets[index];
        }
    } else {
        return false;
    }
}

bool Inkscape::SnapPreferences::isTargetSnappable(Inkscape::SnapTargetType const target1, Inkscape::SnapTargetType const target2) const {
    return isTargetSnappable(target1) || isTargetSnappable(target2);
}

bool Inkscape::SnapPreferences::isTargetSnappable(Inkscape::SnapTargetType const target1, Inkscape::SnapTargetType const target2, Inkscape::SnapTargetType const target3) const {
    return isTargetSnappable(target1) || isTargetSnappable(target2) || isTargetSnappable(target3);
}

bool Inkscape::SnapPreferences::isTargetSnappable(Inkscape::SnapTargetType const target1, Inkscape::SnapTargetType const target2, Inkscape::SnapTargetType const target3, Inkscape::SnapTargetType const target4) const {
    return isTargetSnappable(target1) || isTargetSnappable(target2) || isTargetSnappable(target3) || isTargetSnappable(target4);
}


//bool Inkscape::SnapPreferences::isAnyBBoxSnappable() const {
//    return getSnapModeBBox() || getSnapModeOthers() || (getSnapModeNode() && !_strict_snapping);
//}

//bool Inkscape::SnapPreferences::isAnyNodeOrPathSnappable() const {
//    return getSnapModeNode() || getSnapModeOthers() || (getSnapModeBBox() && !_strict_snapping);
//}

//bool Inkscape::SnapPreferences::isAnyOtherSnappable() const {
//    return getSnapModeOthers();
//}

bool Inkscape::SnapPreferences::isSnapButtonEnabled(Inkscape::SnapTargetType const target) const
{
    bool always_on = false;
    bool group_on = false;
    Inkscape::SnapTargetType index = target;

    _mapTargetToArrayIndex(index, always_on, group_on);

    if (_active_snap_targets[index] == -1) {
        // Catch coding errors
        g_warning("Snap-preferences warning: Using an uninitialized snap target setting");
    } else {
        if (index == target) { // I.e. if it has not been re-mapped, then we have a primary target at hand, which does have its own toggle button
            return _active_snap_targets[index];
        } else { // If it has been re-mapped though, then this target does not have its own toggle button and therefore the button status cannot be read
            g_warning("Snap-preferences warning: Trying to determine the button status of a secondary snap target; However, only primary targets have a button");
        }
    }

    return false;
}


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
