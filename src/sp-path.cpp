#define __SP_PATH_C__

/*
 * SVG <path> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   David Turner <novalis@gnu.org>
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if defined(WIN32) || defined(__APPLE__)
# include <glibmm/i18n.h>
#endif

#include <display/curve.h>
#include <libnr/n-art-bpath.h>
#include <libnr/nr-path.h>
#include <libnr/nr-matrix-fns.h>

#include "svg/svg.h"
#include "xml/repr.h"
#include "attributes.h"

#include "sp-path.h"

#define noPATH_VERBOSE

static void sp_path_class_init(SPPathClass *klass);
static void sp_path_init(SPPath *path);
static void sp_path_finalize(GObject *obj);
static void sp_path_release(SPObject *object);

static void sp_path_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_path_set(SPObject *object, unsigned key, gchar const *value);

static Inkscape::XML::Node *sp_path_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);
static NR::Matrix sp_path_set_transform(SPItem *item, NR::Matrix const &xform);
static gchar * sp_path_description(SPItem *item);

static void sp_path_update(SPObject *object, SPCtx *ctx, guint flags);

static SPShapeClass *parent_class;

/**
 * Gets the GType object for SPPathClass
 */
GType
sp_path_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPPathClass),
            NULL, NULL,
            (GClassInitFunc) sp_path_class_init,
            NULL, NULL,
            sizeof(SPPath),
            16,
            (GInstanceInitFunc) sp_path_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_SHAPE, "SPPath", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 *  Does the object-oriented work of initializing the class structure
 *  including parent class, and registers function pointers for
 *  the functions build, set, write, and set_transform.
 */
static void
sp_path_class_init(SPPathClass * klass)
{
    GObjectClass *gobject_class = (GObjectClass *) klass;
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPItemClass *item_class = (SPItemClass *) klass;

    parent_class = (SPShapeClass *)g_type_class_peek_parent(klass);

    gobject_class->finalize = sp_path_finalize;

    sp_object_class->build = sp_path_build;
    sp_object_class->release = sp_path_release;
    sp_object_class->set = sp_path_set;
    sp_object_class->write = sp_path_write;
    sp_object_class->update = sp_path_update;

    item_class->description = sp_path_description;
    item_class->set_transform = sp_path_set_transform;
}


gint
sp_nodes_in_path(SPPath *path)
{
    SPCurve *curve = SP_SHAPE(path)->curve;
    if (!curve) return 0;
    gint r = curve->end;
    gint i = curve->length - 1;
    if (i > r) i = r; // sometimes after switching from node editor length is wrong, e.g. f6 - draw - f2 - tab - f1, this fixes it
    for (; i >= 0; i --)
        if (SP_CURVE_BPATH(curve)[i].code == NR_MOVETO)
            r --;
    return r;
}

static gchar *
sp_path_description(SPItem * item)
{
    int count = sp_nodes_in_path(SP_PATH(item));
    return g_strdup_printf(ngettext("<b>Path</b> (%i node)",
                                    "<b>Path</b> (%i nodes)",count), count);
}

/**
 * Initializes an SPPath.  Currently does nothing.
 */
static void
sp_path_init(SPPath *path)
{
    new (&path->connEndPair) SPConnEndPair(path);
}

static void
sp_path_finalize(GObject *obj)
{
    SPPath *path = (SPPath *) obj;

    path->connEndPair.~SPConnEndPair();
}

/**
 *  Given a repr, this sets the data items in the path object such as
 *  fill & style attributes, markers, and CSS properties.
 */
static void
sp_path_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    sp_object_read_attr(object, "d");

    /* d is a required attribute */
    gchar const *d = sp_object_getAttribute(object, "d", NULL);
    if (d == NULL) {
        sp_object_set(object, sp_attribute_lookup("d"), "");
    }

    /* Are these calls actually necessary? */
    sp_object_read_attr(object, "marker");
    sp_object_read_attr(object, "marker-start");
    sp_object_read_attr(object, "marker-mid");
    sp_object_read_attr(object, "marker-end");

    sp_conn_end_pair_build(object);

    if (((SPObjectClass *) parent_class)->build) {
        ((SPObjectClass *) parent_class)->build(object, document, repr);
    }
}

