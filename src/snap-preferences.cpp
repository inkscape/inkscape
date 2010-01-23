#define __SNAPPREFERENCES_CPP__

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

#include "inkscape.h"
#include "snap-preferences.h"

Inkscape::SnapPreferences::SnapPreferences() :
    _include_item_center(false),
    _intersectionGG(true),
    _snap_to_grids(true),
    _snap_to_guides(true),
    _snap_enabled_globally(true),
    _snap_postponed_globally(false),
    _snap_to_itemnode(true), _snap_to_itempath(true),
    _snap_to_bboxnode(true), _snap_to_bboxpath(true),
    _snap_to_page_border(false),
    _strict_snapping(true)
{
    setSnapFrom(SnapSourceType(SNAPSOURCE_BBOX_CATEGORY | SNAPSOURCE_NODE_CATEGORY | SNAPSOURCE_OTHER_CATEGORY), true); //Snap any point. In v0.45 and earlier, this was controlled in the preferences tab
}

/*
 *  The snappers have too many parameters to adjust individually. Therefore only
 *  two snapping modes are presented to the user: snapping bounding box corners (to
 *  other bounding boxes, grids or guides), and/or snapping nodes (to other nodes,
 *  paths, grids or guides). To select either of these modes (or both), use the
 *  methods defined below: setSnapModeBBox() and setSnapModeNode().
 *
 * */


void Inkscape::SnapPreferences::setSnapModeBBox(bool enabled)
{
    if (enabled) {
        _snap_from = SnapSourceType(_snap_from | Inkscape::SNAPSOURCE_BBOX_CATEGORY);
    } else {
        _snap_from = SnapSourceType(_snap_from & ~Inkscape::SNAPSOURCE_BBOX_CATEGORY);
    }
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
}

bool Inkscape::SnapPreferences::getSnapModeNode() const
{
    return (_snap_from & Inkscape::SNAPSOURCE_NODE_CATEGORY);
}

bool Inkscape::SnapPreferences::getSnapModeBBoxOrNodes() const
{
    return (_snap_from & (Inkscape::SNAPSOURCE_BBOX_CATEGORY | Inkscape::SNAPSOURCE_NODE_CATEGORY) );
}

bool Inkscape::SnapPreferences::getSnapModeAny() const
{
    return (_snap_from != 0);
}

void Inkscape::SnapPreferences::setSnapModeGuide(bool enabled)
{
    if (enabled) {
        _snap_from = SnapSourceType(_snap_from | Inkscape::SNAPSOURCE_OTHER_CATEGORY);
    } else {
        _snap_from = SnapSourceType(_snap_from & ~Inkscape::SNAPSOURCE_OTHER_CATEGORY);
    }
}

bool Inkscape::SnapPreferences::getSnapModeGuide() const
{
    return (_snap_from & Inkscape::SNAPSOURCE_OTHER_CATEGORY);
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
