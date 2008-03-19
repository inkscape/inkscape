#define __SP_BOX3D_C__

/*
 * SVG <box3d> implementation
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2007      Authors
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>
#include "attributes.h"
#include "xml/document.h"
#include "xml/repr.h"

#include "box3d.h"
#include "box3d-side.h"
#include "box3d-context.h"
#include "proj_pt.h"
#include "transf_mat_3x4.h"
#include "perspective-line.h"
#include "inkscape.h"
#include "persp3d.h"
#include "line-geometry.h"
#include "persp3d-reference.h"
#include "uri.h"
#include "2geom/geom.h"
#include "sp-guide.h"
#include "sp-namedview.h"
#include "prefs-utils.h"

#include "desktop.h"
#include "desktop-handles.h"
#include "macros.h"

static void box3d_class_init(SPBox3DClass *klass);
static void box3d_init(SPBox3D *box3d);

static void box3d_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void box3d_release(SPObject *object);
static void box3d_set(SPObject *object, unsigned int key, const gchar *value);
static void box3d_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *box3d_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static gchar *box3d_description(SPItem *item);
static NR::Matrix box3d_set_transform(SPItem *item, NR::Matrix const &xform);
static void box3d_convert_to_guides(SPItem *item);

static void box3d_ref_changed(SPObject *old_ref, SPObject *ref, SPBox3D *box);

static SPGroupClass *parent_class;

static gint counter = 0;

GType
box3d_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPBox3DClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) box3d_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(SPBox3D),
            16,     /* n_preallocs */
            (GInstanceInitFunc) box3d_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_GROUP, "SPBox3D", &info, (GTypeFlags) 0);
    }

    return type;
}

static void
box3d_class_init(SPBox3DClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPItemClass *item_class = (SPItemClass *) klass;

    parent_class = (SPGroupClass *) g_type_class_ref(SP_TYPE_GROUP);

    sp_object_class->build = box3d_build;
    sp_object_class->release = box3d_release;
    sp_object_class->set = box3d_set;
    sp_object_class->write = box3d_write;
    sp_object_class->update = box3d_update;

    item_class->description = box3d_description;
    item_class->set_transform = box3d_set_transform;
    item_class->convert_to_guides = box3d_convert_to_guides;
}

static void
box3d_init(SPBox3D *box)
{
    box->persp_href = NULL;
    box->persp_ref = new Persp3DReference(SP_OBJECT(box));
}

static void
box3d_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) (parent_class))->build) {
        ((SPObjectClass *) (parent_class))->build(object, document, repr);
    }

    SPBox3D *box = SP_BOX3D (object);
    box->my_counter = counter++;

    /* we initialize the z-orders to zero so that they are updated during dragging */
    for (int i = 0; i < 6; ++i) {
        box->z_orders[i] = 0;
    }

    // TODO: Create/link to the correct perspective

    SPDocument *doc = SP_OBJECT_DOCUMENT(box);
    if (!doc) {
        g_print ("No document for the box!!!!\n");
        return;
    }

    box->persp_ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(box3d_ref_changed), box));

    sp_object_read_attr(object, "inkscape:perspectiveID");
    sp_object_read_attr(object, "inkscape:corner0");
    sp_object_read_attr(object, "inkscape:corner7");
}

/**
 * Virtual release of SPBox3D members before destruction.
 */
static void
box3d_release(SPObject *object)
{
    SPBox3D *box = (SPBox3D *) object;

    if (box->persp_href) {
        g_free(box->persp_href);
    }
    if (box->persp_ref) {
        box->persp_ref->detach();
        delete box->persp_ref;
        box->persp_ref = NULL;
    }

    //persp3d_remove_box (box3d_get_perspective(box), box);

    if (((SPObjectClass *) parent_class)->release)
        ((SPObjectClass *) parent_class)->release(object);
}

static void
box3d_set(SPObject *object, unsigned int key, const gchar *value)
{
    SPBox3D *box = SP_BOX3D(object);

    switch (key) {
        case SP_ATTR_INKSCAPE_BOX3D_PERSPECTIVE_ID:
            if ( value && box->persp_href && ( strcmp(value, box->persp_href) == 0 ) ) {
                /* No change, do nothing. */
            } else {
                if (box->persp_href) {
                    g_free(box->persp_href);
                    box->persp_href = NULL;
                }
                if (value) {
                    box->persp_href = g_strdup(value);

                    // Now do the attaching, which emits the changed signal.
                    try {
                        box->persp_ref->attach(Inkscape::URI(value));
                    } catch (Inkscape::BadURIException &e) {
                        g_warning("%s", e.what());
                        box->persp_ref->detach();
                    }
                } else {
                    // Detach, which emits the changed signal.
                    box->persp_ref->detach();
                }
            }

            // FIXME: Is the following update doubled by some call in either persp3d.cpp or vanishing_point_new.cpp?
            box3d_position_set(box);
            break;
        case SP_ATTR_INKSCAPE_BOX3D_CORNER0:
            if (value && strcmp(value, "0 : 0 : 0 : 0")) {
                box->orig_corner0 = Proj::Pt3(value);
                box->save_corner0 = box->orig_corner0;
                box3d_position_set(box);
            }
            break;
        case SP_ATTR_INKSCAPE_BOX3D_CORNER7:
            if (value && strcmp(value, "0 : 0 : 0 : 0")) {
                box->orig_corner7 = Proj::Pt3(value);
                box->save_corner7 = box->orig_corner7;
                box3d_position_set(box);
            }
            break;
	default:
            if (((SPObjectClass *) (parent_class))->set) {
                ((SPObjectClass *) (parent_class))->set(object, key, value);
            }
            break;
    }
}

/**
 * Gets called when (re)attached to another perspective.
 */
static void
box3d_ref_changed(SPObject *old_ref, SPObject *ref, SPBox3D *box)
{
    if (old_ref) {
        sp_signal_disconnect_by_data(old_ref, box);
        persp3d_remove_box (SP_PERSP3D(old_ref), box);
        /* Note: This sometimes leads to attempts to remove boxes twice from the list of selected/transformed
           boxes in a perspectives, but this should be uncritical. */
        persp3d_remove_box_transform (SP_PERSP3D(old_ref), box);
    }
    if ( SP_IS_PERSP3D(ref) && ref != box ) // FIXME: Comparisons sane?
    {
        persp3d_add_box (SP_PERSP3D(ref), box);
        /* Note: This sometimes leads to attempts to add boxes twice to the list of selected/transformed
           boxes in a perspectives, but this should be uncritical. */
        persp3d_add_box_transform (SP_PERSP3D(ref), box);
    }
}

static void
box3d_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* FIXME?: Perhaps the display updates of box sides should be instantiated from here, but this
           causes evil update loops so it's all done from box3d_position_set, which is called from
           various other places (like the handlers in object-edit.cpp, vanishing-point.cpp, etc. */

    }

    // Invoke parent method
    if (((SPObjectClass *) (parent_class))->update)
        ((SPObjectClass *) (parent_class))->update(object, ctx, flags);
}