static void
sp_path_release(SPObject *object)
{
    SPPath *path = SP_PATH(object);

    path->connEndPair.release();

    if (((SPObjectClass *) parent_class)->release) {
        ((SPObjectClass *) parent_class)->release(object);
    }
}

/**
 *  Sets a value in the path object given by 'key', to 'value'.  This is used
 *  for setting attributes and markers on a path object.
 */
static void
sp_path_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPPath *path = (SPPath *) object;

    switch (key) {
        case SP_ATTR_D:
            if (value) {
                NArtBpath *bpath = sp_svg_read_path(value);
                SPCurve *curve = sp_curve_new_from_bpath(bpath);
                if (curve) {
                    sp_shape_set_curve((SPShape *) path, curve, TRUE);
                    sp_curve_unref(curve);
                }
            } else {
                sp_shape_set_curve((SPShape *) path, NULL, TRUE);
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_PROP_MARKER:
        case SP_PROP_MARKER_START:
        case SP_PROP_MARKER_MID:
        case SP_PROP_MARKER_END:
            sp_shape_set_marker(object, key, value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_CONNECTOR_TYPE:
        case SP_ATTR_CONNECTION_START:
        case SP_ATTR_CONNECTION_END:
            path->connEndPair.setAttr(key, value);
            break;
        default:
            if (((SPObjectClass *) parent_class)->set) {
                ((SPObjectClass *) parent_class)->set(object, key, value);
            }
            break;
    }
}

/**
 *
 * Writes the path object into a Inkscape::XML::Node
 */
static Inkscape::XML::Node *
sp_path_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SPShape *shape = (SPShape *) object;

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = sp_repr_new("svg:path");
    }

    if ( shape->curve != NULL ) {
        NArtBpath *abp = sp_curve_first_bpath(shape->curve);
        if (abp) {
            gchar *str = sp_svg_write_path(abp);
            repr->setAttribute("d", str);
            g_free(str);
        } else {
            repr->setAttribute("d", "");
        }
    } else {
        repr->setAttribute("d", NULL);
    }

    SP_PATH(shape)->connEndPair.writeRepr(repr);

    if (((SPObjectClass *)(parent_class))->write) {
        ((SPObjectClass *)(parent_class))->write(object, repr, flags);
    }

    return repr;
}

static void
sp_path_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        flags &= ~SP_OBJECT_USER_MODIFIED_FLAG_B; // since we change the description, it's not a "just translation" anymore
    }

    if (((SPObjectClass *) parent_class)->update) {
        ((SPObjectClass *) parent_class)->update(object, ctx, flags);
    }

    SPPath *path = SP_PATH(object);
    path->connEndPair.update();
}


/**
 * Writes the given transform into the repr for the given item.
 */
static NR::Matrix
sp_path_set_transform(SPItem *item, NR::Matrix const &xform)
{
    SPShape *shape = (SPShape *) item;

    if (!shape->curve) { // 0 nodes, nothing to transform
        return NR::identity();
    }

    /* Transform the path */
    NRBPath dpath, spath;
    spath.path = SP_CURVE_BPATH(shape->curve);
    nr_path_duplicate_transform(&dpath, &spath, xform);
    SPCurve *curve = sp_curve_new_from_bpath(dpath.path);
    if (curve) {
        sp_shape_set_curve(shape, curve, TRUE);
        sp_curve_unref(curve);
    }

    // Adjust stroke
    sp_item_adjust_stroke(item, NR::expansion(xform));

    // Adjust pattern fill
    sp_item_adjust_pattern(item, xform);

    // Adjust gradient fill
    sp_item_adjust_gradient(item, xform);

    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

    // nothing remains - we've written all of the transform, so return identity
    return NR::identity();
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
