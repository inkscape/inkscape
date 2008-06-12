#define __SP_ELLIPSE_C__

/*
 * SVG <ellipse> and related implementations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Mitsuru Oka
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include "libnr/n-art-bpath.h"
#include "libnr/nr-path.h"
#include "libnr/nr-matrix-fns.h"
#include "svg/svg.h"
#include "svg/path-string.h"
#include "xml/repr.h"
#include "attributes.h"
#include "style.h"
#include "display/curve.h"
#include <glibmm/i18n.h>

#include "document.h"
#include "sp-ellipse.h"

#include "prefs-utils.h"

/* Common parent class */

#define noELLIPSE_VERBOSE

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SP_2PI (2 * M_PI)

#if 1
/* Hmmm... shouldn't this also qualify */
/* Whether it is faster or not, well, nobody knows */
#define sp_round(v,m) (((v) < 0.0) ? ((ceil((v) / (m) - 0.5)) * (m)) : ((floor((v) / (m) + 0.5)) * (m)))
#else
/* we do not use C99 round(3) function yet */
static double sp_round(double x, double y)
{
    double remain;

    g_assert(y > 0.0);

    /* return round(x/y) * y; */

    remain = fmod(x, y);

    if (remain >= 0.5*y)
        return x - remain + y;
    else
        return x - remain;
}
#endif

static void sp_genericellipse_class_init(SPGenericEllipseClass *klass);
static void sp_genericellipse_init(SPGenericEllipse *ellipse);

static void sp_genericellipse_update(SPObject *object, SPCtx *ctx, guint flags);

static void sp_genericellipse_snappoints(SPItem const *item, SnapPointsIter p);

static void sp_genericellipse_set_shape(SPShape *shape);
static void sp_genericellipse_update_patheffect (SPLPEItem *lpeitem, bool write);

static Inkscape::XML::Node *sp_genericellipse_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr,
                                                    guint flags);

static gboolean sp_arc_set_elliptical_path_attribute(SPArc *arc, Inkscape::XML::Node *repr);

static SPShapeClass *ge_parent_class;

GType
sp_genericellipse_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPGenericEllipseClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) sp_genericellipse_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(SPGenericEllipse),
            16,   /* n_preallocs */
            (GInstanceInitFunc) sp_genericellipse_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_SHAPE, "SPGenericEllipse", &info, (GTypeFlags)0);
    }
    return type;
}

static void sp_genericellipse_class_init(SPGenericEllipseClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPItemClass *item_class = (SPItemClass *) klass;
    SPLPEItemClass *lpe_item_class = (SPLPEItemClass *) klass;
    SPShapeClass *shape_class = (SPShapeClass *) klass;

    ge_parent_class = (SPShapeClass*) g_type_class_ref(SP_TYPE_SHAPE);

    sp_object_class->update = sp_genericellipse_update;
    sp_object_class->write = sp_genericellipse_write;

    item_class->snappoints = sp_genericellipse_snappoints;

    shape_class->set_shape = sp_genericellipse_set_shape;
    lpe_item_class->update_patheffect = sp_genericellipse_update_patheffect;
}

static void
sp_genericellipse_init(SPGenericEllipse *ellipse)
{
    ellipse->cx.unset();
    ellipse->cy.unset();
    ellipse->rx.unset();
    ellipse->ry.unset();

    ellipse->start = 0.0;
    ellipse->end = SP_2PI;
    ellipse->closed = TRUE;
}

static void
sp_genericellipse_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        SPGenericEllipse *ellipse = (SPGenericEllipse *) object;
        SPStyle const *style = object->style;
        double const d = 1.0 / NR::expansion(((SPItemCtx const *) ctx)->i2vp);
        double const em = style->font_size.computed;
        double const ex = em * 0.5; // fixme: get from pango or libnrtype
        ellipse->cx.update(em, ex, d);
        ellipse->cy.update(em, ex, d);
        ellipse->rx.update(em, ex, d);
        ellipse->ry.update(em, ex, d);
        sp_shape_set_shape((SPShape *) object);
    }

    if (((SPObjectClass *) ge_parent_class)->update)
        ((SPObjectClass *) ge_parent_class)->update(object, ctx, flags);
}

