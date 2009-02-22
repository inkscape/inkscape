#ifndef __SP_PATTERN_H__
#define __SP_PATTERN_H__

/*
 * SVG <pattern> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtktypeutils.h>

#include "forward.h"

#define SP_TYPE_PATTERN (sp_pattern_get_type ())
#define SP_PATTERN(o) (GTK_CHECK_CAST ((o), SP_TYPE_PATTERN, SPPattern))
#define SP_PATTERN_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_PATTERN, SPPatternClass))
#define SP_IS_PATTERN(o) (GTK_CHECK_TYPE ((o), SP_TYPE_PATTERN))
#define SP_IS_PATTERN_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_PATTERN))

GType sp_pattern_get_type (void);

class SPPatternClass;

#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include "svg/svg-length.h"
#include "sp-paint-server.h"
#include "uri-references.h"

#include <sigc++/connection.h>

class SPPatternReference : public Inkscape::URIReference {
public:
    SPPatternReference (SPObject *obj) : URIReference(obj) {}
    SPPattern *getObject() const {
        return (SPPattern *)URIReference::getObject();
    }

protected:
    virtual bool _acceptObject(SPObject *obj) const {
        return SP_IS_PATTERN (obj);
    }
};

enum {
    SP_PATTERN_UNITS_USERSPACEONUSE,
    SP_PATTERN_UNITS_OBJECTBOUNDINGBOX
};

struct SPPattern : public SPPaintServer {
    /* Reference (href) */
    gchar *href;
    SPPatternReference *ref;

    /* patternUnits and patternContentUnits attribute */
    guint patternUnits : 1;
    guint patternUnits_set : 1;
    guint patternContentUnits : 1;
    guint patternContentUnits_set : 1;
    /* patternTransform attribute */
    Geom::Matrix patternTransform;
    guint patternTransform_set : 1;
    /* Tile rectangle */
    SVGLength x;
    SVGLength y;
    SVGLength width;
    SVGLength height;
    /* VieBox */
    NRRect viewBox;
    guint viewBox_set : 1;

    sigc::connection modified_connection;
};

struct SPPatternClass {
    SPPaintServerClass parent_class;
};

guint pattern_users (SPPattern *pattern);
SPPattern *pattern_chain (SPPattern *pattern);
SPPattern *sp_pattern_clone_if_necessary (SPItem *item, SPPattern *pattern, const gchar *property);
void sp_pattern_transform_multiply (SPPattern *pattern, Geom::Matrix postmul, bool set);

const gchar *pattern_tile (GSList *reprs, Geom::Rect bounds, SPDocument *document, Geom::Matrix transform, Geom::Matrix move);

SPPattern *pattern_getroot (SPPattern *pat);

guint pattern_patternUnits (SPPattern *pat);
guint pattern_patternContentUnits (SPPattern *pat);
Geom::Matrix const &pattern_patternTransform(SPPattern const *pat);
gdouble pattern_x (SPPattern *pat);
gdouble pattern_y (SPPattern *pat);
gdouble pattern_width (SPPattern *pat);
gdouble pattern_height (SPPattern *pat);
NRRect *pattern_viewBox (SPPattern *pat);

#endif //__SP_PATTERN_H__

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
