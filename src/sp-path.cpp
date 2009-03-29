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

#include <glibmm/i18n.h>

#include "live_effects/effect.h"
#include "live_effects/lpeobject.h"
#include "live_effects/lpeobject-reference.h"
#include "sp-lpe-item.h"

#include <display/curve.h>
#include <libnr/nr-matrix-fns.h>
#include <2geom/pathvector.h>
#include <2geom/bezier-curve.h>
#include <2geom/hvlinesegment.h>
#include "helper/geom-curves.h"

#include "svg/svg.h"
#include "xml/repr.h"
#include "attributes.h"

#include "sp-path.h"
#include "sp-guide.h"

#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "event-context.h"
#include "inkscape.h"
#include "style.h"
#include "message-stack.h"
#include "selection.h"

#define noPATH_VERBOSE

static void sp_path_class_init(SPPathClass *klass);
static void sp_path_init(SPPath *path);
static void sp_path_finalize(GObject *obj);
static void sp_path_release(SPObject *object);

static void sp_path_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_path_set(SPObject *object, unsigned key, gchar const *value);

static Inkscape::XML::Node *sp_path_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static Geom::Matrix sp_path_set_transform(SPItem *item, Geom::Matrix const &xform);
static gchar * sp_path_description(SPItem *item);
static void sp_path_convert_to_guides(SPItem *item);

static void sp_path_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_path_update_patheffect(SPLPEItem *lpeitem, bool write);

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
    SPLPEItemClass *lpe_item_class = (SPLPEItemClass *) klass;

    parent_class = (SPShapeClass *)g_type_class_peek_parent(klass);

    gobject_class->finalize = sp_path_finalize;

    sp_object_class->build = sp_path_build;
    sp_object_class->release = sp_path_release;
    sp_object_class->set = sp_path_set;
    sp_object_class->write = sp_path_write;
    sp_object_class->update = sp_path_update;

    item_class->description = sp_path_description;
    item_class->set_transform = sp_path_set_transform;
    item_class->convert_to_guides = sp_path_convert_to_guides;

    lpe_item_class->update_patheffect = sp_path_update_patheffect;
}


gint
sp_nodes_in_path(SPPath *path)
{
    SPCurve *curve = SP_SHAPE(path)->curve;
    if (!curve)
        return 0;
    return curve->nodes_in_path();
}

static gchar *
sp_path_description(SPItem * item)
{
    int count = sp_nodes_in_path(SP_PATH(item));
    if (SP_IS_LPE_ITEM(item) && sp_lpe_item_has_path_effect(SP_LPE_ITEM(item))) {

        Glib::ustring s;

        PathEffectList effect_list =  sp_lpe_item_get_effect_list(SP_LPE_ITEM(item));
        for (PathEffectList::iterator it = effect_list.begin(); it != effect_list.end(); it++)
        {
            LivePathEffectObject *lpeobj = (*it)->lpeobject;
            if (!lpeobj || !lpeobj->get_lpe())
                break;
            if (s.empty())
                s = lpeobj->get_lpe()->getName();
            else
                s = s + ", " + lpeobj->get_lpe()->getName();
        }

        return g_strdup_printf(ngettext("<b>Path</b> (%i node, path effect: %s)",
                                        "<b>Path</b> (%i nodes, path effect: %s)",count), count, s.c_str());
    } else {
        return g_strdup_printf(ngettext("<b>Path</b> (%i node)",
                                        "<b>Path</b> (%i nodes)",count), count);
    }
}

static void
sp_path_convert_to_guides(SPItem *item)
{
    SPPath *path = SP_PATH(item);

    SPCurve *curve = SP_SHAPE(path)->curve;
    if (!curve) return;

    std::list<std::pair<Geom::Point, Geom::Point> > pts;

    Geom::Matrix const i2d (sp_item_i2d_affine(SP_ITEM(path)));

    Geom::PathVector const & pv = curve->get_pathvector();
    for(Geom::PathVector::const_iterator pit = pv.begin(); pit != pv.end(); ++pit) {
        for(Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_default(); ++cit) {
            // only add curves for straight line segments
            if( is_straight_curve(*cit) )
            {
                pts.push_back(std::make_pair(cit->initialPoint() * i2d, cit->finalPoint() * i2d));
            }
        }
    }

    sp_guide_pt_pairs_to_guides(inkscape_active_desktop(), pts);
}

/**
 * Initializes an SPPath.
 */
static void
sp_path_init(SPPath *path)
{
    new (&path->connEndPair) SPConnEndPair(path);

    path->original_curve = NULL;
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
    /* Are these calls actually necessary? */
    sp_object_read_attr(object, "marker");
    sp_object_read_attr(object, "marker-start");
    sp_object_read_attr(object, "marker-mid");
    sp_object_read_attr(object, "marker-end");

    sp_conn_end_pair_build(object);

    if (((SPObjectClass *) parent_class)->build) {
        ((SPObjectClass *) parent_class)->build(object, document, repr);
    }

    sp_object_read_attr(object, "inkscape:original-d");
    sp_object_read_attr(object, "d");

    /* d is a required attribute */
    gchar const *d = sp_object_getAttribute(object, "d", NULL);
    if (d == NULL) {
        sp_object_set(object, sp_attribute_lookup("d"), "");
    }
}

