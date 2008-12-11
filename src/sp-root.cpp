#define __SP_ROOT_C__

/** \file
 * SVG \<svg\> implementation.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>

#include "svg/svg.h"
#include "display/nr-arena-group.h"
#include "attributes.h"
#include "print.h"
#include "document.h"
#include "sp-defs.h"
#include "sp-root.h"
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-translate-ops.h>
#include <libnr/nr-scale-ops.h>
#include <libnr/nr-translate-scale-ops.h>
#include <xml/repr.h>
#include "svg/stringstream.h"
#include "inkscape_version.h"

class SPDesktop;

static void sp_root_class_init(SPRootClass *klass);
static void sp_root_init(SPRoot *root);

static void sp_root_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_root_release(SPObject *object);
static void sp_root_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_root_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
static void sp_root_remove_child(SPObject *object, Inkscape::XML::Node *child);
static void sp_root_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_root_modified(SPObject *object, guint flags);
static Inkscape::XML::Node *sp_root_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static NRArenaItem *sp_root_show(SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_root_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags);
static void sp_root_print(SPItem *item, SPPrintContext *ctx);

static SPGroupClass *parent_class;

/**
 * Returns the type info of sp_root, including its class sizes and initialization routines.
 */
GType
sp_root_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPRootClass),
            NULL, NULL,
            (GClassInitFunc) sp_root_class_init,
            NULL, NULL,
            sizeof(SPRoot),
            16,
            (GInstanceInitFunc) sp_root_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_GROUP, "SPRoot", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * Initializes an SPRootClass object by setting its class and parent class objects, and registering
 * function pointers (i.e.\ gobject-style virtual functions) for various operations.
 */
static void
sp_root_class_init(SPRootClass *klass)
{
    GObjectClass *object_class;
    SPObjectClass *sp_object_class;
    SPItemClass *sp_item_class;

    object_class = G_OBJECT_CLASS(klass);
    sp_object_class = (SPObjectClass *) klass;
    sp_item_class = (SPItemClass *) klass;

    parent_class = (SPGroupClass *)g_type_class_ref(SP_TYPE_GROUP);

    sp_object_class->build = sp_root_build;
    sp_object_class->release = sp_root_release;
    sp_object_class->set = sp_root_set;
    sp_object_class->child_added = sp_root_child_added;
    sp_object_class->remove_child = sp_root_remove_child;
    sp_object_class->update = sp_root_update;
    sp_object_class->modified = sp_root_modified;
    sp_object_class->write = sp_root_write;

    sp_item_class->show = sp_root_show;
    sp_item_class->bbox = sp_root_bbox;
    sp_item_class->print = sp_root_print;
}

/**
 * Initializes an SPRoot object by setting its default parameter values.
 */
static void
sp_root_init(SPRoot *root)
{
    static Inkscape::Version const zero_version(0, 0);

    sp_version_from_string(SVG_VERSION, &root->original.svg);
    root->version.svg = root->original.svg;
    root->version.inkscape = root->original.inkscape =
        root->version.sodipodi = root->original.sodipodi = zero_version;

    root->x.unset();
    root->y.unset();
    root->width.unset(SVGLength::PERCENT, 1.0, 1.0);
    root->height.unset(SVGLength::PERCENT, 1.0, 1.0);

    /* root->viewbox.set_identity(); */
    root->viewBox_set = FALSE;

    root->c2p.setIdentity();

    root->defs = NULL;
}

/**
 * Fills in the data for an SPObject from its Inkscape::XML::Node object.
 * It fills in data such as version, x, y, width, height, etc.
 * It then calls the object's parent class object's build function.
 */
static void
sp_root_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    SPGroup *group = (SPGroup *) object;
    SPRoot *root = (SPRoot *) object;

    if (repr->attribute("sodipodi:docname") || repr->attribute("SP-DOCNAME")) {
        /* so we have a nonzero initial version */
        root->original.sodipodi.major = 0;
        root->original.sodipodi.minor = 1;
    }
    sp_object_read_attr(object, "version");
    sp_object_read_attr(object, "sodipodi:version");
    sp_object_read_attr(object, "inkscape:version");
    /* It is important to parse these here, so objects will have viewport build-time */
    sp_object_read_attr(object, "x");
    sp_object_read_attr(object, "y");
    sp_object_read_attr(object, "width");
    sp_object_read_attr(object, "height");
    sp_object_read_attr(object, "viewBox");
    sp_object_read_attr(object, "preserveAspectRatio");
    sp_object_read_attr(object, "onload");

    if (((SPObjectClass *) parent_class)->build)
        (* ((SPObjectClass *) parent_class)->build) (object, document, repr);

    /* Search for first <defs> node */
    for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL; o = SP_OBJECT_NEXT(o) ) {
        if (SP_IS_DEFS(o)) {
            root->defs = SP_DEFS(o);
            break;
        }
    }

    // clear transform, if any was read in - SVG does not allow transform= on <svg>
    SP_ITEM(object)->transform = NR::identity();
}

