#ifndef __INKSCAPE_CTRLLINE_H__
#define __INKSCAPE_CTRLLINE_H__

/*
 * Simple straight line
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

#include "sp-canvas.h"

struct SPItem;

#define SP_TYPE_CTRLLINE (sp_ctrlline_get_type ())
#define SP_CTRLLINE(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_CTRLLINE, SPCtrlLine))
#define SP_IS_CTRLLINE(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_CTRLLINE))

struct SPCtrlLine : public SPCanvasItem{
    SPItem *item;  // the item to which this line belongs in some sense; may be NULL for some users
    guint32 rgba;
    NR::Point s, e;
    NR::Matrix affine;
};
struct SPCtrlLineClass : public SPCanvasItemClass{};

GtkType sp_ctrlline_get_type (void);

void sp_ctrlline_set_rgba32 (SPCtrlLine *cl, guint32 rgba);
void sp_ctrlline_set_coords (SPCtrlLine *cl, gdouble x0, gdouble y0, gdouble x1, gdouble y1);
void sp_ctrlline_set_coords (SPCtrlLine *cl, const NR::Point start, const NR::Point end);



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