static Inkscape::XML::Node *box3d_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SPBox3D *box = SP_BOX3D(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        // this is where we end up when saving as plain SVG (also in other circumstances?)
        // thus we don' set "sodipodi:type" so that the box is only saved as an ordinary svg:g
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_OBJECT_DOCUMENT(object));
        repr = xml_doc->createElement("svg:g");
    }

    if (flags & SP_OBJECT_WRITE_EXT) {

        if (box->persp_href) {
            repr->setAttribute("inkscape:perspectiveID", box->persp_href);
        } else {
            /* box is not yet linked to a perspective; use the document's current perspective */
            SPDocument *doc = inkscape_active_document();
            if (box->persp_ref->getURI()) {
                gchar *uri_string = box->persp_ref->getURI()->toString();
                repr->setAttribute("inkscape:perspectiveID", uri_string);
                g_free(uri_string);
            } else if (doc) {
                Inkscape::XML::Node *persp_repr = SP_OBJECT_REPR(doc->current_persp3d);
                const gchar *persp_id = persp_repr->attribute("id");
                gchar *href = g_strdup_printf("#%s", persp_id);
                repr->setAttribute("inkscape:perspectiveID", href);
                g_free(href);
            } else {
                g_print ("No active document while creating perspective!!!\n");
            }
        }

        gchar *coordstr0 = box->orig_corner0.coord_string();
        gchar *coordstr7 = box->orig_corner7.coord_string();
        repr->setAttribute("inkscape:corner0", coordstr0);
        repr->setAttribute("inkscape:corner7", coordstr7);
        g_free(coordstr0);
        g_free(coordstr7);

        box->orig_corner0.normalize();
        box->orig_corner7.normalize();

        box->save_corner0 = box->orig_corner0;
        box->save_corner7 = box->orig_corner7;
    }

    if (((SPObjectClass *) (parent_class))->write) {
        ((SPObjectClass *) (parent_class))->write(object, repr, flags);
    }

    return repr;
}

static gchar *
box3d_description(SPItem *item)
{
    g_return_val_if_fail(SP_IS_BOX3D(item), NULL);

    return g_strdup(_("<b>3D Box</b>"));
}

void
box3d_position_set (SPBox3D *box)
{
    /* This draws the curve and calls requestDisplayUpdate() for each side (the latter is done in
       box3d_side_position_set() to avoid update conflicts with the parent box) */
    for (SPObject *child = sp_object_first_child(SP_OBJECT (box)); child != NULL; child = SP_OBJECT_NEXT(child) ) {
        box3d_side_position_set (SP_BOX3D_SIDE (child));
    }
}

static NR::Matrix
box3d_set_transform(SPItem *item, NR::Matrix const &xform)
{
    SPBox3D *box = SP_BOX3D(item);

    /* check whether we need to unlink any boxes from their perspectives */
    Persp3D *persp = box3d_get_perspective(box);
    Persp3D *transf_persp;

    if (!persp3d_has_all_boxes_in_selection (persp)) {
        std::list<SPBox3D *> selboxes = sp_desktop_selection(inkscape_active_desktop())->box3DList();

        /* create a new perspective as a copy of the current one and link the selected boxes to it */
        transf_persp = persp3d_create_xml_element (SP_OBJECT_DOCUMENT(persp), persp);

        for (std::list<SPBox3D *>::iterator b = selboxes.begin(); b != selboxes.end(); ++b) {
            box3d_switch_perspectives(*b, persp, transf_persp);
        }
    } else {
        transf_persp = persp;
    }

    /* only transform the perspective once, even if it has several selected boxes */
    if(!persp3d_was_transformed (transf_persp)) {
        /* concatenate the affine transformation with the perspective mapping; this
           function also triggers repr updates of boxes and the perspective itself */
        persp3d_apply_affine_transformation(transf_persp, xform);
    }

    box3d_mark_transformed(box);

    if (persp3d_all_transformed(transf_persp)) {
        /* all boxes were transformed; make perspective sensitive for further transformations */
        persp3d_unset_transforms(transf_persp);
    }

    NR::Matrix ret(NR::transform(xform));
    gdouble const sw = hypot(ret[0], ret[1]);
    gdouble const sh = hypot(ret[2], ret[3]);

    SPItem *sideitem = NULL;
    for (SPObject *side = sp_object_first_child(box); side != NULL; side = SP_OBJECT_NEXT(side)) {
        sideitem = SP_ITEM(side);

        // Adjust stroke width
        sp_item_adjust_stroke(sideitem, sqrt(fabs(sw * sh)));

        // Adjust pattern fill
        sp_item_adjust_pattern(sideitem, xform);

        // Adjust gradient fill
        sp_item_adjust_gradient(sideitem, xform);

        // Adjust LPE
        sp_item_adjust_livepatheffect(item, xform);
    }

    return NR::identity();
}

Proj::Pt3
box3d_get_proj_corner (guint id, Proj::Pt3 const &c0, Proj::Pt3 const &c7) {
    return Proj::Pt3 ((id & Box3D::X) ? c7[Proj::X] : c0[Proj::X],
                      (id & Box3D::Y) ? c7[Proj::Y] : c0[Proj::Y],
                      (id & Box3D::Z) ? c7[Proj::Z] : c0[Proj::Z],
                      1.0);
}

Proj::Pt3
box3d_get_proj_corner (SPBox3D const *box, guint id) {
    return Proj::Pt3 ((id & Box3D::X) ? box->orig_corner7[Proj::X] : box->orig_corner0[Proj::X],
                      (id & Box3D::Y) ? box->orig_corner7[Proj::Y] : box->orig_corner0[Proj::Y],
                      (id & Box3D::Z) ? box->orig_corner7[Proj::Z] : box->orig_corner0[Proj::Z],
                      1.0);
}

NR::Point
box3d_get_corner_screen (SPBox3D const *box, guint id, bool item_coords) {
    Proj::Pt3 proj_corner (box3d_get_proj_corner (box, id));
    if (!box3d_get_perspective(box)) {
        return NR::Point (NR_HUGE, NR_HUGE);
    }
    NR::Matrix const i2d (sp_item_i2d_affine (SP_ITEM(box)));
    if (item_coords) {
        return box3d_get_perspective(box)->tmat.image(proj_corner).affine() * i2d.inverse();
    } else {
        return box3d_get_perspective(box)->tmat.image(proj_corner).affine();
    }
}

Proj::Pt3
box3d_get_proj_center (SPBox3D *box) {
    box->orig_corner0.normalize();
    box->orig_corner7.normalize();
    return Proj::Pt3 ((box->orig_corner0[Proj::X] + box->orig_corner7[Proj::X]) / 2,
                      (box->orig_corner0[Proj::Y] + box->orig_corner7[Proj::Y]) / 2,
                      (box->orig_corner0[Proj::Z] + box->orig_corner7[Proj::Z]) / 2,
                      1.0);
}

NR::Point
box3d_get_center_screen (SPBox3D *box) {
    Proj::Pt3 proj_center (box3d_get_proj_center (box));
    if (!box3d_get_perspective(box)) {
        return NR::Point (NR_HUGE, NR_HUGE);
    }
    NR::Matrix const i2d (sp_item_i2d_affine (SP_ITEM(box)));
    return box3d_get_perspective(box)->tmat.image(proj_center).affine() * i2d.inverse();
}

/*
 * To keep the snappoint from jumping randomly between the two lines when the mouse pointer is close to
 * their intersection, we remember the last snapped line and keep snapping to this specific line as long
 * as the distance from the intersection to the mouse pointer is less than remember_snap_threshold.
 */

// Should we make the threshold settable in the preferences?
static double remember_snap_threshold = 30;
static guint remember_snap_index = 0;