static void
sp_genericellipse_update_patheffect(SPLPEItem *lpeitem, bool write)
{
    SPShape *shape = (SPShape *) lpeitem;
    sp_genericellipse_set_shape(shape);

    if (write) {
        Inkscape::XML::Node *repr = SP_OBJECT_REPR(shape);
        if ( shape->curve != NULL ) {
            gchar *str = sp_svg_write_path(shape->curve->get_pathvector());
            repr->setAttribute("d", str);
            g_free(str);
        } else {
            repr->setAttribute("d", NULL);
        }
    }

    ((SPObject *)shape)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}


#define C1 0.552

/* fixme: Think (Lauris) */

static void sp_genericellipse_set_shape(SPShape *shape)
{
    double rx, ry, s, e;
    double x0, y0, x1, y1, x2, y2, x3, y3;
    double len;
    gint slice = FALSE;
    gint i;

    SPGenericEllipse *ellipse = (SPGenericEllipse *) shape;

    if ((ellipse->rx.computed < 1e-18) || (ellipse->ry.computed < 1e-18)) return;
    if (fabs(ellipse->end - ellipse->start) < 1e-9) return;

    sp_genericellipse_normalize(ellipse);

    rx = ellipse->rx.computed;
    ry = ellipse->ry.computed;

    // figure out if we have a slice, guarding against rounding errors
    len = fmod(ellipse->end - ellipse->start, SP_2PI);
    if (len < 0.0) len += SP_2PI;
    if (fabs(len) < 1e-8 || fabs(len - SP_2PI) < 1e-8) {
        slice = FALSE;
        ellipse->end = ellipse->start + SP_2PI;
    } else {
        slice = TRUE;
    }

    NR::Matrix aff = NR::Matrix(NR::scale(rx, ry));
    aff[4] = ellipse->cx.computed;
    aff[5] = ellipse->cy.computed;

    NArtBpath bpath[16];
    i = 0;
    if (ellipse->closed) {
        bpath[i].code = NR_MOVETO;
    } else {
        bpath[i].code = NR_MOVETO_OPEN;
    }
    bpath[i].x3 = cos(ellipse->start);
    bpath[i].y3 = sin(ellipse->start);
    i++;

    for (s = ellipse->start; s < ellipse->end; s += M_PI_2) {
        e = s + M_PI_2;
        if (e > ellipse->end)
            e = ellipse->end;
        len = C1 * (e - s) / M_PI_2;
        x0 = cos(s);
        y0 = sin(s);
        x1 = x0 + len * cos(s + M_PI_2);
        y1 = y0 + len * sin(s + M_PI_2);
        x3 = cos(e);
        y3 = sin(e);
        x2 = x3 + len * cos(e - M_PI_2);
        y2 = y3 + len * sin(e - M_PI_2);
#ifdef ELLIPSE_VERBOSE
        g_print("step %d s %f e %f coords %f %f %f %f %f %f\n",
                i, s, e, x1, y1, x2, y2, x3, y3);
#endif
        bpath[i].code = NR_CURVETO;
        bpath[i].x1 = x1;
        bpath[i].y1 = y1;
        bpath[i].x2 = x2;
        bpath[i].y2 = y2;
        bpath[i].x3 = x3;
        bpath[i].y3 = y3;
        i++;
    }

    if (slice && ellipse->closed) {
        bpath[i].code = NR_LINETO;
        bpath[i].x3 = 0.0;
        bpath[i].y3 = 0.0;
        i++;
        bpath[i].code = NR_LINETO;
        bpath[i].x3 = bpath[0].x3;
        bpath[i].y3 = bpath[0].y3;
        i++;
    } else if (ellipse->closed) {
        bpath[i-1].x3 = bpath[0].x3;
        bpath[i-1].y3 = bpath[0].y3;
    }

    bpath[i].code = NR_END;
    SPCurve *c = SPCurve::new_from_bpath(nr_artpath_affine(bpath, aff));
    g_assert(c != NULL);

    sp_lpe_item_perform_path_effect(SP_LPE_ITEM (ellipse), c);
    sp_shape_set_curve_insync((SPShape *) ellipse, c, TRUE);
    c->unref();
}

