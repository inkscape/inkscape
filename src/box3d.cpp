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
#include "box3d.h"

static void sp_3dbox_class_init(SP3DBoxClass *klass);
static void sp_3dbox_init(SP3DBox *box3d);

static void sp_3dbox_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
//static void sp_3dbox_release (SPObject *object);
static void sp_3dbox_set(SPObject *object, unsigned int key, const gchar *value);
static void sp_3dbox_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_3dbox_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static gchar *sp_3dbox_description(SPItem *item);

//static void sp_3dbox_set_shape(SPShape *shape);
static void sp_3dbox_set_shape(SP3DBox *box3d);

static SPGroupClass *parent_class;

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
    //sp_object_class->release = sp_3dbox_release;

    item_class->description = sp_3dbox_description;
}

static void
sp_3dbox_init(SP3DBox *box3d)
{
    box3d->faces[4] = new Box3DFace (box3d);
    //box3d->faces[4]->hook_path_to_3dbox();
    for (int i = 0; i < 6; ++i) {
        if (i == 4) continue;
        box3d->faces[i] = NULL;
    }
}

static void
sp_3dbox_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) (parent_class))->build) {
        ((SPObjectClass *) (parent_class))->build(object, document, repr);
    }

    //sp_object_read_attr(object, "width");
}

/*
static void
sp_3dbox_release (SPObject *object)
{
	SP3DBox *box3d = SP_3DBOX(object);
        for (int i = 0; i < 6; ++i) {
            if (box3d->faces[i]) {
                delete box3d->faces[i]; // FIXME: Anything else to do? Do we need to clean up the face first?
            }
        }

	if (((SPObjectClass *) parent_class)->release) {
	  ((SPObjectClass *) parent_class)->release (object);
	}
}
*/

