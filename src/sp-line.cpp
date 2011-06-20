/*
 * SVG <line> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "attributes.h"
#include "style.h"
#include "sp-line.h"
#include "sp-guide.h"
#include "display/curve.h"
#include <glibmm/i18n.h>
#include "xml/repr.h"
#include "document.h"
#include "inkscape.h"

SPShapeClass * SPLineClass::static_parent_class = 0;

GType SPLine::sp_line_get_type(void)
{
    static GType line_type = 0;

    if (!line_type) {
        GTypeInfo line_info = {
            sizeof(SPLineClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc) SPLineClass::sp_line_class_init,
            NULL,       /* klass_finalize */
            NULL,       /* klass_data */
            sizeof(SPLine),
            16, /* n_preallocs */
            (GInstanceInitFunc) init,
            NULL,       /* value_table */
        };
        line_type = g_type_register_static(SP_TYPE_SHAPE, "SPLine", &line_info,(GTypeFlags)0);
    }
    return line_type;
}

void SPLineClass::sp_line_class_init(SPLineClass *klass)
{
    SPLineClass::static_parent_class = (SPShapeClass *) g_type_class_ref(SP_TYPE_SHAPE);

    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    sp_object_class->build = SPLine::build;
    sp_object_class->set = SPLine::set;
    sp_object_class->write = SPLine::write;

    SPItemClass *item_class = (SPItemClass *) klass;
    item_class->description = SPLine::getDescription;
    item_class->set_transform = SPLine::setTransform;
    item_class->convert_to_guides = SPLine::convertToGuides;

    sp_object_class->update = SPLine::update;

    SPShapeClass *shape_class = (SPShapeClass *) klass;
    shape_class->set_shape = SPLine::setShape;
}

void SPLine::init(SPLine * line)
{
    line->x1.unset();
    line->y1.unset();
    line->x2.unset();
    line->y2.unset();
}


void SPLine::build(SPObject * object, SPDocument * document, Inkscape::XML::Node * repr)
{
    if (((SPObjectClass *) SPLineClass::static_parent_class)->build) {
        ((SPObjectClass *) SPLineClass::static_parent_class)->build(object, document, repr);
    }

    object->readAttr( "x1" );
    object->readAttr( "y1" );
    object->readAttr( "x2" );
    object->readAttr( "y2" );
}

void SPLine::set(SPObject *object, unsigned int key, const gchar *value)
{
    SPLine * line = SP_LINE(object);

    /* fixme: we should really collect updates */

    switch (key) {
        case SP_ATTR_X1:
            line->x1.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y1:
            line->y1.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_X2:
            line->x2.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y2:
            line->y2.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) SPLineClass::static_parent_class)->set) {
                ((SPObjectClass *) SPLineClass::static_parent_class)->set(object, key, value);
            }
            break;
    }
}

void SPLine::update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        SPLine *line = SP_LINE(object);

        SPStyle const *style = object->style;
        SPItemCtx const *ictx = (SPItemCtx const *) ctx;
        double const w = (ictx->vp.x1 - ictx->vp.x0);
        double const h = (ictx->vp.y1 - ictx->vp.y0);
        double const em = style->font_size.computed;
        double const ex = em * 0.5;  // fixme: get from pango or libnrtype.
        line->x1.update(em, ex, w);
        line->x2.update(em, ex, w);
        line->y1.update(em, ex, h);
        line->y2.update(em, ex, h);

        ((SPShape *) object)->setShape();
    }

    if (((SPObjectClass *) SPLineClass::static_parent_class)->update) {
        ((SPObjectClass *) SPLineClass::static_parent_class)->update(object, ctx, flags);
    }
}


Inkscape::XML::Node * SPLine::write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPLine *line  = SP_LINE(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:line");
    }

    if (repr != object->getRepr()) {
        repr->mergeFrom(object->getRepr(), "id");
    }

    sp_repr_set_svg_double(repr, "x1", line->x1.computed);
    sp_repr_set_svg_double(repr, "y1", line->y1.computed);
    sp_repr_set_svg_double(repr, "x2", line->x2.computed);
    sp_repr_set_svg_double(repr, "y2", line->y2.computed);

    if (((SPObjectClass *) (SPLineClass::static_parent_class))->write) {
        ((SPObjectClass *) (SPLineClass::static_parent_class))->write(object, xml_doc, repr, flags);
    }

    return repr;
}

gchar * SPLine::getDescription(SPItem */*item*/)
{
    return g_strdup(_("<b>Line</b>"));
}

void SPLine::convertToGuides(SPItem *item)
{
    SPLine *line = SP_LINE(item);
    Geom::Point points[2];

    Geom::Affine const i2d(item->i2d_affine());

    points[0] = Geom::Point(line->x1.computed, line->y1.computed)*i2d;
    points[1] = Geom::Point(line->x2.computed, line->y2.computed)*i2d;

    SPGuide::createSPGuide(inkscape_active_desktop(), points[0], points[1]);
}

Geom::Affine SPLine::setTransform(SPItem *item, Geom::Affine const &xform)
{
    SPLine *line = SP_LINE(item);
    Geom::Point points[2];

    points[0] = Geom::Point(line->x1.computed, line->y1.computed);
    points[1] = Geom::Point(line->x2.computed, line->y2.computed);

    points[0] *= xform;
    points[1] *= xform;

    line->x1.computed = points[0][Geom::X];
    line->y1.computed = points[0][Geom::Y];
    line->x2.computed = points[1][Geom::X];
    line->y2.computed = points[1][Geom::Y];

    item->adjust_stroke(xform.descrim());

    SP_OBJECT(item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

    return Geom::identity();
}

void SPLine::setShape(SPShape *shape)
{
    SPLine *line = SP_LINE(shape);

    SPCurve *c = new SPCurve();

    c->moveto(line->x1.computed, line->y1.computed);
    c->lineto(line->x2.computed, line->y2.computed);

    shape->setCurveInsync(c, TRUE); // *_insync does not call update, avoiding infinite recursion when set_shape is called by update
    shape->setCurveBeforeLPE(c);

    // LPE's cannot be applied to lines. (the result can (generally) not be represented as SPLine)

    c->unref();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
