/*
 * SVG <rect> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
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


#include <display/curve.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-fns.h>

#include "inkscape.h"
#include "document.h"
#include "attributes.h"
#include "style.h"
#include "sp-rect.h"
#include <glibmm/i18n.h>
#include "xml/repr.h"
#include "sp-guide.h"
#include "preferences.h"

#define noRECT_VERBOSE

static void sp_rect_class_init(SPRectClass *klass);
static void sp_rect_init(SPRect *rect);

static void sp_rect_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_rect_set(SPObject *object, unsigned key, gchar const *value);
static void sp_rect_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_rect_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static gchar *sp_rect_description(SPItem *item);
static Geom::Matrix sp_rect_set_transform(SPItem *item, Geom::Matrix const &xform);
static void sp_rect_convert_to_guides(SPItem *item);

static void sp_rect_set_shape(SPShape *shape);
static void sp_rect_snappoints(SPItem const *item, SnapPointsIter p, Inkscape::SnapPreferences const *snapprefs);

static SPShapeClass *parent_class;

GType
sp_rect_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPRectClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) sp_rect_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(SPRect),
            16,     /* n_preallocs */
            (GInstanceInitFunc) sp_rect_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_SHAPE, "SPRect", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_rect_class_init(SPRectClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPItemClass *item_class = (SPItemClass *) klass;
    SPShapeClass *shape_class = (SPShapeClass *) klass;

    parent_class = (SPShapeClass *)g_type_class_ref(SP_TYPE_SHAPE);

    sp_object_class->build = sp_rect_build;
    sp_object_class->write = sp_rect_write;
    sp_object_class->set = sp_rect_set;
    sp_object_class->update = sp_rect_update;

    item_class->description = sp_rect_description;
    item_class->set_transform = sp_rect_set_transform;
    item_class->convert_to_guides = sp_rect_convert_to_guides;
    item_class->snappoints = sp_rect_snappoints; //override the default sp_shape_snappoints; see sp_rect_snappoints for details

    shape_class->set_shape = sp_rect_set_shape;
}

static void
sp_rect_init(SPRect */*rect*/)
{
    /* Initializing to zero is automatic */
    /* sp_svg_length_unset(&rect->x, SP_SVG_UNIT_NONE, 0.0, 0.0); */
    /* sp_svg_length_unset(&rect->y, SP_SVG_UNIT_NONE, 0.0, 0.0); */
    /* sp_svg_length_unset(&rect->width, SP_SVG_UNIT_NONE, 0.0, 0.0); */
    /* sp_svg_length_unset(&rect->height, SP_SVG_UNIT_NONE, 0.0, 0.0); */
    /* sp_svg_length_unset(&rect->rx, SP_SVG_UNIT_NONE, 0.0, 0.0); */
    /* sp_svg_length_unset(&rect->ry, SP_SVG_UNIT_NONE, 0.0, 0.0); */
}

static void
sp_rect_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) parent_class)->build)
        ((SPObjectClass *) parent_class)->build(object, document, repr);

    sp_object_read_attr(object, "x");
    sp_object_read_attr(object, "y");
    sp_object_read_attr(object, "width");
    sp_object_read_attr(object, "height");
    sp_object_read_attr(object, "rx");
    sp_object_read_attr(object, "ry");
}

