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
    gint z_orders[6]; // z_orders[i] holds the ID of the face at position #i in the group (from top to bottom)

    std::vector<gint> currently_visible_faces;

    // TODO: Keeping/updating the ratios works reasonably well but is still an ad hoc implementation.
    //       Use a mathematically correct model to update the boxes.
    double ratio_x;
    double ratio_y;
    double ratio_z;

    guint front_bits; /* used internally to determine which of two parallel faces is supposed to be the front face */

    // FIXME: If we only allow a single box to be dragged at a time then we can save memory by storing
    //        the old positions centrally in SP3DBoxContext (instead of in each box separately)
    // Also, it may be better not to store the old corners but rather the old lines to which we want to snap
    NR::Point old_center;
    NR::Point old_corner2;
    NR::Point old_corner1;
    NR::Point old_corner0;
    NR::Point old_corner3;
    NR::Point old_corner5;
    NR::Point old_corner7;

    gint my_counter; // for testing only
};

struct SP3DBoxClass {
	SPGroupClass parent_class;
};

GType sp_3dbox_get_type (void);

void sp_3dbox_position_set (SP3DBoxContext &bc);
void sp_3dbox_set_shape(SP3DBox *box3d, bool use_previous_corners = false);
void sp_3dbox_recompute_corners (SP3DBox *box, NR::Point const pt1, NR::Point const pt2, NR::Point const pt3);
void sp_3dbox_set_z_orders_in_the_first_place (SP3DBox *box);
void sp_3dbox_set_z_orders_later_on (SP3DBox *box);
void sp_3dbox_update_curves (SP3DBox *box);
void sp_3dbox_link_to_existing_paths (SP3DBox *box, Inkscape::XML::Node *repr);
void sp_3dbox_set_ratios (SP3DBox *box, Box3D::Axis axes = Box3D::XYZ);
void sp_3dbox_switch_front_face (SP3DBox *box, Box3D::Axis axis);
void sp_3dbox_reshape_after_VP_rotation (SP3DBox *box, Box3D::Axis axis);
void sp_3dbox_move_corner_in_XY_plane (SP3DBox *box, guint id, NR::Point pt, Box3D::Axis axes = Box3D::XY);
void sp_3dbox_move_corner_in_Z_direction (SP3DBox *box, guint id, NR::Point pt, bool constrained = true);
void sp_3dbox_reshape_after_VP_toggling (SP3DBox *box, Box3D::Axis axis);
NR::Maybe<NR::Point> sp_3dbox_get_center (SP3DBox *box);
NR::Maybe<NR::Point> sp_3dbox_get_midpoint_between_corners (SP3DBox *box, guint id_corner1, guint id_corner2);
void sp_3dbox_recompute_XY_corners_from_new_center (SP3DBox *box, NR::Point const new_center);
void sp_3dbox_recompute_Z_corners_from_new_center (SP3DBox *box, NR::Point const new_center);
NR::Point sp_3dbox_get_midpoint_in_axis_direction (NR::Point const &C, NR::Point const &D, Box3D::Axis axis, Box3D::Perspective3D *persp);

void sp_3dbox_update_perspective_lines();
void sp_3dbox_corners_for_perspective_lines (const SP3DBox * box, Box3D::Axis axis, NR::Point &corner1, NR::Point &corner2, NR::Point &corner3, NR::Point &corner4);
guint sp_3dbox_get_corner_id_along_edge (const SP3DBox *box, guint corner, Box3D::Axis axis, Box3D::FrontOrRear rel_pos);
NR::Point sp_3dbox_get_corner_along_edge (const SP3DBox *box, guint corner, Box3D::Axis axis, Box3D::FrontOrRear rel_pos);
guint sp_3dbox_get_front_corner_id (const SP3DBox *box);


gchar * sp_3dbox_get_svg_descr_of_persp (Box3D::Perspective3D *persp);

inline NR::Point sp_3dbox_get_corner (SP3DBox *box, guint id) { return box->corners[id]; }
inline bool sp_3dbox_corners_are_adjacent (guint id_corner1, guint id_corner2) {
  return Box3D::is_single_axis_direction ((Box3D::Axis) (id_corner1 ^ id_corner2));
}

#endif
