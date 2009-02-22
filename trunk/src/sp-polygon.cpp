#define __SP_POLYGON_C__

/*
 * SVG <polygon> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include "attributes.h"
#include "sp-polygon.h"
#include "display/curve.h"
#include <glibmm/i18n.h>
#include <2geom/pathvector.h>
#include <2geom/bezier-curve.h>
#include <2geom/hvlinesegment.h>
#include "helper/geom-curves.h"
#include "svg/stringstream.h"
#include "xml/repr.h"
#include "document.h"

static void sp_polygon_class_init(SPPolygonClass *pc);
static void sp_polygon_init(SPPolygon *polygon);

static void sp_polygon_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static Inkscape::XML::Node *sp_polygon_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static gchar *sp_polygon_description(SPItem *item);

static SPShapeClass *parent_class;

GType sp_polygon_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPPolygonClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_polygon_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPPolygon),
            0, // n_preallocs
            (GInstanceInitFunc)sp_polygon_init,
            0 // value_table
        };
        type = g_type_register_static(SP_TYPE_SHAPE, "SPPolygon", &info, static_cast<GTypeFlags>(0));
    }

    return type;
}

static void sp_polygon_class_init(SPPolygonClass *pc)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) pc;
    SPItemClass *item_class = (SPItemClass *) pc;

    parent_class = (SPShapeClass *) g_type_class_ref(SP_TYPE_SHAPE);

    sp_object_class->build = sp_polygon_build;
    sp_object_class->write = sp_polygon_write;
    sp_object_class->set = sp_polygon_set;

    item_class->description = sp_polygon_description;
}

static void sp_polygon_init(SPPolygon */*polygon*/)
{
    /* Nothing here */
}

static void sp_polygon_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) parent_class)->build) {
        ((SPObjectClass *) parent_class)->build(object, document, repr);
    }

    sp_object_read_attr(object, "points");
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

static Inkscape::XML::Node *sp_polygon_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPShape *shape = SP_SHAPE(object);
    // Tolerable workaround: we need to update the object's curve before we set points=
    // because it's out of sync when e.g. some extension attrs of the polygon or star are changed in XML editor
    sp_shape_set_shape(shape);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:polygon");
    }

    /* We can safely write points here, because all subclasses require it too (Lauris) */
    gchar *str = sp_svg_write_polygon(shape->curve->get_pathvector());
    repr->setAttribute("points", str);
    g_free(str);

    if (((SPObjectClass *) (parent_class))->write) {
        ((SPObjectClass *) (parent_class))->write(object, xml_doc, repr, flags);
    }

    return repr;
}


static gboolean polygon_get_value(gchar const **p, gdouble *v)
{
    while (**p != '\0' && (**p == ',' || **p == '\x20' || **p == '\x9' || **p == '\xD' || **p == '\xA')) {
        (*p)++;
    }

    if (*p == '\0') {
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


void sp_polygon_set(SPObject *object, unsigned int key, const gchar *value)
{
    SPPolygon *polygon = SP_POLYGON(object);

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
            sp_shape_set_curve(SP_SHAPE(polygon), curve, TRUE);
            curve->unref();
            break;
        }
        default:
            if (((SPObjectClass *) parent_class)->set) {
                ((SPObjectClass *) parent_class)->set(object, key, value);
            }
            break;
    }
}

static gchar *sp_polygon_description(SPItem */*item*/)
{
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
