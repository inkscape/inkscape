#define __BOX3D_SIDE_C__

/*
 * 3D box face implementation
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007  Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "box3d-side.h"
#include "document.h"
#include "xml/document.h"
#include "xml/repr.h"
#include "display/curve.h"
#include "svg/svg.h"
#include "attributes.h"
#include "inkscape.h"
#include "persp3d.h"
#include "box3d-context.h"
#include "prefs-utils.h"
#include "desktop-style.h"
#include "box3d.h"

static void box3d_side_class_init (Box3DSideClass *klass);
static void box3d_side_init (Box3DSide *side);

static void box3d_side_build (SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static Inkscape::XML::Node *box3d_side_write (SPObject *object, Inkscape::XML::Node *repr, guint flags);
static void box3d_side_set (SPObject *object, unsigned int key, const gchar *value);
static void box3d_side_update (SPObject *object, SPCtx *ctx, guint flags);

static void box3d_side_set_shape (SPShape *shape);

static void box3d_side_compute_corner_ids(Box3DSide *side, unsigned int corners[4]);

static SPShapeClass *parent_class;

GType
box3d_side_get_type (void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof (Box3DSideClass),
            NULL, NULL,
            (GClassInitFunc) box3d_side_class_init,
            NULL, NULL,
            sizeof (Box3DSide),
            16,
            (GInstanceInitFunc) box3d_side_init,
            NULL,	/* value_table */
        };
        type = g_type_register_static (SP_TYPE_SHAPE, "Box3DSide", &info, (GTypeFlags)0);
    }
    return type;
}

static void
box3d_side_class_init (Box3DSideClass *klass)
{
    GObjectClass * gobject_class;
    SPObjectClass * sp_object_class;
    SPItemClass * item_class;
    SPPathClass * path_class;
    SPShapeClass * shape_class;

    gobject_class = (GObjectClass *) klass;
    sp_object_class = (SPObjectClass *) klass;
    item_class = (SPItemClass *) klass;
    path_class = (SPPathClass *) klass;
    shape_class = (SPShapeClass *) klass;

    parent_class = (SPShapeClass *)g_type_class_ref (SP_TYPE_SHAPE);

    sp_object_class->build = box3d_side_build;
    sp_object_class->write = box3d_side_write;
    sp_object_class->set = box3d_side_set;
    sp_object_class->update = box3d_side_update;

    shape_class->set_shape = box3d_side_set_shape;
}

static void
box3d_side_init (Box3DSide * side)
{
    side->dir1 = Box3D::NONE;
    side->dir2 = Box3D::NONE;
    side->front_or_rear = Box3D::FRONT;
}

static void
box3d_side_build (SPObject * object, SPDocument * document, Inkscape::XML::Node * repr)
{
    if (((SPObjectClass *) parent_class)->build)
        ((SPObjectClass *) parent_class)->build (object, document, repr);

    sp_object_read_attr(object, "inkscape:box3dsidetype");
}

static Inkscape::XML::Node *
box3d_side_write (SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    Box3DSide *side = SP_BOX3D_SIDE (object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        // this is where we end up when saving as plain SVG (also in other circumstances?)
        // thus we don' set "sodipodi:type" so that the box is only saved as an ordinary svg:path
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_OBJECT_DOCUMENT(object));
        repr = xml_doc->createElement("svg:path");
    }

    if (flags & SP_OBJECT_WRITE_EXT) {
        sp_repr_set_int(repr, "inkscape:box3dsidetype", side->dir1 ^ side->dir2 ^ side->front_or_rear);
    }

    sp_shape_set_shape ((SPShape *) object);

    /* Duplicate the path */
    SPCurve *curve = ((SPShape *) object)->curve;
    //Nulls might be possible if this called iteratively
    if ( !curve ) {
        return NULL;
    }
    NArtBpath *bpath = SP_CURVE_BPATH(curve);
    if ( !bpath ) {
        return NULL;
    }
    char *d = sp_svg_write_path ( bpath );
    repr->setAttribute("d", d);
    g_free (d);

    if (((SPObjectClass *) (parent_class))->write)
        ((SPObjectClass *) (parent_class))->write (object, repr, flags);

    return repr;
}

