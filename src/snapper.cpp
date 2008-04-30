/**
 *  \file src/snapper.cpp
 *  \brief Snapper class.
 *
 *  Authors:
 *    Carl Hetherington <inkscape@carlh.net>
 *    Diederik van Lierop <mail@diedenrezi.nl>
 *
 *  Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "libnr/nr-values.h"
#include "sp-namedview.h"
#include "inkscape.h"
#include "desktop.h"

Inkscape::Snapper::PointType const Inkscape::Snapper::SNAPPOINT_BBOX = 0x1;
Inkscape::Snapper::PointType const Inkscape::Snapper::SNAPPOINT_NODE = 0x2;
Inkscape::Snapper::PointType const Inkscape::Snapper::SNAPPOINT_GUIDE = 0x4;

/**
 *  Construct new Snapper for named view.
 *  \param nv Named view.
 *  \param d Snap tolerance.
 */
Inkscape::Snapper::Snapper(SPNamedView const *nv, NR::Coord const t) : _named_view(nv), _snap_enabled(true), _snapper_tolerance(t)
{
    g_assert(_named_view != NULL);
    g_assert(SP_IS_NAMEDVIEW(_named_view));

    setSnapFrom(SNAPPOINT_BBOX | SNAPPOINT_NODE, true); //Snap any point. In v0.45 and earlier, this was controlled in the preferences tab
}

/**
 *  Set snap tolerance.
 *  \param d New snap tolerance (desktop coordinates)
 */
void Inkscape::Snapper::setSnapperTolerance(NR::Coord const d)
{
    _snapper_tolerance = d;
}

/**
 *  \return Snap tolerance (desktop coordinates); depends on current zoom so that it's always the same in screen pixels
 */
NR::Coord Inkscape::Snapper::getSnapperTolerance() const
{
    return _snapper_tolerance / SP_ACTIVE_DESKTOP->current_zoom();
}

bool Inkscape::Snapper::getSnapperAlwaysSnap() const
{
    return _snapper_tolerance == 10000; //TODO: Replace this threshold of 10000 by a constant; see also tolerance-slider.cpp
}

/**
 *  Turn on/off snapping of specific point types.
 *  \param t Point type.
 *  \param s true to snap to this point type, otherwise false;
 */
void Inkscape::Snapper::setSnapFrom(PointType t, bool s)
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
bool Inkscape::Snapper::getSnapFrom(PointType t) const
{
    return (_snap_from & t);
}

/**
 *  \param s true to enable this snapper, otherwise false.
 */

void Inkscape::Snapper::setEnabled(bool s)
{
    _snap_enabled = s;
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
