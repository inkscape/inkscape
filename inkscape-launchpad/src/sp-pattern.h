#ifndef SEEN_SP_PATTERN_H
#define SEEN_SP_PATTERN_H

/*
 * SVG <pattern> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtk.h>

#include "sp-item.h"

#define SP_PATTERN(obj) (dynamic_cast<SPPattern*>((SPObject*)obj))
#define SP_IS_PATTERN(obj) (dynamic_cast<const SPPattern*>((SPObject*)obj) != NULL)

class SPPatternReference;

#include "svg/svg-length.h"
#include "sp-paint-server.h"
#include "uri-references.h"
#include "viewbox.h"

#include <stddef.h>
#include <sigc++/connection.h>


class SPPattern : public SPPaintServer, public SPViewBox {
public:
	SPPattern();
	virtual ~SPPattern();

    /* Reference (href) */
    gchar *href;
    SPPatternReference *ref;

    /* patternUnits and patternContentUnits attribute */
    guint patternUnits : 1;
    guint patternUnits_set : 1;
    guint patternContentUnits : 1;
    guint patternContentUnits_set : 1;
    /* patternTransform attribute */
    Geom::Affine patternTransform;
    guint patternTransform_set : 1;
    /* Tile rectangle */
    SVGLength x;
    SVGLength y;
    SVGLength width;
    SVGLength height;

    sigc::connection modified_connection;

	virtual cairo_pattern_t* pattern_new(cairo_t *ct, Geom::OptRect const &bbox, double opacity);

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();
	virtual void set(unsigned int key, const gchar* value);
	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual void modified(unsigned int flags);
};


class SPPatternReference : public Inkscape::URIReference {
public:
    SPPatternReference (SPObject *obj) : URIReference(obj) {}
    SPPattern *getObject() const {
        return reinterpret_cast<SPPattern *>(URIReference::getObject());
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

guint pattern_users (SPPattern *pattern);
SPPattern *pattern_chain (SPPattern *pattern);
SPPattern *sp_pattern_clone_if_necessary (SPItem *item, SPPattern *pattern, const gchar *property);
void sp_pattern_transform_multiply (SPPattern *pattern, Geom::Affine postmul, bool set);

const gchar *pattern_tile (GSList *reprs, Geom::Rect bounds, SPDocument *document, Geom::Affine transform, Geom::Affine move);

SPPattern *pattern_getroot (SPPattern *pat);

guint pattern_patternUnits (SPPattern const *pat);
guint pattern_patternContentUnits (SPPattern const *pat);
Geom::Affine const &pattern_patternTransform(SPPattern const *pat);
gdouble pattern_x (SPPattern const *pat);
gdouble pattern_y (SPPattern const *pat);
gdouble pattern_width (SPPattern const *pat);
gdouble pattern_height (SPPattern const *pat);
Geom::OptRect pattern_viewBox (SPPattern const *pat);

#endif // SEEN_SP_PATTERN_H

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
