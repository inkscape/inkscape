/*
 * SVG <polyline> implementation
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

#include "config.h"

#include "attributes.h"
#include "sp-polyline.h"
#include "display/curve.h"
#include <glibmm/i18n.h>
#include "xml/repr.h"
#include "document.h"

SPShapeClass * SPPolyLineClass::static_parent_class=0;

GType SPPolyLine::sp_polyline_get_type(void)
{
    static GType polyline_type = 0;

    if (!polyline_type) {
        GTypeInfo polyline_info = {
            sizeof (SPPolyLineClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) SPPolyLineClass::sp_polyline_class_init,
            NULL,   /* klass_finalize */
            NULL,   /* klass_data */
            sizeof (SPPolyLine),
            16,     /* n_preallocs */
            (GInstanceInitFunc) SPPolyLine::init,
            NULL,   /* value_table */
        };
        polyline_type = g_type_register_static (SP_TYPE_SHAPE, "SPPolyLine", &polyline_info, (GTypeFlags)0);
    }
    return polyline_type;
}

void SPPolyLineClass::sp_polyline_class_init(SPPolyLineClass *klass)
{
    GObjectClass * gobject_class = (GObjectClass *) klass;
    SPObjectClass * sp_object_class = (SPObjectClass *) klass;
    SPItemClass * item_class = (SPItemClass *) klass;

    static_parent_class = (SPShapeClass *)g_type_class_ref(SP_TYPE_SHAPE);

    sp_object_class->build = SPPolyLine::build;
    sp_object_class->set = SPPolyLine::set;
    sp_object_class->write = SPPolyLine::write;

    item_class->description = SPPolyLine::getDescription;
}

void SPPolyLine::init(SPPolyLine * /*polyline*/)
{
    /* Nothing here */
}

void SPPolyLine::build(SPObject * object, SPDocument * document, Inkscape::XML::Node * repr)
{

    if (((SPObjectClass *) SPPolyLineClass::static_parent_class)->build) {
        ((SPObjectClass *) SPPolyLineClass::static_parent_class)->build (object, document, repr);
    }

    object->readAttr( "points" );
}

void SPPolyLine::set(SPObject *object, unsigned int key, const gchar *value)
{
    SPPolyLine *polyline = SP_POLYLINE(object);

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
		
            (SP_SHAPE (polyline))->setCurve (curve, TRUE);
            curve->unref();
            break;
	}
	default:
            if (((SPObjectClass *) SPPolyLineClass::static_parent_class)->set) {
                ((SPObjectClass *) SPPolyLineClass::static_parent_class)->set (object, key, value);
            }
            break;
    }
}

Inkscape::XML::Node *SPPolyLine::write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPPolyLine *polyline = SP_POLYLINE (object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:polyline");
    }

    if (repr != object->getRepr()) {
        repr->mergeFrom(object->getRepr(), "id");
    }

    if (((SPObjectClass *) (SPPolyLineClass::static_parent_class))->write) {
        ((SPObjectClass *) (SPPolyLineClass::static_parent_class))->write (object, xml_doc, repr, flags);
    }

    return repr;
}

gchar *SPPolyLine::getDescription(SPItem * /*item*/)
{
    return g_strdup(_("<b>Polyline</b>"));
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
