#define __SP_FESPOTLIGHT_CPP__

/** \file
 * SVG <fespotlight> implementation.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *   Jean-Rene Reinhard <jr@komite.net>
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
#include "spotlight.h"
#include "diffuselighting-fns.h"
#include "specularlighting-fns.h"
#include "xml/repr.h"

#define SP_MACROS_SILENT
#include "macros.h"

/* FeSpotLight class */

static void sp_fespotlight_class_init(SPFeSpotLightClass *klass);
static void sp_fespotlight_init(SPFeSpotLight *fespotlight);

static void sp_fespotlight_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_fespotlight_release(SPObject *object);
static void sp_fespotlight_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_fespotlight_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_fespotlight_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *feSpotLight_parent_class;

GType
sp_fespotlight_get_type()
{
    static GType fespotlight_type = 0;

    if (!fespotlight_type) {
        GTypeInfo fespotlight_info = {
            sizeof(SPFeSpotLightClass),
            NULL, NULL,
            (GClassInitFunc) sp_fespotlight_class_init,
            NULL, NULL,
            sizeof(SPFeSpotLight),
            16,
            (GInstanceInitFunc) sp_fespotlight_init,
            NULL,    /* value_table */
        };
        fespotlight_type = g_type_register_static(SP_TYPE_OBJECT, "SPFeSpotLight", &fespotlight_info, (GTypeFlags)0);
    }
    return fespotlight_type;
}

static void
sp_fespotlight_class_init(SPFeSpotLightClass *klass)
{

    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    feSpotLight_parent_class = (SPObjectClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_fespotlight_build;
    sp_object_class->release = sp_fespotlight_release;
    sp_object_class->write = sp_fespotlight_write;
    sp_object_class->set = sp_fespotlight_set;
    sp_object_class->update = sp_fespotlight_update;
}

static void
sp_fespotlight_init(SPFeSpotLight *fespotlight)
{
    fespotlight->x = 0;
    fespotlight->y = 0;
    fespotlight->z = 0;
    fespotlight->pointsAtX = 0;
    fespotlight->pointsAtY = 0;
    fespotlight->pointsAtZ = 0;
    fespotlight->specularExponent = 1;
    fespotlight->limitingConeAngle = 90;

    fespotlight->x_set = FALSE;
    fespotlight->y_set = FALSE;
    fespotlight->z_set = FALSE;
    fespotlight->pointsAtX_set = FALSE;
    fespotlight->pointsAtY_set = FALSE;
    fespotlight->pointsAtZ_set = FALSE;
    fespotlight->specularExponent_set = FALSE;
    fespotlight->limitingConeAngle_set = FALSE;
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPPointLight variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_fespotlight_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feSpotLight_parent_class)->build) {
        ((SPObjectClass *) feSpotLight_parent_class)->build(object, document, repr);
    }

    //Read values of key attributes from XML nodes into object.
    sp_object_read_attr(object, "x");
    sp_object_read_attr(object, "y");
    sp_object_read_attr(object, "z");
    sp_object_read_attr(object, "pointsAtX");
    sp_object_read_attr(object, "pointsAtY");
    sp_object_read_attr(object, "pointsAtZ");
    sp_object_read_attr(object, "specularExponent");
    sp_object_read_attr(object, "limitingConeAngle");

//is this necessary?
    sp_document_add_resource(document, "fespotlight", object);
}

/**
 * Drops any allocated memory.
 */
static void
sp_fespotlight_release(SPObject *object)
{
    //SPFeSpotLight *fespotlight = SP_FESPOTLIGHT(object);

    if (SP_OBJECT_DOCUMENT(object)) {
        /* Unregister ourselves */
        sp_document_remove_resource(SP_OBJECT_DOCUMENT(object), "fespotlight", SP_OBJECT(object));
    }

//TODO: release resources here
}

/**
 * Sets a specific value in the SPFeSpotLight.
 */
