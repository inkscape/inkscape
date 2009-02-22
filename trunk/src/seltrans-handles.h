#ifndef __SP_SELTRANS_HANDLES_H__
#define __SP_SELTRANS_HANDLES_H__

/*
 * Seltrans knots
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/sodipodi-ctrl.h"
#include <2geom/forward.h>

namespace Inkscape
{
  class SelTrans;
}

class SPSelTransHandle;

// request handlers
gboolean sp_sel_trans_scale_request(Inkscape::SelTrans *seltrans,
				    SPSelTransHandle const &handle, Geom::Point &p, guint state);
gboolean sp_sel_trans_stretch_request(Inkscape::SelTrans *seltrans,
				      SPSelTransHandle const &handle, Geom::Point &p, guint state);
gboolean sp_sel_trans_skew_request(Inkscape::SelTrans *seltrans,
				   SPSelTransHandle const &handle, Geom::Point &p, guint state);
gboolean sp_sel_trans_rotate_request(Inkscape::SelTrans *seltrans,
				     SPSelTransHandle const &handle, Geom::Point &p, guint state);
gboolean sp_sel_trans_center_request(Inkscape::SelTrans *seltrans,
				     SPSelTransHandle const &handle, Geom::Point &p, guint state);

// action handlers
void sp_sel_trans_scale(Inkscape::SelTrans *seltrans, SPSelTransHandle const &handle, Geom::Point &p, guint state);
void sp_sel_trans_stretch(Inkscape::SelTrans *seltrans, SPSelTransHandle const &handle, Geom::Point &p, guint state);
void sp_sel_trans_skew(Inkscape::SelTrans *seltrans, SPSelTransHandle const &handle, Geom::Point &p, guint state);
void sp_sel_trans_rotate(Inkscape::SelTrans *seltrans, SPSelTransHandle const &handle, Geom::Point &p, guint state);
void sp_sel_trans_center(Inkscape::SelTrans *seltrans, SPSelTransHandle const &handle, Geom::Point &p, guint state);

struct SPSelTransHandle {
	GtkAnchorType anchor;
	GdkCursorType cursor;
	guint control;
	void (* action) (Inkscape::SelTrans *seltrans, SPSelTransHandle const &handle, Geom::Point &p, guint state);
	gboolean (* request) (Inkscape::SelTrans *seltrans, SPSelTransHandle const &handle, Geom::Point &p, guint state);
	gdouble x, y;
};

extern SPSelTransHandle const handles_scale[8];
extern SPSelTransHandle const handles_rotate[8];
extern SPSelTransHandle const handle_center;

#endif