static void sp_genericellipse_snappoints(SPItem const *item, SnapPointsIter p)
{
    g_assert(item != NULL);
    g_assert(SP_IS_GENERICELLIPSE(item));
    
    SPGenericEllipse *ellipse = SP_GENERICELLIPSE(item);
    sp_genericellipse_normalize(ellipse);
    NR::Matrix const i2d = from_2geom(sp_item_i2d_affine(item));

    // figure out if we have a slice, whilst guarding against rounding errors
    bool slice = false;
    double len = fmod(ellipse->end - ellipse->start, SP_2PI);
    if (len < 0.0) len += SP_2PI;
    if (fabs(len) < 1e-8 || fabs(len - SP_2PI) < 1e-8) {
        slice = false;
        ellipse->end = ellipse->start + SP_2PI;
    } else {
        slice = true;
    }

    double rx = ellipse->rx.computed;
    double ry = ellipse->ry.computed;    
    double cx = ellipse->cx.computed;
    double cy = ellipse->cy.computed;
    
    // Snap to the 4 quadrant points of the ellipse, but only if the arc
    // spans far enough to include them
    double angle = 0;
    for (angle = 0; angle < SP_2PI; angle += M_PI_2) {
        if (angle >= ellipse->start && angle <= ellipse->end) {
            *p = NR::Point(cx + cos(angle)*rx, cy + sin(angle)*ry) * i2d;
        }
    }
    
    // And if we have a slice, also snap to the endpoints and the centre point 
    if (slice) {
        // Add the centre, if we have a closed slice
        if (ellipse->closed) {
            *p = NR::Point(cx, cy) * i2d;
        }
        // Add the start point, if it's not coincident with a quadrant point
        if (fmod(ellipse->start, M_PI_2) != 0.0 ) {    
            *p = NR::Point(cx + cos(ellipse->start)*rx, cy + sin(ellipse->start)*ry) * i2d;
        } 
        // Add the end point, if it's not coincident with a quadrant point
        if (fmod(ellipse->end, M_PI_2) != 0.0 ) {    
            *p = NR::Point(cx + cos(ellipse->end)*rx, cy + sin(ellipse->end)*ry) * i2d;
        }
    }
}

void
sp_genericellipse_normalize(SPGenericEllipse *ellipse)
{
    ellipse->start = fmod(ellipse->start, SP_2PI);
    ellipse->end = fmod(ellipse->end, SP_2PI);

    if (ellipse->start < 0.0)
        ellipse->start += SP_2PI;
    double diff = ellipse->start - ellipse->end;
    if (diff >= 0.0)
        ellipse->end += diff - fmod(diff, SP_2PI) + SP_2PI;

    /* Now we keep: 0 <= start < end <= 2*PI */
}

static Inkscape::XML::Node *sp_genericellipse_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPGenericEllipse *ellipse = SP_GENERICELLIPSE(object);

    if (flags & SP_OBJECT_WRITE_EXT) {
        if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
            repr = xml_doc->createElement("svg:path");
        }

        sp_repr_set_svg_double(repr, "sodipodi:cx", ellipse->cx.computed);
        sp_repr_set_svg_double(repr, "sodipodi:cy", ellipse->cy.computed);
        sp_repr_set_svg_double(repr, "sodipodi:rx", ellipse->rx.computed);
        sp_repr_set_svg_double(repr, "sodipodi:ry", ellipse->ry.computed);

        if (SP_IS_ARC(ellipse))
            sp_arc_set_elliptical_path_attribute(SP_ARC(object), SP_OBJECT_REPR(object));
    }

    if (((SPObjectClass *) ge_parent_class)->write)
        ((SPObjectClass *) ge_parent_class)->write(object, xml_doc, repr, flags);

    return repr;
}

/* SVG <ellipse> element */

static void sp_ellipse_class_init(SPEllipseClass *klass);
static void sp_ellipse_init(SPEllipse *ellipse);

static void sp_ellipse_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static Inkscape::XML::Node *sp_ellipse_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_ellipse_set(SPObject *object, unsigned int key, gchar const *value);
static gchar *sp_ellipse_description(SPItem *item);

static SPGenericEllipseClass *ellipse_parent_class;

GType
sp_ellipse_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPEllipseClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) sp_ellipse_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(SPEllipse),
            16,   /* n_preallocs */
            (GInstanceInitFunc) sp_ellipse_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_GENERICELLIPSE, "SPEllipse", &info, (GTypeFlags)0);
    }
    return type;
}

static void sp_ellipse_class_init(SPEllipseClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPItemClass *item_class = (SPItemClass *) klass;

    ellipse_parent_class = (SPGenericEllipseClass*) g_type_class_ref(SP_TYPE_GENERICELLIPSE);

    sp_object_class->build = sp_ellipse_build;
    sp_object_class->write = sp_ellipse_write;
    sp_object_class->set = sp_ellipse_set;

    item_class->description = sp_ellipse_description;
}