static void
sp_fespotlight_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeSpotLight *fespotlight = SP_FESPOTLIGHT(object);
    gchar *end_ptr;

    switch (key) {
    case SP_ATTR_X:
        end_ptr = NULL;
        if (value) {
            fespotlight->x = g_ascii_strtod(value, &end_ptr);
            if (end_ptr)
                fespotlight->x_set = TRUE;
        }
        if(!value || !end_ptr) {
            fespotlight->x = 0;
            fespotlight->x_set = FALSE;
        }
        if (object->parent &&
                (SP_IS_FEDIFFUSELIGHTING(object->parent) ||
                 SP_IS_FESPECULARLIGHTING(object->parent))) {
            object->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_Y:
        end_ptr = NULL;
        if (value) {
            fespotlight->y = g_ascii_strtod(value, &end_ptr);
            if (end_ptr)
                fespotlight->y_set = TRUE;
        }
        if(!value || !end_ptr) {
            fespotlight->y = 0;
            fespotlight->y_set = FALSE;
        }
        if (object->parent &&
                (SP_IS_FEDIFFUSELIGHTING(object->parent) ||
                 SP_IS_FESPECULARLIGHTING(object->parent))) {
            object->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_Z:
        end_ptr = NULL;
        if (value) {
            fespotlight->z = g_ascii_strtod(value, &end_ptr);
            if (end_ptr)
                fespotlight->z_set = TRUE;
        }
        if(!value || !end_ptr) {
            fespotlight->z = 0;
            fespotlight->z_set = FALSE;
        }
        if (object->parent &&
                (SP_IS_FEDIFFUSELIGHTING(object->parent) ||
                 SP_IS_FESPECULARLIGHTING(object->parent))) {
            object->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_POINTSATX:
        end_ptr = NULL;
        if (value) {
            fespotlight->pointsAtX = g_ascii_strtod(value, &end_ptr);
            if (end_ptr)
                fespotlight->pointsAtX_set = TRUE;
        }
        if(!value || !end_ptr) {
            fespotlight->pointsAtX = 0;
            fespotlight->pointsAtX_set = FALSE;
        }
        if (object->parent &&
                (SP_IS_FEDIFFUSELIGHTING(object->parent) ||
                 SP_IS_FESPECULARLIGHTING(object->parent))) {
            object->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_POINTSATY:
        end_ptr = NULL;
        if (value) {
            fespotlight->pointsAtY = g_ascii_strtod(value, &end_ptr);
            if (end_ptr)
                fespotlight->pointsAtY_set = TRUE;
        }
        if(!value || !end_ptr) {
            fespotlight->pointsAtY = 0;
            fespotlight->pointsAtY_set = FALSE;
        }
        if (object->parent &&
                (SP_IS_FEDIFFUSELIGHTING(object->parent) ||
                 SP_IS_FESPECULARLIGHTING(object->parent))) {
            object->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_POINTSATZ:
        end_ptr = NULL;
        if (value) {
            fespotlight->pointsAtZ = g_ascii_strtod(value, &end_ptr);
            if (end_ptr)
                fespotlight->pointsAtZ_set = TRUE;
        }
        if(!value || !end_ptr) {
            fespotlight->pointsAtZ = 0;
            fespotlight->pointsAtZ_set = FALSE;
        }
        if (object->parent &&
                (SP_IS_FEDIFFUSELIGHTING(object->parent) ||
                 SP_IS_FESPECULARLIGHTING(object->parent))) {
            object->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_SPECULAREXPONENT:
        end_ptr = NULL;
        if (value) {
            fespotlight->specularExponent = g_ascii_strtod(value, &end_ptr);
            if (end_ptr)
                fespotlight->specularExponent_set = TRUE;
        }
        if(!value || !end_ptr) {
            fespotlight->specularExponent = 1;
            fespotlight->specularExponent_set = FALSE;
        }
        if (object->parent &&
                (SP_IS_FEDIFFUSELIGHTING(object->parent) ||
                 SP_IS_FESPECULARLIGHTING(object->parent))) {
            object->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    case SP_ATTR_LIMITINGCONEANGLE:
        end_ptr = NULL;
        if (value) {
            fespotlight->limitingConeAngle = g_ascii_strtod(value, &end_ptr);
            if (end_ptr)
                fespotlight->limitingConeAngle_set = TRUE;
        }
        if(!value || !end_ptr) {
            fespotlight->limitingConeAngle = 90;
            fespotlight->limitingConeAngle_set = FALSE;
        }
        if (object->parent &&
                (SP_IS_FEDIFFUSELIGHTING(object->parent) ||
                 SP_IS_FESPECULARLIGHTING(object->parent))) {
            object->parent->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
        break;
    default:
        // See if any parents need this value.
        if (((SPObjectClass *) feSpotLight_parent_class)->set) {
            ((SPObjectClass *) feSpotLight_parent_class)->set(object, key, value);
        }
        break;
    }
}

/**
 *  * Receives update notifications.
 *   */
static void
sp_fespotlight_update(SPObject *object, SPCtx *ctx, guint flags)
{
    SPFeSpotLight *feSpotLight = SP_FESPOTLIGHT(object);
    (void)feSpotLight;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        /* do something to trigger redisplay, updates? */
        sp_object_read_attr(object, "x");
        sp_object_read_attr(object, "y");
        sp_object_read_attr(object, "z");
        sp_object_read_attr(object, "pointsAtX");
        sp_object_read_attr(object, "pointsAtY");
        sp_object_read_attr(object, "pointsAtZ");
        sp_object_read_attr(object, "specularExponent");
        sp_object_read_attr(object, "limitingConeAngle");
    }

    if (((SPObjectClass *) feSpotLight_parent_class)->update) {
        ((SPObjectClass *) feSpotLight_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_fespotlight_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    SPFeSpotLight *fespotlight = SP_FESPOTLIGHT(object);

    if (!repr) {
        repr = SP_OBJECT_REPR(object)->duplicate(doc);
    }

    if (fespotlight->x_set)
        sp_repr_set_css_double(repr, "x", fespotlight->x);
    if (fespotlight->y_set)
        sp_repr_set_css_double(repr, "y", fespotlight->y);
    if (fespotlight->z_set)
        sp_repr_set_css_double(repr, "z", fespotlight->z);
    if (fespotlight->pointsAtX_set)
        sp_repr_set_css_double(repr, "pointsAtX", fespotlight->pointsAtX);
    if (fespotlight->pointsAtY_set)
        sp_repr_set_css_double(repr, "pointsAtY", fespotlight->pointsAtY);
    if (fespotlight->pointsAtZ_set)
        sp_repr_set_css_double(repr, "pointsAtZ", fespotlight->pointsAtZ);
    if (fespotlight->specularExponent_set)
        sp_repr_set_css_double(repr, "specularExponent", fespotlight->specularExponent);
    if (fespotlight->limitingConeAngle_set)
        sp_repr_set_css_double(repr, "limitingConeAngle", fespotlight->limitingConeAngle);

    if (((SPObjectClass *) feSpotLight_parent_class)->write) {
        ((SPObjectClass *) feSpotLight_parent_class)->write(object, doc, repr, flags);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
