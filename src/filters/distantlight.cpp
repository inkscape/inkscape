/** \file
 * SVG <fedistantlight> implementation.
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>

#include "attributes.h"
#include "document.h"
#include "filters/distantlight.h"
#include "filters/diffuselighting.h"
#include "filters/specularlighting.h"
#include "xml/repr.h"

#define SP_MACROS_SILENT
#include "macros.h"

/* FeDistantLight class */
static void sp_fedistantlight_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_fedistantlight_release(SPObject *object);
static void sp_fedistantlight_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_fedistantlight_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_fedistantlight_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

G_DEFINE_TYPE(SPFeDistantLight, sp_fedistantlight, SP_TYPE_OBJECT);

static void
sp_fedistantlight_class_init(SPFeDistantLightClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    sp_object_class->build = sp_fedistantlight_build;
    sp_object_class->release = sp_fedistantlight_release;
    sp_object_class->write = sp_fedistantlight_write;
    sp_object_class->set = sp_fedistantlight_set;
    sp_object_class->update = sp_fedistantlight_update;
}

static void
sp_fedistantlight_init(SPFeDistantLight *fedistantlight)
{
    fedistantlight->azimuth = 0;
    fedistantlight->elevation = 0;
    fedistantlight->azimuth_set = FALSE;
    fedistantlight->elevation_set = FALSE;
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPDistantLight variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_fedistantlight_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) sp_fedistantlight_parent_class)->build) {
        ((SPObjectClass *) sp_fedistantlight_parent_class)->build(object, document, repr);
    }

    //Read values of key attributes from XML nodes into object.
    object->readAttr( "azimuth" );
    object->readAttr( "elevation" );

//is this necessary?
    document->addResource("fedistantlight", object);
}

/**
 * Drops any allocated memory.
 */
static void
sp_fedistantlight_release(SPObject *object)
{
    //SPFeDistantLight *fedistantlight = SP_FEDISTANTLIGHT(object);

    if ( object->document ) {
        // Unregister ourselves
        object->document->removeResource("fedistantlight", object);
    }

//TODO: release resources here
}

/**
 * Sets a specific value in the SPFeDistantLight.
 */
static void
sp_fedistantlight_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeDistantLight *fedistantlight = SP_FEDISTANTLIGHT(object);
    gchar *end_ptr;
    switch (key) {
    case SP_ATTR_AZIMUTH:
        end_ptr =NULL;
        if (value) {
            fedistantlight->azimuth = g_ascii_strtod(value, &end_ptr);
            if (end_ptr) {
                fedistantlight->azimuth_set = TRUE;
            }
        }
        if (!value || !end_ptr) {
                fedistantlight->azimuth_set = FALSE;
                fedistantlight->azimuth = 0;
        }
        if (object->parent &&
                (SP_IS_FEDIFFUSELIGHTING(object->parent) ||
                 SP_IS_FESPECULARLIGHTING(object->parent))) {
            object->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_ELEVATION:
        end_ptr =NULL;
        if (value) {
            fedistantlight->elevation = g_ascii_strtod(value, &end_ptr);
            if (end_ptr) {
                fedistantlight->elevation_set = TRUE;
            }
        }
        if (!value || !end_ptr) {
                fedistantlight->elevation_set = FALSE;
                fedistantlight->elevation = 0;
        }
        if (object->parent &&
                (SP_IS_FEDIFFUSELIGHTING(object->parent) ||
                 SP_IS_FESPECULARLIGHTING(object->parent))) {
            object->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    default:
        // See if any parents need this value.
        if (((SPObjectClass *) sp_fedistantlight_parent_class)->set) {
            ((SPObjectClass *) sp_fedistantlight_parent_class)->set(object, key, value);
        }
        break;
    }
}

/**
 *  * Receives update notifications.
 *   */
static void
sp_fedistantlight_update(SPObject *object, SPCtx *ctx, guint flags)
{
    SPFeDistantLight *feDistantLight = SP_FEDISTANTLIGHT(object);
    (void)feDistantLight;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        /* do something to trigger redisplay, updates? */
        object->readAttr( "azimuth" );
        object->readAttr( "elevation" );
    }

    if (((SPObjectClass *) sp_fedistantlight_parent_class)->update) {
        ((SPObjectClass *) sp_fedistantlight_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_fedistantlight_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    SPFeDistantLight *fedistantlight = SP_FEDISTANTLIGHT(object);

    if (!repr) {
        repr = object->getRepr()->duplicate(doc);
    }

    if (fedistantlight->azimuth_set)
        sp_repr_set_css_double(repr, "azimuth", fedistantlight->azimuth);
    if (fedistantlight->elevation_set)
        sp_repr_set_css_double(repr, "elevation", fedistantlight->elevation);

    if (((SPObjectClass *) sp_fedistantlight_parent_class)->write) {
        ((SPObjectClass *) sp_fedistantlight_parent_class)->write(object, doc, repr, flags);
    }

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
