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

#include "box3d.h"

static void sp_3dbox_class_init(SP3DBoxClass *klass);
static void sp_3dbox_init(SP3DBox *box3d);

static void sp_3dbox_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_shape_release (SPObject *object);
static void sp_3dbox_set(SPObject *object, unsigned int key, const gchar *value);
static void sp_3dbox_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_3dbox_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

//static gchar *sp_3dbox_description(SPItem *item);

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

    //item_class->description = sp_3dbox_description;
}

static void
sp_3dbox_init(SP3DBox *box3d)
{
    if (box3d == NULL) { g_warning ("box3d is NULL!\n"); }
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
        NR::Point corner1, corner2, corner3, corner4;
        sp_3dbox_compute_specific_corners (bc, corner1, corner2, corner3, corner4);
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

    // FIXME: Why must the coordinates be flipped vertically???
    //SPDocument *doc = SP_OBJECT_DOCUMENT(box3d);
    //gdouble height = sp_document_height(doc);

    /* Curve-adaption variant: */
    NR::Point corner1, corner2, corner3, corner4;
    corner1 = bc->drag_origin;

    if (bc->extruded) {
        sp_3dbox_compute_specific_corners (bc, corner1, corner2, corner3, corner4);
        for (int i=0; i < 6; ++i) {
            if (!box3d->faces[i]) {
                g_warning ("Face no. %d does not exist!\n", i);
                return;
            }
        }
        box3d->faces[0]->set_face (bc->drag_origin, corner4, Box3D::Z, Box3D::Y);
        box3d->faces[0]->set_curve();
        box3d->faces[5]->set_face (corner3, bc->drag_ptC, Box3D::Y, Box3D::X);
        box3d->faces[5]->set_curve();
        box3d->faces[3]->set_face (bc->drag_origin, corner2, Box3D::X, Box3D::Z);
        box3d->faces[3]->set_curve();
        box3d->faces[2]->set_face (bc->drag_ptB, corner4, Box3D::X, Box3D::Z);
        box3d->faces[2]->set_curve();

        box3d->faces[1]->set_face (corner1, bc->drag_ptC, Box3D::Z, Box3D::Y);
        box3d->faces[1]->set_curve();
    }
    box3d->faces[4]->set_face(bc->drag_origin, bc->drag_ptB, Box3D::Y, Box3D::X);
    box3d->faces[4]->set_curve();
}


void
sp_3dbox_compute_specific_corners (SP3DBoxContext *box3d_context, NR::Point &corner1, NR::Point &corner2, NR::Point &corner3, NR::Point &corner4)
{
        // TODO: Check for numerical stability and handle "wrong" cases more gracefully.
        //       (This now mostly applies to the intersection code in the PerspectiveLine class)
        Box3D::PerspectiveLine pl1 (box3d_context->drag_origin, Box3D::X);
        Box3D::PerspectiveLine pl2 (box3d_context->drag_ptB, Box3D::Y);
        corner1 = pl1.meet(pl2);

        Box3D::PerspectiveLine pl3 (corner1, Box3D::Z);
        Box3D::PerspectiveLine pl4 (box3d_context->drag_ptC, Box3D::Y);
        corner2 = pl3.meet(pl4);

        Box3D::PerspectiveLine pl5 (corner2, Box3D::X);
        Box3D::PerspectiveLine pl6 (box3d_context->drag_origin, Box3D::Z);
        corner3 = pl5.meet(pl6);

        Box3D::PerspectiveLine pl7 (box3d_context->drag_ptC, Box3D::X);
        Box3D::PerspectiveLine pl8 (corner3, Box3D::Y);
        corner4 = pl7.meet(pl8);
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
