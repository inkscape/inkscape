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

#include <display/curve.h>
#include <libnr/nr-matrix-fns.h>
#include <2geom/pathvector.h>
#include <2geom/bezier-curve.h>
#include <2geom/hvlinesegment.h>

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
#include "prefs-utils.h"
#include "selection.h"

#define noPATH_VERBOSE

static void sp_path_class_init(SPPathClass *klass);
static void sp_path_init(SPPath *path);
static void sp_path_finalize(GObject *obj);
static void sp_path_release(SPObject *object);

static void sp_path_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_path_set(SPObject *object, unsigned key, gchar const *value);

static Inkscape::XML::Node *sp_path_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static NR::Matrix sp_path_set_transform(SPItem *item, NR::Matrix const &xform);
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
    if (sp_lpe_item_has_path_effect(SP_LPE_ITEM(item))) {
        return g_strdup_printf(ngettext("<b>Path</b> (%i node, path effect)",
                                        "<b>Path</b> (%i nodes, path effect)",count), count);
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
            if( dynamic_cast<Geom::LineSegment const *>(&*cit) ||
                dynamic_cast<Geom::HLineSegment const *>(&*cit) ||
                dynamic_cast<Geom::VLineSegment const *>(&*cit) )
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
            if (!sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(path))) {
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
            }
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
static NR::Matrix
sp_path_set_transform(SPItem *item, NR::Matrix const &xform)
{
    SPShape *shape = (SPShape *) item;
    SPPath *path = (SPPath *) item;

    if (!shape->curve) { // 0 nodes, nothing to transform
        return NR::identity();
    }

    // Transform the original-d path or the (ordinary) path
    if (path->original_curve) {
        path->original_curve->transform(to_2geom(xform));
        sp_lpe_item_update_patheffect(path, true, true);
    } else {
        shape->curve->transform(to_2geom(xform));
    }

    // Adjust stroke
    sp_item_adjust_stroke(item, NR::expansion(xform));

    // Adjust pattern fill
    sp_item_adjust_pattern(item, xform);

    // Adjust gradient fill
    sp_item_adjust_gradient(item, xform);

    // Adjust LPE
    sp_item_adjust_livepatheffect(item, xform);

    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

    // nothing remains - we've written all of the transform, so return identity
    return NR::identity();
}

static void
sp_path_update_patheffect(SPLPEItem *lpeitem, bool write)
{
    SPShape *shape = (SPShape *) lpeitem;
    SPPath *path = (SPPath *) lpeitem;
    if (path->original_curve) {
        SPCurve *curve = path->original_curve->copy();
        sp_shape_set_curve_insync(shape, curve, TRUE);
        sp_lpe_item_perform_path_effect(SP_LPE_ITEM(shape), curve);
        SP_OBJECT(shape)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        curve->unref();

        if (write) {
            // could also do SP_OBJECT(shape)->updateRepr();  but only the d attribute needs updating.
            Inkscape::XML::Node *repr = SP_OBJECT_REPR(shape);
            if ( shape->curve != NULL ) {
                gchar *str = sp_svg_write_path(shape->curve->get_pathvector());
                repr->setAttribute("d", str);
                g_free(str);
            } else {
                repr->setAttribute("d", NULL);
            }
        }
    } else {

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
    if (path->original_curve) {
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
    if (path->original_curve) {
        return path->original_curve;
    } else {
        return path->curve;
    }
}

/* Create a single dot represented by a circle */
void freehand_create_single_dot(SPEventContext *ec, NR::Point const &pt, char const *tool, guint event_state) {
    g_return_if_fail(!strcmp(tool, "tools.freehand.pen") || !strcmp(tool, "tools.freehand.pencil"));

    SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(ec);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
    Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
    repr->setAttribute("sodipodi:type", "arc");
    SPItem *item = SP_ITEM(desktop->currentLayer()->appendChildRepr(repr));
    Inkscape::GC::release(repr);

    /* apply the tool's current style */
    sp_desktop_apply_style_tool(desktop, repr, tool, false);

    /* find out stroke width (TODO: is there an easier way??) */
    double stroke_width = 3.0;
    gchar const *style_str = NULL;
    style_str = repr->attribute("style");
    if (style_str) {
        SPStyle *style = sp_style_new(SP_ACTIVE_DOCUMENT);
        sp_style_merge_from_style_string(style, style_str);
        stroke_width = style->stroke_width.computed;
        style->stroke_width.computed = 0;
        sp_style_unref(style);
    }
    /* unset stroke and set fill color to former stroke color */
    gchar * str;
    str = g_strdup_printf("fill:#%06x;stroke:none;", sp_desktop_get_color_tool(desktop, tool, false) >> 8);
    repr->setAttribute("style", str);
    g_free(str);

    /* put the circle where the mouse click occurred and set the diameter to the
       current stroke width, multiplied by the amount specified in the preferences */
    NR::Matrix const i2d (from_2geom(sp_item_i2d_affine (item)));
    NR::Point pp = pt * i2d;
    double rad = 0.5 * prefs_get_double_attribute(tool, "dot-size", 3.0);
    if (event_state & GDK_MOD1_MASK) {
        /* TODO: We vary the dot size between 0.5*rad and 1.5*rad, where rad is the dot size
           as specified in prefs. Very simple, but it might be sufficient in practice. If not,
           we need to devise something more sophisticated. */
        double s = g_random_double_range(-0.5, 0.5);
        rad *= (1 + s);
    }
    if (event_state & GDK_SHIFT_MASK) {
        // double the point size
        rad *= 2;
    }

    sp_repr_set_svg_double (repr, "sodipodi:cx", pp[NR::X]);
    sp_repr_set_svg_double (repr, "sodipodi:cy", pp[NR::Y]);
    sp_repr_set_svg_double (repr, "sodipodi:rx", rad * stroke_width);
    sp_repr_set_svg_double (repr, "sodipodi:ry", rad * stroke_width);
    item->updateRepr();

    sp_desktop_selection(desktop)->set(item);

    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Creating single dot"));
    sp_document_done(sp_desktop_document(desktop), SP_VERB_NONE, _("Create single dot"));
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
