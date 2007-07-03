#ifndef __SP_3DBOX_H__
#define __SP_3DBOX_H__

/*
 * SVG <box3d> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007      Authors
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "inkscape.h"
#include "perspective-line.h"

#include "sp-item-group.h"
#include "sp-path.h"
#include "xml/document.h"
#include "xml/repr.h"
#include "line-geometry.h"
#include "box3d-face.h"


#define SP_TYPE_3DBOX            (sp_3dbox_get_type ())
#define SP_3DBOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_3DBOX, SP3DBox))
#define SP_3DBOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_3DBOX, SP3DBoxClass))
#define SP_IS_3DBOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_3DBOX))
#define SP_IS_3DBOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_3DBOX))


struct SP3DBox : public SPGroup {
    NR::Point corners[8];
    Box3DFace *faces[6];
};

struct SP3DBoxClass {
	SPGroupClass parent_class;
};

GType sp_3dbox_get_type (void);

void sp_3dbox_position_set (SP3DBoxContext &bc);
void sp_3dbox_recompute_corners (SP3DBox *box, NR::Point const pt1, NR::Point const pt2, NR::Point const pt3);
void sp_3dbox_update_curves (SP3DBox *box);
void sp_3dbox_move_corner_in_XY_plane (SP3DBox *box, guint id, NR::Point pt);
void sp_3dbox_move_corner_in_constrained_Z_direction (SP3DBox *box, guint id, NR::Point pt);

inline NR::Point sp_3dbox_get_corner (SP3DBox *box, guint id) { return box->corners[id]; }

#endif
