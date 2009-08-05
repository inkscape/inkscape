#define __SP_FESPECULARLIGHTING_CPP__

/** \file
 * SVG <feSpecularLighting> implementation.
 *
 */
/*
 * Authors:
 *   hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Jean-Rene Reinhard <jr@komite.net>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *               2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "attributes.h"
#include "svg/svg.h"
#include "sp-object.h"
#include "svg/svg-color.h"
#include "specularlighting.h"
#include "xml/repr.h"
#include "display/nr-filter-specularlighting.h"

/* FeSpecularLighting base class */

static void sp_feSpecularLighting_class_init(SPFeSpecularLightingClass *klass);
static void sp_feSpecularLighting_init(SPFeSpecularLighting *feSpecularLighting);

static void sp_feSpecularLighting_build(SPObject *object, Document *document, Inkscape::XML::Node *repr);
static void sp_feSpecularLighting_release(SPObject *object);
static void sp_feSpecularLighting_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feSpecularLighting_update(SPObject *object, SPCtx *ctx, guint flags);
//we assume that svg:feSpecularLighting can have any number of children
//only the first one is considered as the light source of the filter
//TODO is that right?
//if not modify child_added and remove_child to raise errors
static void sp_feSpecularLighting_child_added(SPObject *object,
                                    Inkscape::XML::Node *child,
                                    Inkscape::XML::Node *ref);
static void sp_feSpecularLighting_remove_child(SPObject *object, Inkscape::XML::Node *child);
static void sp_feSpecularLighting_order_changed(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *old_ref, Inkscape::XML::Node *new_ref);
static Inkscape::XML::Node *sp_feSpecularLighting_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_feSpecularLighting_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter);
static void sp_feSpecularLighting_children_modified(SPFeSpecularLighting *sp_specularlighting);

static SPFilterPrimitiveClass *feSpecularLighting_parent_class;

GType
sp_feSpecularLighting_get_type()
{
    static GType feSpecularLighting_type = 0;

    if (!feSpecularLighting_type) {
        GTypeInfo feSpecularLighting_info = {
            sizeof(SPFeSpecularLightingClass),
            NULL, NULL,
            (GClassInitFunc) sp_feSpecularLighting_class_init,
            NULL, NULL,
            sizeof(SPFeSpecularLighting),
            16,
            (GInstanceInitFunc) sp_feSpecularLighting_init,
            NULL,    /* value_table */
        };
        feSpecularLighting_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeSpecularLighting", &feSpecularLighting_info, (GTypeFlags)0);
    }
    return feSpecularLighting_type;
}

static void
sp_feSpecularLighting_class_init(SPFeSpecularLightingClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;
    SPFilterPrimitiveClass *sp_primitive_class = (SPFilterPrimitiveClass *)klass;
    feSpecularLighting_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feSpecularLighting_build;
    sp_object_class->release = sp_feSpecularLighting_release;
    sp_object_class->write = sp_feSpecularLighting_write;
    sp_object_class->set = sp_feSpecularLighting_set;
    sp_object_class->update = sp_feSpecularLighting_update;
    sp_object_class->child_added = sp_feSpecularLighting_child_added;
    sp_object_class->remove_child = sp_feSpecularLighting_remove_child;
    sp_object_class->order_changed = sp_feSpecularLighting_order_changed;

    sp_primitive_class->build_renderer = sp_feSpecularLighting_build_renderer;
}

