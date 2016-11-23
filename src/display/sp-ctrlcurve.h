#ifndef SEEN_INKSCAPE_CTRLCURVE_H
#define SEEN_INKSCAPE_CTRLCURVE_H

/*
 * Simple bezier curve (used for Mesh Gradients)
 *
 * Author:
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Derived from sp-ctrlline
 *
 * Copyright (C) 2011 Tavmjong Bah
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

#include "display/sp-ctrlline.h"

class SPItem;

#define SP_TYPE_CTRLCURVE (sp_ctrlcurve_get_type())
#define SP_CTRLCURVE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_CTRLCURVE, SPCtrlCurve))
#define SP_IS_CTRLCURVE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_CTRLCURVE))

struct SPCtrlCurve : public SPCtrlLine {
    void setCoords( gdouble x0, gdouble y0, gdouble x1, gdouble y1,
                    gdouble x2, gdouble y2, gdouble x3, gdouble y3 );

    void setCoords( Geom::Point const &q0, Geom::Point const &q1,
                    Geom::Point const &q2, Geom::Point const &q3);

    Geom::Point p0, p1, p2, p3;

    int corner0;  // Used to store index of corner for finding dragger.
    int corner1;
};

GType sp_ctrlcurve_get_type();

struct SPCtrlCurveClass : public SPCtrlLineClass{};

#endif // SEEN_INKSCAPE_CTRLCURVE_H

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