static Proj::Pt3
box3d_snap (SPBox3D *box, int id, Proj::Pt3 const &pt_proj, Proj::Pt3 const &start_pt) {
    double z_coord = start_pt[Proj::Z];
    double diff_x = box->save_corner7[Proj::X] - box->save_corner0[Proj::X];
    double diff_y = box->save_corner7[Proj::Y] - box->save_corner0[Proj::Y];
    double x_coord = start_pt[Proj::X];
    double y_coord = start_pt[Proj::Y];
    Proj::Pt3 A_proj (x_coord,          y_coord,          z_coord, 1.0);
    Proj::Pt3 B_proj (x_coord + diff_x, y_coord,          z_coord, 1.0);
    Proj::Pt3 C_proj (x_coord + diff_x, y_coord + diff_y, z_coord, 1.0);
    Proj::Pt3 D_proj (x_coord,          y_coord + diff_y, z_coord, 1.0);
    Proj::Pt3 E_proj (x_coord - diff_x, y_coord + diff_y, z_coord, 1.0);

    Persp3D *persp = box3d_get_perspective(box);
    NR::Point A = persp->tmat.image(A_proj).affine();
    NR::Point B = persp->tmat.image(B_proj).affine();
    NR::Point C = persp->tmat.image(C_proj).affine();
    NR::Point D = persp->tmat.image(D_proj).affine();
    NR::Point E = persp->tmat.image(E_proj).affine();
    NR::Point pt = persp->tmat.image(pt_proj).affine();

    // TODO: Replace these lines between corners with lines from a corner to a vanishing point
    //       (this might help to prevent rounding errors if the box is small)
    Box3D::Line pl1(A, B);
    Box3D::Line pl2(A, D);
    Box3D::Line diag1(A, (id == -1 || (!(id & Box3D::X) == !(id & Box3D::Y))) ? C : E);
    Box3D::Line diag2(A, E); // diag2 is only taken into account if id equals -1, i.e., if we are snapping the center

    int num_snap_lines = (id != -1) ? 3 : 4;
    NR::Point snap_pts[num_snap_lines];

    snap_pts[0] = pl1.closest_to (pt);
    snap_pts[1] = pl2.closest_to (pt);
    snap_pts[2] = diag1.closest_to (pt);
    if (id == -1) {
        snap_pts[3] = diag2.closest_to (pt);
    }

    gdouble const zoom = inkscape_active_desktop()->current_zoom();

    // determine the distances to all potential snapping points
    double snap_dists[num_snap_lines];
    for (int i = 0; i < num_snap_lines; ++i) {
        snap_dists[i] = NR::L2 (snap_pts[i] - pt) * zoom;
    }

    // while we are within a given tolerance of the starting point,
    // keep snapping to the same point to avoid jumping
    bool within_tolerance = true;
    for (int i = 0; i < num_snap_lines; ++i) {
        if (snap_dists[i] > remember_snap_threshold) {
            within_tolerance = false;
            break;
        }
    }

    // find the closest snapping point
    int snap_index = -1;
    double snap_dist = NR_HUGE;
    for (int i = 0; i < num_snap_lines; ++i) {
        if (snap_dists[i] < snap_dist) {
            snap_index = i;
            snap_dist = snap_dists[i];
        }
    }

    // snap to the closest point (or the previously remembered one
    // if we are within tolerance of the starting point)
    NR::Point result;
    if (within_tolerance) {
        result = snap_pts[remember_snap_index];
    } else {
        remember_snap_index = snap_index;
        result = snap_pts[snap_index];
    }
    return box3d_get_perspective(box)->tmat.preimage (result, z_coord, Proj::Z);
}

void
box3d_set_corner (SPBox3D *box, const guint id, NR::Point const &new_pos, const Box3D::Axis movement, bool constrained) {
    g_return_if_fail ((movement != Box3D::NONE) && (movement != Box3D::XYZ));

    box->orig_corner0.normalize();
    box->orig_corner7.normalize();

    /* update corners 0 and 7 according to which handle was moved and to the axes of movement */
    if (!(movement & Box3D::Z)) {
        Proj::Pt3 pt_proj (box3d_get_perspective(box)->tmat.preimage (new_pos, (id < 4) ? box->orig_corner0[Proj::Z] :
                                                                      box->orig_corner7[Proj::Z], Proj::Z));
        if (constrained) {
            pt_proj = box3d_snap (box, id, pt_proj, box3d_get_proj_corner (id, box->save_corner0, box->save_corner7));
        }

        // normalizing pt_proj is essential because we want to mingle affine coordinates
        pt_proj.normalize();
        box->orig_corner0 = Proj::Pt3 ((id & Box3D::X) ? box->save_corner0[Proj::X] : pt_proj[Proj::X],
                                       (id & Box3D::Y) ? box->save_corner0[Proj::Y] : pt_proj[Proj::Y],
                                       box->save_corner0[Proj::Z],
                                       1.0);
        box->orig_corner7 = Proj::Pt3 ((id & Box3D::X) ? pt_proj[Proj::X] : box->save_corner7[Proj::X],
                                       (id & Box3D::Y) ? pt_proj[Proj::Y] : box->save_corner7[Proj::Y],
                                       box->save_corner7[Proj::Z],
                                       1.0);
    } else {
        Persp3D *persp = box3d_get_perspective(box);
        Box3D::PerspectiveLine pl(persp->tmat.image(
                                      box3d_get_proj_corner (id, box->save_corner0, box->save_corner7)).affine(),
                                  Proj::Z, persp);
        NR::Point new_pos_snapped(pl.closest_to(new_pos));
        Proj::Pt3 pt_proj (persp->tmat.preimage (new_pos_snapped,
                                          box3d_get_proj_corner (box, id)[(movement & Box3D::Y) ? Proj::X : Proj::Y],
                                          (movement & Box3D::Y) ? Proj::X : Proj::Y));
        bool corner0_move_x = !(id & Box3D::X) && (movement & Box3D::X);
        bool corner0_move_y = !(id & Box3D::Y) && (movement & Box3D::Y);
        bool corner7_move_x =  (id & Box3D::X) && (movement & Box3D::X);
        bool corner7_move_y =  (id & Box3D::Y) && (movement & Box3D::Y);
        // normalizing pt_proj is essential because we want to mingle affine coordinates
        pt_proj.normalize();
        box->orig_corner0 = Proj::Pt3 (corner0_move_x ? pt_proj[Proj::X] : box->orig_corner0[Proj::X],
                                       corner0_move_y ? pt_proj[Proj::Y] : box->orig_corner0[Proj::Y],
                                       (id & Box3D::Z) ? box->orig_corner0[Proj::Z] : pt_proj[Proj::Z],
                                       1.0);
        box->orig_corner7 = Proj::Pt3 (corner7_move_x ? pt_proj[Proj::X] : box->orig_corner7[Proj::X],
                                       corner7_move_y ? pt_proj[Proj::Y] : box->orig_corner7[Proj::Y],
                                       (id & Box3D::Z) ? pt_proj[Proj::Z] : box->orig_corner7[Proj::Z],
                                       1.0);
    }
    // FIXME: Should we update the box here? If so, how?
}