static void
sp_ellipse_init(SPEllipse */*ellipse*/)
{
    /* Nothing special */
}

static void
sp_ellipse_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) ellipse_parent_class)->build)
        (* ((SPObjectClass *) ellipse_parent_class)->build) (object, document, repr);

    sp_object_read_attr(object, "cx");
    sp_object_read_attr(object, "cy");
    sp_object_read_attr(object, "rx");
    sp_object_read_attr(object, "ry");
}

static Inkscape::XML::Node *
sp_ellipse_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPGenericEllipse *ellipse;

    ellipse = SP_GENERICELLIPSE(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:ellipse");
    }

    sp_repr_set_svg_double(repr, "cx", ellipse->cx.computed);
    sp_repr_set_svg_double(repr, "cy", ellipse->cy.computed);
    sp_repr_set_svg_double(repr, "rx", ellipse->rx.computed);
    sp_repr_set_svg_double(repr, "ry", ellipse->ry.computed);

    if (((SPObjectClass *) ellipse_parent_class)->write)
        (* ((SPObjectClass *) ellipse_parent_class)->write) (object, xml_doc, repr, flags);

    return repr;
}

static void
sp_ellipse_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPGenericEllipse *ellipse;

    ellipse = SP_GENERICELLIPSE(object);

    switch (key) {
        case SP_ATTR_CX:
            ellipse->cx.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_CY:
            ellipse->cy.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_RX:
            if (!ellipse->rx.read(value) || (ellipse->rx.value <= 0.0)) {
                ellipse->rx.unset();
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_RY:
            if (!ellipse->ry.read(value) || (ellipse->ry.value <= 0.0)) {
                ellipse->ry.unset();
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) ellipse_parent_class)->set)
                ((SPObjectClass *) ellipse_parent_class)->set(object, key, value);
            break;
    }
}

static gchar *sp_ellipse_description(SPItem */*item*/)
{
    return g_strdup(_("<b>Ellipse</b>"));
}


void
sp_ellipse_position_set(SPEllipse *ellipse, gdouble x, gdouble y, gdouble rx, gdouble ry)
{
    SPGenericEllipse *ge;

    g_return_if_fail(ellipse != NULL);
    g_return_if_fail(SP_IS_ELLIPSE(ellipse));

    ge = SP_GENERICELLIPSE(ellipse);

    ge->cx.computed = x;
    ge->cy.computed = y;
    ge->rx.computed = rx;
    ge->ry.computed = ry;

    ((SPObject *)ge)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/* SVG <circle> element */

static void sp_circle_class_init(SPCircleClass *klass);
static void sp_circle_init(SPCircle *circle);

static void sp_circle_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static Inkscape::XML::Node *sp_circle_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_circle_set(SPObject *object, unsigned int key, gchar const *value);
static gchar *sp_circle_description(SPItem *item);

static SPGenericEllipseClass *circle_parent_class;

GType
sp_circle_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPCircleClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) sp_circle_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(SPCircle),
            16,   /* n_preallocs */
            (GInstanceInitFunc) sp_circle_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_GENERICELLIPSE, "SPCircle", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_circle_class_init(SPCircleClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPItemClass *item_class = (SPItemClass *) klass;

    circle_parent_class = (SPGenericEllipseClass*) g_type_class_ref(SP_TYPE_GENERICELLIPSE);

    sp_object_class->build = sp_circle_build;
    sp_object_class->write = sp_circle_write;
    sp_object_class->set = sp_circle_set;

    item_class->description = sp_circle_description;
}

static void
sp_circle_init(SPCircle */*circle*/)
{
    /* Nothing special */
}

static void
sp_circle_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) circle_parent_class)->build)
        (* ((SPObjectClass *) circle_parent_class)->build)(object, document, repr);

    sp_object_read_attr(object, "cx");
    sp_object_read_attr(object, "cy");
    sp_object_read_attr(object, "r");
}

static Inkscape::XML::Node *
sp_circle_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPGenericEllipse *ellipse;

    ellipse = SP_GENERICELLIPSE(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:circle");
    }

    sp_repr_set_svg_double(repr, "cx", ellipse->cx.computed);
    sp_repr_set_svg_double(repr, "cy", ellipse->cy.computed);
    sp_repr_set_svg_double(repr, "r", ellipse->rx.computed);

    if (((SPObjectClass *) circle_parent_class)->write)
        ((SPObjectClass *) circle_parent_class)->write(object, xml_doc, repr, flags);

    return repr;
}