static void sp_3dbox_set(SPObject *object, unsigned int key, const gchar *value)
{
    //SP3DBox *box3d = SP_3DBOX(object);

    switch (key) {
        /***
        case SP_ATTR_WIDTH:
            rect->width.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        ***/
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
    //SP3DBox *box3d = SP_3DBOX(object);

    /* Invoke parent method */
    if (((SPObjectClass *) (parent_class))->update)
        ((SPObjectClass *) (parent_class))->update(object, ctx, flags);
    
    //sp_3dbox_set_shape (box3d);
}



static Inkscape::XML::Node *sp_3dbox_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SP3DBox *box3d = SP_3DBOX(object);
    // FIXME: How to handle other contexts???
    // FIXME: Is tools_isactive(..) more recommended to check for the current context/tool?
    if (!SP_IS_3DBOX_CONTEXT(inkscape_active_event_context()))
        return repr;
    SP3DBoxContext *bc = SP_3DBOX_CONTEXT(inkscape_active_event_context());

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_OBJECT_DOCUMENT(object));
        repr = xml_doc->createElement("svg:g");
        repr->setAttribute("sodipodi:type", "inkscape:3dbox");
    }


    box3d->faces[4]->set_path_repr();
    if (bc->extruded) {
        for (int i = 0; i < 6; ++i) {
            if (i == 4) continue;
            box3d->faces[i]->set_path_repr();
        }
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
// FIXME: Note that this is _not_ the virtual set_shape() method inherited from SPShape,
//        since SP3DBox is inherited from SPGroup. The following method is "artificially"
//        called from sp_3dbox_update().
//sp_3dbox_set_shape(SPShape *shape)
sp_3dbox_set_shape(SP3DBox *box3d)
{
    // FIXME: How to handle other contexts???
    // FIXME: Is tools_isactive(..) more recommended to check for the current context/tool?
    if (!SP_IS_3DBOX_CONTEXT(inkscape_active_event_context()))
        return;
    SP3DBoxContext *bc = SP_3DBOX_CONTEXT(inkscape_active_event_context());

    /* Only update the curves during dragging; setting the svg representations 
       is expensive and only done once at the end */
    sp_3dbox_recompute_corners (box3d, bc->drag_origin, bc->drag_ptB, bc->drag_ptC);
    if (bc->extruded) {
        box3d->faces[0]->set_corners (box3d->corners[0], box3d->corners[4], box3d->corners[6], box3d->corners[2]);
        box3d->faces[1]->set_corners (box3d->corners[1], box3d->corners[5], box3d->corners[7], box3d->corners[3]);
        box3d->faces[2]->set_corners (box3d->corners[0], box3d->corners[1], box3d->corners[5], box3d->corners[4]);
        box3d->faces[3]->set_corners (box3d->corners[2], box3d->corners[3], box3d->corners[7], box3d->corners[6]);
        box3d->faces[5]->set_corners (box3d->corners[4], box3d->corners[5], box3d->corners[7], box3d->corners[6]);
    }
    box3d->faces[4]->set_corners (box3d->corners[0], box3d->corners[1], box3d->corners[3], box3d->corners[2]);

    sp_3dbox_update_curves (box3d);
}


void sp_3dbox_recompute_corners (SP3DBox *box, NR::Point const A, NR::Point const B, NR::Point const C)
{
    sp_3dbox_move_corner_in_XY_plane (box, 2, A);
    sp_3dbox_move_corner_in_XY_plane (box, 1, B);
    sp_3dbox_move_corner_in_Z_direction (box, 5, C);
}

void
sp_3dbox_update_curves (SP3DBox *box) {
    for (int i = 0; i < 6; ++i) {
        if (box->faces[i]) box->faces[i]->set_curve();
    }
}

void
sp_3dbox_move_corner_in_XY_plane (SP3DBox *box, guint id, NR::Point pt, Box3D::Axis axes)
{
    NR::Point A (box->corners[id ^ Box3D::XY]);
    if (Box3D::is_single_axis_direction (axes)) {
        pt = Box3D::PerspectiveLine (box->corners[id], axes).closest_to(pt);
    }

    /* set the 'front' corners */
    box->corners[id] = pt;

    Box3D::PerspectiveLine pl_one (A, Box3D::Y);
    Box3D::PerspectiveLine pl_two (pt, Box3D::X);
    box->corners[id ^ Box3D::X] = pl_one.meet(pl_two);

    pl_one = Box3D::PerspectiveLine (A, Box3D::X);
    pl_two = Box3D::PerspectiveLine (pt, Box3D::Y);
    box->corners[id ^ Box3D::Y] = pl_one.meet(pl_two);

    /* set the 'rear' corners */
    NR::Point B (box->corners[id ^ Box3D::XYZ]);

    pl_one = Box3D::PerspectiveLine (box->corners[id ^ Box3D::X], Box3D::Z);
    pl_two = Box3D::PerspectiveLine (B, Box3D::Y);
    box->corners[id ^ Box3D::XZ] = pl_one.meet(pl_two);

    pl_one = Box3D::PerspectiveLine (box->corners[id ^ Box3D::XZ], Box3D::X);
    pl_two = Box3D::PerspectiveLine (pt, Box3D::Z);
    box->corners[id ^ Box3D::Z] = pl_one.meet(pl_two);

    pl_one = Box3D::PerspectiveLine (box->corners[id ^ Box3D::Z], Box3D::Y);
    pl_two = Box3D::PerspectiveLine (B, Box3D::X);
    box->corners[id ^ Box3D::YZ] = pl_one.meet(pl_two);
    
}

void
sp_3dbox_move_corner_in_Z_direction (SP3DBox *box, guint id, NR::Point pt, bool constrained)
{
    if (!constrained) sp_3dbox_move_corner_in_XY_plane (box, id, pt, Box3D::XY);

    /* set the four corners of the face containing corners[id] */
    box->corners[id] = Box3D::PerspectiveLine (box->corners[id], Box3D::Z).closest_to(pt);

    Box3D::PerspectiveLine pl_one (box->corners[id], Box3D::X);
    Box3D::PerspectiveLine pl_two (box->corners[id ^ Box3D::XZ], Box3D::Z);
    box->corners[id ^ Box3D::X] = pl_one.meet(pl_two);

    pl_one = Box3D::PerspectiveLine (box->corners[id ^ Box3D::X], Box3D::Y);
    pl_two = Box3D::PerspectiveLine (box->corners[id ^ Box3D::XYZ], Box3D::Z);
    box->corners[id ^ Box3D::XY] = pl_one.meet(pl_two);

    pl_one = Box3D::PerspectiveLine (box->corners[id], Box3D::Y);
    pl_two = Box3D::PerspectiveLine (box->corners[id ^ Box3D::YZ], Box3D::Z);
    box->corners[id ^ Box3D::Y] = pl_one.meet(pl_two);
}

NR::Maybe<NR::Point>
sp_3dbox_get_center (SP3DBox *box)
{
    return sp_3dbox_get_midpoint_between_corners (box, 0, 7);
}

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

        Box3D::PerspectiveLine pl (*adjacent_face_center, orth_dir);
        return pl.intersect(Box3D::PerspectiveLine(box->corners[id_corner1], corner_axes));
    } else {
        Box3D::Axis dir = Box3D::extract_single_axis_direction (corner_axes);
        Box3D::Line diag1 (box->corners[id_corner1], box->corners[id_corner2]);
        Box3D::Line diag2 (box->corners[id_corner1 ^ dir], box->corners[id_corner2 ^ dir]);
        return diag1.intersect(diag2);
    }
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
