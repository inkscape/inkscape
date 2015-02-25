/** \file
 * SVG <fespotlight> implementation.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *   Jean-Rene Reinhard <jr@komite.net>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006,2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#include "attributes.h"
#include "document.h"
#include "filters/spotlight.h"
#include "filters/diffuselighting.h"
#include "filters/specularlighting.h"
#include "xml/repr.h"

#define SP_MACROS_SILENT
#include "macros.h"

SPFeSpotLight::SPFeSpotLight()
    : SPObject(), x(0), x_set(FALSE), y(0), y_set(FALSE), z(0), z_set(FALSE), pointsAtX(0), pointsAtX_set(FALSE),
      pointsAtY(0), pointsAtY_set(FALSE), pointsAtZ(0), pointsAtZ_set(FALSE),
      specularExponent(1), specularExponent_set(FALSE), limitingConeAngle(90),
      limitingConeAngle_set(FALSE)
{
}

SPFeSpotLight::~SPFeSpotLight() {
}


/**
 * Reads the Inkscape::XML::Node, and initializes SPPointLight variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPFeSpotLight::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPObject::build(document, repr);

    //Read values of key attributes from XML nodes into object.
    this->readAttr( "x" );
    this->readAttr( "y" );
    this->readAttr( "z" );
    this->readAttr( "pointsAtX" );
    this->readAttr( "pointsAtY" );
    this->readAttr( "pointsAtZ" );
    this->readAttr( "specularExponent" );
    this->readAttr( "limitingConeAngle" );

//is this necessary?
    document->addResource("fespotlight", this);
}

/**
 * Drops any allocated memory.
 */
void SPFeSpotLight::release() {
    if ( this->document ) {
        // Unregister ourselves
        this->document->removeResource("fespotlight", this);
    }

//TODO: release resources here
}

/**
 * Sets a specific value in the SPFeSpotLight.
 */