static void
sp_path_release(SPObject *object)
{
    SPPath *path = SP_PATH(object);

    path->connEndPair.release();

    if (path->original_curve) {
        path->original_curve = path->original_curve->unref();
    }

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
        case SP_ATTR_INKSCAPE_ORIGINAL_D:
                if (value) {
                    Geom::PathVector pv = sp_svg_read_pathv(value);
                    SPCurve *curve = new SPCurve(pv);
                    if (curve) {
                        sp_path_set_original_curve(path, curve, TRUE, true);
                        curve->unref();
                    }
                } else {
                    sp_path_set_original_curve(path, NULL, TRUE, true);
                }
                object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
       case SP_ATTR_D:
                if (value) {
                    Geom::PathVector pv = sp_svg_read_pathv(value);
                    SPCurve *curve = new SPCurve(pv);
                    if (curve) {
                        sp_shape_set_curve((SPShape *) path, curve, TRUE);
                        curve->unref();
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
sp_path_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPShape *shape = (SPShape *) object;

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:path");
    }

    if ( shape->curve != NULL ) {
        gchar *str = sp_svg_write_path(shape->curve->get_pathvector());
        repr->setAttribute("d", str);
        g_free(str);
    } else {
        repr->setAttribute("d", NULL);
    }

    SPPath *path = (SPPath *) object;
    if ( path->original_curve != NULL ) {
        gchar *str = sp_svg_write_path(path->original_curve->get_pathvector());
        repr->setAttribute("inkscape:original-d", str);
        g_free(str);
    } else {
        repr->setAttribute("inkscape:original-d", NULL);
    }

    SP_PATH(shape)->connEndPair.writeRepr(repr);

    if (((SPObjectClass *)(parent_class))->write) {
        ((SPObjectClass *)(parent_class))->write(object, xml_doc, repr, flags);
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
static Geom::Matrix
sp_path_set_transform(SPItem *item, Geom::Matrix const &xform)
{
    SPShape *shape = (SPShape *) item;
    SPPath *path = (SPPath *) item;

    if (!shape->curve) { // 0 nodes, nothing to transform
        return Geom::identity();
    }

    // Transform the original-d path if this is a valid LPE item, other else the (ordinary) path
    if (path->original_curve && SP_IS_LPE_ITEM(item) && 
                                sp_lpe_item_has_path_effect(SP_LPE_ITEM(item))) {
        path->original_curve->transform(xform);
    } else {
        shape->curve->transform(xform);
    }

    // Adjust stroke
    sp_item_adjust_stroke(item, xform.descrim());

    // Adjust pattern fill
    sp_item_adjust_pattern(item, xform);

    // Adjust gradient fill
    sp_item_adjust_gradient(item, xform);

    // Adjust LPE
    sp_item_adjust_livepatheffect(item, xform);

    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

    // nothing remains - we've written all of the transform, so return identity
    return Geom::identity();
}


static void
sp_path_update_patheffect(SPLPEItem *lpeitem, bool write)
{
    SPShape * const shape = (SPShape *) lpeitem;
    SPPath * const path = (SPPath *) lpeitem;
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(shape);

    if (path->original_curve) {
        SPCurve *curve = path->original_curve->copy();
        /* if a path does not have an lpeitem applied, then reset the curve to the original_curve.
         * This is very important for LPEs to work properly! (the bbox might be recalculated depending on the curve in shape)*/
        sp_shape_set_curve_insync(shape, curve, TRUE);

        bool success = sp_lpe_item_perform_path_effect(SP_LPE_ITEM(shape), curve);
        if (success && write) {
            // could also do SP_OBJECT(shape)->updateRepr();  but only the d attribute needs updating.
            if ( shape->curve != NULL ) {
                gchar *str = sp_svg_write_path(shape->curve->get_pathvector());
                repr->setAttribute("d", str);
                g_free(str);
            } else {
                repr->setAttribute("d", NULL);
            }
        } else {
            // LPE was unsuccesfull. Read the old 'd'-attribute.
            if (gchar const * value = repr->attribute("d")) {
                Geom::PathVector pv = sp_svg_read_pathv(value);
                SPCurve *oldcurve = new SPCurve(pv);
                if (oldcurve) {
                    sp_shape_set_curve(shape, oldcurve, TRUE);
                    oldcurve->unref();
                }
            }
        }
        SP_OBJECT(shape)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        curve->unref();
    }
}


/**
 * Adds a original_curve to the path.  If owner is specified, a reference
 * will be made, otherwise the curve will be copied into the path.
 * Any existing curve in the path will be unreferenced first.
 * This routine triggers reapplication of an effect if present
 * and also triggers a request to update the display. Does not write
 * result to XML when write=false.
 */
void
sp_path_set_original_curve (SPPath *path, SPCurve *curve, unsigned int owner, bool write)
{
    if (path->original_curve) {
        path->original_curve = path->original_curve->unref();
    }
    if (curve) {
        if (owner) {
            path->original_curve = curve->ref();
        } else {
            path->original_curve = curve->copy();
        }
    }
    sp_lpe_item_update_patheffect(path, true, write);
    SP_OBJECT(path)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Return duplicate of original_curve (if any exists) or NULL if there is no curve
 */
SPCurve *
sp_path_get_original_curve (SPPath *path)
{
    if (path->original_curve) {
        return path->original_curve->copy();
    }
    return NULL;
}

/**
 * Return duplicate of edittable curve which is original_curve if it exists or
 * shape->curve if not.
 */
SPCurve*
sp_path_get_curve_for_edit (SPPath *path)
{
    if (path->original_curve && SP_IS_LPE_ITEM(path) && 
                                sp_lpe_item_has_path_effect(SP_LPE_ITEM(path))) {
        return sp_path_get_original_curve(path);
    } else {
        return sp_shape_get_curve( (SPShape *) path );
    }
}

/**
 * Return a reference to original_curve if it exists or
 * shape->curve if not.
 */
const SPCurve*
sp_path_get_curve_reference (SPPath *path)
{
    if (path->original_curve && SP_IS_LPE_ITEM(path) && 
                                sp_lpe_item_has_path_effect(SP_LPE_ITEM(path))) {
        return path->original_curve;
    } else {
        return path->curve;
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
