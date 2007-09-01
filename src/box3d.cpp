#define __SP_3DBOX_C__

/*
 * SVG <box3d> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007      Authors
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>
#include "attributes.h"
#include "svg/stringstream.h"
#include "box3d.h"
#include "desktop-handles.h"

static void sp_3dbox_class_init(SP3DBoxClass *klass);
static void sp_3dbox_init(SP3DBox *box3d);

static void sp_3dbox_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_3dbox_release (SPObject *object);
static void sp_3dbox_set(SPObject *object, unsigned int key, const gchar *value);
static void sp_3dbox_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_3dbox_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static gchar *sp_3dbox_description(SPItem *item);

//static void sp_3dbox_set_shape(SPShape *shape);
//static void sp_3dbox_set_shape(SP3DBox *box3d);

static void sp_3dbox_update_corner_with_value_from_svg (SPObject *object, guint corner_id, const gchar *value);
static void sp_3dbox_update_perspective (Box3D::Perspective3D *persp, const gchar *value);
static gchar * sp_3dbox_get_corner_coords_string (SP3DBox *box, guint id);
static std::pair<gdouble, gdouble> sp_3dbox_get_coord_pair_from_string (const gchar *);
static gchar * sp_3dbox_get_perspective_string (SP3DBox *box);

static SPGroupClass *parent_class;

static gint counter = 0;

GType
sp_3dbox_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SP3DBoxClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) sp_3dbox_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(SP3DBox),
            16,     /* n_preallocs */
            (GInstanceInitFunc) sp_3dbox_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_GROUP, "SP3DBox", &info, (GTypeFlags) 0);
    }

    return type;
}

static void
sp_3dbox_class_init(SP3DBoxClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPItemClass *item_class = (SPItemClass *) klass;

    parent_class = (SPGroupClass *) g_type_class_ref(SP_TYPE_GROUP);

    sp_object_class->build = sp_3dbox_build;
    sp_object_class->set = sp_3dbox_set;
    sp_object_class->write = sp_3dbox_write;
    sp_object_class->update = sp_3dbox_update;
    sp_object_class->release = sp_3dbox_release;

    item_class->description = sp_3dbox_description;
}

static void
sp_3dbox_init(SP3DBox *box)
{
    for (int i = 0; i < 8; ++i) box->corners[i] = NR::Point(0,0);
    for (int i = 0; i < 6; ++i) box->faces[i] = NULL;
}

static void
sp_3dbox_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) (parent_class))->build) {
        ((SPObjectClass *) (parent_class))->build(object, document, repr);
    }

    SP3DBox *box = SP_3DBOX (object);

    box->my_counter = counter++;

    /* we initialize the z-orders to zero so that they are updated during dragging */
    for (int i = 0; i < 6; ++i) {
        box->z_orders[i] = 0;
    }

    box->front_bits = 0x0;

    
    if (repr->attribute ("inkscape:perspective") == NULL) {
        // we are creating a new box; link it to the current perspective
        document->current_perspective->add_box (box);
    } else {
        // create a new perspective that we can compare with existing ones
        Box3D::Perspective3D *persp = new Box3D::Perspective3D (Box3D::VanishingPoint (0,0),
                                                                Box3D::VanishingPoint (0,0),
                                                                Box3D::VanishingPoint (0,0),
                                                                document);
        sp_3dbox_update_perspective (persp, repr->attribute ("inkscape:perspective"));
        Box3D::Perspective3D *comp =  document->find_perspective (persp);
        if (comp == NULL) {
            // perspective doesn't exist yet
            document->add_perspective (persp);
            persp->add_box (box);
        } else {
            // link the box to the existing perspective and delete the temporary one
            comp->add_box (box);
            delete persp;
            //g_assert (Box3D::get_persp_of_box (box) == comp);

            // FIXME: If the paths of the box's faces do not correspond to the svg representation of the perspective
            //        the box is shown with a "wrong" initial shape that is only corrected after dragging.
            //        Should we "repair" this by updating the paths at the end of sp_3dbox_build()?
            //        Maybe it would be better to simply destroy and rebuild them in sp_3dbox_link_to_existing_paths().
        }
    }

    sp_object_read_attr(object, "inkscape:box3dcornerA");
    sp_object_read_attr(object, "inkscape:box3dcornerB");
    sp_object_read_attr(object, "inkscape:box3dcornerC");

    // TODO: We create all faces in the beginning, but only the non-degenerate ones
    //       should be written to the svg representation later in sp_3dbox_write.
    Box3D::Axis cur_plane, axis, dir1, dir2;
    Box3D::FrontOrRear cur_pos;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 2; ++j) {
            cur_plane = Box3D::planes[i];
            cur_pos = Box3D::face_positions[j];
            // FIXME: The following code could theoretically be moved to
            //        the constructor of Box3DFace (but see the comment there).
            axis = (cur_pos == Box3D::FRONT ? Box3D::NONE : Box3D::third_axis_direction (cur_plane));
            dir1 = extract_first_axis_direction (cur_plane);
            dir2 = extract_second_axis_direction (cur_plane);
            
            box->faces[Box3D::face_to_int(cur_plane ^ cur_pos)] =
                new Box3DFace (box, box->corners[axis], box->corners[axis ^ dir1],
                                    box->corners[axis ^ dir1 ^ dir2], box->corners[axis ^ dir2],
                                    cur_plane, cur_pos);
        }
    }

    // Check whether the paths of the faces of the box need to be linked to existing paths in the
    // document (e.g., after a 'redo' operation or after opening a file) and do so if necessary.
    sp_3dbox_link_to_existing_paths (box, repr);

    sp_3dbox_set_ratios (box, Box3D::XYZ);

    // Store the center (if it already exists) and certain corners for later use during center-dragging
    NR::Maybe<NR::Point> cen = sp_3dbox_get_center (box);
    if (cen) {
        box->old_center = *cen;
    }
    box->old_corner2 = box->corners[2];
    box->old_corner1 = box->corners[1];
    box->old_corner0 = box->corners[0];
    box->old_corner3 = box->corners[3];
    box->old_corner5 = box->corners[5];
    box->old_corner7 = box->corners[7];
}

static void
sp_3dbox_release (SPObject *object)
{
	SP3DBox *box = SP_3DBOX(object);
        for (int i = 0; i < 6; ++i) {
            if (box->faces[i]) {
                delete box->faces[i]; // FIXME: Anything else to do? Do we need to clean up the face first?
            }
        }

        // FIXME: We do not duplicate perspectives if they are the same for several boxes.
        //        Thus, don't delete the perspective when deleting a box but rather unlink the box from it.
        SP_OBJECT_DOCUMENT (G_OBJECT (box))->get_persp_of_box (box)->remove_box (box);

	if (((SPObjectClass *) parent_class)->release) {
	  ((SPObjectClass *) parent_class)->release (object);
	}
}

