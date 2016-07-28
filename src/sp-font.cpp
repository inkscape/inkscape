#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*
 * SVG <font> element implementation
 *
 * Author:
 *   Felipe C. da S. Sanches <juca@members.fsf.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2008, Felipe C. da S. Sanches
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "xml/repr.h"
#include "attributes.h"
#include "sp-font.h"
#include "sp-glyph.h"
#include "sp-missing-glyph.h"
#include "document.h"

#include "display/nr-svgfonts.h"


//I think we should have extra stuff here and in the set method in order to set default value as specified at http://www.w3.org/TR/SVG/fonts.html

// TODO determine better values and/or make these dynamic:
double FNT_DEFAULT_ADV = 90; // TODO determine proper default
double FNT_DEFAULT_ASCENT = 90; // TODO determine proper default
double FNT_UNITS_PER_EM = 90; // TODO determine proper default

SPFont::SPFont() : SPObject() {
    this->horiz_origin_x = 0;
    this->horiz_origin_y = 0;
    this->horiz_adv_x = FNT_DEFAULT_ADV;
    this->vert_origin_x = FNT_DEFAULT_ADV / 2.0;
    this->vert_origin_y = FNT_DEFAULT_ASCENT;
    this->vert_adv_y = FNT_UNITS_PER_EM;
}

SPFont::~SPFont() {
}

void SPFont::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPObject::build(document, repr);

	this->readAttr( "horiz-origin-x" );
	this->readAttr( "horiz-origin-y" );
	this->readAttr( "horiz-adv-x" );
	this->readAttr( "vert-origin-x" );
	this->readAttr( "vert-origin-y" );
	this->readAttr( "vert-adv-y" );

	document->addResource("font", this);
}

/**
 * Callback for child_added event.
 */
void SPFont::child_added(Inkscape::XML::Node *child, Inkscape::XML::Node *ref) {
    SPObject::child_added(child, ref);

    this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}


/**
 * Callback for remove_child event.
 */
void SPFont::remove_child(Inkscape::XML::Node* child) {
    SPObject::remove_child(child);

    this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

void SPFont::release() {
    this->document->removeResource("font", this);

    SPObject::release();
}

void SPFont::set(unsigned int key, const gchar *value) {
    // TODO these are floating point, so some epsilon comparison would be good
    switch (key) {
        case SP_ATTR_HORIZ_ORIGIN_X:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;

            if (number != this->horiz_origin_x){
                this->horiz_origin_x = number;
                this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_HORIZ_ORIGIN_Y:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;

            if (number != this->horiz_origin_y){
                this->horiz_origin_y = number;
                this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_HORIZ_ADV_X:
        {
            double number = value ? g_ascii_strtod(value, 0) : FNT_DEFAULT_ADV;

            if (number != this->horiz_adv_x){
                this->horiz_adv_x = number;
                this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_VERT_ORIGIN_X:
        {
            double number = value ? g_ascii_strtod(value, 0) : FNT_DEFAULT_ADV / 2.0;

            if (number != this->vert_origin_x){
                this->vert_origin_x = number;
                this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_VERT_ORIGIN_Y:
        {
            double number = value ? g_ascii_strtod(value, 0) : FNT_DEFAULT_ASCENT;

            if (number != this->vert_origin_y){
                this->vert_origin_y = number;
                this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_VERT_ADV_Y:
        {
            double number = value ? g_ascii_strtod(value, 0) : FNT_UNITS_PER_EM;

            if (number != this->vert_adv_y){
                this->vert_adv_y = number;
                this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        default:
        	SPObject::set(key, value);
            break;
    }
}

/**
 * Receives update notifications.
 */
void SPFont::update(SPCtx *ctx, guint flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG)) {
        this->readAttr( "horiz-origin-x" );
        this->readAttr( "horiz-origin-y" );
        this->readAttr( "horiz-adv-x" );
        this->readAttr( "vert-origin-x" );
        this->readAttr( "vert-origin-y" );
        this->readAttr( "vert-adv-y" );
    }

    SPObject::update(ctx, flags);
}

#define COPY_ATTR(rd,rs,key) (rd)->setAttribute((key), rs->attribute(key));

Inkscape::XML::Node* SPFont::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:font");
    }

    sp_repr_set_svg_double(repr, "horiz-origin-x", this->horiz_origin_x);
    sp_repr_set_svg_double(repr, "horiz-origin-y", this->horiz_origin_y);
    sp_repr_set_svg_double(repr, "horiz-adv-x", this->horiz_adv_x);
    sp_repr_set_svg_double(repr, "vert-origin-x", this->vert_origin_x);
    sp_repr_set_svg_double(repr, "vert-origin-y", this->vert_origin_y);
    sp_repr_set_svg_double(repr, "vert-adv-y", this->vert_adv_y);

    if (repr != this->getRepr()) {
        // All the below COPY_ATTR funtions are directly using 
        //  the XML Tree while they shouldn't
        COPY_ATTR(repr, this->getRepr(), "horiz-origin-x");
        COPY_ATTR(repr, this->getRepr(), "horiz-origin-y");
        COPY_ATTR(repr, this->getRepr(), "horiz-adv-x");
        COPY_ATTR(repr, this->getRepr(), "vert-origin-x");
        COPY_ATTR(repr, this->getRepr(), "vert-origin-y");
        COPY_ATTR(repr, this->getRepr(), "vert-adv-y");
    }

    SPObject::write(xml_doc, repr, flags);

    return repr;
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