/**
 * This is a destructor routine for SPRoot objects.  It de-references any \<def\> items and calls
 * the parent class destructor.
 */
static void
sp_root_release(SPObject *object)
{
    SPRoot *root = (SPRoot *) object;
    root->defs = NULL;

    if (((SPObjectClass *) parent_class)->release)
        ((SPObjectClass *) parent_class)->release(object);
}

/**
 * Sets the attribute given by key for SPRoot objects to the value specified by value.
 */
static void
sp_root_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPRoot *root = SP_ROOT(object);

    switch (key) {
        case SP_ATTR_VERSION:
            if (!sp_version_from_string(value, &root->version.svg)) {
                root->version.svg = root->original.svg;
            }
            break;
        case SP_ATTR_SODIPODI_VERSION:
            if (!sp_version_from_string(value, &root->version.sodipodi)) {
                root->version.sodipodi = root->original.sodipodi;
            }
        case SP_ATTR_INKSCAPE_VERSION:
            if (!sp_version_from_string(value, &root->version.inkscape)) {
                root->version.inkscape = root->original.inkscape;
            }
            break;
        case SP_ATTR_X:
            if (!root->x.readAbsolute(value)) {
                /* fixme: em, ex, % are probably valid, but require special treatment (Lauris) */
                root->x.unset();
            }
            /* fixme: I am almost sure these do not require viewport flag (Lauris) */
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y:
            if (!root->y.readAbsolute(value)) {
                /* fixme: em, ex, % are probably valid, but require special treatment (Lauris) */
                root->y.unset();
            }
            /* fixme: I am almost sure these do not require viewport flag (Lauris) */
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
            break;
        case SP_ATTR_WIDTH:
            if (!root->width.readAbsolute(value) || !(root->width.computed > 0.0)) {
                /* fixme: em, ex, % are probably valid, but require special treatment (Lauris) */
                root->width.unset(SVGLength::PERCENT, 1.0, 1.0);
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
            break;
        case SP_ATTR_HEIGHT:
            if (!root->height.readAbsolute(value) || !(root->height.computed > 0.0)) {
                /* fixme: em, ex, % are probably valid, but require special treatment (Lauris) */
                root->height.unset(SVGLength::PERCENT, 1.0, 1.0);
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
            break;
        case SP_ATTR_VIEWBOX:
            if (value) {
                double x, y, width, height;
                char *eptr;
                /* fixme: We have to take original item affine into account */
                /* fixme: Think (Lauris) */
                eptr = (gchar *) value;
                x = g_ascii_strtod(eptr, &eptr);
                while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
                y = g_ascii_strtod(eptr, &eptr);
                while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
                width = g_ascii_strtod(eptr, &eptr);
                while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
                height = g_ascii_strtod(eptr, &eptr);
                while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
                if ((width > 0) && (height > 0)) {
                    /* Set viewbox */
                    root->viewBox.x0 = x;
                    root->viewBox.y0 = y;
                    root->viewBox.x1 = x + width;
                    root->viewBox.y1 = y + height;
                    root->viewBox_set = TRUE;
                } else {
                    root->viewBox_set = FALSE;
                }
            } else {
                root->viewBox_set = FALSE;
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
            break;
        case SP_ATTR_PRESERVEASPECTRATIO:
            /* Do setup before, so we can use break to escape */
            root->aspect_set = FALSE;
            root->aspect_align = SP_ASPECT_XMID_YMID;
            root->aspect_clip = SP_ASPECT_MEET;
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
            if (value) {
                int len;
                gchar c[256];
                gchar const *p, *e;
                unsigned int align, clip;
                p = value;
                while (*p && *p == 32) p += 1;
                if (!*p) break;
                e = p;
                while (*e && *e != 32) e += 1;
                len = e - p;
                if (len > 8) break;
                memcpy(c, value, len);
                c[len] = 0;
                /* Now the actual part */
                if (!strcmp(c, "none")) {
                    align = SP_ASPECT_NONE;
                } else if (!strcmp(c, "xMinYMin")) {
                    align = SP_ASPECT_XMIN_YMIN;
                } else if (!strcmp(c, "xMidYMin")) {
                    align = SP_ASPECT_XMID_YMIN;
                } else if (!strcmp(c, "xMaxYMin")) {
                    align = SP_ASPECT_XMAX_YMIN;
                } else if (!strcmp(c, "xMinYMid")) {
                    align = SP_ASPECT_XMIN_YMID;
                } else if (!strcmp(c, "xMidYMid")) {
                    align = SP_ASPECT_XMID_YMID;
                } else if (!strcmp(c, "xMaxYMin")) {
                    align = SP_ASPECT_XMAX_YMID;
                } else if (!strcmp(c, "xMinYMax")) {
                    align = SP_ASPECT_XMIN_YMAX;
                } else if (!strcmp(c, "xMidYMax")) {
                    align = SP_ASPECT_XMID_YMAX;
                } else if (!strcmp(c, "xMaxYMax")) {
                    align = SP_ASPECT_XMAX_YMAX;
                } else {
                    break;
                }
                clip = SP_ASPECT_MEET;
                while (*e && *e == 32) e += 1;
                if (*e) {
                    if (!strcmp(e, "meet")) {
                        clip = SP_ASPECT_MEET;
                    } else if (!strcmp(e, "slice")) {
                        clip = SP_ASPECT_SLICE;
                    } else {
                        break;
                    }
                }
                root->aspect_set = TRUE;
                root->aspect_align = align;
                root->aspect_clip = clip;
            }
            break;
        case SP_ATTR_ONLOAD:
            root->onload = (char *) value;
            break;
        default:
            /* Pass the set event to the parent */
            if (((SPObjectClass *) parent_class)->set) {
                ((SPObjectClass *) parent_class)->set(object, key, value);
            }
            break;
    }
}

/**
 * This routine is for adding a child SVG object to an SPRoot object.
 * The SPRoot object is taken to be an SPGroup.
 */
static void
sp_root_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    SPRoot *root = (SPRoot *) object;
    SPGroup *group = (SPGroup *) object;

    if (((SPObjectClass *) (parent_class))->child_added)
        (* ((SPObjectClass *) (parent_class))->child_added)(object, child, ref);

    SPObject *co = object->document->getObjectByRepr(child);
    g_assert (co != NULL || !strcmp("comment", child->name())); // comment repr node has no object

    if (co && SP_IS_DEFS(co)) {
        SPObject *c;
        /* We search for first <defs> node - it is not beautiful, but works */
        for (c = sp_object_first_child(SP_OBJECT(group)) ; c != NULL; c = SP_OBJECT_NEXT(c) ) {
            if (SP_IS_DEFS(c)) {
                root->defs = SP_DEFS(c);
                break;
            }
        }
    }
}

/**
 * Removes the given child from this SPRoot object.
 */
static void sp_root_remove_child(SPObject *object, Inkscape::XML::Node *child)
{
    SPRoot *root = (SPRoot *) object;

    if ( root->defs && SP_OBJECT_REPR(root->defs) == child ) {
        SPObject *iter;
        /* We search for first remaining <defs> node - it is not beautiful, but works */
        for ( iter = sp_object_first_child(object) ; iter ; iter = SP_OBJECT_NEXT(iter) ) {
            if ( SP_IS_DEFS(iter) && (SPDefs *)iter != root->defs ) {
                root->defs = (SPDefs *)iter;
                break;
            }
        }
        if (!iter) {
            /* we should probably create a new <defs> here? */
            root->defs = NULL;
        }
    }

    if (((SPObjectClass *) (parent_class))->remove_child)
        (* ((SPObjectClass *) (parent_class))->remove_child)(object, child);
}

/**
 * This callback routine updates the SPRoot object when its attributes have been changed.
 */
static void
sp_root_update(SPObject *object, SPCtx *ctx, guint flags)
{
    SPItemView *v;

    SPItem *item = SP_ITEM(object);
    SPRoot *root = SP_ROOT(object);
    SPItemCtx *ictx = (SPItemCtx *) ctx;

    /* fixme: This will be invoked too often (Lauris) */
    /* fixme: We should calculate only if parent viewport has changed (Lauris) */
    /* If position is specified as percentage, calculate actual values */
    if (root->x.unit == SVGLength::PERCENT) {
        root->x.computed = root->x.value * (ictx->vp.x1 - ictx->vp.x0);
    }
    if (root->y.unit == SVGLength::PERCENT) {
        root->y.computed = root->y.value * (ictx->vp.y1 - ictx->vp.y0);
    }
    if (root->width.unit == SVGLength::PERCENT) {
        root->width.computed = root->width.value * (ictx->vp.x1 - ictx->vp.x0);
    }
    if (root->height.unit == SVGLength::PERCENT) {
        root->height.computed = root->height.value * (ictx->vp.y1 - ictx->vp.y0);
    }

    /* Create copy of item context */
    SPItemCtx rctx = *ictx;

    /* Calculate child to parent transformation */
    root->c2p.setIdentity();

    if (object->parent) {
        /*
         * fixme: I am not sure whether setting x and y does or does not
         * fixme: translate the content of inner SVG.
         * fixme: Still applying translation and setting viewport to width and
         * fixme: height seems natural, as this makes the inner svg element
         * fixme: self-contained. The spec is vague here.
         */
        root->c2p = Geom::Matrix(Geom::Translate(root->x.computed,
                                                 root->y.computed));
    }

    if (root->viewBox_set) {
        double x, y, width, height;
        /* Determine actual viewbox in viewport coordinates */
        if (root->aspect_align == SP_ASPECT_NONE) {
            x = 0.0;
            y = 0.0;
            width = root->width.computed;
            height = root->height.computed;
        } else {
            double scalex, scaley, scale;
            /* Things are getting interesting */
            scalex = root->width.computed / (root->viewBox.x1 - root->viewBox.x0);
            scaley = root->height.computed / (root->viewBox.y1 - root->viewBox.y0);
            scale = (root->aspect_clip == SP_ASPECT_MEET) ? MIN(scalex, scaley) : MAX(scalex, scaley);
            width = (root->viewBox.x1 - root->viewBox.x0) * scale;
            height = (root->viewBox.y1 - root->viewBox.y0) * scale;
            /* Now place viewbox to requested position */
            /* todo: Use an array lookup to find the 0.0/0.5/1.0 coefficients,
               as is done for dialogs/align.cpp. */
            switch (root->aspect_align) {
                case SP_ASPECT_XMIN_YMIN:
                    x = 0.0;
                    y = 0.0;
                    break;
                case SP_ASPECT_XMID_YMIN:
                    x = 0.5 * (root->width.computed - width);
                    y = 0.0;
                    break;
                case SP_ASPECT_XMAX_YMIN:
                    x = 1.0 * (root->width.computed - width);
                    y = 0.0;
                    break;
                case SP_ASPECT_XMIN_YMID:
                    x = 0.0;
                    y = 0.5 * (root->height.computed - height);
                    break;
                case SP_ASPECT_XMID_YMID:
                    x = 0.5 * (root->width.computed - width);
                    y = 0.5 * (root->height.computed - height);
                    break;
                case SP_ASPECT_XMAX_YMID:
                    x = 1.0 * (root->width.computed - width);
                    y = 0.5 * (root->height.computed - height);
                    break;
                case SP_ASPECT_XMIN_YMAX:
                    x = 0.0;
                    y = 1.0 * (root->height.computed - height);
                    break;
                case SP_ASPECT_XMID_YMAX:
                    x = 0.5 * (root->width.computed - width);
                    y = 1.0 * (root->height.computed - height);
                    break;
                case SP_ASPECT_XMAX_YMAX:
                    x = 1.0 * (root->width.computed - width);
                    y = 1.0 * (root->height.computed - height);
                    break;
                default:
                    x = 0.0;
                    y = 0.0;
                    break;
            }
        }

        /* Compose additional transformation from scale and position */
        Geom::Point const viewBox_min(root->viewBox.x0,
                                    root->viewBox.y0);
        Geom::Point const viewBox_max(root->viewBox.x1,
                                    root->viewBox.y1);
        Geom::Scale const viewBox_length( viewBox_max - viewBox_min );
        Geom::Scale const new_length(width, height);

        /* Append viewbox transformation */
        /* TODO: The below looks suspicious to me (pjrm): I wonder whether the RHS
           expression should have c2p at the beginning rather than at the end.  Test it. */
        root->c2p = Geom::Translate(-viewBox_min) * ( new_length * viewBox_length.inverse() ) * Geom::Translate(x, y) * root->c2p;
    }

    rctx.i2doc = root->c2p * rctx.i2doc;

    /* Initialize child viewport */
    if (root->viewBox_set) {
        rctx.vp.x0 = root->viewBox.x0;
        rctx.vp.y0 = root->viewBox.y0;
        rctx.vp.x1 = root->viewBox.x1;
        rctx.vp.y1 = root->viewBox.y1;
    } else {
        /* fixme: I wonder whether this logic is correct (Lauris) */
        if (object->parent) {
            rctx.vp.x0 = root->x.computed;
            rctx.vp.y0 = root->y.computed;
        } else {
            rctx.vp.x0 = 0.0;
            rctx.vp.y0 = 0.0;
        }
        rctx.vp.x1 = root->width.computed;
        rctx.vp.y1 = root->height.computed;
    }

    rctx.i2vp = NR::identity();

    /* And invoke parent method */
    if (((SPObjectClass *) (parent_class))->update)
        ((SPObjectClass *) (parent_class))->update(object, (SPCtx *) &rctx, flags);

    /* As last step set additional transform of arena group */
    for (v = item->display; v != NULL; v = v->next) {
        nr_arena_group_set_child_transform(NR_ARENA_GROUP(v->arenaitem), root->c2p);
    }
}

/**
 * Calls the <tt>modified</tt> routine of the SPRoot object's parent class.
 * Also, if the viewport has been modified, it sets the document size to the new
 * height and width.
 */
static void
sp_root_modified(SPObject *object, guint flags)
{
    SPRoot *root = SP_ROOT(object);

    if (((SPObjectClass *) (parent_class))->modified)
        (* ((SPObjectClass *) (parent_class))->modified)(object, flags);

    /* fixme: (Lauris) */
    if (!object->parent && (flags & SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        sp_document_resized_signal_emit (SP_OBJECT_DOCUMENT(root), root->width.computed, root->height.computed);
    }
}

/**
 * Writes the object into the repr object, then calls the parent's write routine.
 */
static Inkscape::XML::Node *
sp_root_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPRoot *root = SP_ROOT(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:svg");
    }

    if (flags & SP_OBJECT_WRITE_EXT) {
        repr->setAttribute("sodipodi:version", SODIPODI_VERSION);
        repr->setAttribute("inkscape:version", INKSCAPE_VERSION);
    }

    repr->setAttribute("version", SVG_VERSION);

    if (fabs(root->x.computed) > 1e-9)
        sp_repr_set_svg_double(repr, "x", root->x.computed);
    if (fabs(root->y.computed) > 1e-9)
        sp_repr_set_svg_double(repr, "y", root->y.computed);

    /* Unlike all other SPObject, here we want to preserve absolute units too (and only here,
     * according to the recommendation in http://www.w3.org/TR/SVG11/coords.html#Units).
     */
    repr->setAttribute("width", sp_svg_length_write_with_units(root->width).c_str());
    repr->setAttribute("height", sp_svg_length_write_with_units(root->height).c_str());

    if (root->viewBox_set) {
        Inkscape::SVGOStringStream os;
        os << root->viewBox.x0 << " " << root->viewBox.y0 << " " << root->viewBox.x1 - root->viewBox.x0 << " " << root->viewBox.y1 - root->viewBox.y0;
        repr->setAttribute("viewBox", os.str().c_str());
    }

    if (((SPObjectClass *) (parent_class))->write)
        ((SPObjectClass *) (parent_class))->write(object, xml_doc, repr, flags);

    return repr;
}

/**
 * Displays the SPRoot item on the NRArena.
 */
static NRArenaItem *
sp_root_show(SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
    SPRoot *root = SP_ROOT(item);

    NRArenaItem *ai;
    if (((SPItemClass *) (parent_class))->show) {
        ai = ((SPItemClass *) (parent_class))->show(item, arena, key, flags);
        if (ai) {
            nr_arena_group_set_child_transform(NR_ARENA_GROUP(ai), root->c2p);
        }
    } else {
        ai = NULL;
    }

    return ai;
}

/**
 * Virtual bbox callback.
 */
static void
sp_root_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags)
{
    SPRoot const *root = SP_ROOT(item);

    if (((SPItemClass *) (parent_class))->bbox) {
        Geom::Matrix const product( to_2geom(root->c2p) * transform );
        ((SPItemClass *) (parent_class))->bbox(item, bbox,
                                               product,
                                               flags);
    }
}

/**
 * Virtual print callback.
 */
static void
sp_root_print(SPItem *item, SPPrintContext *ctx)
{
    SPRoot *root = SP_ROOT(item);

    sp_print_bind(ctx, root->c2p, 1.0);

    if (((SPItemClass *) (parent_class))->print) {
        ((SPItemClass *) (parent_class))->print(item, ctx);
    }

    sp_print_release(ctx);
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