static void sp_3dbox_set(SPObject *object, unsigned int key, const gchar *value)
{
    switch (key) {
        case SP_ATTR_INKSCAPE_3DBOX_CORNER_A:
            sp_3dbox_update_corner_with_value_from_svg (object, 2, value);
            break;
        case SP_ATTR_INKSCAPE_3DBOX_CORNER_B:
            sp_3dbox_update_corner_with_value_from_svg (object, 1, value);
            break;
        case SP_ATTR_INKSCAPE_3DBOX_CORNER_C:
            sp_3dbox_update_corner_with_value_from_svg (object, 5, value);
            break;
        case SP_ATTR_INKSCAPE_3DBOX_PERSPECTIVE:
        {
            SP3DBox *box = SP_3DBOX (object);
            sp_3dbox_update_perspective (SP_OBJECT_DOCUMENT (object)->get_persp_of_box (box), value);
            break;
        }
	default:
            if (((SPObjectClass *) (parent_class))->set) {
                ((SPObjectClass *) (parent_class))->set(object, key, value);
            }
            break;
    }
}

static void
sp_3dbox_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        SP3DBox *box = SP_3DBOX(object);
        Inkscape::XML::Node *repr = SP_OBJECT_REPR(object);
        sp_3dbox_link_to_existing_paths (box, repr);
        SP3DBoxContext *bc = SP_3DBOX_CONTEXT (inkscape_active_event_context());
        bc->_vpdrag->updateDraggers();
        // FIXME: Should we update the corners here, too? Maybe this is the reason why the handles
        //        are off after an undo/redo! On the other hand, if we do so we get warnings about
        //        updates occuring while other updats are in progress ...
    }

    /* Invoke parent method */
    if (((SPObjectClass *) (parent_class))->update)
        ((SPObjectClass *) (parent_class))->update(object, ctx, flags);
}

static Inkscape::XML::Node *sp_3dbox_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SP3DBox *box = SP_3DBOX(object);
    // FIXME: How to handle other contexts???
    // FIXME: Is tools_isactive(..) more recommended to check for the current context/tool?
    if (!SP_IS_3DBOX_CONTEXT(inkscape_active_event_context()))
        return repr;

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_OBJECT_DOCUMENT(object));
        repr = xml_doc->createElement("svg:g");
        repr->setAttribute("sodipodi:type", "inkscape:3dbox");
        /* Hook paths to the faces of the box */
        for (int i = 0; i < 6; ++i) {
            box->faces[i]->hook_path_to_3dbox();
        }
    }

    for (int i = 0; i < 6; ++i) {
        box->faces[i]->set_path_repr();
    }

    if (flags & SP_OBJECT_WRITE_EXT) {
        gchar *str;
        str = sp_3dbox_get_corner_coords_string (box, 2);
        repr->setAttribute("inkscape:box3dcornerA", str);

        str = sp_3dbox_get_corner_coords_string (box, 1);
        repr->setAttribute("inkscape:box3dcornerB", str);

        str = sp_3dbox_get_corner_coords_string (box, 5);
        repr->setAttribute("inkscape:box3dcornerC", str);

        str = sp_3dbox_get_perspective_string (box);
        repr->setAttribute("inkscape:perspective", str);
        sp_3dbox_set_ratios (box);

        g_free ((void *) str);

        /* store center and construction-corners for later use during center-dragging */
        NR::Maybe<NR::Point> cen = sp_3dbox_get_center (box);
        if (cen) {
            box->old_center = *cen;
        }
        box->old_corner2 = box->corners[2];
        box->old_corner1 = box->corners[1];
        box->old_corner0 = box->corners[0];
        box->old_corner3 = box->corners[3];
        box->old_corner5 = box->corners[5];
        box->old_corner7 = box->corners[7];
    }

    if (((SPObjectClass *) (parent_class))->write) {
        ((SPObjectClass *) (parent_class))->write(object, repr, flags);
    }

    return repr;
}

static gchar *
sp_3dbox_description(SPItem *item)
{
    g_return_val_if_fail(SP_IS_3DBOX(item), NULL);

    return g_strdup(_("<b>3D Box</b>"));
}

void sp_3dbox_set_ratios (SP3DBox *box, Box3D::Axis axes)
{
    Box3D::Perspective3D *persp = SP_OBJECT_DOCUMENT (G_OBJECT (box))->get_persp_of_box (box);
    NR::Point pt;

    if (axes & Box3D::X) {
        pt = persp->get_vanishing_point (Box3D::X)->get_pos();
        box->ratio_x = NR::L2 (pt - box->corners[2]) / NR::L2 (pt - box->corners[3]);
    }

    if (axes & Box3D::Y) {
        pt = persp->get_vanishing_point (Box3D::Y)->get_pos();
        box->ratio_y = NR::L2 (pt - box->corners[2]) / NR::L2 (pt - box->corners[0]);
    }

    if (axes & Box3D::Z) {
        pt = persp->get_vanishing_point (Box3D::Z)->get_pos();
        box->ratio_z = NR::L2 (pt - box->corners[4]) / NR::L2 (pt - box->corners[0]);
    }
}

void
sp_3dbox_switch_front_face (SP3DBox *box, Box3D::Axis axis)
{
    if (SP_OBJECT_DOCUMENT (G_OBJECT (box))->get_persp_of_box (box)->get_vanishing_point (axis)->is_finite()) {
        box->front_bits = box->front_bits ^ axis;
    }
}


void
sp_3dbox_position_set (SP3DBoxContext &bc)
{
    SP3DBox *box3d = SP_3DBOX(bc.item);

    sp_3dbox_set_shape(box3d);

    // FIXME: Why does the following call not automatically update the children
    //        of box3d (which is an SPGroup, which should do this)?
    //SP_OBJECT(box3d)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);

    /**
    SP_OBJECT(box3d->path_face1)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    SP_OBJECT(box3d->path_face2)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    SP_OBJECT(box3d->path_face3)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    SP_OBJECT(box3d->path_face4)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    SP_OBJECT(box3d->path_face5)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    SP_OBJECT(box3d->path_face6)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    ***/
}

static void
sp_3dbox_set_shape_from_points (SP3DBox *box, NR::Point const &cornerA, NR::Point const &cornerB, NR::Point const &cornerC)
{
    sp_3dbox_recompute_corners (box, cornerA, cornerB, cornerC);

    // FIXME: How to handle other contexts???
    // FIXME: Is tools_isactive(..) more recommended to check for the current context/tool?
    if (!SP_IS_3DBOX_CONTEXT(inkscape_active_event_context()))
        return;
    SP3DBoxContext *bc = SP_3DBOX_CONTEXT(inkscape_active_event_context());

    if (bc->extruded) {
        box->faces[0]->set_corners (box->corners[0], box->corners[4], box->corners[6], box->corners[2]);
        box->faces[1]->set_corners (box->corners[1], box->corners[5], box->corners[7], box->corners[3]);
        box->faces[2]->set_corners (box->corners[0], box->corners[1], box->corners[5], box->corners[4]);
        box->faces[3]->set_corners (box->corners[2], box->corners[3], box->corners[7], box->corners[6]);
        box->faces[5]->set_corners (box->corners[4], box->corners[5], box->corners[7], box->corners[6]);
    }
    box->faces[4]->set_corners (box->corners[0], box->corners[1], box->corners[3], box->corners[2]);

    sp_3dbox_update_curves (box);
}

void
// FIXME: Note that this is _not_ the virtual set_shape() method inherited from SPShape,
//        since SP3DBox is inherited from SPGroup. The following method is "artificially"
//        called from sp_3dbox_update().
//sp_3dbox_set_shape(SPShape *shape)
sp_3dbox_set_shape(SP3DBox *box, bool use_previous_corners)
{
    if (!SP_IS_3DBOX_CONTEXT(inkscape_active_event_context()))
        return;
    SP3DBoxContext *bc = SP_3DBOX_CONTEXT(inkscape_active_event_context());

    if (!use_previous_corners) {
        sp_3dbox_set_shape_from_points (box, bc->drag_origin, bc->drag_ptB, bc->drag_ptC);
    } else {
        sp_3dbox_set_shape_from_points (box, box->corners[2], box->corners[1], box->corners[5]);
    }
}