static void
box3d_side_set (SPObject *object, unsigned int key, const gchar *value)
{
    Box3DSide *side = SP_BOX3D_SIDE (object);

    // TODO: In case the box was recreated (by undo, e.g.) we need to recreate the path
    //       (along with other info?) from the parent box.

    /* fixme: we should really collect updates */
    switch (key) {
        case SP_ATTR_INKSCAPE_BOX3D_SIDE_TYPE:
            if (value) {
                guint desc = atoi (value);

                if (!Box3D::is_face_id(desc)) {
                    g_print ("desc is not a face id: =%s=\n", value);
                }
                g_return_if_fail (Box3D::is_face_id (desc));
                Box3D::Axis plane = (Box3D::Axis) (desc & 0x7);
                plane = (Box3D::is_plane(plane) ? plane : Box3D::orth_plane_or_axis(plane));
                side->dir1 = Box3D::extract_first_axis_direction(plane);
                side->dir2 = Box3D::extract_second_axis_direction(plane);
                side->front_or_rear = (Box3D::FrontOrRear) (desc & 0x8);

                object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
    default:
        if (((SPObjectClass *) parent_class)->set)
            ((SPObjectClass *) parent_class)->set (object, key, value);
        break;
    }
}

static void
box3d_side_update (SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        flags &= ~SP_OBJECT_USER_MODIFIED_FLAG_B; // since we change the description, it's not a "just translation" anymore
    }

    if (flags & (SP_OBJECT_MODIFIED_FLAG |
                 SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        sp_shape_set_shape ((SPShape *) object);
    }

    if (((SPObjectClass *) parent_class)->update)
        ((SPObjectClass *) parent_class)->update (object, ctx, flags);
}

void
box3d_side_position_set (Box3DSide *side) {
    box3d_side_set_shape (SP_SHAPE (side));

    /* This call is responsible for live update of the sides during the initial drag */
    SP_OBJECT(side)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
box3d_side_set_shape (SPShape *shape)
{
    Box3DSide *side = SP_BOX3D_SIDE (shape);
    if (!SP_OBJECT_DOCUMENT(side)->root) {
        // avoid a warning caused by sp_document_height() (which is called from sp_item_i2d_affine() below)
        // when reading a file containing 3D boxes
        return;
    }

    SPObject *parent = SP_OBJECT(side)->parent;
    if (!SP_IS_BOX3D(parent)) {
        g_warning ("Parent of 3D box side is not a 3D box.\n");
        return;
    }
    SPBox3D *box = SP_BOX3D(parent);

    Persp3D *persp = box3d_side_perspective(side);
    if (!persp) {
        return;
    }

    SPCurve *c = new SPCurve();
    // TODO: Draw the correct quadrangle here
    //       To do this, determine the perspective of the box, the orientation of the side (e.g., XY-FRONT)
    //       compute the coordinates of the corners in P^3, project them onto the canvas, and draw the
    //       resulting path.

    unsigned int corners[4];
    box3d_side_compute_corner_ids(side, corners);

    c->moveto(box3d_get_corner_screen(box, corners[0]));
    c->lineto(box3d_get_corner_screen(box, corners[1]));
    c->lineto(box3d_get_corner_screen(box, corners[2]));
    c->lineto(box3d_get_corner_screen(box, corners[3]));

    c->closepath();
    sp_lpe_item_perform_path_effect(SP_LPE_ITEM (side), c);
    sp_shape_set_curve_insync (SP_SHAPE (side), c, TRUE);
    c->unref();
}

void
box3d_side_apply_style (Box3DSide *side) {
    Inkscape::XML::Node *repr_face = SP_OBJECT_REPR(SP_OBJECT(side));

    gchar *descr = g_strconcat ("desktop.", box3d_side_axes_string (side), NULL);
    const gchar * cur_style = prefs_get_string_attribute(descr, "style");
    g_free (descr);    
    
    SPDesktop *desktop = inkscape_active_desktop();
    bool use_current = prefs_get_int_attribute("tools.shapes.3dbox", "usecurrent", 0);
    if (use_current && cur_style !=NULL) {
        /* use last used style */
        repr_face->setAttribute("style", cur_style);
    } else {
        /* use default style */
        GString *pstring = g_string_new("");
        g_string_printf (pstring, "tools.shapes.3dbox.%s", box3d_side_axes_string(side));
        sp_desktop_apply_style_tool (desktop, repr_face, pstring->str, false);
    }
}

gchar *
box3d_side_axes_string(Box3DSide *side)
{
    GString *pstring = g_string_new("");
    g_string_printf (pstring, "%s", Box3D::string_from_axes ((Box3D::Axis) (side->dir1 ^ side->dir2)));
    switch ((Box3D::Axis) (side->dir1 ^ side->dir2)) {
        case Box3D::XY:
            g_string_append_printf (pstring, (side->front_or_rear == Box3D::FRONT) ? "front" : "rear");
            break;
        case Box3D::XZ:
            g_string_append_printf (pstring, (side->front_or_rear == Box3D::FRONT) ? "top" : "bottom");
            break;
        case Box3D::YZ:
            g_string_append_printf (pstring, (side->front_or_rear == Box3D::FRONT) ? "right" : "left");
            break;
        default:
            break;
    }
    return pstring->str;
}

static void
box3d_side_compute_corner_ids(Box3DSide *side, unsigned int corners[4]) {
    Box3D::Axis orth = Box3D::third_axis_direction (side->dir1, side->dir2);

    corners[0] = (side->front_or_rear ? orth : 0);
    corners[1] = corners[0] ^ side->dir1;
    corners[2] = corners[0] ^ side->dir1 ^ side->dir2;
    corners[3] = corners[0] ^ side->dir2;
}

Persp3D *
box3d_side_perspective(Box3DSide *side) {
    return SP_BOX3D(SP_OBJECT(side)->parent)->persp_ref->getObject();
}

Inkscape::XML::Node *
box3d_side_convert_to_path(Box3DSide *side) {
    // TODO: Copy over all important attributes (see sp_selected_item_to_curved_repr() for an example)
    SPDocument *doc = SP_OBJECT_DOCUMENT(side);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
    repr->setAttribute("d", SP_OBJECT_REPR(side)->attribute("d"));
    repr->setAttribute("style", SP_OBJECT_REPR(side)->attribute("style"));

    return repr;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
