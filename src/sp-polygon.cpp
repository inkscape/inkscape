/*
 * SVG <polygon> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "attributes.h"
#include "sp-polygon.h"
#include "display/curve.h"
#include <glibmm/i18n.h>
#include <2geom/pathvector.h>
#include <2geom/curves.h>
#include "helper/geom-curves.h"
#include "svg/stringstream.h"
#include "xml/repr.h"
#include "document.h"

SPPolygon::SPPolygon() : SPShape() {
}

SPPolygon::~SPPolygon() {
}

void SPPolygon::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPPolygon* object = this;

    SPShape::build(document, repr);

    object->readAttr( "points" );
}

/*
 * sp_svg_write_polygon: Write points attribute for polygon tag.
 * pathv may only contain paths with only straight line segments
 * Return value: points attribute string.
 */
static gchar *sp_svg_write_polygon(Geom::PathVector const & pathv)
{
    Inkscape::SVGOStringStream os;

    for (Geom::PathVector::const_iterator pit = pathv.begin(); pit != pathv.end(); ++pit) {
        for (Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_default(); ++cit) {
            if ( is_straight_curve(*cit) )
            {
                os << cit->finalPoint()[0] << "," << cit->finalPoint()[1] << " ";
            } else {
                g_error("sp_svg_write_polygon: polygon path contains non-straight line segments");
            }
        }
    }

    return g_strdup(os.str().c_str());
}

Inkscape::XML::Node* SPPolygon::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    // Tolerable workaround: we need to update the object's curve before we set points=
    // because it's out of sync when e.g. some extension attrs of the polygon or star are changed in XML editor
	this->set_shape();

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:polygon");
    }

    /* We can safely write points here, because all subclasses require it too (Lauris) */
    /* While saving polygon element without points attribute _curve is NULL (see bug 1202753) */
    if (this->_curve != NULL) {
        gchar *str = sp_svg_write_polygon(this->_curve->get_pathvector());
        repr->setAttribute("points", str);
        g_free(str);
    }

    SPShape::write(xml_doc, repr, flags);

    return repr;
}


static gboolean polygon_get_value(gchar const **p, gdouble *v)
{
    while (**p != '\0' && (**p == ',' || **p == '\x20' || **p == '\x9' || **p == '\xD' || **p == '\xA')) {
        (*p)++;
    }

    if (**p == '\0') {
        return false;
    }

    gchar *e = NULL;
    *v = g_ascii_strtod(*p, &e);

    if (e == *p) {
        return false;
    }

    *p = e;

    return true;
}

void SPPolygon::set(unsigned int key, const gchar* value) {
    switch (key) {
        case SP_ATTR_POINTS: {
            if (!value) {
                /* fixme: The points attribute is required.  We should handle its absence as per
                 * http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing. */
                break;
            }

            SPCurve *curve = new SPCurve();
            gboolean hascpt = FALSE;

            gchar const *cptr = value;
            bool has_error = false;

            while (TRUE) {
                gdouble x;

                if (!polygon_get_value(&cptr, &x)) {
                    break;
                }

                gdouble y;

                if (!polygon_get_value(&cptr, &y)) {
                    /* fixme: It is an error for an odd number of points to be specified.  We
                     * should display the points up to now (as we currently do, though perhaps
                     * without the closepath: the spec isn't quite clear on whether to do a
                     * closepath or not, though I'd guess it's best not to do a closepath), but
                     * then flag the document as in error, as per
                     * http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing.
                     *
                     * (Ref: http://www.w3.org/TR/SVG11/shapes.html#PolygonElement.) */
                    has_error = true;
                    break;
                }

                if (hascpt) {
                    curve->lineto(x, y);
                } else {
                    curve->moveto(x, y);
                    hascpt = TRUE;
                }
            }

            if (has_error || *cptr != '\0') {
                /* TODO: Flag the document as in error, as per
                 * http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing. */
            } else if (hascpt) {
                /* We might have done a moveto but no lineto.  I'm not sure how we're supposed to represent
                 * a single-point polygon in SPCurve. TODO: add a testcase with only one coordinate pair */
                curve->closepath();
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

gchar* SPPolygon::description() const {
	return g_strdup(_("<b>Polygon</b>"));
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
