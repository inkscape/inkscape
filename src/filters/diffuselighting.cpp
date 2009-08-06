#define __SP_FEDIFFUSELIGHTING_CPP__

/** \file
 * SVG <feDiffuseLighting> implementation.
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
#include "diffuselighting.h"
#include "xml/repr.h"
#include "display/nr-filter-diffuselighting.h"

/* FeDiffuseLighting base class */

static void sp_feDiffuseLighting_class_init(SPFeDiffuseLightingClass *klass);
static void sp_feDiffuseLighting_init(SPFeDiffuseLighting *feDiffuseLighting);

static void sp_feDiffuseLighting_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feDiffuseLighting_release(SPObject *object);
static void sp_feDiffuseLighting_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feDiffuseLighting_update(SPObject *object, SPCtx *ctx, guint flags);
//we assume that svg:feDiffuseLighting can have any number of children
//only the first one is considered as the light source of the filter
//TODO is that right?
//if not modify child_added and remove_child to raise errors
static void sp_feDiffuseLighting_child_added(SPObject *object,
                                    Inkscape::XML::Node *child,
                                    Inkscape::XML::Node *ref);
static void sp_feDiffuseLighting_remove_child(SPObject *object, Inkscape::XML::Node *child);
static void sp_feDiffuseLighting_order_changed(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *old_ref, Inkscape::XML::Node *new_ref);
static Inkscape::XML::Node *sp_feDiffuseLighting_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_feDiffuseLighting_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter);
static void sp_feDiffuseLighting_children_modified(SPFeDiffuseLighting *sp_diffuselighting);

static SPFilterPrimitiveClass *feDiffuseLighting_parent_class;

GType
sp_feDiffuseLighting_get_type()
{
    static GType feDiffuseLighting_type = 0;

    if (!feDiffuseLighting_type) {
        GTypeInfo feDiffuseLighting_info = {
            sizeof(SPFeDiffuseLightingClass),
            NULL, NULL,
            (GClassInitFunc) sp_feDiffuseLighting_class_init,
            NULL, NULL,
            sizeof(SPFeDiffuseLighting),
            16,
            (GInstanceInitFunc) sp_feDiffuseLighting_init,
            NULL,    /* value_table */
        };
        feDiffuseLighting_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeDiffuseLighting", &feDiffuseLighting_info, (GTypeFlags)0);
    }
    return feDiffuseLighting_type;
}

static void
sp_feDiffuseLighting_class_init(SPFeDiffuseLightingClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;
    SPFilterPrimitiveClass *sp_primitive_class = (SPFilterPrimitiveClass *)klass;
    feDiffuseLighting_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feDiffuseLighting_build;
    sp_object_class->release = sp_feDiffuseLighting_release;
    sp_object_class->write = sp_feDiffuseLighting_write;
    sp_object_class->set = sp_feDiffuseLighting_set;
    sp_object_class->update = sp_feDiffuseLighting_update;
    sp_object_class->child_added = sp_feDiffuseLighting_child_added;
    sp_object_class->remove_child = sp_feDiffuseLighting_remove_child;
    sp_object_class->order_changed = sp_feDiffuseLighting_order_changed;

    sp_primitive_class->build_renderer = sp_feDiffuseLighting_build_renderer;
}

