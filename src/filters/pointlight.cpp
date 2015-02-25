/** \file
 * SVG <fepointlight> implementation.
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
#include "filters/pointlight.h"
#include "filters/diffuselighting.h"
#include "filters/specularlighting.h"
#include "xml/repr.h"

#define SP_MACROS_SILENT
#include "macros.h"

SPFePointLight::SPFePointLight() 
    : SPObject(), x(0), x_set(FALSE), y(0), y_set(FALSE), z(0), z_set(FALSE) {
}

SPFePointLight::~SPFePointLight() {
}


/**
 * Reads the Inkscape::XML::Node, and initializes SPPointLight variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPFePointLight::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPObject::build(document, repr);

    //Read values of key attributes from XML nodes into object.
    this->readAttr( "x" );
    this->readAttr( "y" );
    this->readAttr( "z" );

//is this necessary?
    document->addResource("fepointlight", this);
}

/**
 * Drops any allocated memory.
 */
void SPFePointLight::release() {
    if ( this->document ) {
        // Unregister ourselves
        this->document->removeResource("fepointlight", this);
    }

//TODO: release resources here
}

/**
 * Sets a specific value in the SPFePointLight.
 */
void SPFePointLight::set(unsigned int key, gchar const *value) {
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

        if (!value || !end_ptr) {
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

        if (!value || !end_ptr) {
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

        if (!value || !end_ptr) {
            this->z = 0;
            this->z_set = FALSE;
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
void SPFePointLight::update(SPCtx *ctx, guint flags) {
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        /* do something to trigger redisplay, updates? */
        this->readAttr( "x" );
        this->readAttr( "y" );
        this->readAttr( "z" );
    }

    SPObject::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPFePointLight::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    if (!repr) {
        repr = this->getRepr()->duplicate(doc);
    }

    if (this->x_set)
        sp_repr_set_css_double(repr, "x", this->x);
    if (this->y_set)
        sp_repr_set_css_double(repr, "y", this->y);
    if (this->z_set)
        sp_repr_set_css_double(repr, "z", this->z);

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
