#include <cairo.h>

#include "sp-linear-gradient.h"

#include "attributes.h"
#include "xml/repr.h"

/*
 * Linear Gradient
 */
SPLinearGradient::SPLinearGradient() : SPGradient() {
    this->x1.unset(SVGLength::PERCENT, 0.0, 0.0);
    this->y1.unset(SVGLength::PERCENT, 0.0, 0.0);
    this->x2.unset(SVGLength::PERCENT, 1.0, 1.0);
    this->y2.unset(SVGLength::PERCENT, 0.0, 0.0);
}

SPLinearGradient::~SPLinearGradient() {
}

void SPLinearGradient::build(SPDocument *document, Inkscape::XML::Node *repr) {
    SPGradient::build(document, repr);

    this->readAttr( "x1" );
    this->readAttr( "y1" );
    this->readAttr( "x2" );
    this->readAttr( "y2" );
}

/**
 * Callback: set attribute.
 */
void SPLinearGradient::set(unsigned int key, const gchar* value) {
    switch (key) {
        case SP_ATTR_X1:
            this->x1.readOrUnset(value, SVGLength::PERCENT, 0.0, 0.0);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_Y1:
            this->y1.readOrUnset(value, SVGLength::PERCENT, 0.0, 0.0);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_X2:
            this->x2.readOrUnset(value, SVGLength::PERCENT, 1.0, 1.0);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_Y2:
            this->y2.readOrUnset(value, SVGLength::PERCENT, 0.0, 0.0);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        default:
            SPGradient::set(key, value);
            break;
    }
}

/**
 * Callback: write attributes to associated repr.
 */
Inkscape::XML::Node* SPLinearGradient::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:linearGradient");
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || this->x1._set) {
        sp_repr_set_svg_double(repr, "x1", this->x1.computed);
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || this->y1._set) {
        sp_repr_set_svg_double(repr, "y1", this->y1.computed);
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || this->x2._set) {
        sp_repr_set_svg_double(repr, "x2", this->x2.computed);
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || this->y2._set) {
        sp_repr_set_svg_double(repr, "y2", this->y2.computed);
    }

    SPGradient::write(xml_doc, repr, flags);

    return repr;
}

cairo_pattern_t* SPLinearGradient::pattern_new(cairo_t * /*ct*/, Geom::OptRect const &bbox, double opacity) {
    this->ensureVector();

    cairo_pattern_t *cp = cairo_pattern_create_linear(
        this->x1.computed, this->y1.computed,
        this->x2.computed, this->y2.computed);

    sp_gradient_pattern_common_setup(cp, this, bbox, opacity);

    return cp;
}
