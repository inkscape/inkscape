#ifndef __INKSCAPE_CTRLPOINT_H__
#define __INKSCAPE_CTRLPOINT_H__

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

#include "sp-canvas.h"

struct SPItem;

#define SP_TYPE_CTRLPOINT (sp_ctrlpoint_get_type ())
#define SP_CTRLPOINT(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_CTRLPOINT, SPCtrlPoint))
#define SP_IS_CTRLPOINT(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_CTRLPOINT))

struct SPCtrlPoint : public SPCanvasItem{
    SPItem *item;  // the item to which this line belongs in some sense; may be NULL for some users
    guint32 rgba;
    Geom::Point pt;
    Geom::Matrix affine;
    double radius;
};
struct SPCtrlPointClass : public SPCanvasItemClass{};

GType sp_ctrlpoint_get_type (void);

void sp_ctrlpoint_set_color (SPCtrlPoint *cp, guint32 rgba);
void sp_ctrlpoint_set_coords (SPCtrlPoint *cp, const gdouble x, const gdouble y);
void sp_ctrlpoint_set_coords (SPCtrlPoint *cp, const Geom::Point pt);
void sp_ctrlpoint_set_radius (SPCtrlPoint *cp, const double r);



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
