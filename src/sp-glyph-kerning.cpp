/**
 * SVG <hkern> and <vkern> elements implementation
 * W3C SVG 1.1 spec, page 476, section 20.7
 *
 * Authors:
 *   Felipe C. da S. Sanches <juca@members.fsf.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "xml/repr.h"
#include "attributes.h"
#include "sp-glyph-kerning.h"

#include "document.h"
#include <string>
#include <cstring>


SPGlyphKerning::SPGlyphKerning() 
    : SPObject()
//TODO: correct these values:
    , u1(NULL)
    , g1(NULL)
    , u2(NULL)
    , g2(NULL)
    , k(0)
{
}

void SPGlyphKerning::build(SPDocument *document, Inkscape::XML::Node *repr)
{
    SPObject::build(document, repr);

    this->readAttr( "u1" );
    this->readAttr( "g1" );
    this->readAttr( "u2" );
    this->readAttr( "g2" );
    this->readAttr( "k" );
}

void SPGlyphKerning::release()
{
    SPObject::release();
}

GlyphNames::GlyphNames(const gchar* value)
{
    if (value) {
        names = g_strdup(value);
    }
}

GlyphNames::~GlyphNames()
{
    if (names) {
    	g_free(names);
    }
}

bool GlyphNames::contains(const char* name)
{
    if (!(this->names) || !name) {
    	return false;
    }
    
    std::istringstream is(this->names);
    std::string str;
    std::string s(name);
    
    while (is >> str) {
        if (str == s) {
            return true;
        }
    }
    
    return false;
}

void SPGlyphKerning::set(unsigned int key, const gchar *value)
{
    switch (key) {
        case SP_ATTR_U1:
        {
            if (this->u1) {
                delete this->u1;
            }
            
            this->u1 = new UnicodeRange(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_U2:
        {
            if (this->u2) {
                delete this->u2;
            }
            
            this->u2 = new UnicodeRange(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_G1:
        {
            if (this->g1) {
                delete this->g1;
            }
            
            this->g1 = new GlyphNames(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_G2:
        {
            if (this->g2) {
                delete this->g2;
            }
            
            this->g2 = new GlyphNames(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
             break;
        }
        case SP_ATTR_K:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;
            
            if (number != this->k){
                this->k = number;
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

/**
 * Receives update notifications.
 */
void SPGlyphKerning::update(SPCtx *ctx, guint flags)
{
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        /* do something to trigger redisplay, updates? */
        this->readAttr( "u1" );
        this->readAttr( "u2" );
        this->readAttr( "g2" );
        this->readAttr( "k" );
    }

    SPObject::update(ctx, flags);
}

#define COPY_ATTR(rd,rs,key) (rd)->setAttribute((key), rs->attribute(key));

Inkscape::XML::Node* SPGlyphKerning::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:glyphkerning"); // fix this!
    }

    if (repr != this->getRepr()) {
        // All the COPY_ATTR functions below use
        // XML Tree directly, while they shouldn't.
        COPY_ATTR(repr, this->getRepr(), "u1");
        COPY_ATTR(repr, this->getRepr(), "g1");
        COPY_ATTR(repr, this->getRepr(), "u2");
        COPY_ATTR(repr, this->getRepr(), "g2");
        COPY_ATTR(repr, this->getRepr(), "k");
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
