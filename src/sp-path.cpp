/*
 * SVG <path> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   David Turner <novalis@gnu.org>
 *   Abhishek Sharma
 *   Johan Engelen
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 1999-2012 Authors
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

#include "display/curve.h"
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
static Geom::Affine sp_path_set_transform(SPItem *item, Geom::Affine const &xform);
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


gint SPPath::nodesInPath() const
{
    return curve ? curve->nodes_in_path() : 0;
}

static gchar *
sp_path_description(SPItem * item)
{
    int count = SP_PATH(item)->nodesInPath();
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

    Geom::Affine const i2dt(path->i2dt_affine());

    Geom::PathVector const & pv = curve->get_pathvector();
    for(Geom::PathVector::const_iterator pit = pv.begin(); pit != pv.end(); ++pit) {
        for(Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_default(); ++cit) {
            // only add curves for straight line segments
            if( is_straight_curve(*cit) )
            {
                pts.push_back(std::make_pair(cit->initialPoint() * i2dt, cit->finalPoint() * i2dt));
            }
        }
    }

    sp_guide_pt_pairs_to_guides(item->document, pts);
}

/**
 * Initializes an SPPath.
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
    /* Are these calls actually necessary? */
    object->readAttr( "marker" );
    object->readAttr( "marker-start" );
    object->readAttr( "marker-mid" );
    object->readAttr( "marker-end" );

    sp_conn_end_pair_build(object);

    if (((SPObjectClass *) parent_class)->build) {
        ((SPObjectClass *) parent_class)->build(object, document, repr);
    }

    object->readAttr( "inkscape:original-d" );
    object->readAttr( "d" );

    /* d is a required attribute */
    gchar const *d = object->getAttribute("d", NULL);
    if (d == NULL) {
        object->setKeyValue( sp_attribute_lookup("d"), "");
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
        case SP_ATTR_INKSCAPE_ORIGINAL_D:
                if (value) {
                    Geom::PathVector pv = sp_svg_read_pathv(value);
                    SPCurve *curve = new SPCurve(pv);
                    if (curve) {
                        path->set_original_curve(curve, TRUE, true);
                        curve->unref();
                    }
                } else {
                    path->set_original_curve(NULL, TRUE, true);
                }
                object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
       case SP_ATTR_D:
                if (value) {
                    Geom::PathVector pv = sp_svg_read_pathv(value);
                    SPCurve *curve = new SPCurve(pv);
                    if (curve) {
                        ((SPShape *) path)->setCurve(curve, TRUE);
                        curve->unref();
                    }
                } else {
                    ((SPShape *) path)->setCurve(NULL, TRUE);
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
        case SP_ATTR_CONNECTOR_CURVATURE:
        case SP_ATTR_CONNECTION_START:
        case SP_ATTR_CONNECTION_END:
        case SP_ATTR_CONNECTION_START_POINT:
        case SP_ATTR_CONNECTION_END_POINT:
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

#ifdef PATH_VERBOSE
g_message("sp_path_write writes 'd' attribute");
#endif
    if ( shape->curve != NULL ) {
        gchar *str = sp_svg_write_path(shape->curve->get_pathvector());
        repr->setAttribute("d", str);
        g_free(str);
    } else {
        repr->setAttribute("d", NULL);
    }

    if (flags & SP_OBJECT_WRITE_EXT) {
        if ( shape->curve_before_lpe != NULL ) {
            gchar *str = sp_svg_write_path(shape->curve_before_lpe->get_pathvector());
            repr->setAttribute("inkscape:original-d", str);
            g_free(str);
        } else {
            repr->setAttribute("inkscape:original-d", NULL);
        }
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
static Geom::Affine
sp_path_set_transform(SPItem *item, Geom::Affine const &xform)
{
    if (!SP_IS_PATH(item)) {
        return Geom::identity();
    }
    SPPath *path = SP_PATH(item);

    if (!path->curve) { // 0 nodes, nothing to transform
        return Geom::identity();
    }

    // Transform the original-d path if this is a valid LPE item, other else the (ordinary) path
    if (path->curve_before_lpe && sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(item))) {
        path->curve_before_lpe->transform(xform);
    } else {
        path->curve->transform(xform);
    }

    // Adjust stroke
    item->adjust_stroke(xform.descrim());

    // Adjust pattern fill
    item->adjust_pattern(xform);

    // Adjust gradient fill
    item->adjust_gradient(xform);

    // Adjust LPE
    item->adjust_livepatheffect(xform);

    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

    // nothing remains - we've written all of the transform, so return identity
    return Geom::identity();
}


static void
sp_path_update_patheffect(SPLPEItem *lpeitem, bool write)
{
    SPShape * const shape = (SPShape *) lpeitem;
    Inkscape::XML::Node *repr = shape->getRepr();

#ifdef PATH_VERBOSE
g_message("sp_path_update_patheffect");
#endif

    if (shape->curve_before_lpe && sp_lpe_item_has_path_effect_recursive(lpeitem)) {
        SPCurve *curve = shape->curve_before_lpe->copy();
        /* if a path has an lpeitem applied, then reset the curve to the curve_before_lpe.
         * This is very important for LPEs to work properly! (the bbox might be recalculated depending on the curve in shape)*/
        shape->setCurveInsync(curve, TRUE);

        bool success = sp_lpe_item_perform_path_effect(SP_LPE_ITEM(shape), curve);
        if (success && write) {
            // could also do shape->getRepr()->updateRepr();  but only the d attribute needs updating.
#ifdef PATH_VERBOSE
g_message("sp_path_update_patheffect writes 'd' attribute");
#endif
            if ( shape->curve != NULL ) {
                gchar *str = sp_svg_write_path(shape->curve->get_pathvector());
                repr->setAttribute("d", str);
                g_free(str);
            } else {
                repr->setAttribute("d", NULL);
            }
        } else if (!success) {
            // LPE was unsuccesfull. Read the old 'd'-attribute.
            if (gchar const * value = repr->attribute("d")) {
                Geom::PathVector pv = sp_svg_read_pathv(value);
                SPCurve *oldcurve = new SPCurve(pv);
                if (oldcurve) {
                    shape->setCurve(oldcurve, TRUE);
                    oldcurve->unref();
                }
            }
        }
        shape->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
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
void SPPath::set_original_curve (SPCurve *new_curve, unsigned int owner, bool write)
{
    if (curve_before_lpe) {
        curve_before_lpe = curve_before_lpe->unref();
    }
    if (new_curve) {
        if (owner) {
            curve_before_lpe = new_curve->ref();
        } else {
            curve_before_lpe = new_curve->copy();
        }
    }
    sp_lpe_item_update_patheffect(this, true, write);
    requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Return duplicate of curve_before_lpe (if any exists) or NULL if there is no curve
 */
SPCurve * SPPath::get_original_curve () const
{
    if (curve_before_lpe) {
        return curve_before_lpe->copy();
    }
    return NULL;
}

/**
 * Return duplicate of edittable curve which is curve_before_lpe if it exists or
 * shape->curve if not.
 */
SPCurve* SPPath::get_curve_for_edit () const
{
    if (!SP_IS_PATH(this)) {
        return NULL;
    }
    if (curve_before_lpe && sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(this))) {
        return get_original_curve();
    } else {
        return getCurve();
    }
}

/**
 * Returns \c curve_before_lpe if it is not NULL and a valid LPE is applied or
 * \c curve if not.
 */
const SPCurve* SPPath::get_curve_reference () const
{
    if (!SP_IS_PATH(this)) {
        return NULL;
    }
    if (curve_before_lpe && sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(this))) {
        return curve_before_lpe;
    } else {
        return curve;
    }
}

/**
 * Returns \c curve_before_lpe if it is not NULL and a valid LPE is applied or \c curve if not.
 * \todo should only be available to class friends!
 */
SPCurve* SPPath::get_curve ()
{
    if (!SP_IS_PATH(this)) {
        return NULL;
    }
    if (curve_before_lpe && sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(this))) {
        return curve_before_lpe;
    } else {
        return curve;
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