void SPFeSpotLight::set(unsigned int key, gchar const *value) {
    gchar *end_ptr;

    switch (key) {
    case SP_ATTR_X:
        end_ptr = NULL;
        
        if (value) {
            this->x = g_ascii_strtod(value, &end_ptr);
            
            if (end_ptr) {
                this->x_set = TRUE;
            }
        }
        
        if(!value || !end_ptr) {
            this->x = 0;
            this->x_set = FALSE;
        }
        
        if (this->parent &&
                (SP_IS_FEDIFFUSELIGHTING(this->parent) ||
                 SP_IS_FESPECULARLIGHTING(this->parent))) {
            this->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_Y:
        end_ptr = NULL;
        
        if (value) {
            this->y = g_ascii_strtod(value, &end_ptr);
            
            if (end_ptr) {
                this->y_set = TRUE;
            }
        }
        
        if(!value || !end_ptr) {
            this->y = 0;
            this->y_set = FALSE;
        }
        
        if (this->parent &&
                (SP_IS_FEDIFFUSELIGHTING(this->parent) ||
                 SP_IS_FESPECULARLIGHTING(this->parent))) {
            this->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_Z:
        end_ptr = NULL;
        
        if (value) {
            this->z = g_ascii_strtod(value, &end_ptr);
            
            if (end_ptr) {
                this->z_set = TRUE;
            }
        }
        
        if(!value || !end_ptr) {
            this->z = 0;
            this->z_set = FALSE;
        }
        
        if (this->parent &&
                (SP_IS_FEDIFFUSELIGHTING(this->parent) ||
                 SP_IS_FESPECULARLIGHTING(this->parent))) {
            this->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_POINTSATX:
        end_ptr = NULL;
        
        if (value) {
            this->pointsAtX = g_ascii_strtod(value, &end_ptr);
            
            if (end_ptr) {
                this->pointsAtX_set = TRUE;
            }
        }
        
        if(!value || !end_ptr) {
            this->pointsAtX = 0;
            this->pointsAtX_set = FALSE;
        }
        
        if (this->parent &&
                (SP_IS_FEDIFFUSELIGHTING(this->parent) ||
                 SP_IS_FESPECULARLIGHTING(this->parent))) {
            this->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_POINTSATY:
        end_ptr = NULL;
        
        if (value) {
            this->pointsAtY = g_ascii_strtod(value, &end_ptr);
            
            if (end_ptr) {
                this->pointsAtY_set = TRUE;
            }
        }
        
        if(!value || !end_ptr) {
            this->pointsAtY = 0;
            this->pointsAtY_set = FALSE;
        }
        
        if (this->parent &&
                (SP_IS_FEDIFFUSELIGHTING(this->parent) ||
                 SP_IS_FESPECULARLIGHTING(this->parent))) {
            this->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_POINTSATZ:
        end_ptr = NULL;
        
        if (value) {
            this->pointsAtZ = g_ascii_strtod(value, &end_ptr);
            
            if (end_ptr) {
                this->pointsAtZ_set = TRUE;
            }
        }
        
        if(!value || !end_ptr) {
            this->pointsAtZ = 0;
            this->pointsAtZ_set = FALSE;
        }
        
        if (this->parent &&
                (SP_IS_FEDIFFUSELIGHTING(this->parent) ||
                 SP_IS_FESPECULARLIGHTING(this->parent))) {
            this->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_SPECULAREXPONENT:
        end_ptr = NULL;
        
        if (value) {
            this->specularExponent = g_ascii_strtod(value, &end_ptr);
            
            if (end_ptr) {
                this->specularExponent_set = TRUE;
            }
        }
        
        if(!value || !end_ptr) {
            this->specularExponent = 1;
            this->specularExponent_set = FALSE;
        }
        
        if (this->parent &&
                (SP_IS_FEDIFFUSELIGHTING(this->parent) ||
                 SP_IS_FESPECULARLIGHTING(this->parent))) {
            this->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_LIMITINGCONEANGLE:
        end_ptr = NULL;
        
        if (value) {
            this->limitingConeAngle = g_ascii_strtod(value, &end_ptr);
            
            if (end_ptr) {
                this->limitingConeAngle_set = TRUE;
            }
        }
        
        if(!value || !end_ptr) {
            this->limitingConeAngle = 90;
            this->limitingConeAngle_set = FALSE;
        }
        
        if (this->parent &&
                (SP_IS_FEDIFFUSELIGHTING(this->parent) ||
                 SP_IS_FESPECULARLIGHTING(this->parent))) {
            this->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    default:
        // See if any parents need this value.
    	SPObject::set(key, value);
        break;
    }
}

/**
 *  * Receives update notifications.
 *   */
void SPFeSpotLight::update(SPCtx *ctx, guint flags) {
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        /* do something to trigger redisplay, updates? */
        this->readAttr( "x" );
        this->readAttr( "y" );
        this->readAttr( "z" );
        this->readAttr( "pointsAtX" );
        this->readAttr( "pointsAtY" );
        this->readAttr( "pointsAtZ" );
        this->readAttr( "specularExponent" );
        this->readAttr( "limitingConeAngle" );
    }

    SPObject::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPFeSpotLight::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    if (!repr) {
        repr = this->getRepr()->duplicate(doc);
    }

    if (this->x_set)
        sp_repr_set_css_double(repr, "x", this->x);
    if (this->y_set)
        sp_repr_set_css_double(repr, "y", this->y);
    if (this->z_set)
        sp_repr_set_css_double(repr, "z", this->z);
    if (this->pointsAtX_set)
        sp_repr_set_css_double(repr, "pointsAtX", this->pointsAtX);
    if (this->pointsAtY_set)
        sp_repr_set_css_double(repr, "pointsAtY", this->pointsAtY);
    if (this->pointsAtZ_set)
        sp_repr_set_css_double(repr, "pointsAtZ", this->pointsAtZ);
    if (this->specularExponent_set)
        sp_repr_set_css_double(repr, "specularExponent", this->specularExponent);
    if (this->limitingConeAngle_set)
        sp_repr_set_css_double(repr, "limitingConeAngle", this->limitingConeAngle);

    SPObject::write(doc, repr, flags);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
