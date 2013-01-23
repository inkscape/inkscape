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

static void                 sp_polyline_build(SPObject * object, SPDocument * document, Inkscape::XML::Node * repr);
static void                 sp_polyline_set(SPObject *object, unsigned int key, const gchar *value);
static Inkscape::XML::Node* sp_polyline_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static gchar*               sp_polyline_get_description(SPItem * item);


G_DEFINE_TYPE(SPPolyLine, sp_polyline, SP_TYPE_SHAPE);

static void
sp_polyline_class_init(SPPolyLineClass *klass)
{
    SPObjectClass * sp_object_class = (SPObjectClass *) klass;
    SPItemClass * item_class = (SPItemClass *) klass;

    sp_object_class->build = sp_polyline_build;
    sp_object_class->set   = sp_polyline_set;
    sp_object_class->write = sp_polyline_write;

    item_class->description = sp_polyline_get_description;
}

static void
sp_polyline_init(SPPolyLine * /*polyline*/)
{
}

static void
sp_polyline_build(SPObject * object, SPDocument * document, Inkscape::XML::Node * repr)
{
    if (((SPObjectClass *) sp_polyline_parent_class)->build) {
        ((SPObjectClass *) sp_polyline_parent_class)->build (object, document, repr);
    }

    object->readAttr( "points" );
}

static void
sp_polyline_set(SPObject *object, unsigned int key, const gchar *value)
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
            if (((SPObjectClass *) sp_polyline_parent_class)->set) {
                ((SPObjectClass *) sp_polyline_parent_class)->set (object, key, value);
            }
            break;
    }
}

static Inkscape::XML::Node*
sp_polyline_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SP_POLYLINE(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:polyline");
    }

    if (repr != object->getRepr()) {
        repr->mergeFrom(object->getRepr(), "id");
    }

    if (((SPObjectClass *) (sp_polyline_parent_class))->write) {
        ((SPObjectClass *) (sp_polyline_parent_class))->write (object, xml_doc, repr, flags);
    }

    return repr;
}

static gchar*
sp_polyline_get_description(SPItem * /*item*/)
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