void box3d_set_center (SPBox3D *box, NR::Point const &new_pos, NR::Point const &old_pos, const Box3D::Axis movement, bool constrained) {
    g_return_if_fail ((movement != Box3D::NONE) && (movement != Box3D::XYZ));

    box->orig_corner0.normalize();
    box->orig_corner7.normalize();

    Persp3D *persp = box3d_get_perspective(box);
    if (!(movement & Box3D::Z)) {
        double coord = (box->orig_corner0[Proj::Z] + box->orig_corner7[Proj::Z]) / 2;
        double radx = (box->orig_corner7[Proj::X] - box->orig_corner0[Proj::X]) / 2;
        double rady = (box->orig_corner7[Proj::Y] - box->orig_corner0[Proj::Y]) / 2;

        Proj::Pt3 pt_proj (persp->tmat.preimage (new_pos, coord, Proj::Z));
        if (constrained) {
            Proj::Pt3 old_pos_proj (persp->tmat.preimage (old_pos, coord, Proj::Z));
            old_pos_proj.normalize();
            pt_proj = box3d_snap (box, -1, pt_proj, old_pos_proj);
        }
        // normalizing pt_proj is essential because we want to mingle affine coordinates
        pt_proj.normalize();
        box->orig_corner0 = Proj::Pt3 ((movement & Box3D::X) ? pt_proj[Proj::X] - radx : box->orig_corner0[Proj::X],
                                       (movement & Box3D::Y) ? pt_proj[Proj::Y] - rady : box->orig_corner0[Proj::Y],
                                       box->orig_corner0[Proj::Z],
                                       1.0);
        box->orig_corner7 = Proj::Pt3 ((movement & Box3D::X) ? pt_proj[Proj::X] + radx : box->orig_corner7[Proj::X],
                                       (movement & Box3D::Y) ? pt_proj[Proj::Y] + rady : box->orig_corner7[Proj::Y],
                                       box->orig_corner7[Proj::Z],
                                       1.0);
    } else {
        double coord = (box->orig_corner0[Proj::X] + box->orig_corner7[Proj::X]) / 2;
        double radz = (box->orig_corner7[Proj::Z] - box->orig_corner0[Proj::Z]) / 2;

        Box3D::PerspectiveLine pl(old_pos, Proj::Z, persp);
        NR::Point new_pos_snapped(pl.closest_to(new_pos));
        Proj::Pt3 pt_proj (persp->tmat.preimage (new_pos_snapped, coord, Proj::X));

        /* normalizing pt_proj is essential because we want to mingle affine coordinates */
        pt_proj.normalize();
        box->orig_corner0 = Proj::Pt3 (box->orig_corner0[Proj::X],
                                       box->orig_corner0[Proj::Y],
                                       pt_proj[Proj::Z] - radz,
                                       1.0);
        box->orig_corner7 = Proj::Pt3 (box->orig_corner7[Proj::X],
                                       box->orig_corner7[Proj::Y],
                                       pt_proj[Proj::Z] + radz,
                                       1.0);
    }
}

/*
 * Manipulates corner1 through corner4 to contain the indices of the corners
 * from which the perspective lines in the direction of 'axis' emerge
 */
void box3d_corners_for_PLs (const SPBox3D * box, Proj::Axis axis,
                            NR::Point &corner1, NR::Point &corner2, NR::Point &corner3, NR::Point &corner4)
{
    Persp3D *persp = box3d_get_perspective(box);
    g_return_if_fail (persp);
    //box->orig_corner0.normalize();
    //box->orig_corner7.normalize();
    double coord = (box->orig_corner0[axis] > box->orig_corner7[axis]) ?
        box->orig_corner0[axis] :
        box->orig_corner7[axis];

    Proj::Pt3 c1, c2, c3, c4;
    // FIXME: This can certainly be done more elegantly/efficiently than by a case-by-case analysis.
    switch (axis) {
        case Proj::X:
            c1 = Proj::Pt3 (coord, box->orig_corner0[Proj::Y], box->orig_corner0[Proj::Z], 1.0);
            c2 = Proj::Pt3 (coord, box->orig_corner7[Proj::Y], box->orig_corner0[Proj::Z], 1.0);
            c3 = Proj::Pt3 (coord, box->orig_corner7[Proj::Y], box->orig_corner7[Proj::Z], 1.0);
            c4 = Proj::Pt3 (coord, box->orig_corner0[Proj::Y], box->orig_corner7[Proj::Z], 1.0);
            break;
        case Proj::Y:
            c1 = Proj::Pt3 (box->orig_corner0[Proj::X], coord, box->orig_corner0[Proj::Z], 1.0);
            c2 = Proj::Pt3 (box->orig_corner7[Proj::X], coord, box->orig_corner0[Proj::Z], 1.0);
            c3 = Proj::Pt3 (box->orig_corner7[Proj::X], coord, box->orig_corner7[Proj::Z], 1.0);
            c4 = Proj::Pt3 (box->orig_corner0[Proj::X], coord, box->orig_corner7[Proj::Z], 1.0);
            break;
        case Proj::Z:
            c1 = Proj::Pt3 (box->orig_corner7[Proj::X], box->orig_corner7[Proj::Y], coord, 1.0);
            c2 = Proj::Pt3 (box->orig_corner7[Proj::X], box->orig_corner0[Proj::Y], coord, 1.0);
            c3 = Proj::Pt3 (box->orig_corner0[Proj::X], box->orig_corner0[Proj::Y], coord, 1.0);
            c4 = Proj::Pt3 (box->orig_corner0[Proj::X], box->orig_corner7[Proj::Y], coord, 1.0);
            break;
        default:
            return;
    }
    corner1 = persp->tmat.image(c1).affine();
    corner2 = persp->tmat.image(c2).affine();
    corner3 = persp->tmat.image(c3).affine();
    corner4 = persp->tmat.image(c4).affine();
}

/* Auxiliary function: Checks whether the half-line from A to B crosses the line segment joining C and D */
static bool
box3d_half_line_crosses_joining_line (Geom::Point const &A, Geom::Point const &B,
                                      Geom::Point const &C, Geom::Point const &D) {
    Geom::Point E; // the point of intersection
    Geom::Point n0 = (B - A).ccw();
    double d0 = dot(n0,A);

    Geom::Point n1 = (D - C).ccw();
    double d1 = dot(n1,C);
    Geom::IntersectorKind intersects = Geom::line_intersection(n0, d0, n1, d1, E);
    if (intersects == Geom::coincident || intersects == Geom::parallel) {
        return false;
    }

    if ((dot(C,n0) < d0) == (dot(D,n0) < d0)) {
        // C and D lie on the same side of the line AB
        return false;
    }
    if ((dot(A,n1) < d1) != (dot(B,n1) < d1)) {
        // A and B lie on different sides of the line CD
        return true;
    } else if (Geom::distance(E,A) < Geom::distance(E,B)) {
        // The line CD passes on the "wrong" side of A
        return false;
    }

    // The line CD passes on the "correct" side of A
    return true;
}

static bool
box3d_XY_axes_are_swapped (SPBox3D *box) {
    Persp3D *persp = box3d_get_perspective(box);
    g_return_val_if_fail(persp, false);
    Box3D::PerspectiveLine l1(box3d_get_corner_screen(box, 3, false), Proj::X, persp);
    Box3D::PerspectiveLine l2(box3d_get_corner_screen(box, 3, false), Proj::Y, persp);
    NR::Point v1(l1.direction());
    NR::Point v2(l2.direction());
    v1.normalize();
    v2.normalize();

    return (v1[NR::X]*v2[NR::Y] - v1[NR::Y]*v2[NR::X] > 0);
}

