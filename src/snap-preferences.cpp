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

Inkscape::SnapPreferences::PointType const Inkscape::SnapPreferences::SNAPPOINT_NODE  = 0x1;
Inkscape::SnapPreferences::PointType const Inkscape::SnapPreferences::SNAPPOINT_BBOX  = 0x2;
Inkscape::SnapPreferences::PointType const Inkscape::SnapPreferences::SNAPPOINT_GUIDE = 0x4;


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
	setSnapFrom(SNAPPOINT_BBOX | SNAPPOINT_NODE, true); //Snap any point. In v0.45 and earlier, this was controlled in the preferences tab
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
        _snap_from |= Inkscape::SnapPreferences::SNAPPOINT_BBOX;
    } else {
        _snap_from &= ~Inkscape::SnapPreferences::SNAPPOINT_BBOX;
    }
}

bool Inkscape::SnapPreferences::getSnapModeBBox() const
{
	return (_snap_from & Inkscape::SnapPreferences::SNAPPOINT_BBOX);
}

void Inkscape::SnapPreferences::setSnapModeNode(bool enabled)
{
	if (enabled) {
        _snap_from |= Inkscape::SnapPreferences::SNAPPOINT_NODE;
    } else {
        _snap_from &= ~Inkscape::SnapPreferences::SNAPPOINT_NODE;
    }
}

bool Inkscape::SnapPreferences::getSnapModeNode() const
{
	return (_snap_from & Inkscape::SnapPreferences::SNAPPOINT_NODE);
}

bool Inkscape::SnapPreferences::getSnapModeBBoxOrNodes() const
{
	return (_snap_from & (Inkscape::SnapPreferences::SNAPPOINT_BBOX | Inkscape::SnapPreferences::SNAPPOINT_NODE) );
}

bool Inkscape::SnapPreferences::getSnapModeAny() const
{
	return (_snap_from != 0);
}

void Inkscape::SnapPreferences::setSnapModeGuide(bool enabled)
{
	if (enabled) {
        _snap_from |= Inkscape::SnapPreferences::SNAPPOINT_GUIDE;
    } else {
        _snap_from &= ~Inkscape::SnapPreferences::SNAPPOINT_GUIDE;
    }
}

bool Inkscape::SnapPreferences::getSnapModeGuide() const
{
	return (_snap_from & Inkscape::SnapPreferences::SNAPPOINT_GUIDE);
}

/**
 *  Turn on/off snapping of specific point types.
 *  \param t Point type.
 *  \param s true to snap to this point type, otherwise false;
 */
void Inkscape::SnapPreferences::setSnapFrom(PointType t, bool s)
{
    if (s) {
        _snap_from |= t;
    } else {
        _snap_from &= ~t;
    }
}

/**
 *  \param t Point type.
 *  \return true if snapper will snap this type of point, otherwise false.
 */
bool Inkscape::SnapPreferences::getSnapFrom(PointType t) const
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
