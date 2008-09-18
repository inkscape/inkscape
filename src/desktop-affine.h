#ifndef __SP_DESKTOP_AFFINE_H__
#define __SP_DESKTOP_AFFINE_H__

/*
 * Desktop transformations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"
#include <2geom/forward.h>

Geom::Matrix const sp_desktop_root2dt_affine(SPDesktop const *dt);
Geom::Matrix const sp_desktop_dt2root_affine(SPDesktop const *dt);

Geom::Point sp_desktop_root2dt_xy_point(SPDesktop const *dt, const Geom::Point p);
Geom::Point sp_desktop_dt2root_xy_point(SPDesktop const *dt, const Geom::Point p);

#endif

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