static inline void
box3d_aux_set_z_orders (int z_orders[6], int a, int b, int c, int d, int e, int f) {
    z_orders[0] = a;
    z_orders[1] = b;
    z_orders[2] = c;
    z_orders[3] = d;
    z_orders[4] = e;
    z_orders[5] = f;
}

static inline void
box3d_swap_z_orders (int z_orders[6]) {
    int tmp;
    for (int i = 0; i < 3; ++i) {
        tmp = z_orders[i];
        z_orders[i] = z_orders[5-i];
        z_orders[5-i] = tmp;
    }
}

/*
 * In standard perspective we have:
 * 2 = front face
 * 1 = top face
 * 0 = left face
 * 3 = right face
 * 4 = bottom face
 * 5 = rear face
 */

/* All VPs infinite */
static void
box3d_set_new_z_orders_case0 (SPBox3D *box, int z_orders[6], Box3D::Axis central_axis) {
    Persp3D *persp = box3d_get_perspective(box);
    NR::Point xdir(persp3d_get_infinite_dir(persp, Proj::X));
    NR::Point ydir(persp3d_get_infinite_dir(persp, Proj::Y));
    NR::Point zdir(persp3d_get_infinite_dir(persp, Proj::Z));

    bool swapped = box3d_XY_axes_are_swapped(box);

    switch(central_axis) {
        case Box3D::X:
            if (!swapped) {
                box3d_aux_set_z_orders (z_orders, 2, 0, 4, 1, 3, 5);
            } else {
                box3d_aux_set_z_orders (z_orders, 3, 1, 5, 2, 4, 0);
            }
            break;
        case Box3D::Y:
            if (!swapped) {
                box3d_aux_set_z_orders (z_orders, 2, 3, 1, 4, 0, 5);
            } else {
                box3d_aux_set_z_orders (z_orders, 5, 0, 4, 1, 3, 2);
            }
            break;
        case Box3D::Z:
            if (!swapped) {
                box3d_aux_set_z_orders (z_orders, 2, 0, 1, 4, 3, 5);
            } else {
                box3d_aux_set_z_orders (z_orders, 5, 3, 4, 1, 0, 2);
            }
            break;
        case Box3D::NONE:
            if (!swapped) {
                box3d_aux_set_z_orders (z_orders, 2, 3, 4, 1, 0, 5);
            } else {
                box3d_aux_set_z_orders (z_orders, 5, 0, 1, 4, 3, 2);
            }
            break;
        default:
            g_assert_not_reached();
            break;
    }
}

/* Precisely one finite VP */
static void
box3d_set_new_z_orders_case1 (SPBox3D *box, int z_orders[6], Box3D::Axis central_axis, Box3D::Axis fin_axis) {
    Persp3D *persp = box3d_get_perspective(box);
    NR::Point vp(persp3d_get_VP(persp, Box3D::toProj(fin_axis)).affine());

    // note: in some of the case distinctions below we rely upon the fact that oaxis1 and oaxis2 are ordered
    Box3D::Axis oaxis1 = Box3D::get_remaining_axes(fin_axis).first;
    Box3D::Axis oaxis2 = Box3D::get_remaining_axes(fin_axis).second;
    int inside1 = 0;
    int inside2 = 0;
    inside1 = box3d_pt_lies_in_PL_sector (box, vp, 3, 3 ^ oaxis2, oaxis1);
    inside2 = box3d_pt_lies_in_PL_sector (box, vp, 3, 3 ^ oaxis1, oaxis2);

    bool swapped = box3d_XY_axes_are_swapped(box);

    switch(central_axis) {
        case Box3D::X:
            if (!swapped) {
                box3d_aux_set_z_orders (z_orders, 2, 4, 0, 1, 3, 5);
            } else {
                box3d_aux_set_z_orders (z_orders, 5, 3, 1, 0, 2, 4);
            }
            break;
        case Box3D::Y:
            if (inside2 > 0) {
                box3d_aux_set_z_orders (z_orders, 1, 2, 3, 0, 5, 4);
            } else if (inside2 < 0) {
                box3d_aux_set_z_orders (z_orders, 2, 3, 1, 4, 0, 5);
            } else {
                if (!swapped) {
                    box3d_aux_set_z_orders (z_orders, 2, 3, 1, 5, 0, 4);
                } else {
                    box3d_aux_set_z_orders (z_orders, 5, 0, 4, 1, 3, 2);
                }
            }
            break;
        case Box3D::Z:
            if (inside2) {
                if (!swapped) {
                    box3d_aux_set_z_orders (z_orders, 2, 1, 3, 0, 4, 5);
                } else {
                    box3d_aux_set_z_orders (z_orders, 5, 3, 4, 0, 1, 2);
                }
            } else if (inside1) {
                if (!swapped) {
                    box3d_aux_set_z_orders (z_orders, 2, 0, 1, 4, 3, 5);
                } else {
                    box3d_aux_set_z_orders (z_orders, 5, 3, 4, 1, 0, 2);
                }
            } else {
                // "regular" case
                if (!swapped) {
                    box3d_aux_set_z_orders (z_orders, 0, 1, 2, 5, 4, 3);
                } else {
                    box3d_aux_set_z_orders (z_orders, 5, 3, 4, 0, 2, 1);
                }
            }
            break;
        case Box3D::NONE:
            if (!swapped) {
                box3d_aux_set_z_orders (z_orders, 2, 3, 4, 5, 0, 1);
            } else {
                box3d_aux_set_z_orders (z_orders, 5, 0, 1, 3, 2, 4);
            }
            break;
        default:
            g_assert_not_reached();
    }
}