void sp_3dbox_recompute_corners (SP3DBox *box, NR::Point const A, NR::Point const B, NR::Point const C)
{
    sp_3dbox_move_corner_in_XY_plane (box, 2, A);
    sp_3dbox_move_corner_in_XY_plane (box, 1, B);
    sp_3dbox_move_corner_in_Z_direction (box, 5, C);
}

inline static double
normalized_angle (double angle) {
    if (angle < -M_PI) {
        return angle + 2*M_PI;
    } else if (angle > M_PI) {
        return angle - 2*M_PI;
    }
    return angle;
}

static gdouble
sp_3dbox_corner_angle_to_VP (SP3DBox *box, Box3D::Axis axis, guint extreme_corner)
{
    Box3D::VanishingPoint *vp = SP_OBJECT_DOCUMENT (G_OBJECT (box))->get_persp_of_box (box)->get_vanishing_point (axis);
    NR::Point dir;

    if (vp->is_finite()) {
        dir = NR::unit_vector (vp->get_pos() - box->corners[extreme_corner]);
    } else {
        dir = NR::unit_vector (vp->v_dir);
    }

    return atan2 (dir[NR::Y], dir[NR::X]);
}


bool sp_3dbox_recompute_z_orders (SP3DBox *box)
{
    gint new_z_orders[6];

    // TODO: Determine the front corner depending on the distance from VPs and/or the user presets
    guint front_corner = sp_3dbox_get_front_corner_id (box);

    gdouble dir_1x = sp_3dbox_corner_angle_to_VP (box, Box3D::X, front_corner);
    gdouble dir_3x = sp_3dbox_corner_angle_to_VP (box, Box3D::X, front_corner ^ Box3D::Y);

    gdouble dir_1y = sp_3dbox_corner_angle_to_VP (box, Box3D::Y, front_corner);
    //gdouble dir_0y = sp_3dbox_corner_angle_to_VP (box, Box3D::Y, front_corner ^ Box3D::X);

    gdouble dir_1z = sp_3dbox_corner_angle_to_VP (box, Box3D::Z, front_corner);
    gdouble dir_3z = sp_3dbox_corner_angle_to_VP (box, Box3D::Z, front_corner ^ Box3D::Y);

    // Still not perfect, but only fails in some rather degenerate cases.
    // I suspect that there is a more elegant model, though. :)
    new_z_orders[0] = Box3D::face_containing_corner (Box3D::XY, front_corner);
    if (normalized_angle (dir_1y - dir_1z) > 0) {
        new_z_orders[1] = Box3D::face_containing_corner (Box3D::YZ, front_corner);
        if (normalized_angle (dir_1x - dir_1z) > 0) {
            new_z_orders[2] = Box3D::face_containing_corner (Box3D::XZ, front_corner ^ Box3D::Y);
        } else {
            new_z_orders[2] = Box3D::face_containing_corner (Box3D::XZ, front_corner);
        }
    } else {
        if (normalized_angle (dir_3x - dir_3z) > 0) {
            new_z_orders[1] = Box3D::face_containing_corner (Box3D::XZ, front_corner ^ Box3D::Y);
            new_z_orders[2] = Box3D::face_containing_corner (Box3D::YZ, front_corner ^ Box3D::X);
        } else {
            if (normalized_angle (dir_1x - dir_1z) > 0) {
                new_z_orders[1] = Box3D::face_containing_corner (Box3D::YZ, front_corner ^ Box3D::X);
                new_z_orders[2] = Box3D::face_containing_corner (Box3D::XZ, front_corner);
            } else {
                new_z_orders[1] = Box3D::face_containing_corner (Box3D::XZ, front_corner);
                new_z_orders[2] = Box3D::face_containing_corner (Box3D::YZ, front_corner ^ Box3D::X);
            }
        }
    }

    new_z_orders[3] = Box3D::opposite_face (new_z_orders[2]);
    new_z_orders[4] = Box3D::opposite_face (new_z_orders[1]);
    new_z_orders[5] = Box3D::opposite_face (new_z_orders[0]);

    /* We only need to look for changes among the topmost three faces because the order
       of the other ones is just inverted. */
    if ((box->z_orders[0] != new_z_orders[0]) ||
        (box->z_orders[1] != new_z_orders[1]) ||
        (box->z_orders[2] != new_z_orders[2]))
    {
        for (int i = 0; i < 6; ++i) {
            box->z_orders[i] = new_z_orders[i];
        }
        return true;
    }

    return false;
}

// convenience
static bool sp_3dbox_is_subset_or_superset (std::vector<gint> const &list1, std::vector<gint> const &list2)
{
    return (std::includes (list1.begin(), list1.end(), list2.begin(), list2.end()) ||
            std::includes (list2.begin(), list2.end(), list1.begin(), list1.end()));
}

static bool sp_3dbox_differ_by_opposite_faces (std::vector<gint> const &list1, std::vector<gint> const &list2)
{
    std::vector<gint> diff1;
    std::vector<gint> diff2;
    std::set_difference (list1.begin(), list1.end(), list2.begin(), list2.end(),
                         std::insert_iterator<std::vector<gint> >(diff1, diff1.begin()));
    std::set_difference (list2.begin(), list2.end(), list1.begin(), list1.end(),
                         std::insert_iterator<std::vector<gint> >(diff2, diff2.begin()));

    if (diff1.size() == 3 || diff1.size() != diff2.size())
        return false;

    for (guint i = 0; i < diff1.size(); ++i) {
        if (std::find (diff2.begin(), diff2.end(), Box3D::opposite_face (diff1[i])) == diff2.end()) {
            return false;
        }
    }
    return true;
}

static gint
sp_3dbox_face_containing_diagonal_corners (guint corner1, guint corner2)
{
    Box3D::Axis plane = (Box3D::Axis) (corner1 ^ corner2);
    if (!Box3D::is_plane (plane)) {
        g_warning ("Corners %d and %d should span a plane.\n", corner1, corner2);
        return 0;
    }

    return Box3D::face_containing_corner (plane, corner1);
}

static std::vector<gint> sp_3dbox_adjacent_faces_of_edge (guint corner1, guint corner2) {
    std::vector<gint> adj_faces;
    Box3D::Axis edge = (Box3D::Axis) (corner1 ^ corner2);
    if (!Box3D::is_single_axis_direction (edge)) {
        return adj_faces;
    }

    Box3D::Axis plane = Box3D::orth_plane_or_axis (edge);
    Box3D::Axis axis1 = Box3D::extract_first_axis_direction (plane);
    Box3D::Axis axis2 = Box3D::extract_second_axis_direction (plane);
    adj_faces.push_back (Box3D::face_containing_corner ((Box3D::Axis) (edge ^ axis1), corner1));
    adj_faces.push_back (Box3D::face_containing_corner ((Box3D::Axis) (edge ^ axis2), corner1));
    return adj_faces;
}