static void
sp_feDiffuseLighting_init(SPFeDiffuseLighting *feDiffuseLighting)
{
    feDiffuseLighting->surfaceScale = 1;
    feDiffuseLighting->diffuseConstant = 1;
    feDiffuseLighting->lighting_color = 0xffffffff;
    //TODO kernelUnit
    feDiffuseLighting->renderer = NULL;

    feDiffuseLighting->surfaceScale_set = FALSE;
    feDiffuseLighting->diffuseConstant_set = FALSE;
    feDiffuseLighting->lighting_color_set = FALSE;
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeDiffuseLighting variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feDiffuseLighting_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feDiffuseLighting_parent_class)->build) {
        ((SPObjectClass *) feDiffuseLighting_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
    sp_object_read_attr(object, "surfaceScale");
    sp_object_read_attr(object, "diffuseConstant");
    sp_object_read_attr(object, "kernelUnitLength");
    sp_object_read_attr(object, "lighting-color");
    
}

/**
 * Drops any allocated memory.
 */
static void
sp_feDiffuseLighting_release(SPObject *object)
{
    if (((SPObjectClass *) feDiffuseLighting_parent_class)->release)
        ((SPObjectClass *) feDiffuseLighting_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPFeDiffuseLighting.
 */
static void
sp_feDiffuseLighting_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeDiffuseLighting *feDiffuseLighting = SP_FEDIFFUSELIGHTING(object);
    gchar const *cend_ptr = NULL;
    gchar *end_ptr = NULL;
    
    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
//TODO test forbidden values
        case SP_ATTR_SURFACESCALE:
            end_ptr = NULL;
            if (value) {
                feDiffuseLighting->surfaceScale = g_ascii_strtod(value, &end_ptr);
                if (end_ptr) {
                    feDiffuseLighting->surfaceScale_set = TRUE;
                }
            } 
            if (!value || !end_ptr) {
                feDiffuseLighting->surfaceScale = 1;
                feDiffuseLighting->surfaceScale_set = FALSE;
            }
            if (feDiffuseLighting->renderer) {
                feDiffuseLighting->renderer->surfaceScale = feDiffuseLighting->surfaceScale;
            }
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_DIFFUSECONSTANT:
            end_ptr = NULL;
            if (value) {
                feDiffuseLighting->diffuseConstant = g_ascii_strtod(value, &end_ptr);
                if (end_ptr && feDiffuseLighting->diffuseConstant >= 0) {
                    feDiffuseLighting->diffuseConstant_set = TRUE;
                } else {
                    end_ptr = NULL;
                    g_warning("feDiffuseLighting: diffuseConstant should be a positive number ... defaulting to 1");
                }
            } 
            if (!value || !end_ptr) {
                feDiffuseLighting->diffuseConstant = 1;
                feDiffuseLighting->diffuseConstant_set = FALSE;
            }
            if (feDiffuseLighting->renderer) {
                feDiffuseLighting->renderer->diffuseConstant = feDiffuseLighting->diffuseConstant;
    }
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_KERNELUNITLENGTH:
            //TODO kernelUnit
            //feDiffuseLighting->kernelUnitLength.set(value);
            /*TODOif (feDiffuseLighting->renderer) {
                feDiffuseLighting->renderer->surfaceScale = feDiffuseLighting->renderer;
            }
            */
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_PROP_LIGHTING_COLOR:
            cend_ptr = NULL;
            feDiffuseLighting->lighting_color = sp_svg_read_color(value, &cend_ptr, 0xffffffff);
            //if a value was read
            if (cend_ptr) {
                feDiffuseLighting->lighting_color_set = TRUE; 
            } else {
                //lighting_color already contains the default value
                feDiffuseLighting->lighting_color_set = FALSE; 
            }
            if (feDiffuseLighting->renderer) {
                feDiffuseLighting->renderer->lighting_color = feDiffuseLighting->lighting_color;
            }
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) feDiffuseLighting_parent_class)->set)
                ((SPObjectClass *) feDiffuseLighting_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feDiffuseLighting_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG)) {
        sp_object_read_attr(object, "surfaceScale");
        sp_object_read_attr(object, "diffuseConstant");
        sp_object_read_attr(object, "kernelUnit");
        sp_object_read_attr(object, "lighting-color");
    }

    if (((SPObjectClass *) feDiffuseLighting_parent_class)->update) {
        ((SPObjectClass *) feDiffuseLighting_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feDiffuseLighting_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    SPFeDiffuseLighting *fediffuselighting = SP_FEDIFFUSELIGHTING(object);
    
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values _and children_ into it */
    if (!repr) {
        repr = SP_OBJECT_REPR(object)->duplicate(doc);
        //repr = doc->createElement("svg:feDiffuseLighting");
    }
    
    if (fediffuselighting->surfaceScale_set)
        sp_repr_set_css_double(repr, "surfaceScale", fediffuselighting->surfaceScale);
    else
        repr->setAttribute("surfaceScale", NULL);
    if (fediffuselighting->diffuseConstant_set)
        sp_repr_set_css_double(repr, "diffuseConstant", fediffuselighting->diffuseConstant);
    else
        repr->setAttribute("diffuseConstant", NULL);
   /*TODO kernelUnits */ 
    if (fediffuselighting->lighting_color_set) {
        gchar c[64];
        sp_svg_write_color(c, sizeof(c), fediffuselighting->lighting_color);
        repr->setAttribute("lighting-color", c);
    } else
        repr->setAttribute("lighting-color", NULL);
        
    if (((SPObjectClass *) feDiffuseLighting_parent_class)->write) {
        ((SPObjectClass *) feDiffuseLighting_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

/**
 * Callback for child_added event.
 */
static void
sp_feDiffuseLighting_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    SPFeDiffuseLighting *f = SP_FEDIFFUSELIGHTING(object);

    if (((SPObjectClass *) feDiffuseLighting_parent_class)->child_added)
        (* ((SPObjectClass *) feDiffuseLighting_parent_class)->child_added)(object, child, ref);

    sp_feDiffuseLighting_children_modified(f);
    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}
            

/**
 * Callback for remove_child event.
 */
static void
sp_feDiffuseLighting_remove_child(SPObject *object, Inkscape::XML::Node *child)
{   
    SPFeDiffuseLighting *f = SP_FEDIFFUSELIGHTING(object);

    if (((SPObjectClass *) feDiffuseLighting_parent_class)->remove_child)
        (* ((SPObjectClass *) feDiffuseLighting_parent_class)->remove_child)(object, child);   

    sp_feDiffuseLighting_children_modified(f);
    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_feDiffuseLighting_order_changed (SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *old_ref, Inkscape::XML::Node *new_ref)
{
    SPFeDiffuseLighting *f = SP_FEDIFFUSELIGHTING(object);
    if (((SPObjectClass *) (feDiffuseLighting_parent_class))->order_changed)
        (* ((SPObjectClass *) (feDiffuseLighting_parent_class))->order_changed) (object, child, old_ref, new_ref);

    sp_feDiffuseLighting_children_modified(f);
    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void sp_feDiffuseLighting_children_modified(SPFeDiffuseLighting *sp_diffuselighting)
{
   if (sp_diffuselighting->renderer) {
        sp_diffuselighting->renderer->light_type = Inkscape::Filters::NO_LIGHT;
        if (SP_IS_FEDISTANTLIGHT(sp_diffuselighting->children)) {
            sp_diffuselighting->renderer->light_type = Inkscape::Filters::DISTANT_LIGHT;
            sp_diffuselighting->renderer->light.distant = SP_FEDISTANTLIGHT(sp_diffuselighting->children);
        }
        if (SP_IS_FEPOINTLIGHT(sp_diffuselighting->children)) {
            sp_diffuselighting->renderer->light_type = Inkscape::Filters::POINT_LIGHT;
            sp_diffuselighting->renderer->light.point = SP_FEPOINTLIGHT(sp_diffuselighting->children);
        }
        if (SP_IS_FESPOTLIGHT(sp_diffuselighting->children)) {
            sp_diffuselighting->renderer->light_type = Inkscape::Filters::SPOT_LIGHT;
            sp_diffuselighting->renderer->light.spot = SP_FESPOTLIGHT(sp_diffuselighting->children);
        }
   }
}

static void sp_feDiffuseLighting_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeDiffuseLighting *sp_diffuselighting = SP_FEDIFFUSELIGHTING(primitive);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_DIFFUSELIGHTING);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterDiffuseLighting *nr_diffuselighting = dynamic_cast<Inkscape::Filters::FilterDiffuseLighting*>(nr_primitive);
    g_assert(nr_diffuselighting != NULL);

    sp_diffuselighting->renderer = nr_diffuselighting;
    sp_filter_primitive_renderer_common(primitive, nr_primitive);

    nr_diffuselighting->diffuseConstant = sp_diffuselighting->diffuseConstant;
    nr_diffuselighting->surfaceScale = sp_diffuselighting->surfaceScale;
    nr_diffuselighting->lighting_color = sp_diffuselighting->lighting_color;
    //We assume there is at most one child
    nr_diffuselighting->light_type = Inkscape::Filters::NO_LIGHT;
    if (SP_IS_FEDISTANTLIGHT(primitive->children)) {
        nr_diffuselighting->light_type = Inkscape::Filters::DISTANT_LIGHT;
        nr_diffuselighting->light.distant = SP_FEDISTANTLIGHT(primitive->children);
    }
    if (SP_IS_FEPOINTLIGHT(primitive->children)) {
        nr_diffuselighting->light_type = Inkscape::Filters::POINT_LIGHT;
        nr_diffuselighting->light.point = SP_FEPOINTLIGHT(primitive->children);
    }
    if (SP_IS_FESPOTLIGHT(primitive->children)) {
        nr_diffuselighting->light_type = Inkscape::Filters::SPOT_LIGHT;
        nr_diffuselighting->light.spot = SP_FESPOTLIGHT(primitive->children);
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
