#ifndef SEEN_INKSCAPE_CTRLPOINT_H
#define SEEN_INKSCAPE_CTRLPOINT_H

/*
 * A simple point
 *
 * Author:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL
 */

#include "sp-canvas-item.h"

class SPItem;

#define SP_TYPE_CTRLPOINT (sp_ctrlpoint_get_type ())
#define SP_CTRLPOINT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_CTRLPOINT, SPCtrlPoint))
#define SP_IS_CTRLPOINT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_CTRLPOINT))

struct SPCtrlPoint : public SPCanvasItem {
    SPItem *item;  // the item to which this line belongs in some sense; may be NULL for some users
    guint32 rgba;
    Geom::Point pt;
    Geom::Affine affine;
    double lenght;
    bool is_circle;
};
struct SPCtrlPointClass : public SPCanvasItemClass{};

GType sp_ctrlpoint_get_type (void);

void sp_ctrlpoint_set_color (SPCtrlPoint *cp, guint32 rgba);
void sp_ctrlpoint_set_coords (SPCtrlPoint *cp, const gdouble x, const gdouble y);
void sp_ctrlpoint_set_coords (SPCtrlPoint *cp, const Geom::Point pt);
void sp_ctrlpoint_set_lenght (SPCtrlPoint *cp, const double r);
void sp_ctrlpoint_set_circle (SPCtrlPoint *cp, const bool circle);



#endif // SEEN_INKSCAPE_CTRLPOINT_H

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
