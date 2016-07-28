/*
 * SVG <box3d> implementation
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
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
#include "ui/tools/box3d-tool.h"
#include "proj_pt.h"
#include "transf_mat_3x4.h"
#include "perspective-line.h"
#include "inkscape.h"
#include "persp3d.h"
#include "line-geometry.h"
#include "persp3d-reference.h"
#include "uri.h"
#include <2geom/line.h>
#include "sp-guide.h"
#include "sp-namedview.h"
#include "preferences.h"

#include "desktop.h"

#include "macros.h"

static void box3d_ref_changed(SPObject *old_ref, SPObject *ref, SPBox3D *box);

static gint counter = 0;

SPBox3D::SPBox3D() : SPGroup() {
    this->my_counter = 0;
    this->swapped = Box3D::NONE;

    this->persp_href = NULL;
    this->persp_ref = new Persp3DReference(this);

    /* we initialize the z-orders to zero so that they are updated during dragging */
    for (int i = 0; i < 6; ++i) {
        z_orders[i] = 0;
    }
}

SPBox3D::~SPBox3D() {
}

void SPBox3D::build(SPDocument *document, Inkscape::XML::Node *repr) {
    SPGroup::build(document, repr);

    my_counter = counter++;

    /* we initialize the z-orders to zero so that they are updated during dragging */
    for (int i = 0; i < 6; ++i) {
        z_orders[i] = 0;
    }

    // TODO: Create/link to the correct perspective

    if ( document ) {
        persp_ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(box3d_ref_changed), this));

        readAttr( "inkscape:perspectiveID" );
        readAttr( "inkscape:corner0" );
        readAttr( "inkscape:corner7" );
    }
}

void SPBox3D::release() {
    SPBox3D* object = this;
    SPBox3D *box = object;

    if (box->persp_href) {
        g_free(box->persp_href);
    }

    // We have to store this here because the Persp3DReference gets destroyed below, but we need to
    // access it to call persp3d_remove_box(), which cannot be called earlier because the reference
    // needs to be destroyed first.
    Persp3D *persp = box3d_get_perspective(box);

    if (box->persp_ref) {
        box->persp_ref->detach();
        delete box->persp_ref;
        box->persp_ref = NULL;
    }

    if (persp) {
        persp3d_remove_box (persp, box);
        /*
        // TODO: This deletes a perspective when the last box referring to it is gone. Eventually,
        // it would be nice to have this but currently it crashes when undoing/redoing box deletion
        // Reason: When redoing a box deletion, the associated perspective is deleted twice, first
        // by the following code and then again by the redo mechanism! Perhaps we should perform
        // deletion of the perspective from another location "outside" the undo/redo mechanism?
        if (persp->perspective_impl->boxes.empty()) {
            SPDocument *doc = box->document;
            persp->deleteObject();
            doc->setCurrentPersp3D(persp3d_document_first_persp(doc));
        }
        */
    }

    SPGroup::release();
}

void SPBox3D::set(unsigned int key, const gchar* value) {
    SPBox3D* object = this;
    SPBox3D *box = object;

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
            SPGroup::set(key, value);
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
        Persp3D *oldPersp = dynamic_cast<Persp3D *>(old_ref);
        if (oldPersp) {
            persp3d_remove_box(oldPersp, box);
        }
    }
    Persp3D *persp = dynamic_cast<Persp3D *>(ref);
    if ( persp && (ref != box) ) // FIXME: Comparisons sane?
    {
        persp3d_add_box(persp, box);
    }
}

void SPBox3D::update(SPCtx *ctx, guint flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* FIXME?: Perhaps the display updates of box sides should be instantiated from here, but this
           causes evil update loops so it's all done from box3d_position_set, which is called from
           various other places (like the handlers in object-edit.cpp, vanishing-point.cpp, etc. */

    }

    // Invoke parent method
    SPGroup::update(ctx, flags);
}

