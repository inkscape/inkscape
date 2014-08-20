#ifndef __INKSCAPE_CTRLQUADR_H__
#define __INKSCAPE_CTRLQUADR_H__

/*
 * Quadrilateral
 *
 * Authors:
 *   bulia byak
 *
 * Copyright (C) 2005 authors
 *
 * Released under GNU GPL
 */

#include <2geom/geom.h>

#define SP_TYPE_CTRLQUADR (sp_ctrlquadr_get_type ())
#define SP_CTRLQUADR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_CTRLQUADR, SPCtrlQuadr))
#define SP_IS_CTRLQUADR(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_CTRLQUADR))

struct SPCtrlQuadr;
struct SPCtrlQuadrClass;

GType sp_ctrlquadr_get_type (void);

void sp_ctrlquadr_set_rgba32 (SPCtrlQuadr *cl, guint32 rgba);
void sp_ctrlquadr_set_coords (SPCtrlQuadr *cl, const Geom::Point p1, const Geom::Point p2, const Geom::Point p3, const Geom::Point p4);


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
