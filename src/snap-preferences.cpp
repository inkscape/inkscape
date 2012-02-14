/*
 * Storing of snapping preferences.
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
    _strict_snapping(true),
    _snap_perp(false),
    _snap_tang(false)
{
    // Check for powers of two; see the comments in snap-enums.h
    g_assert((SNAPTARGET_BBOX_CATEGORY != 0) && !(SNAPTARGET_BBOX_CATEGORY & (SNAPTARGET_BBOX_CATEGORY - 1)));
    g_assert((SNAPTARGET_NODE_CATEGORY != 0) && !(SNAPTARGET_NODE_CATEGORY & (SNAPTARGET_NODE_CATEGORY - 1)));
    g_assert((SNAPTARGET_DATUMS_CATEGORY != 0) && !(SNAPTARGET_DATUMS_CATEGORY & (SNAPTARGET_DATUMS_CATEGORY - 1)));
    g_assert((SNAPTARGET_OTHERS_CATEGORY != 0) && !(SNAPTARGET_OTHERS_CATEGORY & (SNAPTARGET_OTHERS_CATEGORY - 1)));

    for (int n = 0; n < SNAPTARGET_MAX_ENUM_VALUE; n++) {
        _active_snap_targets[n] = -1;
    }
}

bool Inkscape::SnapPreferences::isAnyDatumSnappable() const
{
    return isTargetSnappable(SNAPTARGET_GUIDE, SNAPTARGET_GRID, SNAPTARGET_PAGE_BORDER);
}

bool Inkscape::SnapPreferences::isAnyCategorySnappable() const
{
    return isTargetSnappable(SNAPTARGET_NODE_CATEGORY, SNAPTARGET_BBOX_CATEGORY, SNAPTARGET_OTHERS_CATEGORY) || isTargetSnappable(SNAPTARGET_GUIDE, SNAPTARGET_GRID, SNAPTARGET_PAGE_BORDER);
}

void Inkscape::SnapPreferences::_mapTargetToArrayIndex(Inkscape::SnapTargetType &target, bool &always_on, bool &group_on) const
{
    if (target == SNAPTARGET_BBOX_CATEGORY ||
            target == SNAPTARGET_NODE_CATEGORY ||
            target == SNAPTARGET_OTHERS_CATEGORY ||
            target == SNAPTARGET_DATUMS_CATEGORY) {
        // These main targets should be handled separately, because otherwise we might call isTargetSnappable()
        // for them (to check whether the corresponding group is on) which would lead to an infinite recursive loop
        always_on = (target == SNAPTARGET_DATUMS_CATEGORY);
        group_on = true;
        return;
    }

    if (target & SNAPTARGET_BBOX_CATEGORY) {
        group_on = isTargetSnappable(SNAPTARGET_BBOX_CATEGORY); // Only if the group with bbox sources/targets has been enabled, then we might snap to any of the bbox targets
        return;
    }

    if (target & SNAPTARGET_NODE_CATEGORY) {
        group_on = isTargetSnappable(SNAPTARGET_NODE_CATEGORY); // Only if the group with path/node sources/targets has been enabled, then we might snap to any of the nodes/paths
        switch (target) {
            case SNAPTARGET_RECT_CORNER:
                target = SNAPTARGET_NODE_CUSP;
                break;
            case SNAPTARGET_ELLIPSE_QUADRANT_POINT:
                target = SNAPTARGET_NODE_SMOOTH;
                break;
            case SNAPTARGET_PATH_GUIDE_INTERSECTION:
                target = SNAPTARGET_PATH_INTERSECTION;
                break;
            case SNAPTARGET_PATH_PERPENDICULAR:
            case SNAPTARGET_PATH_TANGENTIAL:
                target = SNAPTARGET_PATH;
                break;
            default:
                break;
        }

        return;
    }

    if (target & SNAPTARGET_DATUMS_CATEGORY) {
        group_on = true; // These snap targets cannot be disabled as part of a disabled group;
        switch (target) {
            // Some snap targets don't have their own toggle. These targets are called "secondary targets". We will re-map
            // them to their cousin which does have a toggle, and which is called a "primary target"
            case SNAPTARGET_GRID_INTERSECTION:
            case SNAPTARGET_GRID_PERPENDICULAR:
                target = SNAPTARGET_GRID;
                break;
            case SNAPTARGET_GUIDE_INTERSECTION:
            case SNAPTARGET_GUIDE_ORIGIN:
            case SNAPTARGET_GUIDE_PERPENDICULAR:
                target = SNAPTARGET_GUIDE;
                break;
            case SNAPTARGET_PAGE_CORNER:
                target = SNAPTARGET_PAGE_BORDER;
                break;

            // Some snap targets cannot be toggled at all, and are therefore always enabled
            case SNAPTARGET_GRID_GUIDE_INTERSECTION:
                always_on = true; // Doesn't have it's own button
                break;

            // These are only listed for completeness
            case SNAPTARGET_GRID:
            case SNAPTARGET_GUIDE:
            case SNAPTARGET_PAGE_BORDER:
            case SNAPTARGET_DATUMS_CATEGORY:
                break;
            default:
                g_warning("Snap-preferences warning: Undefined snap target (#%i)", target);
                break;
        }
        return;
    }

    if (target & SNAPTARGET_OTHERS_CATEGORY) {
        // Only if the group with "other" snap sources/targets has been enabled, then we might snap to any of those targets
        // ... but this doesn't hold for the page border, grids, and guides
        group_on = isTargetSnappable(SNAPTARGET_OTHERS_CATEGORY);
        switch (target) {
            // Some snap targets don't have their own toggle. These targets are called "secondary targets". We will re-map
            // them to their cousin which does have a toggle, and which is called a "primary target"
            case SNAPTARGET_TEXT_ANCHOR:
                target = SNAPTARGET_TEXT_BASELINE;
                break;

            case SNAPTARGET_IMG_CORNER: // Doesn't have its own button, on if the group is on
                target = SNAPTARGET_OTHERS_CATEGORY;
                break;
            // Some snap targets cannot be toggled at all, and are therefore always enabled
            case SNAPTARGET_CONSTRAINED_ANGLE:
            case SNAPTARGET_CONSTRAINT:
                always_on = true; // Doesn't have it's own button
                break;

            // These are only listed for completeness
            case SNAPTARGET_OBJECT_MIDPOINT:
            case SNAPTARGET_ROTATION_CENTER:
            case SNAPTARGET_TEXT_BASELINE:
            case SNAPTARGET_OTHERS_CATEGORY:
                break;
            default:
                g_warning("Snap-preferences warning: Undefined snap target (#%i)", target);
                break;
        }

        return;
    }

    if (target == SNAPTARGET_UNDEFINED ) {
        g_warning("Snap-preferences warning: Undefined snaptarget (#%i)", target);
    } else {
        g_warning("Snap-preferences warning: Snaptarget not handled (#%i)", target);
    }
}

void Inkscape::SnapPreferences::setTargetSnappable(Inkscape::SnapTargetType const target, bool enabled)
{
    bool always_on = false;
    bool group_on = false; // Only needed as a dummy
    Inkscape::SnapTargetType index = target;

    _mapTargetToArrayIndex(index, always_on, group_on);

    if (always_on) { // If true, then this snap target is always active and cannot be toggled
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

    if (group_on) { // If true, then this snap target is in a snap group that has been enabled (e.g. bbox group, nodes/paths group, or "others" group
        if (always_on) { // If true, then this snap target is always active and cannot be toggled
            return true;
        } else {
            if (_active_snap_targets[index] == -1) {
                // Catch coding errors
                g_warning("Snap-preferences warning: Using an uninitialized snap target setting (#%i)", index);
                // This happens if setTargetSnappable() has not been called for this parameter, e.g. from within sp_namedview_set,
                // or if this target index doesn't exist at all
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

bool Inkscape::SnapPreferences::isTargetSnappable(Inkscape::SnapTargetType const target1, Inkscape::SnapTargetType const target2, Inkscape::SnapTargetType const target3, Inkscape::SnapTargetType const target4, Inkscape::SnapTargetType const target5) const {
    return isTargetSnappable(target1) || isTargetSnappable(target2) || isTargetSnappable(target3) || isTargetSnappable(target4) || isTargetSnappable(target5);
}

bool Inkscape::SnapPreferences::isSnapButtonEnabled(Inkscape::SnapTargetType const target) const
{
    bool always_on = false; // Only needed as a dummy
    bool group_on = false; // Only needed as a dummy
    Inkscape::SnapTargetType index = target;

    _mapTargetToArrayIndex(index, always_on, group_on);

    if (_active_snap_targets[index] == -1) {
        // Catch coding errors
        g_warning("Snap-preferences warning: Using an uninitialized snap target setting (#%i)", index);
        // This happens if setTargetSnappable() has not been called for this parameter, e.g. from within sp_namedview_set,
        // or if this target index doesn't exist at all
    } else {
        if (index == target) { // I.e. if it has not been re-mapped, then we have a primary target at hand, which does have its own toggle button
            return _active_snap_targets[index];
        } else { // If it has been re-mapped though, then this target does not have its own toggle button and therefore the button status cannot be read
            g_warning("Snap-preferences warning: Trying to determine the button status of a secondary snap target (#%i); However, only primary targets have a button", index);
        }
    }

    return false;
}

Inkscape::SnapTargetType Inkscape::SnapPreferences::source2target(Inkscape::SnapSourceType source) const
{
    switch (source)
    {
        case SNAPSOURCE_UNDEFINED:
            return SNAPTARGET_UNDEFINED;
        case SNAPSOURCE_BBOX_CATEGORY:
            return SNAPTARGET_BBOX_CATEGORY;
        case SNAPSOURCE_BBOX_CORNER:
            return SNAPTARGET_BBOX_CORNER;
        case SNAPSOURCE_BBOX_MIDPOINT:
            return SNAPTARGET_BBOX_MIDPOINT;
        case SNAPSOURCE_BBOX_EDGE_MIDPOINT:
            return SNAPTARGET_BBOX_EDGE_MIDPOINT;
        case SNAPSOURCE_NODE_CATEGORY:
            return SNAPTARGET_NODE_CATEGORY;
        case SNAPSOURCE_NODE_SMOOTH:
            return SNAPTARGET_NODE_SMOOTH;
        case SNAPSOURCE_NODE_CUSP:
            return SNAPTARGET_NODE_CUSP;
        case SNAPSOURCE_LINE_MIDPOINT:
            return SNAPTARGET_LINE_MIDPOINT;
        case SNAPSOURCE_PATH_INTERSECTION:
            return SNAPTARGET_PATH_INTERSECTION;
        case SNAPSOURCE_RECT_CORNER:
            return SNAPTARGET_RECT_CORNER;
        case SNAPSOURCE_ELLIPSE_QUADRANT_POINT:
            return SNAPTARGET_ELLIPSE_QUADRANT_POINT;
        case SNAPSOURCE_DATUMS_CATEGORY:
            return SNAPTARGET_DATUMS_CATEGORY;
        case SNAPSOURCE_GUIDE:
            return SNAPTARGET_GUIDE;
        case SNAPSOURCE_GUIDE_ORIGIN:
            return SNAPTARGET_GUIDE_ORIGIN;
        case SNAPSOURCE_OTHERS_CATEGORY:
            return SNAPTARGET_OTHERS_CATEGORY;
        case SNAPSOURCE_ROTATION_CENTER:
            return SNAPTARGET_ROTATION_CENTER;
        case SNAPSOURCE_OBJECT_MIDPOINT:
            return SNAPTARGET_OBJECT_MIDPOINT;
        case SNAPSOURCE_IMG_CORNER:
            return SNAPTARGET_IMG_CORNER;
        case SNAPSOURCE_TEXT_ANCHOR:
            return SNAPTARGET_TEXT_ANCHOR;

        case SNAPSOURCE_NODE_HANDLE:
        case SNAPSOURCE_OTHER_HANDLE:
        case SNAPSOURCE_CONVEX_HULL_CORNER:
            // For these snapsources there doesn't exist an equivalent snap target
            return SNAPTARGET_NODE_CATEGORY;
        case SNAPSOURCE_GRID_PITCH:
            return SNAPTARGET_GRID;
        default:
            g_warning("Mapping of snap source to snap target undefined");
            return SNAPTARGET_UNDEFINED;
    }
}

bool Inkscape::SnapPreferences::isSourceSnappable(Inkscape::SnapSourceType const source) const
{
    return isTargetSnappable(source2target(source));
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
