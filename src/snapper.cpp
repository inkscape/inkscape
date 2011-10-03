/**
 *  @file src/snapper.cpp
 *  Snapper class.
 *
 *  Authors:
 *    Carl Hetherington <inkscape@carlh.net>
 *    Diederik van Lierop <mail@diedenrezi.nl>
 *
 *  Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "sp-namedview.h"
#include "inkscape.h"
#include "desktop.h"

/**
 *  Construct new Snapper for named view.
 *  @param nv Named view.
 *  @param d Snap tolerance.
 */
Inkscape::Snapper::Snapper(SnapManager *sm, Geom::Coord const /*t*/) :
    _snapmanager(sm),
    _snap_enabled(true),
    _snap_visible_only(true)
{
    g_assert(_snapmanager != NULL);
}

/**
 *  @param s true to enable this snapper, otherwise false.
 */

void Inkscape::Snapper::setEnabled(bool s)
{
    _snap_enabled = s;
}

void Inkscape::Snapper::setSnapVisibleOnly(bool s)
{
    _snap_visible_only = s;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
