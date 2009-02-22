#define __SP_LINE_C__

/*
 * SVG <line> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "attributes.h"
#include "style.h"
#include "sp-line.h"
#include "display/curve.h"
#include <glibmm/i18n.h>
#include <libnr/nr-matrix-fns.h>
#include <xml/repr.h>
#include "document.h"

static void sp_line_class_init (SPLineClass *klass);
static void sp_line_init (SPLine *line);

static void sp_line_build (SPObject * object, SPDocument * document, Inkscape::XML::Node * repr);
static void sp_line_set (SPObject *object, unsigned int key, const gchar *value);
static Inkscape::XML::Node *sp_line_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static gchar *sp_line_description (SPItem * item);
static Geom::Matrix sp_line_set_transform(SPItem *item, Geom::Matrix const &xform);

static void sp_line_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_line_set_shape (SPShape *shape);

static SPShapeClass *parent_class;

GType
sp_line_get_type (void)
{
	static GType line_type = 0;

	if (!line_type) {
		GTypeInfo line_info = {
			sizeof (SPLineClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_line_class_init,
			NULL,	/* klass_finalize */
			NULL,	/* klass_data */
			sizeof (SPLine),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_line_init,
			NULL,	/* value_table */
		};
		line_type = g_type_register_static (SP_TYPE_SHAPE, "SPLine", &line_info, (GTypeFlags)0);
	}
	return line_type;
}

static void
sp_line_class_init (SPLineClass *klass)
{
	parent_class = (SPShapeClass *) g_type_class_ref (SP_TYPE_SHAPE);

	SPObjectClass *sp_object_class = (SPObjectClass *) klass;
	sp_object_class->build = sp_line_build;
	sp_object_class->set = sp_line_set;
	sp_object_class->write = sp_line_write;

	SPItemClass *item_class = (SPItemClass *) klass;
	item_class->description = sp_line_description;
	item_class->set_transform = sp_line_set_transform;

	sp_object_class->update = sp_line_update;

	SPShapeClass *shape_class = (SPShapeClass *) klass;
	shape_class->set_shape = sp_line_set_shape;
}

static void
sp_line_init (SPLine * line)
{
	line->x1.unset();
	line->y1.unset();
	line->x2.unset();
	line->y2.unset();
}


static void
sp_line_build (SPObject * object, SPDocument * document, Inkscape::XML::Node * repr)
{
        if (((SPObjectClass *) parent_class)->build) {
		((SPObjectClass *) parent_class)->build (object, document, repr);
        }

	sp_object_read_attr (object, "x1");
	sp_object_read_attr (object, "y1");
	sp_object_read_attr (object, "x2");
	sp_object_read_attr (object, "y2");
}

static void
sp_line_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPLine * line = SP_LINE (object);

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
		if (((SPObjectClass *) parent_class)->set)
			((SPObjectClass *) parent_class)->set (object, key, value);
		break;
	}
}

static void
sp_line_update (SPObject *object, SPCtx *ctx, guint flags)
{
	if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
		SPLine *line = SP_LINE (object);

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

		sp_shape_set_shape ((SPShape *) object);
	}

	if (((SPObjectClass *) parent_class)->update)
		((SPObjectClass *) parent_class)->update (object, ctx, flags);
}


static Inkscape::XML::Node *
sp_line_write (SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
	SPLine *line  = SP_LINE (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = xml_doc->createElement("svg:line");
	}

	if (repr != SP_OBJECT_REPR (object)) {
		repr->mergeFrom(SP_OBJECT_REPR (object), "id");
	}

	sp_repr_set_svg_double(repr, "x1", line->x1.computed);
	sp_repr_set_svg_double(repr, "y1", line->y1.computed);
	sp_repr_set_svg_double(repr, "x2", line->x2.computed);
	sp_repr_set_svg_double(repr, "y2", line->y2.computed);

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, xml_doc, repr, flags);

	return repr;
}

static gchar *
sp_line_description(SPItem */*item*/)
{
    return g_strdup(_("<b>Line</b>"));
}

static Geom::Matrix
sp_line_set_transform (SPItem *item, Geom::Matrix const &xform)
{
	SPLine *line = SP_LINE (item);
	Geom::Point points[2];

	points[0] = Geom::Point(line->x1.computed, line->y1.computed);
	points[1] = Geom::Point(line->x2.computed, line->y2.computed);

	points[0] *= xform;
	points[1] *= xform;

	line->x1.computed = points[0][Geom::X];
	line->y1.computed = points[0][Geom::Y];
	line->x2.computed = points[1][Geom::X];
	line->y2.computed = points[1][Geom::Y];

	sp_item_adjust_stroke(item, xform.descrim());

	SP_OBJECT (item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

	return Geom::identity();
}

static void
sp_line_set_shape (SPShape *shape)
{
	SPLine *line = SP_LINE (shape);

	SPCurve *c = new SPCurve ();

	c->moveto(line->x1.computed, line->y1.computed);
	c->lineto(line->x2.computed, line->y2.computed);

	sp_shape_set_curve_insync (shape, c, TRUE); // *_insync does not call update, avoiding infinite recursion when set_shape is called by update

	c->unref();
}