/* Precisely 2 finite VPs */
static void
box3d_set_new_z_orders_case2 (SPBox3D *box, int z_orders[6], Box3D::Axis central_axis, Box3D::Axis /*infinite_axis*/) {
    Persp3D *persp = box3d_get_perspective(box);

    NR::Point c3(box3d_get_corner_screen(box, 3, false));
    NR::Point xdir(persp3d_get_PL_dir_from_pt(persp, c3, Proj::X));
    NR::Point ydir(persp3d_get_PL_dir_from_pt(persp, c3, Proj::Y));
    NR::Point zdir(persp3d_get_PL_dir_from_pt(persp, c3, Proj::Z));

    bool swapped = box3d_XY_axes_are_swapped(box);

    int insidexy = box3d_VP_lies_in_PL_sector (box, Proj::X, 3, 3 ^ Box3D::Z, Box3D::Y);
    //int insidexz = box3d_VP_lies_in_PL_sector (box, Proj::X, 3, 3 ^ Box3D::Y, Box3D::Z);

    int insideyx = box3d_VP_lies_in_PL_sector (box, Proj::Y, 3, 3 ^ Box3D::Z, Box3D::X);
    int insideyz = box3d_VP_lies_in_PL_sector (box, Proj::Y, 3, 3 ^ Box3D::X, Box3D::Z);

    //int insidezx = box3d_VP_lies_in_PL_sector (box, Proj::Z, 3, 3 ^ Box3D::Y, Box3D::X);
    int insidezy = box3d_VP_lies_in_PL_sector (box, Proj::Z, 3, 3 ^ Box3D::X, Box3D::Y);

    switch(central_axis) {
        case Box3D::X:
            if (!swapped) {
                if (insidezy == -1) {
                    box3d_aux_set_z_orders (z_orders, 2, 4, 0, 1, 3, 5);
                } else if (insidexy == 1) {
                    box3d_aux_set_z_orders (z_orders, 2, 4, 0, 5, 1, 3);
                } else {
                    box3d_aux_set_z_orders (z_orders, 2, 4, 0, 1, 3, 5);
                }
            } else {
                if (insideyz == -1) {
                    box3d_aux_set_z_orders (z_orders, 3, 1, 5, 0, 2, 4);
                } else {
                    if (!swapped) {
                        box3d_aux_set_z_orders (z_orders, 3, 1, 5, 2, 4, 0);
                    } else {
                        if (insidexy == 0) {
                            box3d_aux_set_z_orders (z_orders, 3, 5, 1, 0, 2, 4);
                        } else {
                            box3d_aux_set_z_orders (z_orders, 3, 1, 5, 0, 2, 4);
                        }
                    }
                }
            }
            break;
        case Box3D::Y:
            if (!swapped) {
                if (insideyz == 1) {
                    box3d_aux_set_z_orders (z_orders, 2, 3, 1, 0, 5, 4);
                } else {
                    box3d_aux_set_z_orders (z_orders, 2, 3, 1, 5, 0, 4);
                }
            } else {
                if (insideyx == 1) {
                    box3d_aux_set_z_orders (z_orders, 4, 0, 5, 1, 3, 2);
                } else {
                    box3d_aux_set_z_orders (z_orders, 5, 0, 4, 1, 3, 2);
                }
            }
            break;
        case Box3D::Z:
            if (!swapped) {
                if (insidezy == 1) {
                    box3d_aux_set_z_orders (z_orders, 2, 1, 0, 4, 3, 5);
                } else if (insidexy == -1) {
                    box3d_aux_set_z_orders (z_orders, 2, 1, 0, 5, 4, 3);
                } else {
                    box3d_aux_set_z_orders (z_orders, 2, 0, 1, 5, 3, 4);
                }
            } else {
                box3d_aux_set_z_orders (z_orders, 3, 4, 5, 1, 0, 2);
            }
            break;
        case Box3D::NONE:
            if (!swapped) {
                box3d_aux_set_z_orders (z_orders, 2, 3, 4, 1, 0, 5);
            } else {
                box3d_aux_set_z_orders (z_orders, 5, 0, 1, 4, 3, 2);
            }
            break;
        default:
            g_assert_not_reached();
            break;
    }
}

/*
 * It can happen that during dragging the box is everted.
 * In this case the opposite sides in this direction need to be swapped
 */
static Box3D::Axis
box3d_everted_directions (SPBox3D *box) {
    Box3D::Axis ev = Box3D::NONE;

    box->orig_corner0.normalize();
    box->orig_corner7.normalize();

    if (box->orig_corner0[Proj::X] < box->orig_corner7[Proj::X])
        ev = (Box3D::Axis) (ev ^ Box3D::X);
    if (box->orig_corner0[Proj::Y] < box->orig_corner7[Proj::Y])
        ev = (Box3D::Axis) (ev ^ Box3D::Y);
    if (box->orig_corner0[Proj::Z] > box->orig_corner7[Proj::Z]) // FIXME: Remove the need to distinguish signs among the cases
        ev = (Box3D::Axis) (ev ^ Box3D::Z);

    return ev;
}

static void
box3d_swap_sides(int z_orders[6], Box3D::Axis axis) {
    int pos1 = -1;
    int pos2 = -1;

    for (int i = 0; i < 6; ++i) {
        if (!(Box3D::int_to_face(z_orders[i]) & axis)) {
            if (pos1 == -1) {
                pos1 = i;
            } else {
                pos2 = i;
                break;
            }
        }
    }

    int tmp = z_orders[pos1];
    z_orders[pos1] = z_orders[pos2];
    z_orders[pos2] = tmp;
}


bool
box3d_recompute_z_orders (SPBox3D *box) {
    Persp3D *persp = box3d_get_perspective(box);

    if (!persp)
        return false;

    int z_orders[6];

    NR::Point c3(box3d_get_corner_screen(box, 3, false));

    // determine directions from corner3 to the VPs
    int num_finite = 0;
    Box3D::Axis axis_finite = Box3D::NONE;
    Box3D::Axis axis_infinite = Box3D::NONE;
    NR::Point dirs[3];
    for (int i = 0; i < 3; ++i) {
        dirs[i] = persp3d_get_PL_dir_from_pt(persp, c3, Box3D::toProj(Box3D::axes[i]));
        if (persp3d_VP_is_finite(persp, Proj::axes[i])) {
            num_finite++;
            axis_finite = Box3D::axes[i];
        } else {
            axis_infinite = Box3D::axes[i];
        }
    }

    // determine the "central" axis (if there is one)
    Box3D::Axis central_axis = Box3D::NONE;
    if(Box3D::lies_in_sector(dirs[0], dirs[1], dirs[2])) {
        central_axis = Box3D::Z;
    } else if(Box3D::lies_in_sector(dirs[1], dirs[2], dirs[0])) {
        central_axis = Box3D::X;
    } else if(Box3D::lies_in_sector(dirs[2], dirs[0], dirs[1])) {
        central_axis = Box3D::Y;
    }

    switch (num_finite) {
        case 0:
            // TODO: Remark: In this case (and maybe one of the others, too) the z-orders for all boxes
            //               coincide, hence only need to be computed once in a more central location.
            box3d_set_new_z_orders_case0(box, z_orders, central_axis);
            break;
        case 1:
            box3d_set_new_z_orders_case1(box, z_orders, central_axis, axis_finite);
            break;
        case 2:
        case 3:
            box3d_set_new_z_orders_case2(box, z_orders, central_axis, axis_infinite);
            break;
        default:
        /*
         * For each VP F, check wether the half-line from the corner3 to F crosses the line segment
         * joining the other two VPs. If this is the case, it determines the "central" corner from
         * which the visible sides can be deduced. Otherwise, corner3 is the central corner.
         */
        // FIXME: We should eliminate the use of NR::Point altogether
        Box3D::Axis central_axis = Box3D::NONE;
        NR::Point vp_x = persp3d_get_VP(persp, Proj::X).affine();
        NR::Point vp_y = persp3d_get_VP(persp, Proj::Y).affine();
        NR::Point vp_z = persp3d_get_VP(persp, Proj::Z).affine();
        Geom::Point vpx(vp_x[NR::X], vp_x[NR::Y]);
        Geom::Point vpy(vp_y[NR::X], vp_y[NR::Y]);
        Geom::Point vpz(vp_z[NR::X], vp_z[NR::Y]);

        NR::Point c3 = box3d_get_corner_screen(box, 3, false);
        Geom::Point corner3(c3[NR::X], c3[NR::Y]);

        if (box3d_half_line_crosses_joining_line (corner3, vpx, vpy, vpz)) {
            central_axis = Box3D::X;
        } else if (box3d_half_line_crosses_joining_line (corner3, vpy, vpz, vpx)) {
            central_axis = Box3D::Y;
        } else if (box3d_half_line_crosses_joining_line (corner3, vpz, vpx, vpy)) {
            central_axis = Box3D::Z;
        }

        unsigned int central_corner = 3 ^ central_axis;
        if (central_axis == Box3D::Z) {
            central_corner = central_corner ^ Box3D::XYZ;
        }
        if (box3d_XY_axes_are_swapped(box)) {
            central_corner = central_corner ^ Box3D::XYZ;
        }

        NR::Point c1(box3d_get_corner_screen(box, 1, false));
        NR::Point c2(box3d_get_corner_screen(box, 2, false));
        NR::Point c7(box3d_get_corner_screen(box, 7, false));

        Geom::Point corner1(c1[NR::X], c1[NR::Y]);
        Geom::Point corner2(c2[NR::X], c2[NR::Y]);
        Geom::Point corner7(c7[NR::X], c7[NR::Y]);
        // FIXME: At present we don't use the information about central_corner computed above.
        switch (central_axis) {
            case Box3D::Y:
                if (!box3d_half_line_crosses_joining_line(vpz, vpy, corner3, corner2)) {
                    box3d_aux_set_z_orders (z_orders, 2, 3, 1, 5, 0, 4);
                } else {
                    // degenerate case
                    box3d_aux_set_z_orders (z_orders, 2, 1, 3, 0, 5, 4);
                }
                break;

            case Box3D::Z:
                if (box3d_half_line_crosses_joining_line(vpx, vpz, corner3, corner1)) {
                    // degenerate case
                    box3d_aux_set_z_orders (z_orders, 2, 0, 1, 4, 3, 5);
                } else if (box3d_half_line_crosses_joining_line(vpx, vpy, corner3, corner7)) {
                    // degenerate case
                    box3d_aux_set_z_orders (z_orders, 2, 1, 0, 5, 3, 4);
                } else {
                    box3d_aux_set_z_orders (z_orders, 2, 1, 0, 3, 4, 5);
                }
                break;

            case Box3D::X:
                if (box3d_half_line_crosses_joining_line(vpz, vpx, corner3, corner1)) {
                    // degenerate case
                    box3d_aux_set_z_orders (z_orders, 2, 1, 0, 4, 5, 3);
                } else {
                    box3d_aux_set_z_orders (z_orders, 2, 4, 0, 5, 1, 3);
                }
                break;

            case Box3D::NONE:
                box3d_aux_set_z_orders (z_orders, 2, 3, 4, 1, 0, 5);
                break;

            default:
                g_assert_not_reached();
                break;
        } // end default case
    }

    // TODO: If there are still errors in z-orders of everted boxes, we need to choose a variable corner
    //       instead of the hard-coded corner #3 in the computations above
    Box3D::Axis ev = box3d_everted_directions(box);
    for (int i = 0; i < 3; ++i) {
        if (ev & Box3D::axes[i]) {
            box3d_swap_sides(z_orders, Box3D::axes[i]);
        }
    }

    // Check whether anything actually changed
    for (int i = 0; i < 6; ++i) {
        if (box->z_orders[i] != z_orders[i]) {
            for (int j = i; j < 6; ++j) {
                box->z_orders[j] = z_orders[j];
            }
            return true;
        }
    }
    return false;
}

