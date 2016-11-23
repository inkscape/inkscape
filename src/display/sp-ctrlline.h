#ifndef SEEN_INKSCAPE_CTRLLINE_H
#define SEEN_INKSCAPE_CTRLLINE_H

/*
 * Simple straight line
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2012 Authors
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

#include "sp-canvas-item.h"

class SPItem;

#define SP_TYPE_CTRLLINE (sp_ctrlline_get_type())
#define SP_CTRLLINE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_CTRLLINE, SPCtrlLine))
#define SP_IS_CTRLLINE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_CTRLLINE))

struct SPCtrlLine : public SPCanvasItem {
    void setRgba32(guint32 rgba);

    void setCoords(gdouble x0, gdouble y0, gdouble x1, gdouble y1);

    void setCoords(Geom::Point const &start, Geom::Point const &end);


    SPItem *item;  // the item to which this line belongs in some sense; may be NULL for some users
    bool is_fill;  // fill or stroke... used with meshes.

    guint32 rgba;
    Geom::Point s;
    Geom::Point e;
    Geom::Affine affine;
};

GType sp_ctrlline_get_type();

struct SPCtrlLineClass : public SPCanvasItemClass{};



#endif // SEEN_INKSCAPE_CTRLLINE_H

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