static std::vector<gint> sp_3dbox_faces_meeting_in_corner (guint corner) {
    std::vector<gint> faces;
    for (int i = 0; i < 3; ++i) {
        faces.push_back (sp_3dbox_face_containing_diagonal_corners (corner, corner ^ Box3D::planes[i]));
    }
    return faces;
}

static void sp_3dbox_remaining_faces (std::vector<gint> const &faces, std::vector<gint> &rem_faces)
{
    rem_faces.clear();
    for (gint i = 0; i < 6; ++i) {
        if (std::find (faces.begin(), faces.end(), i) == faces.end()) {
            rem_faces.push_back (i);
        }
    }
}

/*
 * Given two adjacent edges (\a c2,\a c1) and (\a c2, \a c3) of \a box (with common corner \a c2),
 * check whether both lie on the convex hull of the point configuration given by \a box's corners.
 */
static bool
sp_3dbox_is_border_edge_pair (SP3DBox *box, guint const c1, guint const c2, guint const c3)
{
    Box3D::Axis edge21 = (Box3D::Axis) (c2 ^ c1);
    Box3D::Axis edge23 = (Box3D::Axis) (c2 ^ c3);
    Box3D::Axis rear_axis = Box3D::orth_plane_or_axis ((Box3D::Axis) (edge21 ^ edge23));
 
    NR::Point corner2 = box->corners[c2];
    NR::Point dir21 = box->corners[c1] - corner2;
    NR::Point dir23 = box->corners[c3] - corner2;

    if (!Box3D::lies_in_sector (dir21, dir23, box->corners[c2 ^ edge21 ^ edge23] - corner2) ||
        !Box3D::lies_in_sector (dir21, dir23, box->corners[c2 ^ rear_axis] - corner2) ||
        !Box3D::lies_in_sector (dir21, dir23, box->corners[c2 ^ rear_axis ^ edge21] - corner2) ||
        !Box3D::lies_in_sector (dir21, dir23, box->corners[c2 ^ rear_axis ^ edge21 ^ edge23] - corner2) ||
        !Box3D::lies_in_sector (dir21, dir23, box->corners[c2 ^ rear_axis ^ edge23] - corner2)) {
        // corner triple c1, c2, c3 doesn't bound the convex hull
        return false;
    }
    // corner triple c1, c2, c3 bounds the convex hull
    return true;    
}

/*
 * Test whether there are any adjacent corners of \a corner (i.e., connected with it along one of the axes)
 * such that the corresponding edges bound the convex hull of the box (as a point configuration in the plane)
 * If this is the case, return the corresponding two adjacent corners; otherwise return (-1, -1).
 */
static Box3D::Axis
sp_3dbox_axis_pair_bounding_convex_hull (SP3DBox *box, guint corner)
 {
    guint adj1 = corner ^ Box3D::X;
    guint adj2 = corner ^ Box3D::Y;
    guint adj3 = corner ^ Box3D::Z;

    if (sp_3dbox_is_border_edge_pair (box, adj1, corner, adj2)) {
        return Box3D::XY;
    }
    if (sp_3dbox_is_border_edge_pair (box, adj1, corner, adj3)) {
        return Box3D::XZ;
    }
    if (sp_3dbox_is_border_edge_pair (box, adj2, corner, adj3)) {
        return Box3D::YZ;
    }
    return Box3D::NONE;
}

// inside_hull is modified 'in place' by the following function
static void sp_3dbox_corner_configuration (SP3DBox *box, std::vector<gint> &on_hull, std::vector<gint> &inside_hull)
{
    for (int i = 0; i < 8; ++i) {
        Box3D::Axis bounding_edges = sp_3dbox_axis_pair_bounding_convex_hull (box, i);
        if (bounding_edges != Box3D::NONE) {
            on_hull.push_back (i);
        } else {
            inside_hull.push_back (i);
        }
    }
}

/* returns true if there was a change in the z-orders (which triggers an update of the repr) */
static bool sp_3dbox_recompute_z_orders_by_corner_configuration (SP3DBox *box)
{
    gint new_z_orders[6];
    Box3D::Axis front_rear_axis = Box3D::Z;

    std::vector<gint> on_hull;
    std::vector<gint> inside_hull;
    std::vector<gint> visible_faces;

    sp_3dbox_corner_configuration (box, on_hull, inside_hull);

    switch (on_hull.size()) {
        case 4:
            {
                // the following works because on_hull is sorted
                gint front_face = sp_3dbox_face_containing_diagonal_corners (on_hull[0], on_hull[3]);
                visible_faces.push_back (front_face);
            }
            break;

        case 6:
        {
            guint c1 = inside_hull[0] ^ Box3D::XYZ;
            guint c2 = inside_hull[1] ^ Box3D::XYZ;
            Box3D::Axis edge = (Box3D::Axis) (c1 ^ c2);
            if (Box3D::is_single_axis_direction (edge)) {
                visible_faces = sp_3dbox_adjacent_faces_of_edge (c1, c2);
            } else if (c1 == c2 ^ Box3D::XYZ) {
                guint c_cmp = sp_3dbox_get_corner_id_along_edge (box, 0, front_rear_axis, Box3D::FRONT);
                guint visible_front_corner = (((c_cmp & front_rear_axis) == (c1 & front_rear_axis)) ? c1 : c2);
                visible_faces = sp_3dbox_faces_meeting_in_corner (visible_front_corner);
            } else {
                /* Under what conditions do we end up here? Can we safely ignore this case? */
                return false;
            }
            break;
        }

        default:
            /* Under what conditions do we end up here? Can we safely ignore this case? */
            return false;
    }

    /* catch weird corner configurations; these should be theoretically impossible, but maybe
       occur in (almost) degenerate cases due to rounding errors, for example */
    if (std::find (visible_faces.begin(), visible_faces.end(), -1) != visible_faces.end()) {
        return false;
    }

    /* sort the list of visible faces for later use (although it may be already sorted anyway) */
    std::sort (visible_faces.begin(), visible_faces.end());

    std::vector<gint> invisible_faces;
    sp_3dbox_remaining_faces (visible_faces, invisible_faces);


    if (!sp_3dbox_is_subset_or_superset (visible_faces, box->currently_visible_faces) &&
        !sp_3dbox_differ_by_opposite_faces (visible_faces, box->currently_visible_faces)) {
        std::swap (visible_faces, invisible_faces);
        if (!sp_3dbox_is_subset_or_superset (visible_faces, box->currently_visible_faces) &&
            !sp_3dbox_differ_by_opposite_faces (visible_faces, box->currently_visible_faces)) {
            /* Hopefully this case is only caused by rounding errors or something similar;
               does it need further investigation? */
            return false;
        }
    }

    box->currently_visible_faces = visible_faces;

    // set new z-orders according to the visible/invisible faces
    guint vis_size = visible_faces.size();
    for (guint i = 0; i < vis_size; ++i) {
        new_z_orders[i] = visible_faces[i];
    }
    for (guint i = 0; i < invisible_faces.size(); ++i) {
        new_z_orders[vis_size + i] = invisible_faces[i];
    }

    // test whether any z-orders actually changed and indicate this in the return status
    for (int i = 0; i < 6; ++i) {
        if (box->z_orders[i] != new_z_orders[i]) {
            // we update the z-orders starting from the index where the change occurs
            for (int j = i; j < 6; ++j) {
                box->z_orders[j] = new_z_orders[j];
            }
            return true;
        }
    }
    return false;
}

