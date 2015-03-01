/** @file
 * @solid color class.
 */
/* Authors:
 *   Tavmjong Bah <tavjong@free.fr>
 *
 * Copyright (C) 2014 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include <cairo.h>

#include "sp-solid-color.h"

#include "attributes.h"
#include "style.h"
#include "xml/repr.h"

#include "sp-item.h"
#include "style-internal.h"


/*
 * Solid Color
 */
SPSolidColor::SPSolidColor() : SPPaintServer() {
}

SPSolidColor::~SPSolidColor() {
}

void SPSolidColor::build(SPDocument* doc, Inkscape::XML::Node* repr) {
    SPPaintServer::build(doc, repr);

    this->readAttr( "style" );
    this->readAttr( "solid-color" );
    this->readAttr( "solid-opacity" );
}

/**
 * Virtual build: set solidcolor attributes from its associated XML node.
 */

void SPSolidColor::set(unsigned int key, const gchar* value) {

    if (SP_ATTRIBUTE_IS_CSS(key)) {
        style->readFromObject( this );
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
    } else {
        SPPaintServer::set(key, value);
    }
}

/**
 * Virtual set: set attribute to value.
 */

Inkscape::XML::Node* SPSolidColor::write(Inkscape::XML::Document* xml_doc, Inkscape::XML::Node* repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:solidColor");
    }

    SPObject::write(xml_doc, repr, flags);

    return repr;
}

cairo_pattern_t* SPSolidColor::pattern_new(cairo_t * /*ct*/, Geom::OptRect const & /*bbox*/, double opacity) {

    SPIColor *c = &(this->style->solid_color);
    cairo_pattern_t *cp = cairo_pattern_create_rgba ( c->value.color.v.c[0], c->value.color.v.c[1], c->value.color.v.c[2], SP_SCALE24_TO_FLOAT(this->style->solid_opacity.value) * opacity );

    return cp;
}


/**
 * Virtual write: write object attributes to repr.
 */

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
