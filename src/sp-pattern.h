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

#define SP_PATTERN(obj) (dynamic_cast<SPPattern*>((SPObject*)obj))
#define SP_IS_PATTERN(obj) (dynamic_cast<const SPPattern*>((SPObject*)obj) != NULL)

class SPPatternReference;
class SPItem;
typedef struct _GSList GSList;

#include "svg/svg-length.h"
#include "sp-paint-server.h"
#include "uri-references.h"
#include "viewbox.h"

#include <sigc++/connection.h>


class SPPattern : public SPPaintServer, public SPViewBox {
public:
	SPPattern();
	virtual ~SPPattern();

    /* Reference (href) */
    char *href;
    SPPatternReference *ref;

    /* patternUnits and patternContentUnits attribute */
    unsigned int patternUnits : 1;
    unsigned int patternUnits_set : 1;
    unsigned int patternContentUnits : 1;
    unsigned int patternContentUnits_set : 1;
    /* patternTransform attribute */
    Geom::Affine patternTransform;
    unsigned int patternTransform_set : 1;
    /* Tile rectangle */
    SVGLength x;
    SVGLength y;
    SVGLength width;
    SVGLength height;

    sigc::connection modified_connection;

    bool isValid() const;

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

unsigned int pattern_users (SPPattern *pattern);
SPPattern *pattern_chain (SPPattern *pattern);
SPPattern *sp_pattern_clone_if_necessary (SPItem *item, SPPattern *pattern, const char *property);
void sp_pattern_transform_multiply (SPPattern *pattern, Geom::Affine postmul, bool set);

const char *pattern_tile (const std::vector<Inkscape::XML::Node*> &reprs, Geom::Rect bounds, SPDocument *document, Geom::Affine transform, Geom::Affine move);

SPPattern *pattern_getroot (SPPattern *pat);

unsigned int pattern_patternUnits (SPPattern const *pat);
unsigned int pattern_patternContentUnits (SPPattern const *pat);
Geom::Affine const &pattern_patternTransform(SPPattern const *pat);
double pattern_x (SPPattern const *pat);
double pattern_y (SPPattern const *pat);
double pattern_width (SPPattern const *pat);
double pattern_height (SPPattern const *pat);
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