// FIXME: Can we unify this and the next function for setting the z-orders?
void sp_3dbox_set_z_orders_in_the_first_place (SP3DBox *box)
{
    // For efficiency reasons, we only set the new z-orders if something really changed
    if (sp_3dbox_recompute_z_orders (box)) {
        box->faces[box->z_orders[0]]->lower_to_bottom ();
        box->faces[box->z_orders[1]]->lower_to_bottom ();
        box->faces[box->z_orders[2]]->lower_to_bottom ();
        box->faces[box->z_orders[3]]->lower_to_bottom ();
        box->faces[box->z_orders[4]]->lower_to_bottom ();
        box->faces[box->z_orders[5]]->lower_to_bottom ();
    }
}

void sp_3dbox_set_z_orders_later_on (SP3DBox *box)
{
    // For efficiency reasons, we only set the new z-orders if something really changed
    if (sp_3dbox_recompute_z_orders_by_corner_configuration (box)) {
        box->faces[box->z_orders[0]]->lower_to_bottom ();
        box->faces[box->z_orders[1]]->lower_to_bottom ();
        box->faces[box->z_orders[2]]->lower_to_bottom ();
        box->faces[box->z_orders[3]]->lower_to_bottom ();
        box->faces[box->z_orders[4]]->lower_to_bottom ();
        box->faces[box->z_orders[5]]->lower_to_bottom ();
    }
}

void
sp_3dbox_update_curves (SP3DBox *box) {
    for (int i = 0; i < 6; ++i) {
        if (box->faces[i]) box->faces[i]->set_curve();
    }
}

/**
 * In some situations (e.g., after cloning boxes, undo & redo, or reading boxes from a file) there are
 * paths already present in the document which correspond to the faces of newly created boxes, but their
 * 'path' members don't link to them yet. The following function corrects this if necessary.
 */
void
sp_3dbox_link_to_existing_paths (SP3DBox *box, Inkscape::XML::Node *repr) {
    // TODO: We should probably destroy the existing paths and recreate them because we don't know
    //       precisely which path corresponds to which face. Does this make a difference?
    //       In sp_3dbox_write we write the correct paths anyway, don't we? But we could get into
    //       trouble at a later stage when we only write single faces for degenerate boxes.

    SPDocument *document = SP_OBJECT_DOCUMENT(box);
    guint face_id = 0;

    for (Inkscape::XML::Node *i = sp_repr_children(repr); i != NULL; i = sp_repr_next(i)) {
        if (face_id > 5) {
            g_warning ("SVG representation of 3D boxes must contain 6 paths or less.\n");
            break;
        }

        SPObject *face_object = document->getObjectByRepr((Inkscape::XML::Node *) i);
        if (!SP_IS_PATH(face_object)) {
            g_warning ("SVG representation of 3D boxes should only contain paths.\n");
            continue;
        }
        // TODO: Currently we don't check whether all paths are being linked to different faces.
        //       This is no problem with valid SVG files. It may lead to crashes, however,
        //       in case a file is corrupt (e.g., two or more faces have identical descriptions).
        gint id = Box3DFace::descr_to_id (i->attribute ("inkscape:box3dface"));
        box->faces[id]->hook_path_to_3dbox(SP_PATH(face_object));
        ++face_id;
    }
    if (face_id < 6) {
        //g_warning ("SVG representation of 3D boxes should contain exactly 6 paths (degenerate boxes are not yet supported).\n");
        // TODO: Check whether it is safe to add the remaining paths to the box and do so in case it is.
        //       (But we also land here for newly created boxes where we shouldn't add any paths because
        //       This is done in sp_3dbox_write later on.
    }
}

void
sp_3dbox_reshape_after_VP_rotation (SP3DBox *box, Box3D::Axis axis)
{
    Box3D::Perspective3D *persp = inkscape_active_document()->get_persp_of_box (box);
    Box3D::VanishingPoint *vp = persp->get_vanishing_point (axis);

    guint c1 = (axis == Box3D::Z) ? 1 : sp_3dbox_get_front_corner_id (box); // hack
    guint c2 = c1 ^ axis;
    NR::Point v = box->corners[c1] - box->corners[c2];
    double dist = NR::L2 (v) * ((NR::dot (v, vp->v_dir) < 0) ? 1 : -1); // "directed" distance

    Box3D::PerspectiveLine pline (box->corners[c1], axis, persp);
    NR::Point pt = pline.point_from_lambda (dist);

    sp_3dbox_move_corner_in_Z_direction (box, c2, pt, axis == Box3D::Z);
}

void
sp_3dbox_move_corner_in_XY_plane (SP3DBox *box, guint id, NR::Point pt, Box3D::Axis axes)
{
    Box3D::Perspective3D * persp = SP_OBJECT_DOCUMENT (G_OBJECT (box))->get_persp_of_box (box);

    NR::Point A (box->corners[id ^ Box3D::XY]);
    if (Box3D::is_single_axis_direction (axes)) {
        pt = Box3D::PerspectiveLine (box->corners[id], axes, persp).closest_to(pt);
    }

    /* set the 'front' corners */
    box->corners[id] = pt;

    Box3D::PerspectiveLine pl_one (A, Box3D::Y, persp);
    Box3D::PerspectiveLine pl_two (pt, Box3D::X, persp);
    box->corners[id ^ Box3D::X] = pl_one.meet(pl_two);

    pl_one = Box3D::PerspectiveLine (A, Box3D::X, persp);
    pl_two = Box3D::PerspectiveLine (pt, Box3D::Y, persp);
    box->corners[id ^ Box3D::Y] = pl_one.meet(pl_two);

    /* set the 'rear' corners */
    NR::Point B (box->corners[id ^ Box3D::XYZ]);

    pl_one = Box3D::PerspectiveLine (box->corners[id ^ Box3D::X], Box3D::Z, persp);
    pl_two = Box3D::PerspectiveLine (B, Box3D::Y, persp);
    box->corners[id ^ Box3D::XZ] = pl_one.meet(pl_two);

    pl_one = Box3D::PerspectiveLine (box->corners[id ^ Box3D::XZ], Box3D::X, persp);
    pl_two = Box3D::PerspectiveLine (pt, Box3D::Z, persp);
    box->corners[id ^ Box3D::Z] = pl_one.meet(pl_two);

    pl_one = Box3D::PerspectiveLine (box->corners[id ^ Box3D::Z], Box3D::Y, persp);
    pl_two = Box3D::PerspectiveLine (B, Box3D::X, persp);
    box->corners[id ^ Box3D::YZ] = pl_one.meet(pl_two);
    
}

void
sp_3dbox_move_corner_in_Z_direction (SP3DBox *box, guint id, NR::Point pt, bool constrained)
{
    if (!constrained) sp_3dbox_move_corner_in_XY_plane (box, id, pt, Box3D::XY);

    Box3D::Perspective3D * persp = SP_OBJECT_DOCUMENT (G_OBJECT (box))->get_persp_of_box (box);

    /* set the four corners of the face containing corners[id] */
    box->corners[id] = Box3D::PerspectiveLine (box->corners[id], Box3D::Z, persp).closest_to(pt);

    Box3D::PerspectiveLine pl_one (box->corners[id], Box3D::X, persp);
    Box3D::PerspectiveLine pl_two (box->corners[id ^ Box3D::XZ], Box3D::Z, persp);
    box->corners[id ^ Box3D::X] = pl_one.meet(pl_two);

    pl_one = Box3D::PerspectiveLine (box->corners[id ^ Box3D::X], Box3D::Y, persp);
    pl_two = Box3D::PerspectiveLine (box->corners[id ^ Box3D::XYZ], Box3D::Z, persp);
    box->corners[id ^ Box3D::XY] = pl_one.meet(pl_two);

    pl_one = Box3D::PerspectiveLine (box->corners[id], Box3D::Y, persp);
    pl_two = Box3D::PerspectiveLine (box->corners[id ^ Box3D::YZ], Box3D::Z, persp);
    box->corners[id ^ Box3D::Y] = pl_one.meet(pl_two);
}