static void
sp_rect_set(SPObject *object, unsigned key, gchar const *value)
{
    SPRect *rect = SP_RECT(object);

    /* fixme: We need real error processing some time */

    switch (key) {
        case SP_ATTR_X:
            rect->x.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y:
            rect->y.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_WIDTH:
            if (!rect->width.read(value) || rect->width.value < 0.0) {
                rect->width.unset();
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_HEIGHT:
            if (!rect->height.read(value) || rect->height.value < 0.0) {
                rect->height.unset();
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_RX:
            if (!rect->rx.read(value) || rect->rx.value < 0.0) {
                rect->rx.unset();
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_RY:
            if (!rect->ry.read(value) || rect->ry.value < 0.0) {
                rect->ry.unset();
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) parent_class)->set)
                ((SPObjectClass *) parent_class)->set(object, key, value);
            break;
    }
}

static void
sp_rect_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        SPRect *rect = (SPRect *) object;
        SPStyle *style = object->style;
        SPItemCtx const *ictx = (SPItemCtx const *) ctx;
        double const d = ictx->i2vp.descrim();
        double const w = (ictx->vp.x1 - ictx->vp.x0) / d;
        double const h = (ictx->vp.y1 - ictx->vp.y0) / d;
        double const em = style->font_size.computed;
        double const ex = 0.5 * em;  // fixme: get x height from pango or libnrtype.
        rect->x.update(em, ex, w);
        rect->y.update(em, ex, h);
        rect->width.update(em, ex, w);
        rect->height.update(em, ex, h);
        rect->rx.update(em, ex, w);
        rect->ry.update(em, ex, h);
        sp_shape_set_shape((SPShape *) object);
        flags &= ~SP_OBJECT_USER_MODIFIED_FLAG_B; // since we change the description, it's not a "just translation" anymore
    }

    if (((SPObjectClass *) parent_class)->update)
        ((SPObjectClass *) parent_class)->update(object, ctx, flags);
}

static Inkscape::XML::Node *
sp_rect_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPRect *rect = SP_RECT(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:rect");
    }

    sp_repr_set_svg_double(repr, "width", rect->width.computed);
    sp_repr_set_svg_double(repr, "height", rect->height.computed);
    if (rect->rx._set) sp_repr_set_svg_double(repr, "rx", rect->rx.computed);
    if (rect->ry._set) sp_repr_set_svg_double(repr, "ry", rect->ry.computed);
    sp_repr_set_svg_double(repr, "x", rect->x.computed);
    sp_repr_set_svg_double(repr, "y", rect->y.computed);

    if (((SPObjectClass *) parent_class)->write)
        ((SPObjectClass *) parent_class)->write(object, xml_doc, repr, flags);

    return repr;
}

static gchar *
sp_rect_description(SPItem *item)
{
    g_return_val_if_fail(SP_IS_RECT(item), NULL);

    return g_strdup(_("<b>Rectangle</b>"));
}

#define C1 0.554

static void
sp_rect_set_shape(SPShape *shape)
{
    SPRect *rect = (SPRect *) shape;

    if ((rect->height.computed < 1e-18) || (rect->width.computed < 1e-18)) {
        sp_shape_set_curve_insync(SP_SHAPE(rect), NULL, TRUE);
        return;
    }

    SPCurve *c = new SPCurve();

    double const x = rect->x.computed;
    double const y = rect->y.computed;
    double const w = rect->width.computed;
    double const h = rect->height.computed;
    double const w2 = w / 2;
    double const h2 = h / 2;
    double const rx = std::min(( rect->rx._set
                                 ? rect->rx.computed
                                 : ( rect->ry._set
                                     ? rect->ry.computed
                                     : 0.0 ) ),
                               .5 * rect->width.computed);
    double const ry = std::min(( rect->ry._set
                                 ? rect->ry.computed
                                 : ( rect->rx._set
                                     ? rect->rx.computed
                                     : 0.0 ) ),
                               .5 * rect->height.computed);
    /* TODO: Handle negative rx or ry as per
     * http://www.w3.org/TR/SVG11/shapes.html#RectElementRXAttribute once Inkscape has proper error
     * handling (see http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing).
     */

    /* We don't use proper circular/elliptical arcs, but bezier curves can approximate a 90-degree
     * arc fairly well.
     */
    if ((rx > 1e-18) && (ry > 1e-18)) {
        c->moveto(x + rx, y);
        if (rx < w2) c->lineto(x + w - rx, y);
        c->curveto(x + w - rx * (1 - C1), y,     x + w, y + ry * (1 - C1),       x + w, y + ry);
        if (ry < h2) c->lineto(x + w, y + h - ry);
        c->curveto(x + w, y + h - ry * (1 - C1),     x + w - rx * (1 - C1), y + h,       x + w - rx, y + h);
        if (rx < w2) c->lineto(x + rx, y + h);
        c->curveto(x + rx * (1 - C1), y + h,     x, y + h - ry * (1 - C1),       x, y + h - ry);
        if (ry < h2) c->lineto(x, y + ry);
        c->curveto(x, y + ry * (1 - C1),     x + rx * (1 - C1), y,       x + rx, y);
    } else {
        c->moveto(x + 0.0, y + 0.0);
        c->lineto(x + w, y + 0.0);
        c->lineto(x + w, y + h);
        c->lineto(x + 0.0, y + h);
        c->lineto(x + 0.0, y + 0.0);
    }

    c->closepath_current();
    sp_shape_set_curve_insync(SP_SHAPE(rect), c, TRUE);
    c->unref();
}