static void
sp_feSpecularLighting_init(SPFeSpecularLighting *feSpecularLighting)
{
    feSpecularLighting->surfaceScale = 1;
    feSpecularLighting->specularConstant = 1;
    feSpecularLighting->specularExponent = 1;
    feSpecularLighting->lighting_color = 0xffffffff;
    //TODO kernelUnit
    feSpecularLighting->renderer = NULL;
    
    feSpecularLighting->surfaceScale_set = FALSE;
    feSpecularLighting->specularConstant_set = FALSE;
    feSpecularLighting->specularExponent_set = FALSE;
    feSpecularLighting->lighting_color_set = FALSE;
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeSpecularLighting variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feSpecularLighting_build(SPObject *object, Document *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feSpecularLighting_parent_class)->build) {
        ((SPObjectClass *) feSpecularLighting_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
    sp_object_read_attr(object, "surfaceScale");
    sp_object_read_attr(object, "specularConstant");
    sp_object_read_attr(object, "specularExponent");
    sp_object_read_attr(object, "kernelUnitLength");
    sp_object_read_attr(object, "lighting-color");
    
}

/**
 * Drops any allocated memory.
 */
static void
sp_feSpecularLighting_release(SPObject *object)
{
    if (((SPObjectClass *) feSpecularLighting_parent_class)->release)
        ((SPObjectClass *) feSpecularLighting_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPFeSpecularLighting.
 */
static void
sp_feSpecularLighting_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeSpecularLighting *feSpecularLighting = SP_FESPECULARLIGHTING(object);
    gchar const *cend_ptr = NULL;
    gchar *end_ptr = NULL;
    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
//TODO test forbidden values
        case SP_ATTR_SURFACESCALE:
            end_ptr = NULL;
            if (value) {
                feSpecularLighting->surfaceScale = g_ascii_strtod(value, &end_ptr);
                if (end_ptr) {
                    feSpecularLighting->surfaceScale_set = TRUE;
                } else {
                    g_warning("feSpecularLighting: surfaceScale should be a number ... defaulting to 1");
                }

            }
            //if the attribute is not set or has an unreadable value
            if (!value || !end_ptr) {
                feSpecularLighting->surfaceScale = 1;
                feSpecularLighting->surfaceScale_set = FALSE;
            }
            if (feSpecularLighting->renderer) {
                feSpecularLighting->renderer->surfaceScale = feSpecularLighting->surfaceScale;
            }
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_SPECULARCONSTANT:
            end_ptr = NULL;
            if (value) {
                feSpecularLighting->specularConstant = g_ascii_strtod(value, &end_ptr);
                if (end_ptr && feSpecularLighting->specularConstant >= 0) {
                    feSpecularLighting->specularConstant_set = TRUE;
                } else {
                    end_ptr = NULL;
                    g_warning("feSpecularLighting: specularConstant should be a positive number ... defaulting to 1");
                }
            }
            if (!value || !end_ptr) {
                feSpecularLighting->specularConstant = 1;
                feSpecularLighting->specularConstant_set = FALSE;
            }
            if (feSpecularLighting->renderer) {
                feSpecularLighting->renderer->specularConstant = feSpecularLighting->specularConstant;
            }
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_SPECULAREXPONENT:
            end_ptr = NULL;
            if (value) {
                feSpecularLighting->specularExponent = g_ascii_strtod(value, &end_ptr);
                if (feSpecularLighting->specularExponent >= 1 && feSpecularLighting->specularExponent <= 128) {
                    feSpecularLighting->specularExponent_set = TRUE;
                } else {
                    end_ptr = NULL;
                    g_warning("feSpecularLighting: specularExponent should be a number in range [1, 128] ... defaulting to 1");
                }
            } 
            if (!value || !end_ptr) {
                feSpecularLighting->specularExponent = 1;
                feSpecularLighting->specularExponent_set = FALSE;
            }
            if (feSpecularLighting->renderer) {
                feSpecularLighting->renderer->specularExponent = feSpecularLighting->specularExponent;
            }
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_KERNELUNITLENGTH:
            //TODO kernelUnit
            //feSpecularLighting->kernelUnitLength.set(value);
            /*TODOif (feSpecularLighting->renderer) {
                feSpecularLighting->renderer->surfaceScale = feSpecularLighting->renderer;
            }
            */
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_PROP_LIGHTING_COLOR:
            cend_ptr = NULL;
            feSpecularLighting->lighting_color = sp_svg_read_color(value, &cend_ptr, 0xffffffff);
            //if a value was read
            if (cend_ptr) {
                feSpecularLighting->lighting_color_set = TRUE;
            } else {
                //lighting_color already contains the default value
                feSpecularLighting->lighting_color_set = FALSE;
            }
            if (feSpecularLighting->renderer) {
                feSpecularLighting->renderer->lighting_color = feSpecularLighting->lighting_color;
            }
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) feSpecularLighting_parent_class)->set)
                ((SPObjectClass *) feSpecularLighting_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feSpecularLighting_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG)) {
        sp_object_read_attr(object, "surfaceScale");
        sp_object_read_attr(object, "specularConstant");
        sp_object_read_attr(object, "specularExponent");
        sp_object_read_attr(object, "kernelUnitLength");
        sp_object_read_attr(object, "lighting-color");
    }

    if (((SPObjectClass *) feSpecularLighting_parent_class)->update) {
        ((SPObjectClass *) feSpecularLighting_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feSpecularLighting_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    SPFeSpecularLighting *fespecularlighting = SP_FESPECULARLIGHTING(object);
    
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values _and children_ into it */
    if (!repr) {
        repr = SP_OBJECT_REPR(object)->duplicate(doc);
        //repr = doc->createElement("svg:feSpecularLighting");
    }

    if (fespecularlighting->surfaceScale_set)
        sp_repr_set_css_double(repr, "surfaceScale", fespecularlighting->surfaceScale);
    if (fespecularlighting->specularConstant_set)
        sp_repr_set_css_double(repr, "specularConstant", fespecularlighting->specularConstant);
    if (fespecularlighting->specularExponent_set)
        sp_repr_set_css_double(repr, "specularExponent", fespecularlighting->specularExponent);
   /*TODO kernelUnits */ 
    if (fespecularlighting->lighting_color_set) {
        gchar c[64];
        sp_svg_write_color(c, sizeof(c), fespecularlighting->lighting_color);
        repr->setAttribute("lighting-color", c);
    }
    if (((SPObjectClass *) feSpecularLighting_parent_class)->write) {
        ((SPObjectClass *) feSpecularLighting_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

/**
 * Callback for child_added event.
 */
static void
sp_feSpecularLighting_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    SPFeSpecularLighting *f = SP_FESPECULARLIGHTING(object);

    if (((SPObjectClass *) feSpecularLighting_parent_class)->child_added)
        (* ((SPObjectClass *) feSpecularLighting_parent_class)->child_added)(object, child, ref);

    sp_feSpecularLighting_children_modified(f);
    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}
            

/**
 * Callback for remove_child event.
 */
static void
sp_feSpecularLighting_remove_child(SPObject *object, Inkscape::XML::Node *child)
{   
    SPFeSpecularLighting *f = SP_FESPECULARLIGHTING(object);

    if (((SPObjectClass *) feSpecularLighting_parent_class)->remove_child)
        (* ((SPObjectClass *) feSpecularLighting_parent_class)->remove_child)(object, child);   

    sp_feSpecularLighting_children_modified(f);
    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_feSpecularLighting_order_changed (SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *old_ref, Inkscape::XML::Node *new_ref)
{
    SPFeSpecularLighting *f = SP_FESPECULARLIGHTING(object);
    if (((SPObjectClass *) (feSpecularLighting_parent_class))->order_changed)
        (* ((SPObjectClass *) (feSpecularLighting_parent_class))->order_changed) (object, child, old_ref, new_ref);

    sp_feSpecularLighting_children_modified(f);
    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void sp_feSpecularLighting_children_modified(SPFeSpecularLighting *sp_specularlighting)
{
   if (sp_specularlighting->renderer) {
        sp_specularlighting->renderer->light_type = Inkscape::Filters::NO_LIGHT;
        if (SP_IS_FEDISTANTLIGHT(sp_specularlighting->children)) {
            sp_specularlighting->renderer->light_type = Inkscape::Filters::DISTANT_LIGHT;
            sp_specularlighting->renderer->light.distant = SP_FEDISTANTLIGHT(sp_specularlighting->children);
        }
        if (SP_IS_FEPOINTLIGHT(sp_specularlighting->children)) {
            sp_specularlighting->renderer->light_type = Inkscape::Filters::POINT_LIGHT;
            sp_specularlighting->renderer->light.point = SP_FEPOINTLIGHT(sp_specularlighting->children);
        }
        if (SP_IS_FESPOTLIGHT(sp_specularlighting->children)) {
            sp_specularlighting->renderer->light_type = Inkscape::Filters::SPOT_LIGHT;
            sp_specularlighting->renderer->light.spot = SP_FESPOTLIGHT(sp_specularlighting->children);
        }
   }
}

static void sp_feSpecularLighting_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeSpecularLighting *sp_specularlighting = SP_FESPECULARLIGHTING(primitive);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_SPECULARLIGHTING);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterSpecularLighting *nr_specularlighting = dynamic_cast<Inkscape::Filters::FilterSpecularLighting*>(nr_primitive);
    g_assert(nr_specularlighting != NULL);

    sp_specularlighting->renderer = nr_specularlighting;
    sp_filter_primitive_renderer_common(primitive, nr_primitive);

    nr_specularlighting->specularConstant = sp_specularlighting->specularConstant;
    nr_specularlighting->specularExponent = sp_specularlighting->specularExponent;
    nr_specularlighting->surfaceScale = sp_specularlighting->surfaceScale;
    nr_specularlighting->lighting_color = sp_specularlighting->lighting_color;
    //We assume there is at most one child
    nr_specularlighting->light_type = Inkscape::Filters::NO_LIGHT;
    if (SP_IS_FEDISTANTLIGHT(primitive->children)) {
        nr_specularlighting->light_type = Inkscape::Filters::DISTANT_LIGHT;
        nr_specularlighting->light.distant = SP_FEDISTANTLIGHT(primitive->children);
    }
    if (SP_IS_FEPOINTLIGHT(primitive->children)) {
        nr_specularlighting->light_type = Inkscape::Filters::POINT_LIGHT;
        nr_specularlighting->light.point = SP_FEPOINTLIGHT(primitive->children);
    }
    if (SP_IS_FESPOTLIGHT(primitive->children)) {
        nr_specularlighting->light_type = Inkscape::Filters::SPOT_LIGHT;
        nr_specularlighting->light.spot = SP_FESPOTLIGHT(primitive->children);
    }
        
    //nr_offset->set_dx(sp_offset->dx);
    //nr_offset->set_dy(sp_offset->dy);
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