static void
sp_3dbox_reshape_edge_after_VP_toggling (SP3DBox *box, const guint corner, const Box3D::Axis axis, Box3D::Perspective3D *persp)
{
    /* Hmm, perhaps we should simply use one of the corners as the pivot point.
       But this way we minimize the amount of reshaping.
       On second thought, we need to find a way to ensure that all boxes sharing the same
       perspective are updated consistently _as a group_. That is, they should also retain
       their relative positions towards each other. */
    NR::Maybe<NR::Point> pt = sp_3dbox_get_midpoint_between_corners (box, corner, corner ^ axis);
    g_return_if_fail (pt);

    Box3D::Axis axis2 = ((axis == Box3D::Y) ? Box3D::X : Box3D::Y);

    Box3D::PerspectiveLine line1 (box->corners[corner], axis2, persp);
    Box3D::PerspectiveLine line2 (box->corners[corner ^ axis], axis2, persp);

    Box3D::PerspectiveLine line3 (*pt, axis, persp);

    NR::Point new_corner1 = line1.meet (line3);
    NR::Point new_corner2 = line2.meet (line3);

    box->corners[corner] = new_corner1;
    box->corners[corner ^ axis] = new_corner2;
}

void
sp_3dbox_reshape_after_VP_toggling (SP3DBox *box, Box3D::Axis axis)
{
    Box3D::Perspective3D *persp = SP_OBJECT_DOCUMENT (G_OBJECT (box))->get_persp_of_box (box);
    std::pair<Box3D::Axis, Box3D::Axis> dirs = Box3D::get_remaining_axes (axis);

    sp_3dbox_reshape_edge_after_VP_toggling (box, 0, axis, persp);
    sp_3dbox_reshape_edge_after_VP_toggling (box, 0 ^ dirs.first, axis, persp);
    sp_3dbox_reshape_edge_after_VP_toggling (box, 0 ^ dirs.first ^ dirs.second, axis, persp);
    sp_3dbox_reshape_edge_after_VP_toggling (box, 0 ^ dirs.second, axis, persp);
}

NR::Maybe<NR::Point>
sp_3dbox_get_center (SP3DBox *box)
{
    return sp_3dbox_get_midpoint_between_corners (box, 0, 7);
}

NR::Point
sp_3dbox_get_midpoint_in_axis_direction (NR::Point const &C, NR::Point const &D, Box3D::Axis axis, Box3D::Perspective3D *persp)
{
    Box3D::PerspectiveLine pl (D, axis, persp);
    return pl.pt_with_given_cross_ratio (C, D, -1.0);
}

// TODO: The following function can probably be rewritten in a much more elegant and robust way
//        by using projective coordinates for all points and using the cross ratio.
NR::Maybe<NR::Point>
sp_3dbox_get_midpoint_between_corners (SP3DBox *box, guint id_corner1, guint id_corner2)
{
    Box3D::Axis corner_axes = (Box3D::Axis) (id_corner1 ^ id_corner2);

    // Is all this sufficiently precise also for degenerate cases?
    if (sp_3dbox_corners_are_adjacent (id_corner1, id_corner2)) {
        Box3D::Axis orth_dir = get_perpendicular_axis_direction (corner_axes);

        Box3D::Line diag1 (box->corners[id_corner1], box->corners[id_corner2 ^ orth_dir]);
        Box3D::Line diag2 (box->corners[id_corner1 ^ orth_dir], box->corners[id_corner2]);
        NR::Maybe<NR::Point> adjacent_face_center = diag1.intersect(diag2);

        if (!adjacent_face_center) return NR::Nothing();

        Box3D::Perspective3D * persp = SP_OBJECT_DOCUMENT (G_OBJECT (box))->get_persp_of_box (box);

        Box3D::PerspectiveLine pl (*adjacent_face_center, orth_dir, persp);
        return pl.intersect(Box3D::PerspectiveLine(box->corners[id_corner1], corner_axes, persp));
    } else {
        Box3D::Axis dir = Box3D::extract_first_axis_direction (corner_axes);
        Box3D::Line diag1 (box->corners[id_corner1], box->corners[id_corner2]);
        Box3D::Line diag2 (box->corners[id_corner1 ^ dir], box->corners[id_corner2 ^ dir]);
        return diag1.intersect(diag2);
    }
}

static gchar *
sp_3dbox_get_corner_coords_string (SP3DBox *box, guint id)
{
    id = id % 8;
    Inkscape::SVGOStringStream os;
    os << box->corners[id][NR::X] << "," << box->corners[id][NR::Y];
    return g_strdup(os.str().c_str());
}

static std::pair<gdouble, gdouble>
sp_3dbox_get_coord_pair_from_string (const gchar *coords)
{
    gchar **coordpair = g_strsplit( coords, ",", 0);
    // We might as well rely on g_ascii_strtod to convert the NULL pointer to 0.0,
    // but we include the following test anyway
    if (coordpair[0] == NULL || coordpair[1] == NULL) {
        g_strfreev (coordpair);
        g_warning ("Coordinate conversion failed.\n");
        return std::make_pair(0.0, 0.0);
    }

    gdouble coord1 = g_ascii_strtod(coordpair[0], NULL);
    gdouble coord2 = g_ascii_strtod(coordpair[1], NULL);
    g_strfreev (coordpair);

    return std::make_pair(coord1, coord2);
}

static gchar *
sp_3dbox_get_perspective_string (SP3DBox *box)
{
    
    return sp_3dbox_get_svg_descr_of_persp (SP_OBJECT_DOCUMENT (G_OBJECT (box))->get_persp_of_box (box));
}
  
gchar *
sp_3dbox_get_svg_descr_of_persp (Box3D::Perspective3D *persp)
{
    // FIXME: We should move this code to perspective3d.cpp, but this yields compiler errors. Why?
    Inkscape::SVGOStringStream os;

    Box3D::VanishingPoint vp = *(persp->get_vanishing_point (Box3D::X));
    os << vp[NR::X] << "," << vp[NR::Y] << ",";
    os << vp.v_dir[NR::X] << "," << vp.v_dir[NR::Y] << ",";
    if (vp.is_finite()) {
        os << "finite,";
    } else {
        os << "infinite,";
    }

    vp = *(persp->get_vanishing_point (Box3D::Y));
    os << vp[NR::X] << "," << vp[NR::Y] << ",";
    os << vp.v_dir[NR::X] << "," << vp.v_dir[NR::Y] << ",";
    if (vp.is_finite()) {
        os << "finite,";
    } else {
        os << "infinite,";
    }

    vp = *(persp->get_vanishing_point (Box3D::Z));
    os << vp[NR::X] << "," << vp[NR::Y] << ",";
    os << vp.v_dir[NR::X] << "," << vp.v_dir[NR::Y] << ",";
    if (vp.is_finite()) {
        os << "finite";
    } else {
        os << "infinite";
    }

    return g_strdup(os.str().c_str());
}