static std::map<int, Box3DSide *>
box3d_get_sides (SPBox3D *box) {
    std::map<int, Box3DSide *> sides;
    for (SPObject *side = sp_object_first_child(box); side != NULL; side = SP_OBJECT_NEXT(side)) {
        sides[Box3D::face_to_int(sp_repr_get_int_attribute(SP_OBJECT_REPR(side),
                                                           "inkscape:box3dsidetype", -1))] = SP_BOX3D_SIDE(side);
    }
    sides.erase(-1);
    return sides;
}


// TODO: Check whether the box is everted in any direction and swap the sides opposite to this direction
void
box3d_set_z_orders (SPBox3D *box) {
    // For efficiency reasons, we only set the new z-orders if something really changed
    if (box3d_recompute_z_orders (box)) {
        std::map<int, Box3DSide *> sides = box3d_get_sides(box);
        std::map<int, Box3DSide *>::iterator side;
        for (unsigned int i = 0; i < 6; ++i) {
            side = sides.find(box->z_orders[i]);
            if (side != sides.end()) {
                SP_ITEM((*side).second)->lowerToBottom();
            }
        }
    }
}

/*
 * Auxiliary function for z-order recomputing:
 * Determines whether \a pt lies in the sector formed by the two PLs from the corners with IDs
 * \a i21 and \a id2 to the VP in direction \a axis. If the VP is infinite, we say that \a pt
 * lies in the sector if it lies between the two (parallel) PLs.
 * \ret *  0 if \a pt doesn't lie in the sector
 *      *  1 if \a pt lies in the sector and either VP is finite of VP is infinite and the direction
 *           from the edge between the two corners to \a pt points towards the VP
 *      * -1 otherwise
 */
// TODO: Maybe it would be useful to have a similar method for projective points pt because then we
//       can use it for VPs and perhaps merge the case distinctions during z-order recomputation.
int
box3d_pt_lies_in_PL_sector (SPBox3D const *box, NR::Point const &pt, int id1, int id2, Box3D::Axis axis) {
    Persp3D *persp = box3d_get_perspective(box);

    // the two corners
    NR::Point c1(box3d_get_corner_screen(box, id1, false));
    NR::Point c2(box3d_get_corner_screen(box, id2, false));

    int ret = 0;
    if (persp3d_VP_is_finite(persp, Box3D::toProj(axis))) {
        NR::Point vp(persp3d_get_VP(persp, Box3D::toProj(axis)).affine());
        NR::Point v1(c1 - vp);
        NR::Point v2(c2 - vp);
        NR::Point w(pt - vp);
        ret = static_cast<int>(Box3D::lies_in_sector(v1, v2, w));
    } else {
        Box3D::PerspectiveLine pl1(c1, Box3D::toProj(axis), persp);
        Box3D::PerspectiveLine pl2(c2, Box3D::toProj(axis), persp);
        if (pl1.lie_on_same_side(pt, c2) && pl2.lie_on_same_side(pt, c1)) {
            // test whether pt lies "towards" or "away from" the VP
            Box3D::Line edge(c1,c2);
            NR::Point c3(box3d_get_corner_screen(box, id1 ^ axis, false));
            if (edge.lie_on_same_side(pt, c3)) {
                ret = 1;
            } else {
                ret = -1;
            }
        }
    }
    return ret;
}

int
box3d_VP_lies_in_PL_sector (SPBox3D const *box, Proj::Axis vpdir, int id1, int id2, Box3D::Axis axis) {
    Persp3D *persp = box3d_get_perspective(box);

    if (!persp3d_VP_is_finite(persp, vpdir)) {
        return 0;
    } else {
        return box3d_pt_lies_in_PL_sector(box, persp3d_get_VP(persp, vpdir).affine(), id1, id2, axis);
    }
}

/* swap the coordinates of corner0 and corner7 along the specified axis */
static void
box3d_swap_coords(SPBox3D *box, Proj::Axis axis, bool smaller = true) {
    box->orig_corner0.normalize();
    box->orig_corner7.normalize();
    if ((box->orig_corner0[axis] < box->orig_corner7[axis]) != smaller) {
        double tmp = box->orig_corner0[axis];
        box->orig_corner0[axis] = box->orig_corner7[axis];
        box->orig_corner7[axis] = tmp;
    }
    // Should we also swap the coordinates of save_corner0 and save_corner7?
}

/* ensure that the coordinates of corner0 and corner7 are in the correct order (to prevent everted boxes) */
void
box3d_relabel_corners(SPBox3D *box) {
    box3d_swap_coords(box, Proj::X, false);
    box3d_swap_coords(box, Proj::Y, false);
    box3d_swap_coords(box, Proj::Z, true);
}

