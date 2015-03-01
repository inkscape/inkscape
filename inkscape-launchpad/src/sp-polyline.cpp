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

SPPolyLine::SPPolyLine() : SPShape() {
}

SPPolyLine::~SPPolyLine() {
}

void SPPolyLine::build(SPDocument * document, Inkscape::XML::Node * repr) {
    SPShape::build(document, repr);

    this->readAttr("points");
}

void SPPolyLine::set(unsigned int key, const gchar* value) {
    switch (key) {
	case SP_ATTR_POINTS: {
            SPCurve * curve;
            const gchar * cptr;
            char * eptr;
            gboolean hascpt;

            if (!value) {
            	break;
            }

            curve = new SPCurve ();
            hascpt = FALSE;

            cptr = value;
            eptr = NULL;

            while (TRUE) {
                gdouble x, y;

                while (*cptr != '\0' && (*cptr == ',' || *cptr == '\x20' || *cptr == '\x9' || *cptr == '\xD' || *cptr == '\xA')) {
                    cptr++;
                }

                if (!*cptr) {
                	break;
                }

                x = g_ascii_strtod (cptr, &eptr);

                if (eptr == cptr) {
                	break;
                }

                cptr = eptr;

                while (*cptr != '\0' && (*cptr == ',' || *cptr == '\x20' || *cptr == '\x9' || *cptr == '\xD' || *cptr == '\xA')) {
                    cptr++;
                }

                if (!*cptr) {
                	break;
                }

                y = g_ascii_strtod (cptr, &eptr);

                if (eptr == cptr) {
                	break;
                }

                cptr = eptr;

                if (hascpt) {
                    curve->lineto(x, y);
                } else {
                    curve->moveto(x, y);
                    hascpt = TRUE;
                }
            }
		
            this->setCurve(curve, TRUE);
            curve->unref();
            break;
	}
	default:
            SPShape::set(key, value);
            break;
    }
}

Inkscape::XML::Node* SPPolyLine::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:polyline");
    }

    if (repr != this->getRepr()) {
        repr->mergeFrom(this->getRepr(), "id");
    }

    SPShape::write(xml_doc, repr, flags);

    return repr;
}

gchar* SPPolyLine::description() const {
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