// auxiliary function
static std::pair<NR::Point, NR::Point>
sp_3dbox_new_midpoints (Box3D::Perspective3D *persp, Box3D::Axis axis, NR::Point const &M0, NR::Point const &M, NR::Point const &A, NR::Point const &B)
{
    double cr1 = Box3D::cross_ratio (*persp->get_vanishing_point (axis), M0, M, A);
    double cr2 = Box3D::cross_ratio (*persp->get_vanishing_point (axis), M, B, M0);
    if (fabs (cr1 - 1) < Box3D::epsilon) {
        // FIXME: cr == 1 is a degenerate case; how should we deal with it?
        return std::make_pair (NR::Point (0,0), NR::Point (0,0));
    }
    if (cr1 == NR_HUGE) {
        return std::make_pair (A, B);
    }
    Box3D::PerspectiveLine pl (M0, axis, persp);
    NR::Point B_new = pl.pt_with_given_cross_ratio (M0, M, cr1 / (cr1 - 1));
    NR::Point A_new = pl.pt_with_given_cross_ratio (M0, M, 1 - cr2);
    return std::make_pair (A_new, B_new);
}

void sp_3dbox_recompute_Z_corners_from_new_center (SP3DBox *box, NR::Point const new_center)
{
    // TODO: Clean this function up

    Box3D::Perspective3D *persp = sp_desktop_document (inkscape_active_desktop())->get_persp_of_box (box);
    NR::Point old_center = box->old_center;

    Box3D::PerspectiveLine aux_line1 (old_center, Box3D::Z, persp);
    Box3D::PerspectiveLine aux_line2 (new_center, Box3D::Y, persp);
    NR::Point Z1 = aux_line1.meet (aux_line2);

    NR::Point A0 (sp_3dbox_get_midpoint_in_axis_direction (box->old_corner2, box->old_corner0, Box3D::Y, persp));
    NR::Point B0 (sp_3dbox_get_midpoint_in_axis_direction (box->old_corner7, box->old_corner5, Box3D::Y, persp));
    Box3D::PerspectiveLine aux_line3 (A0, Box3D::X, persp);
    Box3D::PerspectiveLine aux_line4 (B0, Box3D::X, persp);

    NR::Point C0 = aux_line3.meet (aux_line1);
    NR::Point D0 = aux_line4.meet (aux_line1);

    std::pair<NR::Point, NR::Point> new_midpts = sp_3dbox_new_midpoints (persp, Box3D::Z, old_center, Z1, C0, D0);
    NR::Point C1 (new_midpts.first);
    NR::Point D1 (new_midpts.second);
    Box3D::PerspectiveLine aux_line5 (C1, Box3D::X, persp);
    Box3D::PerspectiveLine aux_line6 (D1, Box3D::X, persp);

    Box3D::PerspectiveLine aux_line7 (A0, Box3D::Z, persp);
    Box3D::PerspectiveLine aux_line8 (B0, Box3D::Z, persp);

    NR::Point A1 = aux_line5.meet (aux_line7);
    NR::Point B1 = aux_line6.meet (aux_line8);

    Box3D::PerspectiveLine aux_line9  (box->old_corner2, Box3D::Z, persp);
    Box3D::PerspectiveLine aux_line10 (box->old_corner5, Box3D::Z, persp);

    Box3D::PerspectiveLine aux_line11 (A1, Box3D::Y, persp);
    Box3D::PerspectiveLine aux_line12 (B1, Box3D::Y, persp);

    NR::Point new_corner2 = aux_line9.meet (aux_line11);
    NR::Point new_corner5 = aux_line10.meet (aux_line12);

    Box3D::PerspectiveLine aux_line13 (A1, Box3D::X, persp);
    NR::Point E1 = aux_line13.meet (aux_line8);
    Box3D::PerspectiveLine aux_line14 (E1, Box3D::Y, persp);

    NR::Point new_corner1 = aux_line10.meet (aux_line14);

    sp_3dbox_set_shape_from_points (box, new_corner2, new_corner1, new_corner5);
}

void sp_3dbox_recompute_XY_corners_from_new_center (SP3DBox *box, NR::Point const new_center)
{
    // TODO: Clean this function up

    Box3D::Perspective3D *persp = sp_desktop_document (inkscape_active_desktop())->get_persp_of_box (box);
    NR::Point old_center = box->old_center;

    NR::Point A0 (sp_3dbox_get_midpoint_in_axis_direction (box->old_corner2, box->old_corner0, Box3D::Y, persp));
    NR::Point B0 (sp_3dbox_get_midpoint_in_axis_direction (box->old_corner1, box->old_corner3, Box3D::Y, persp));

    /* we first move the box along the X-axis ... */
    Box3D::PerspectiveLine aux_line1 (old_center, Box3D::X, persp);
    Box3D::PerspectiveLine aux_line2 (new_center, Box3D::Y, persp);
    NR::Point Z1 = aux_line1.meet (aux_line2);

    Box3D::PerspectiveLine ref_line (B0, Box3D::X, persp);
    Box3D::PerspectiveLine pline2 (old_center, Box3D::Z, persp);
    Box3D::PerspectiveLine pline3 (Z1, Box3D::Z, persp);
    NR::Point M0 = ref_line.meet (pline2);
    NR::Point M1 = ref_line.meet (pline3);

    std::pair<NR::Point, NR::Point> new_midpts = sp_3dbox_new_midpoints (persp, Box3D::X, M0, M1, A0, B0);
    NR::Point A1 (new_midpts.first);
    NR::Point B1 (new_midpts.second);

    /* ... and then along the Y-axis */
    Box3D::PerspectiveLine pline4 (box->old_corner1, Box3D::X, persp);
    Box3D::PerspectiveLine pline5 (box->old_corner3, Box3D::X, persp);
    Box3D::PerspectiveLine aux_line3 (M1, Box3D::Y, persp);
    NR::Point C1 = aux_line3.meet (pline4);
    NR::Point D1 = aux_line3.meet (pline5);

    Box3D::PerspectiveLine aux_line4 (new_center, Box3D::Z, persp);
    NR::Point M2 = aux_line4.meet (aux_line3);

    std::pair<NR::Point, NR::Point> other_new_midpts = sp_3dbox_new_midpoints (persp, Box3D::Y, M1, M2, C1, D1);
    NR::Point C2 (other_new_midpts.first);
    NR::Point D2 (other_new_midpts.second);

    Box3D::PerspectiveLine plXC (C2, Box3D::X, persp);
    Box3D::PerspectiveLine plXD (D2, Box3D::X, persp);
    Box3D::PerspectiveLine plYA (A1, Box3D::Y, persp);
    Box3D::PerspectiveLine plYB (B1, Box3D::Y, persp);

    NR::Point new_corner2 (plXD.meet (plYA));
    NR::Point new_corner1 (plXC.meet (plYB));

    NR::Point tmp_corner1 (pline4.meet (plYB));
    Box3D::PerspectiveLine pline6 (box->old_corner5, Box3D::X, persp);
    Box3D::PerspectiveLine pline7 (tmp_corner1, Box3D::Z, persp);
    NR::Point tmp_corner5 (pline6.meet (pline7));

    Box3D::PerspectiveLine pline8 (tmp_corner5, Box3D::Y, persp);
    Box3D::PerspectiveLine pline9 (new_corner1, Box3D::Z, persp);
    NR::Point new_corner5 (pline8.meet (pline9));

    sp_3dbox_set_shape_from_points (box, new_corner2, new_corner1, new_corner5);
}

