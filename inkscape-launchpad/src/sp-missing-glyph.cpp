#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*
 * SVG <missing-glyph> element implementation
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
#include "sp-missing-glyph.h"
#include "document.h"

SPMissingGlyph::SPMissingGlyph() : SPObject() {
//TODO: correct these values:
    this->d = NULL;
    this->horiz_adv_x = 0;
    this->vert_origin_x = 0;
    this->vert_origin_y = 0;
    this->vert_adv_y = 0;
}

SPMissingGlyph::~SPMissingGlyph() {
}

void SPMissingGlyph::build(SPDocument* doc, Inkscape::XML::Node* repr) {
    SPObject::build(doc, repr);

    this->readAttr( "d" );
    this->readAttr( "horiz-adv-x" );
    this->readAttr( "vert-origin-x" );
    this->readAttr( "vert-origin-y" );
    this->readAttr( "vert-adv-y" );
}

void SPMissingGlyph::release() {
	SPObject::release();
}


void SPMissingGlyph::set(unsigned int key, const gchar* value) {
    switch (key) {
        case SP_ATTR_D:
        {
            if (this->d) {
                g_free(this->d);
            }
            this->d = g_strdup(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_HORIZ_ADV_X:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;
            if (number != this->horiz_adv_x){
                this->horiz_adv_x = number;
                this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_VERT_ORIGIN_X:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;
            if (number != this->vert_origin_x){
                this->vert_origin_x = number;
                this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_VERT_ORIGIN_Y:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;
            if (number != this->vert_origin_y){
                this->vert_origin_y = number;
                this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_VERT_ADV_Y:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;
            if (number != this->vert_adv_y){
                this->vert_adv_y = number;
                this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        default:
        {
            SPObject::set(key, value);
            break;
        }
    }
}

#define COPY_ATTR(rd,rs,key) (rd)->setAttribute((key), rs->attribute(key));

Inkscape::XML::Node* SPMissingGlyph::write(Inkscape::XML::Document* xml_doc, Inkscape::XML::Node* repr, guint flags) {
	    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
	        repr = xml_doc->createElement("svg:glyph");
	    }

	/* I am commenting out this part because I am not certain how does it work. I will have to study it later. Juca
	    repr->setAttribute("d", glyph->d);
	    sp_repr_set_svg_double(repr, "horiz-adv-x", glyph->horiz_adv_x);
	    sp_repr_set_svg_double(repr, "vert-origin-x", glyph->vert_origin_x);
	    sp_repr_set_svg_double(repr, "vert-origin-y", glyph->vert_origin_y);
	    sp_repr_set_svg_double(repr, "vert-adv-y", glyph->vert_adv_y);
	*/
	    if (repr != this->getRepr()) {

	    	// TODO
	        // All the COPY_ATTR functions below use
	        //  XML Tree directly while they shouldn't.
	        COPY_ATTR(repr, this->getRepr(), "d");
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