static void
sp_circle_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPGenericEllipse *ge;

    ge = SP_GENERICELLIPSE(object);

    switch (key) {
        case SP_ATTR_CX:
            ge->cx.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_CY:
            ge->cy.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_R:
            if (!ge->rx.read(value) || ge->rx.value <= 0.0) {
                ge->rx.unset();
            }
            ge->ry = ge->rx;
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) circle_parent_class)->set)
                ((SPObjectClass *) circle_parent_class)->set(object, key, value);
            break;
    }
}

static gchar *sp_circle_description(SPItem */*item*/)
{
    return g_strdup(_("<b>Circle</b>"));
}

/* <path sodipodi:type="arc"> element */

static void sp_arc_class_init(SPArcClass *klass);
static void sp_arc_init(SPArc *arc);

static void sp_arc_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static Inkscape::XML::Node *sp_arc_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_arc_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_arc_modified(SPObject *object, guint flags);

static gchar *sp_arc_description(SPItem *item);

static SPGenericEllipseClass *arc_parent_class;

GType
sp_arc_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPArcClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) sp_arc_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(SPArc),
            16,   /* n_preallocs */
            (GInstanceInitFunc) sp_arc_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_GENERICELLIPSE, "SPArc", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_arc_class_init(SPArcClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPItemClass *item_class = (SPItemClass *) klass;

    arc_parent_class = (SPGenericEllipseClass*) g_type_class_ref(SP_TYPE_GENERICELLIPSE);

    sp_object_class->build = sp_arc_build;
    sp_object_class->write = sp_arc_write;
    sp_object_class->set = sp_arc_set;
    sp_object_class->modified = sp_arc_modified;

    item_class->description = sp_arc_description;
}

static void
sp_arc_init(SPArc */*arc*/)
{
    /* Nothing special */
}

static void
sp_arc_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) arc_parent_class)->build)
        (* ((SPObjectClass *) arc_parent_class)->build) (object, document, repr);

    Inkscape::Version version = sp_object_get_sodipodi_version(object);

    sp_object_read_attr(object, "sodipodi:cx");
    sp_object_read_attr(object, "sodipodi:cy");
    sp_object_read_attr(object, "sodipodi:rx");
    sp_object_read_attr(object, "sodipodi:ry");

    sp_object_read_attr(object, "sodipodi:start");
    sp_object_read_attr(object, "sodipodi:end");
    sp_object_read_attr(object, "sodipodi:open");
}

/*
 * sp_arc_set_elliptical_path_attribute:
 *
 * Convert center to endpoint parameterization and set it to repr.
 *
 * See SVG 1.0 Specification W3C Recommendation
 * ``F.6 Ellptical arc implementation notes'' for more detail.
 */
static gboolean
sp_arc_set_elliptical_path_attribute(SPArc *arc, Inkscape::XML::Node *repr)
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(arc);

    Inkscape::SVG::PathString str;

    NR::Point p1 = sp_arc_get_xy(arc, ge->start);
    NR::Point p2 = sp_arc_get_xy(arc, ge->end);
    double rx = ge->rx.computed;
    double ry = ge->ry.computed;

    str.moveTo(p1);

    double dt = fmod(ge->end - ge->start, SP_2PI);
    if (fabs(dt) < 1e-6) {
        NR::Point ph = sp_arc_get_xy(arc, (ge->start + ge->end) / 2.0);
        str.arcTo(rx, ry, 0, true, true, ph)
           .arcTo(rx, ry, 0, true, true, p2)
           .closePath();
    } else {
        bool fa = (fabs(dt) > M_PI);
        bool fs = (dt > 0);
        str.arcTo(rx, ry, 0, fa, fs, p2);
        if (ge->closed) {
            NR::Point center = NR::Point(ge->cx.computed, ge->cy.computed);
            str.lineTo(center).closePath();
        }
    }

    repr->setAttribute("d", str.c_str());
    return true;
}

