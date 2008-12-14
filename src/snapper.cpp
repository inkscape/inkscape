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

/**
 *  Construct new Snapper for named view.
 *  \param nv Named view.
 *  \param d Snap tolerance.
 */
Inkscape::Snapper::Snapper(SnapManager *sm, Geom::Coord const t) :
	_snapmanager(sm),
	_snap_enabled(true),
	_snapper_tolerance(std::max(t, 1.0))
{
    g_assert(_snapmanager != NULL);
}

/**
 *  Set snap tolerance.
 *  \param d New snap tolerance (desktop coordinates)
 */
void Inkscape::Snapper::setSnapperTolerance(Geom::Coord const d)
{
    _snapper_tolerance = std::max(d, 1.0);
}

/**
 *  \return Snap tolerance (desktop coordinates); depends on current zoom so that it's always the same in screen pixels
 */
Geom::Coord Inkscape::Snapper::getSnapperTolerance() const
{
	SPDesktop const *dt = _snapmanager->getDesktop();
	double const zoom =  dt ? dt->current_zoom() : 1;
	return _snapper_tolerance / zoom;
}

bool Inkscape::Snapper::getSnapperAlwaysSnap() const
{
    return _snapper_tolerance == 10000; //TODO: Replace this threshold of 10000 by a constant; see also tolerance-slider.cpp
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