/* fixme: Think (Lauris) */

void
sp_rect_position_set(SPRect *rect, gdouble x, gdouble y, gdouble width, gdouble height)
{
    g_return_if_fail(rect != NULL);
    g_return_if_fail(SP_IS_RECT(rect));

    rect->x.computed = x;
    rect->y.computed = y;
    rect->width.computed = width;
    rect->height.computed = height;

    SP_OBJECT(rect)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
sp_rect_set_rx(SPRect *rect, gboolean set, gdouble value)
{
    g_return_if_fail(rect != NULL);
    g_return_if_fail(SP_IS_RECT(rect));

    rect->rx._set = set;
    if (set) rect->rx.computed = value;

    SP_OBJECT(rect)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
sp_rect_set_ry(SPRect *rect, gboolean set, gdouble value)
{
    g_return_if_fail(rect != NULL);
    g_return_if_fail(SP_IS_RECT(rect));

    rect->ry._set = set;
    if (set) rect->ry.computed = value;

    SP_OBJECT(rect)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/*
 * Initially we'll do:
 * Transform x, y, set x, y, clear translation
 */

/* fixme: Use preferred units somehow (Lauris) */
/* fixme: Alternately preserve whatever units there are (lauris) */

static Geom::Matrix
sp_rect_set_transform(SPItem *item, Geom::Matrix const &xform)
{
    SPRect *rect = SP_RECT(item);

    /* Calculate rect start in parent coords. */
    Geom::Point pos( Geom::Point(rect->x.computed, rect->y.computed) * xform );

    /* This function takes care of translation and scaling, we return whatever parts we can't
       handle. */
    Geom::Matrix ret(Geom::Matrix(xform).without_translation());
    gdouble const sw = hypot(ret[0], ret[1]);
    gdouble const sh = hypot(ret[2], ret[3]);
    if (sw > 1e-9) {
        ret[0] /= sw;
        ret[1] /= sw;
    } else {
        ret[0] = 1.0;
        ret[1] = 0.0;
    }
    if (sh > 1e-9) {
        ret[2] /= sh;
        ret[3] /= sh;
    } else {
        ret[2] = 0.0;
        ret[3] = 1.0;
    }

    /* fixme: Would be nice to preserve units here */
    rect->width = rect->width.computed * sw;
    rect->height = rect->height.computed * sh;
    if (rect->rx._set) {
        rect->rx = rect->rx.computed * sw;
    }
    if (rect->ry._set) {
        rect->ry = rect->ry.computed * sh;
    }

    /* Find start in item coords */
    pos = pos * ret.inverse();
    rect->x = pos[Geom::X];
    rect->y = pos[Geom::Y];

    sp_rect_set_shape(rect);

    // Adjust stroke width
    sp_item_adjust_stroke(item, sqrt(fabs(sw * sh)));

    // Adjust pattern fill
    sp_item_adjust_pattern(item, xform * ret.inverse());

    // Adjust gradient fill
    sp_item_adjust_gradient(item, xform * ret.inverse());

    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

    return ret;
}


/**
Returns the ratio in which the vector from p0 to p1 is stretched by transform
 */
static gdouble
vector_stretch(Geom::Point p0, Geom::Point p1, Geom::Matrix xform)
{
    if (p0 == p1)
        return 0;
    return (Geom::distance(p0 * xform, p1 * xform) / Geom::distance(p0, p1));
}

void
sp_rect_set_visible_rx(SPRect *rect, gdouble rx)
{
    if (rx == 0) {
        rect->rx.computed = 0;
        rect->rx._set = false;
    } else {
        rect->rx.computed = rx / vector_stretch(
            Geom::Point(rect->x.computed + 1, rect->y.computed),
            Geom::Point(rect->x.computed, rect->y.computed),
            SP_ITEM(rect)->transform);
        rect->rx._set = true;
    }
    SP_OBJECT(rect)->updateRepr();
}

void
sp_rect_set_visible_ry(SPRect *rect, gdouble ry)
{
    if (ry == 0) {
        rect->ry.computed = 0;
        rect->ry._set = false;
    } else {
        rect->ry.computed = ry / vector_stretch(
            Geom::Point(rect->x.computed, rect->y.computed + 1),
            Geom::Point(rect->x.computed, rect->y.computed),
            SP_ITEM(rect)->transform);
        rect->ry._set = true;
    }
    SP_OBJECT(rect)->updateRepr();
}

gdouble
sp_rect_get_visible_rx(SPRect *rect)
{
    if (!rect->rx._set)
        return 0;
    return rect->rx.computed * vector_stretch(
        Geom::Point(rect->x.computed + 1, rect->y.computed),
        Geom::Point(rect->x.computed, rect->y.computed),
        SP_ITEM(rect)->transform);
}

gdouble
sp_rect_get_visible_ry(SPRect *rect)
{
    if (!rect->ry._set)
        return 0;
    return rect->ry.computed * vector_stretch(
        Geom::Point(rect->x.computed, rect->y.computed + 1),
        Geom::Point(rect->x.computed, rect->y.computed),
        SP_ITEM(rect)->transform);
}

void
sp_rect_compensate_rxry(SPRect *rect, Geom::Matrix xform)
{
    if (rect->rx.computed == 0 && rect->ry.computed == 0)
        return; // nothing to compensate

    // test unit vectors to find out compensation:
    Geom::Point c(rect->x.computed, rect->y.computed);
    Geom::Point cx = c + Geom::Point(1, 0);
    Geom::Point cy = c + Geom::Point(0, 1);

    // apply previous transform if any
    c *= SP_ITEM(rect)->transform;
    cx *= SP_ITEM(rect)->transform;
    cy *= SP_ITEM(rect)->transform;

    // find out stretches that we need to compensate
    gdouble eX = vector_stretch(cx, c, xform);
    gdouble eY = vector_stretch(cy, c, xform);

    // If only one of the radii is set, set both radii so they have the same visible length
    // This is needed because if we just set them the same length in SVG, they might end up unequal because of transform
    if ((rect->rx._set && !rect->ry._set) || (rect->ry._set && !rect->rx._set)) {
        gdouble r = MAX(rect->rx.computed, rect->ry.computed);
        rect->rx.computed = r / eX;
        rect->ry.computed = r / eY;
    } else {
        rect->rx.computed = rect->rx.computed / eX;
        rect->ry.computed = rect->ry.computed / eY;
    }

    // Note that a radius may end up larger than half-side if the rect is scaled down;
    // that's ok because this preserves the intended radii in case the rect is enlarged again,
    // and set_shape will take care of trimming too large radii when generating d=

    rect->rx._set = rect->ry._set = true;
}

void
sp_rect_set_visible_width(SPRect *rect, gdouble width)
{
    rect->width.computed = width / vector_stretch(
        Geom::Point(rect->x.computed + 1, rect->y.computed),
        Geom::Point(rect->x.computed, rect->y.computed),
        SP_ITEM(rect)->transform);
    rect->width._set = true;
    SP_OBJECT(rect)->updateRepr();
}

void
sp_rect_set_visible_height(SPRect *rect, gdouble height)
{
    rect->height.computed = height / vector_stretch(
        Geom::Point(rect->x.computed, rect->y.computed + 1),
        Geom::Point(rect->x.computed, rect->y.computed),
        SP_ITEM(rect)->transform);
    rect->height._set = true;
    SP_OBJECT(rect)->updateRepr();
}

gdouble
sp_rect_get_visible_width(SPRect *rect)
{
    if (!rect->width._set)
        return 0;
    return rect->width.computed * vector_stretch(
        Geom::Point(rect->x.computed + 1, rect->y.computed),
        Geom::Point(rect->x.computed, rect->y.computed),
        SP_ITEM(rect)->transform);
}

gdouble
sp_rect_get_visible_height(SPRect *rect)
{
    if (!rect->height._set)
        return 0;
    return rect->height.computed * vector_stretch(
        Geom::Point(rect->x.computed, rect->y.computed + 1),
        Geom::Point(rect->x.computed, rect->y.computed),
        SP_ITEM(rect)->transform);
}

/**
 * Sets the snappoint p to the unrounded corners of the rectangle
 */
static void sp_rect_snappoints(SPItem const *item, SnapPointsIter p, Inkscape::SnapPreferences const *snapprefs)
{
    /* This method overrides sp_shape_snappoints, which is the default for any shape. The default method
    returns all eight points along the path of a rounded rectangle, but not the real corners. Snapping
    the startpoint and endpoint of each rounded corner is not very useful and really confusing. Instead
    we could snap either the real corners, or not snap at all. Bulia Byak opted to snap the real corners,
    but it should be noted that this might be confusing in some cases with relatively large radii. With
    small radii though the user will easily understand which point is snapping. */

    g_assert(item != NULL);
    g_assert(SP_IS_RECT(item));

    SPRect *rect = SP_RECT(item);

    Geom::Matrix const i2d (sp_item_i2d_affine (item));

    Geom::Point p0 = Geom::Point(rect->x.computed, rect->y.computed) * i2d;
    Geom::Point p1 = Geom::Point(rect->x.computed, rect->y.computed + rect->height.computed) * i2d;
    Geom::Point p2 = Geom::Point(rect->x.computed + rect->width.computed, rect->y.computed + rect->height.computed) * i2d;
    Geom::Point p3 = Geom::Point(rect->x.computed + rect->width.computed, rect->y.computed) * i2d;

    *p = p0;
    *p = p1;
    *p = p2;
    *p = p3;

    if (snapprefs->getSnapMidpoints()) {
    	*p = (p0 + p1)/2;
    	*p = (p1 + p2)/2;
    	*p = (p2 + p3)/2;
    	*p = (p3 + p0)/2;
    }
}

void
sp_rect_convert_to_guides(SPItem *item) {
    SPRect *rect = SP_RECT(item);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (!prefs->getBool("/tools/shapes/rect/convertguides", true)) {
        sp_item_convert_to_guides(SP_ITEM(rect));
        return;
    }

    std::list<std::pair<Geom::Point, Geom::Point> > pts;

    Geom::Matrix const i2d (sp_item_i2d_affine(SP_ITEM(rect)));

    Geom::Point A1(Geom::Point(rect->x.computed, rect->y.computed) * i2d);
    Geom::Point A2(Geom::Point(rect->x.computed, rect->y.computed + rect->height.computed) * i2d);
    Geom::Point A3(Geom::Point(rect->x.computed + rect->width.computed, rect->y.computed + rect->height.computed) * i2d);
    Geom::Point A4(Geom::Point(rect->x.computed + rect->width.computed, rect->y.computed) * i2d);

    pts.push_back(std::make_pair(A1, A2));
    pts.push_back(std::make_pair(A2, A3));
    pts.push_back(std::make_pair(A3, A4));
    pts.push_back(std::make_pair(A4, A1));

    sp_guide_pt_pairs_to_guides(inkscape_active_desktop(), pts);
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