static Inkscape::XML::Node *
sp_arc_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(object);
    SPArc *arc = SP_ARC(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:path");
    }

    if (flags & SP_OBJECT_WRITE_EXT) {
        repr->setAttribute("sodipodi:type", "arc");
        sp_repr_set_svg_double(repr, "sodipodi:cx", ge->cx.computed);
        sp_repr_set_svg_double(repr, "sodipodi:cy", ge->cy.computed);
        sp_repr_set_svg_double(repr, "sodipodi:rx", ge->rx.computed);
        sp_repr_set_svg_double(repr, "sodipodi:ry", ge->ry.computed);

        // write start and end only if they are non-trivial; otherwise remove
        gdouble len = fmod(ge->end - ge->start, SP_2PI);
        if (len < 0.0) len += SP_2PI;
        if (!(fabs(len) < 1e-8 || fabs(len - SP_2PI) < 1e-8)) {
            sp_repr_set_svg_double(repr, "sodipodi:start", ge->start);
            sp_repr_set_svg_double(repr, "sodipodi:end", ge->end);
            repr->setAttribute("sodipodi:open", (!ge->closed) ? "true" : NULL);
        } else {
            repr->setAttribute("sodipodi:end", NULL);
            repr->setAttribute("sodipodi:start", NULL);
            repr->setAttribute("sodipodi:open", NULL);
        }
    }

    // write d=
    sp_arc_set_elliptical_path_attribute(arc, repr);

    if (((SPObjectClass *) arc_parent_class)->write)
        ((SPObjectClass *) arc_parent_class)->write(object, xml_doc, repr, flags);

    return repr;
}

static void
sp_arc_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(object);

    switch (key) {
        case SP_ATTR_SODIPODI_CX:
            ge->cx.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_SODIPODI_CY:
            ge->cy.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_SODIPODI_RX:
            if (!ge->rx.read(value) || ge->rx.computed <= 0.0) {
                ge->rx.unset();
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_SODIPODI_RY:
            if (!ge->ry.read(value) || ge->ry.computed <= 0.0) {
                ge->ry.unset();
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_SODIPODI_START:
            if (value) {
                sp_svg_number_read_d(value, &ge->start);
            } else {
                ge->start = 0;
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_SODIPODI_END:
            if (value) {
                sp_svg_number_read_d(value, &ge->end);
            } else {
                ge->end = 2 * M_PI;
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_SODIPODI_OPEN:
            ge->closed = (!value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) arc_parent_class)->set)
                ((SPObjectClass *) arc_parent_class)->set(object, key, value);
            break;
    }
}

static void
sp_arc_modified(SPObject *object, guint flags)
{
    if (flags & SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG) {
        sp_shape_set_shape((SPShape *) object);
    }

    if (((SPObjectClass *) arc_parent_class)->modified)
        ((SPObjectClass *) arc_parent_class)->modified(object, flags);
}

static gchar *sp_arc_description(SPItem *item)
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(item);

    gdouble len = fmod(ge->end - ge->start, SP_2PI);
    if (len < 0.0) len += SP_2PI;
    if (!(fabs(len) < 1e-8 || fabs(len - SP_2PI) < 1e-8)) {
        if (ge->closed) {
            return g_strdup(_("<b>Segment</b>"));
        } else {
            return g_strdup(_("<b>Arc</b>"));
        }
    } else {
        return g_strdup(_("<b>Ellipse</b>"));
    }
}

void
sp_arc_position_set(SPArc *arc, gdouble x, gdouble y, gdouble rx, gdouble ry)
{
    g_return_if_fail(arc != NULL);
    g_return_if_fail(SP_IS_ARC(arc));

    SPGenericEllipse *ge = SP_GENERICELLIPSE(arc);

    ge->cx.computed = x;
    ge->cy.computed = y;
    ge->rx.computed = rx;
    ge->ry.computed = ry;
    if (prefs_get_double_attribute("tools.shapes.arc", "start", 0.0) != 0)
        ge->start = prefs_get_double_attribute("tools.shapes.arc", "start", 0.0);
    if (prefs_get_double_attribute("tools.shapes.arc", "end", 0.0) != 0)
        ge->end = prefs_get_double_attribute("tools.shapes.arc", "end", 0.0);
    if (!prefs_get_string_attribute("tools.shapes.arc", "open"))
        ge->closed = 1;
    else
        ge->closed = 0;

    ((SPObject *)arc)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

NR::Point sp_arc_get_xy(SPArc *arc, gdouble arg)
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(arc);

    return NR::Point(ge->rx.computed * cos(arg) + ge->cx.computed,
                     ge->ry.computed * sin(arg) + ge->cy.computed);
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
