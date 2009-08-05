#define __SP_POLYLINE_C__

/*
 * SVG <polyline> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include "attributes.h"
#include "sp-polyline.h"
#include "display/curve.h"
#include <glibmm/i18n.h>
#include "xml/repr.h"
#include "document.h"

static void sp_polyline_class_init (SPPolyLineClass *klass);
static void sp_polyline_init (SPPolyLine *polyline);

static void sp_polyline_build (SPObject * object, Document * document, Inkscape::XML::Node * repr);
static void sp_polyline_set (SPObject *object, unsigned int key, const gchar *value);
static Inkscape::XML::Node *sp_polyline_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static gchar * sp_polyline_description (SPItem * item);

static SPShapeClass *parent_class;

GType
sp_polyline_get_type (void)
{
	static GType polyline_type = 0;

	if (!polyline_type) {
		GTypeInfo polyline_info = {
			sizeof (SPPolyLineClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_polyline_class_init,
			NULL,	/* klass_finalize */
			NULL,	/* klass_data */
			sizeof (SPPolyLine),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_polyline_init,
			NULL,	/* value_table */
		};
		polyline_type = g_type_register_static (SP_TYPE_SHAPE, "SPPolyLine", &polyline_info, (GTypeFlags)0);
	}
	return polyline_type;
}

static void
sp_polyline_class_init (SPPolyLineClass *klass)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;

	gobject_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;

	parent_class = (SPShapeClass *)g_type_class_ref (SP_TYPE_SHAPE);

	sp_object_class->build = sp_polyline_build;
	sp_object_class->set = sp_polyline_set;
	sp_object_class->write = sp_polyline_write;

	item_class->description = sp_polyline_description;
}

static void
sp_polyline_init (SPPolyLine * /*polyline*/)
{
    /* Nothing here */
}

static void
sp_polyline_build (SPObject * object, Document * document, Inkscape::XML::Node * repr)
{

	if (((SPObjectClass *) parent_class)->build)
		((SPObjectClass *) parent_class)->build (object, document, repr);

	sp_object_read_attr (object, "points");
}

static void
sp_polyline_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPPolyLine *polyline;

	polyline = SP_POLYLINE (object);

	switch (key) {
	case SP_ATTR_POINTS: {
		SPCurve * curve;
		const gchar * cptr;
		char * eptr;
		gboolean hascpt;

		if (!value) break;
		curve = new SPCurve ();
		hascpt = FALSE;

		cptr = value;
		eptr = NULL;

		while (TRUE) {
			gdouble x, y;

			while (*cptr != '\0' && (*cptr == ',' || *cptr == '\x20' || *cptr == '\x9' || *cptr == '\xD' || *cptr == '\xA')) {
				cptr++;
			}
			if (!*cptr) break;

			x = g_ascii_strtod (cptr, &eptr);
			if (eptr == cptr) break;
			cptr = eptr;

			while (*cptr != '\0' && (*cptr == ',' || *cptr == '\x20' || *cptr == '\x9' || *cptr == '\xD' || *cptr == '\xA')) {
				cptr++;
			}
			if (!*cptr) break;

			y = g_ascii_strtod (cptr, &eptr);
			if (eptr == cptr) break;
			cptr = eptr;
			if (hascpt) {
				curve->lineto(x, y);
			} else {
				curve->moveto(x, y);
				hascpt = TRUE;
			}
		}
		
		sp_shape_set_curve (SP_SHAPE (polyline), curve, TRUE);
		curve->unref();
		break;
	}
	default:
		if (((SPObjectClass *) parent_class)->set)
			((SPObjectClass *) parent_class)->set (object, key, value);
		break;
	}
}

static Inkscape::XML::Node *
sp_polyline_write (SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
	SPPolyLine *polyline;

	polyline = SP_POLYLINE (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = xml_doc->createElement("svg:polyline");
	}

	if (repr != SP_OBJECT_REPR (object)) {
		repr->mergeFrom(SP_OBJECT_REPR (object), "id");
	}

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, xml_doc, repr, flags);

	return repr;
}

static gchar *
sp_polyline_description(SPItem */*item*/)
{
    return g_strdup(_("<b>Polyline</b>"));
}