void sp_3dbox_update_perspective_lines()
{
    SPEventContext *ec = inkscape_active_event_context();
    if (!SP_IS_3DBOX_CONTEXT (ec))
        return;

    SP_3DBOX_CONTEXT (ec)->_vpdrag->updateLines();
}

/*
 * Manipulates corner1 through corner4 to contain the indices of the corners
 * from which the perspective lines in the direction of 'axis' emerge
 */
void sp_3dbox_corners_for_perspective_lines (const SP3DBox * box, Box3D::Axis axis, 
        				     NR::Point &corner1, NR::Point &corner2, NR::Point &corner3, NR::Point &corner4)
{
    // along which axis to switch when takint
    Box3D::Axis switch_axis;
    if (axis == Box3D::X || axis == Box3D::Y) {
        switch_axis = (box->front_bits & axis) ? Box3D::Z : Box3D::NONE;
    } else {
        switch_axis = (box->front_bits & axis) ? Box3D::X : Box3D::NONE;
    }

    switch (axis) {
        case Box3D::X:
            corner1 = sp_3dbox_get_corner_along_edge (box, 0 ^ switch_axis, axis, Box3D::REAR);
            corner2 = sp_3dbox_get_corner_along_edge (box, 2 ^ switch_axis, axis, Box3D::REAR);
            corner3 = sp_3dbox_get_corner_along_edge (box, 4 ^ switch_axis, axis, Box3D::REAR);
            corner4 = sp_3dbox_get_corner_along_edge (box, 6 ^ switch_axis, axis, Box3D::REAR);
            break;
        case Box3D::Y:
            corner1 = sp_3dbox_get_corner_along_edge (box, 0 ^ switch_axis, axis, Box3D::REAR);
            corner2 = sp_3dbox_get_corner_along_edge (box, 1 ^ switch_axis, axis, Box3D::REAR);
            corner3 = sp_3dbox_get_corner_along_edge (box, 4 ^ switch_axis, axis, Box3D::REAR);
            corner4 = sp_3dbox_get_corner_along_edge (box, 5 ^ switch_axis, axis, Box3D::REAR);
            break;
        case Box3D::Z:
            corner1 = sp_3dbox_get_corner_along_edge (box, 1 ^ switch_axis, axis, Box3D::REAR);
            corner2 = sp_3dbox_get_corner_along_edge (box, 3 ^ switch_axis, axis, Box3D::REAR);
            corner3 = sp_3dbox_get_corner_along_edge (box, 0 ^ switch_axis, axis, Box3D::REAR);
            corner4 = sp_3dbox_get_corner_along_edge (box, 2 ^ switch_axis, axis, Box3D::REAR);
            break;
        default:
            // do nothing
            break;
    }            
}

/**
 * Returns the id of the corner on the edge along 'axis' and passing through 'corner' that
 * lies on the front/rear face in this direction.
 */
guint
sp_3dbox_get_corner_id_along_edge (const SP3DBox *box, guint corner, Box3D::Axis axis, Box3D::FrontOrRear rel_pos)
{
    guint result;
    guint other_corner = corner ^ axis;
    Box3D::VanishingPoint *vp = SP_OBJECT_DOCUMENT (G_OBJECT (box))->get_persp_of_box (box)->get_vanishing_point(axis);
    if (vp->is_finite()) {
        result = (  NR::L2 (vp->get_pos() - box->corners[corner])
                  < NR::L2 (vp->get_pos() - box->corners[other_corner]) ? other_corner : corner);
    } else {
        // clear the axis bit and switch to the appropriate corner along axis, depending on the value of front_bits
        result = ((corner & (0xF ^ axis)) ^ (box->front_bits & axis));
    }

    if (rel_pos == Box3D::FRONT) {
        return result;
    } else {
        return result ^ axis;
    }
}

NR::Point
sp_3dbox_get_corner_along_edge (const SP3DBox *box, guint corner, Box3D::Axis axis, Box3D::FrontOrRear rel_pos)
{
    return box->corners[sp_3dbox_get_corner_id_along_edge (box, corner, axis, rel_pos)];
}

guint
sp_3dbox_get_front_corner_id (const SP3DBox *box)
{
    guint front_corner = 1; // this could in fact be any corner, but we choose the one that is normally in front
    front_corner = sp_3dbox_get_corner_id_along_edge (box, front_corner, Box3D::X, Box3D::FRONT);
    front_corner = sp_3dbox_get_corner_id_along_edge (box, front_corner, Box3D::Y, Box3D::FRONT);
    front_corner = sp_3dbox_get_corner_id_along_edge (box, front_corner, Box3D::Z, Box3D::FRONT);
    return front_corner;
}

// auxiliary functions
static void
sp_3dbox_update_corner_with_value_from_svg (SPObject *object, guint corner_id, const gchar *value)
{
    if (value == NULL) return;
    SP3DBox *box = SP_3DBOX(object);

    std::pair<gdouble, gdouble> coord_pair = sp_3dbox_get_coord_pair_from_string (value);
    box->corners[corner_id] = NR::Point (coord_pair.first, coord_pair.second);
    sp_3dbox_recompute_corners (box, box->corners[2], box->corners[1], box->corners[5]);
    object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_3dbox_update_perspective (Box3D::Perspective3D *persp, const gchar *value)
{
    // WARNING! This function changes the perspective associated to 'box'. Since there may be
    // many other boxes linked to the same perspective, their perspective is also changed.
    // If this behaviour is not desired in all cases, we need a different function.
    if (value == NULL) return;

    gchar **vps = g_strsplit( value, ",", 0);
    for (int i = 0; i < 15; ++i) {
        if (vps[i] == NULL) {
            g_warning ("Malformed svg attribute 'perspective'\n");
            return;
        }
    }

    persp->set_vanishing_point (Box3D::X, g_ascii_strtod (vps[0], NULL), g_ascii_strtod (vps[1], NULL),
                                          g_ascii_strtod (vps[2], NULL), g_ascii_strtod (vps[3], NULL),
                                          strcmp (vps[4], "finite") == 0 ? Box3D::VP_FINITE : Box3D::VP_INFINITE);
    persp->set_vanishing_point (Box3D::Y, g_ascii_strtod (vps[5], NULL), g_ascii_strtod (vps[6], NULL),
                                          g_ascii_strtod (vps[7], NULL), g_ascii_strtod (vps[8], NULL),
                                          strcmp (vps[9], "finite") == 0 ? Box3D::VP_FINITE : Box3D::VP_INFINITE);
    persp->set_vanishing_point (Box3D::Z, g_ascii_strtod (vps[10], NULL), g_ascii_strtod (vps[11], NULL),
                                          g_ascii_strtod (vps[12], NULL), g_ascii_strtod (vps[13], NULL),
                                          strcmp (vps[14], "finite") == 0 ? Box3D::VP_FINITE : Box3D::VP_INFINITE);

    // update the other boxes linked to the same perspective
    persp->reshape_boxes (Box3D::XYZ);
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