static void
box3d_check_for_swapped_coords(SPBox3D *box, Proj::Axis axis, bool smaller) {
    box->orig_corner0.normalize();
    box->orig_corner7.normalize();

    if ((box->orig_corner0[axis] < box->orig_corner7[axis]) != smaller) {
        box->swapped = (Box3D::Axis) (box->swapped | Proj::toAffine(axis));
    } else {
        box->swapped = (Box3D::Axis) (box->swapped & ~Proj::toAffine(axis));
    }
}

static void
box3d_exchange_coords(SPBox3D *box) {
    box->orig_corner0.normalize();
    box->orig_corner7.normalize();

    for (int i = 0; i < 3; ++i) {
        if (box->swapped & Box3D::axes[i]) {
            double tmp = box->orig_corner0[i];
            box->orig_corner0[i] = box->orig_corner7[i];
            box->orig_corner7[i] = tmp;
        }
    }
}

void
box3d_check_for_swapped_coords(SPBox3D *box) {
    box3d_check_for_swapped_coords(box, Proj::X, false);
    box3d_check_for_swapped_coords(box, Proj::Y, false);
    box3d_check_for_swapped_coords(box, Proj::Z, true);

    box3d_exchange_coords(box);
}

void
box3d_add_to_selection(SPBox3D *box) {
    Persp3D *persp = box3d_get_perspective(box);
    g_return_if_fail(persp);
    persp3d_add_box_transform(persp, box);
}

void
box3d_remove_from_selection(SPBox3D *box) {
    Persp3D *persp = box3d_get_perspective(box);
    if (!persp) {
        /* this can happen if a box is deleted through undo and the persp_ref is already detached;
           should we rebuild the boxes of each perspective in this case or is it safe to leave it alone? */
        return;
    }
    persp3d_remove_box_transform(persp, box);
}

void
box3d_mark_transformed(SPBox3D *box) {
    Persp3D *persp = box3d_get_perspective(box);
    g_return_if_fail(persp);
    persp3d_set_box_transformed(persp, box, true);
}

static void
box3d_extract_boxes_rec(SPObject *obj, std::list<SPBox3D *> &boxes) {
    if (SP_IS_BOX3D(obj)) {
        boxes.push_back(SP_BOX3D(obj));
    } else if (SP_IS_GROUP(obj)) {
        for (SPObject *child = sp_object_first_child(obj); child != NULL; child = SP_OBJECT_NEXT(child) ) {
            box3d_extract_boxes_rec(child, boxes);
        }
    }
}

std::list<SPBox3D *>
box3d_extract_boxes(SPObject *obj) {
    std::list<SPBox3D *> boxes;
    box3d_extract_boxes_rec(obj, boxes);
    return boxes;
}

Persp3D *
box3d_get_perspective(SPBox3D const *box) {
    return box->persp_ref->getObject();
}

void
box3d_switch_perspectives(SPBox3D *box, Persp3D *old_persp, Persp3D *new_persp, bool recompute_corners) {
    if (recompute_corners) {
        box->orig_corner0.normalize();
        box->orig_corner7.normalize();
        double z0 = box->orig_corner0[Proj::Z];
        double z7 = box->orig_corner7[Proj::Z];
        NR::Point corner0_screen = box3d_get_corner_screen(box, 0, false);
        NR::Point corner7_screen = box3d_get_corner_screen(box, 7, false);

        box->orig_corner0 = new_persp->tmat.preimage(corner0_screen, z0, Proj::Z);
        box->orig_corner7 = new_persp->tmat.preimage(corner7_screen, z7, Proj::Z);
    }

    persp3d_remove_box (old_persp, box);
    persp3d_add_box (new_persp, box);

    persp3d_remove_box_transform (old_persp, box);
    persp3d_add_box_transform (new_persp, box);

    gchar *href = g_strdup_printf("#%s", SP_OBJECT_REPR(new_persp)->attribute("id"));
    SP_OBJECT_REPR(box)->setAttribute("inkscape:perspectiveID", href);
    g_free(href);
}

/* Converts the 3D box to an ordinary SPGroup, adds it to the XML tree at the same position as
   the original box and deletes the latter */
Inkscape::XML::Node *
box3d_convert_to_group(SPBox3D *box) {
    SPDocument *doc = SP_OBJECT_DOCUMENT(box);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    // remember position of the box
    int pos = SP_OBJECT_REPR(box)->position();

    // remember important attributes
    Inkscape::XML::Node *repr_source = SP_OBJECT_REPR(box);
    gchar const *id = repr_source->attribute("id");
    gchar const *style = repr_source->attribute("style");
    gchar const *mask = repr_source->attribute("mask");
    gchar const *clip_path = repr_source->attribute("clip-path");

    // create a new group and add the sides (converted to ordinary paths) as its children
    Inkscape::XML::Node *grepr = xml_doc->createElement("svg:g");

    Inkscape::XML::Node *repr;
    for (SPObject *child = sp_object_first_child(SP_OBJECT(box)); child != NULL; child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_BOX3D_SIDE(child)) {
            repr = box3d_side_convert_to_path(SP_BOX3D_SIDE(child));
            grepr->appendChild(repr);
        } else {
            g_warning("Non-side item encountered as child of a 3D box.");
        }
    }

    // add the new group to the box's parent and set remembered position
    SPObject *parent = SP_OBJECT_PARENT(box);
    SP_OBJECT_REPR(parent)->appendChild(grepr);
    grepr->setPosition(pos);
    grepr->setAttribute("style", style);
    if (mask)
       grepr->setAttribute("mask", mask);
    if (clip_path)
       grepr->setAttribute("clip-path", clip_path);

    SP_OBJECT(box)->deleteObject(true);

    grepr->setAttribute("id", id);

    return grepr;
}

static inline void
box3d_push_back_corner_pair(SPBox3D *box, std::list<std::pair<Geom::Point, Geom::Point> > &pts, int c1, int c2) {
    pts.push_back(std::make_pair(box3d_get_corner_screen(box, c1, false).to_2geom(),
                                 box3d_get_corner_screen(box, c2, false).to_2geom()));
}

void
box3d_convert_to_guides(SPItem *item) {
    SPBox3D *box = SP_BOX3D(item);

    if (prefs_get_int_attribute("tools.shapes.3dbox", "convertguides", 1) == 0) {
        sp_item_convert_to_guides(SP_ITEM(box));
        return;
    }

    SPDocument *doc = SP_OBJECT_DOCUMENT(box);

    std::list<std::pair<Geom::Point, Geom::Point> > pts;

    /* perspective lines in X direction */
    box3d_push_back_corner_pair(box, pts, 0, 1);
    box3d_push_back_corner_pair(box, pts, 2, 3);
    box3d_push_back_corner_pair(box, pts, 4, 5);
    box3d_push_back_corner_pair(box, pts, 6, 7);

    /* perspective lines in Y direction */
    box3d_push_back_corner_pair(box, pts, 0, 2);
    box3d_push_back_corner_pair(box, pts, 1, 3);
    box3d_push_back_corner_pair(box, pts, 4, 6);
    box3d_push_back_corner_pair(box, pts, 5, 7);

    /* perspective lines in Z direction */
    box3d_push_back_corner_pair(box, pts, 0, 4);
    box3d_push_back_corner_pair(box, pts, 1, 5);
    box3d_push_back_corner_pair(box, pts, 2, 6);
    box3d_push_back_corner_pair(box, pts, 3, 7);

    sp_guide_pt_pairs_to_guides(doc, pts);
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