Inkscape::XML::Node* SPBox3D::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    SPBox3D* object = this;
    SPBox3D *box = object;

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        // this is where we end up when saving as plain SVG (also in other circumstances?)
        // thus we don' set "sodipodi:type" so that the box is only saved as an ordinary svg:g
        repr = xml_doc->createElement("svg:g");
    }

    if (flags & SP_OBJECT_WRITE_EXT) {

        if (box->persp_href) {
            repr->setAttribute("inkscape:perspectiveID", box->persp_href);
        } else {
            /* box is not yet linked to a perspective; use the document's current perspective */
            SPDocument *doc = object->document;
            if (box->persp_ref->getURI()) {
                gchar *uri_string = box->persp_ref->getURI()->toString();
                repr->setAttribute("inkscape:perspectiveID", uri_string);
                g_free(uri_string);
            } else {
                Glib::ustring href = "#";
                href += doc->getCurrentPersp3D()->getId();
                repr->setAttribute("inkscape:perspectiveID", href.c_str());
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

    SPGroup::write(xml_doc, repr, flags);

    return repr;
}

const char* SPBox3D::display_name() {
    return _("3D Box");
}

void box3d_position_set(SPBox3D *box)
{
    /* This draws the curve and calls requestDisplayUpdate() for each side (the latter is done in
       box3d_side_position_set() to avoid update conflicts with the parent box) */
    for ( SPObject *obj = box->firstChild(); obj; obj = obj->getNext() ) {
        Box3DSide *side = dynamic_cast<Box3DSide *>(obj);
        if (side) {
            box3d_side_position_set(side);
        }
    }
}

Geom::Affine SPBox3D::set_transform(Geom::Affine const &xform) {
    // We don't apply the transform to the box directly but instead to its perspective (which is
    // done in sp_selection_apply_affine). Here we only adjust strokes, patterns, etc.

    Geom::Affine ret(Geom::Affine(xform).withoutTranslation());
    gdouble const sw = hypot(ret[0], ret[1]);
    gdouble const sh = hypot(ret[2], ret[3]);

    for ( SPObject *child = firstChild(); child; child = child->getNext() ) {
        SPItem *childitem = dynamic_cast<SPItem *>(child);
        if (childitem) {
            // Adjust stroke width
            childitem->adjust_stroke(sqrt(fabs(sw * sh)));

            // Adjust pattern fill
            childitem->adjust_pattern(xform);

            // Adjust gradient fill
            childitem->adjust_gradient(xform);

            // Adjust LPE
            childitem->adjust_livepatheffect(xform);
        }
    }

    return Geom::identity();
}

static Proj::Pt3
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

Geom::Point
box3d_get_corner_screen (SPBox3D const *box, guint id, bool item_coords) {
    Proj::Pt3 proj_corner (box3d_get_proj_corner (box, id));
    if (!box3d_get_perspective(box)) {
        return Geom::Point (Geom::infinity(), Geom::infinity());
    }
    Geom::Affine const i2d(box->i2dt_affine ());
    if (item_coords) {
        return box3d_get_perspective(box)->perspective_impl->tmat.image(proj_corner).affine() * i2d.inverse();
    } else {
        return box3d_get_perspective(box)->perspective_impl->tmat.image(proj_corner).affine();
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

Geom::Point
box3d_get_center_screen (SPBox3D *box) {
    Proj::Pt3 proj_center (box3d_get_proj_center (box));
    if (!box3d_get_perspective(box)) {
        return Geom::Point (Geom::infinity(), Geom::infinity());
    }
    Geom::Affine const i2d( box->i2dt_affine() );
    return box3d_get_perspective(box)->perspective_impl->tmat.image(proj_center).affine() * i2d.inverse();
}

/*
 * To keep the snappoint from jumping randomly between the two lines when the mouse pointer is close to
 * their intersection, we remember the last snapped line and keep snapping to this specific line as long
 * as the distance from the intersection to the mouse pointer is less than remember_snap_threshold.
 */

// Should we make the threshold settable in the preferences?
static double remember_snap_threshold = 30;
static guint remember_snap_index = 0;

// constant for sizing the array of points to be considered:
static const int MAX_POINT_COUNT = 4;

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

    Persp3DImpl *persp_impl = box3d_get_perspective(box)->perspective_impl;
    Geom::Point A = persp_impl->tmat.image(A_proj).affine();
    Geom::Point B = persp_impl->tmat.image(B_proj).affine();
    Geom::Point C = persp_impl->tmat.image(C_proj).affine();
    Geom::Point D = persp_impl->tmat.image(D_proj).affine();
    Geom::Point E = persp_impl->tmat.image(E_proj).affine();
    Geom::Point pt = persp_impl->tmat.image(pt_proj).affine();

    // TODO: Replace these lines between corners with lines from a corner to a vanishing point
    //       (this might help to prevent rounding errors if the box is small)
    Box3D::Line pl1(A, B);
    Box3D::Line pl2(A, D);
    Box3D::Line diag1(A, (id == -1 || (!(id & Box3D::X) == !(id & Box3D::Y))) ? C : E);
    Box3D::Line diag2(A, E); // diag2 is only taken into account if id equals -1, i.e., if we are snapping the center

    int num_snap_lines = (id != -1) ? 3 : 4;
    Geom::Point snap_pts[MAX_POINT_COUNT];

    snap_pts[0] = pl1.closest_to (pt);
    snap_pts[1] = pl2.closest_to (pt);
    snap_pts[2] = diag1.closest_to (pt);
    if (id == -1) {
        snap_pts[3] = diag2.closest_to (pt);
    }

    gdouble const zoom = SP_ACTIVE_DESKTOP->current_zoom();

    // determine the distances to all potential snapping points
    double snap_dists[MAX_POINT_COUNT];
    for (int i = 0; i < num_snap_lines; ++i) {
        snap_dists[i] = Geom::L2 (snap_pts[i] - pt) * zoom;
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
    double snap_dist = Geom::infinity();
    for (int i = 0; i < num_snap_lines; ++i) {
        if (snap_dists[i] < snap_dist) {
            snap_index = i;
            snap_dist = snap_dists[i];
        }
    }

    // snap to the closest point (or the previously remembered one
    // if we are within tolerance of the starting point)
    Geom::Point result;
    if (within_tolerance) {
        result = snap_pts[remember_snap_index];
    } else {
        remember_snap_index = snap_index;
        result = snap_pts[snap_index];
    }
    return box3d_get_perspective(box)->perspective_impl->tmat.preimage (result, z_coord, Proj::Z);
}

SPBox3D * SPBox3D::createBox3D(SPItem * parent)
{
    SPBox3D *box3d = 0;
    Inkscape::XML::Document *xml_doc = parent->document->rdoc;
    Inkscape::XML::Node *repr = xml_doc->createElement("svg:g");
    repr->setAttribute("sodipodi:type", "inkscape:box3d");
    box3d = reinterpret_cast<SPBox3D *>(parent->appendChildRepr(repr));
    return box3d;
}

void
box3d_set_corner (SPBox3D *box, const guint id, Geom::Point const &new_pos, const Box3D::Axis movement, bool constrained) {
    g_return_if_fail ((movement != Box3D::NONE) && (movement != Box3D::XYZ));

    box->orig_corner0.normalize();
    box->orig_corner7.normalize();

    /* update corners 0 and 7 according to which handle was moved and to the axes of movement */
    if (!(movement & Box3D::Z)) {
        Persp3DImpl *persp_impl = box3d_get_perspective(box)->perspective_impl;
        Proj::Pt3 pt_proj (persp_impl->tmat.preimage (new_pos, (id < 4) ? box->orig_corner0[Proj::Z] :
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
        Persp3DImpl *persp_impl = box3d_get_perspective(box)->perspective_impl;
        Box3D::PerspectiveLine pl(persp_impl->tmat.image(
                                      box3d_get_proj_corner (id, box->save_corner0, box->save_corner7)).affine(),
                                  Proj::Z, persp);
        Geom::Point new_pos_snapped(pl.closest_to(new_pos));
        Proj::Pt3 pt_proj (persp_impl->tmat.preimage (new_pos_snapped,
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

void box3d_set_center (SPBox3D *box, Geom::Point const &new_pos, Geom::Point const &old_pos, const Box3D::Axis movement, bool constrained) {
    g_return_if_fail ((movement != Box3D::NONE) && (movement != Box3D::XYZ));

    box->orig_corner0.normalize();
    box->orig_corner7.normalize();

    Persp3D *persp = box3d_get_perspective(box);
    if (!(movement & Box3D::Z)) {
        double coord = (box->orig_corner0[Proj::Z] + box->orig_corner7[Proj::Z]) / 2;
        double radx = (box->orig_corner7[Proj::X] - box->orig_corner0[Proj::X]) / 2;
        double rady = (box->orig_corner7[Proj::Y] - box->orig_corner0[Proj::Y]) / 2;

        Proj::Pt3 pt_proj (persp->perspective_impl->tmat.preimage (new_pos, coord, Proj::Z));
        if (constrained) {
            Proj::Pt3 old_pos_proj (persp->perspective_impl->tmat.preimage (old_pos, coord, Proj::Z));
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
        Geom::Point new_pos_snapped(pl.closest_to(new_pos));
        Proj::Pt3 pt_proj (persp->perspective_impl->tmat.preimage (new_pos_snapped, coord, Proj::X));

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
                            Geom::Point &corner1, Geom::Point &corner2, Geom::Point &corner3, Geom::Point &corner4)
{
    Persp3D *persp = box3d_get_perspective(box);
    g_return_if_fail (persp);
    Persp3DImpl *persp_impl = persp->perspective_impl;
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
    corner1 = persp_impl->tmat.image(c1).affine();
    corner2 = persp_impl->tmat.image(c2).affine();
    corner3 = persp_impl->tmat.image(c3).affine();
    corner4 = persp_impl->tmat.image(c4).affine();
}

/* Auxiliary function: Checks whether the half-line from A to B crosses the line segment joining C and D */
static bool
box3d_half_line_crosses_joining_line (Geom::Point const &A, Geom::Point const &B,
                                      Geom::Point const &C, Geom::Point const &D) {
    Geom::Point n0 = (B - A).ccw();
    double d0 = dot(n0,A);

    Geom::Point n1 = (D - C).ccw();
    double d1 = dot(n1,C);

    Geom::Line lineAB(A,B);
    Geom::Line lineCD(C,D);

    Geom::OptCrossing inters = Geom::OptCrossing(); // empty by default
    try
    {
        inters = Geom::intersection(lineAB, lineCD);
    }
    catch (Geom::InfiniteSolutions& e)
    {
        // We're probably dealing with parallel lines, so they don't really cross
        return false;
    }

    if (!inters) {
        return false;
    }

    Geom::Point E = lineAB.pointAt((*inters).ta); // the point of intersection

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
    Geom::Point v1(l1.direction());
    Geom::Point v2(l2.direction());
    v1.normalize();
    v2.normalize();

    return (v1[Geom::X]*v2[Geom::Y] - v1[Geom::Y]*v2[Geom::X] > 0);
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
    Geom::Point vp(persp3d_get_VP(persp, Box3D::toProj(fin_axis)).affine());

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
    Geom::Point c3(box3d_get_corner_screen(box, 3, false));

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

    if ((pos1 != -1) && (pos2 != -1)){
        int tmp = z_orders[pos1];
        z_orders[pos1] = z_orders[pos2];
        z_orders[pos2] = tmp;
    }
}


bool
box3d_recompute_z_orders (SPBox3D *box) {
    Persp3D *persp = box3d_get_perspective(box);

    if (!persp)
        return false;

    int z_orders[6];

    Geom::Point c3(box3d_get_corner_screen(box, 3, false));

    // determine directions from corner3 to the VPs
    int num_finite = 0;
    Box3D::Axis axis_finite = Box3D::NONE;
    Box3D::Axis axis_infinite = Box3D::NONE;
    Geom::Point dirs[3];
    for (int i = 0; i < 3; ++i) {
        dirs[i] = persp3d_get_PL_dir_from_pt(persp, c3, Box3D::toProj(Box3D::axes[i]));
        if (persp3d_VP_is_finite(persp->perspective_impl, Proj::axes[i])) {
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
        // FIXME: We should eliminate the use of Geom::Point altogether
        Box3D::Axis central_axis = Box3D::NONE;
        Geom::Point vp_x = persp3d_get_VP(persp, Proj::X).affine();
        Geom::Point vp_y = persp3d_get_VP(persp, Proj::Y).affine();
        Geom::Point vp_z = persp3d_get_VP(persp, Proj::Z).affine();
        Geom::Point vpx(vp_x[Geom::X], vp_x[Geom::Y]);
        Geom::Point vpy(vp_y[Geom::X], vp_y[Geom::Y]);
        Geom::Point vpz(vp_z[Geom::X], vp_z[Geom::Y]);

        Geom::Point c3 = box3d_get_corner_screen(box, 3, false);
        Geom::Point corner3(c3[Geom::X], c3[Geom::Y]);

        if (box3d_half_line_crosses_joining_line (corner3, vpx, vpy, vpz)) {
            central_axis = Box3D::X;
        } else if (box3d_half_line_crosses_joining_line (corner3, vpy, vpz, vpx)) {
            central_axis = Box3D::Y;
        } else if (box3d_half_line_crosses_joining_line (corner3, vpz, vpx, vpy)) {
            central_axis = Box3D::Z;
        }

        // FIXME: At present, this is not used.  Why is it calculated?
        /*
        unsigned int central_corner = 3 ^ central_axis;
        if (central_axis == Box3D::Z) {
            central_corner = central_corner ^ Box3D::XYZ;
        }
        if (box3d_XY_axes_are_swapped(box)) {
            central_corner = central_corner ^ Box3D::XYZ;
        }
        */

        Geom::Point c1(box3d_get_corner_screen(box, 1, false));
        Geom::Point c2(box3d_get_corner_screen(box, 2, false));
        Geom::Point c7(box3d_get_corner_screen(box, 7, false));

        Geom::Point corner1(c1[Geom::X], c1[Geom::Y]);
        Geom::Point corner2(c2[Geom::X], c2[Geom::Y]);
        Geom::Point corner7(c7[Geom::X], c7[Geom::Y]);
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

static std::map<int, Box3DSide *> box3d_get_sides(SPBox3D *box)
{
    std::map<int, Box3DSide *> sides;
    for ( SPObject *obj = box->firstChild(); obj; obj = obj->getNext() ) {
        Box3DSide *side = dynamic_cast<Box3DSide *>(obj);
        if (side) {
            sides[Box3D::face_to_int(side->getFaceId())] = side;
        }
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
                ((*side).second)->lowerToBottom();
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
box3d_pt_lies_in_PL_sector (SPBox3D const *box, Geom::Point const &pt, int id1, int id2, Box3D::Axis axis) {
    Persp3D *persp = box3d_get_perspective(box);

    // the two corners
    Geom::Point c1(box3d_get_corner_screen(box, id1, false));
    Geom::Point c2(box3d_get_corner_screen(box, id2, false));

    int ret = 0;
    if (persp3d_VP_is_finite(persp->perspective_impl, Box3D::toProj(axis))) {
        Geom::Point vp(persp3d_get_VP(persp, Box3D::toProj(axis)).affine());
        Geom::Point v1(c1 - vp);
        Geom::Point v2(c2 - vp);
        Geom::Point w(pt - vp);
        ret = static_cast<int>(Box3D::lies_in_sector(v1, v2, w));
    } else {
        Box3D::PerspectiveLine pl1(c1, Box3D::toProj(axis), persp);
        Box3D::PerspectiveLine pl2(c2, Box3D::toProj(axis), persp);
        if (pl1.lie_on_same_side(pt, c2) && pl2.lie_on_same_side(pt, c1)) {
            // test whether pt lies "towards" or "away from" the VP
            Box3D::Line edge(c1,c2);
            Geom::Point c3(box3d_get_corner_screen(box, id1 ^ axis, false));
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

    if (!persp3d_VP_is_finite(persp->perspective_impl, vpdir)) {
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

static void box3d_extract_boxes_rec(SPObject *obj, std::list<SPBox3D *> &boxes) {
    SPBox3D *box = dynamic_cast<SPBox3D *>(obj);
    if (box) {
        boxes.push_back(box);
    } else if (dynamic_cast<SPGroup *>(obj)) {
        for ( SPObject *child = obj->firstChild(); child; child = child->getNext() ) {
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
        Geom::Point corner0_screen = box3d_get_corner_screen(box, 0, false);
        Geom::Point corner7_screen = box3d_get_corner_screen(box, 7, false);

        box->orig_corner0 = new_persp->perspective_impl->tmat.preimage(corner0_screen, z0, Proj::Z);
        box->orig_corner7 = new_persp->perspective_impl->tmat.preimage(corner7_screen, z7, Proj::Z);
    }

    persp3d_remove_box (old_persp, box);
    persp3d_add_box (new_persp, box);

    Glib::ustring href = "#";
    href += new_persp->getId();
    box->setAttribute("inkscape:perspectiveID", href.c_str());
}

/* Converts the 3D box to an ordinary SPGroup, adds it to the XML tree at the same position as
   the original box and deletes the latter */
SPGroup *box3d_convert_to_group(SPBox3D *box)
{
    SPDocument *doc = box->document;
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    // remember position of the box
    int pos = box->getPosition();

    // remember important attributes
    gchar const *id = box->getAttribute("id");
    gchar const *style = box->getAttribute("style");
    gchar const *mask = box->getAttribute("mask");
    gchar const *clip_path = box->getAttribute("clip-path");

    // create a new group and add the sides (converted to ordinary paths) as its children
    Inkscape::XML::Node *grepr = xml_doc->createElement("svg:g");

    for ( SPObject *obj = box->firstChild(); obj; obj = obj->getNext() ) {
        Box3DSide *side = dynamic_cast<Box3DSide *>(obj);
        if (side) {
            Inkscape::XML::Node *repr = box3d_side_convert_to_path(side);
            grepr->appendChild(repr);
        } else {
            g_warning("Non-side item encountered as child of a 3D box.");
        }
    }

    // add the new group to the box's parent and set remembered position
    SPObject *parent = box->parent;
    parent->appendChild(grepr);
    grepr->setPosition(pos);
    grepr->setAttribute("style", style);
    if (mask)
       grepr->setAttribute("mask", mask);
    if (clip_path)
       grepr->setAttribute("clip-path", clip_path);

    box->deleteObject(true);

    grepr->setAttribute("id", id);

    SPGroup *group = dynamic_cast<SPGroup *>(doc->getObjectByRepr(grepr));
    g_assert(group != NULL);
    return group;
}

const char *SPBox3D::displayName() const {
    return _("3D Box");
}

gchar *SPBox3D::description() const {
    // We could put more details about the 3d box here
    return g_strdup("");
}

static inline void
box3d_push_back_corner_pair(SPBox3D const *box, std::list<std::pair<Geom::Point, Geom::Point> > &pts, int c1, int c2) {
    pts.push_back(std::make_pair(box3d_get_corner_screen(box, c1, false),
                                 box3d_get_corner_screen(box, c2, false)));
}

void SPBox3D::convert_to_guides() const {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (!prefs->getBool("/tools/shapes/3dbox/convertguides", true)) {
        this->convert_to_guides();
        return;
    }

    std::list<std::pair<Geom::Point, Geom::Point> > pts;

    /* perspective lines in X direction */
    box3d_push_back_corner_pair(this, pts, 0, 1);
    box3d_push_back_corner_pair(this, pts, 2, 3);
    box3d_push_back_corner_pair(this, pts, 4, 5);
    box3d_push_back_corner_pair(this, pts, 6, 7);

    /* perspective lines in Y direction */
    box3d_push_back_corner_pair(this, pts, 0, 2);
    box3d_push_back_corner_pair(this, pts, 1, 3);
    box3d_push_back_corner_pair(this, pts, 4, 6);
    box3d_push_back_corner_pair(this, pts, 5, 7);

    /* perspective lines in Z direction */
    box3d_push_back_corner_pair(this, pts, 0, 4);
    box3d_push_back_corner_pair(this, pts, 1, 5);
    box3d_push_back_corner_pair(this, pts, 2, 6);
    box3d_push_back_corner_pair(this, pts, 3, 7);

    sp_guide_pt_pairs_to_guides(this->document, pts);
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
