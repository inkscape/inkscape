#define __SP_FEBLEND_CPP__

/** \file
 * SVG <feBlend> implementation.
 *
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006,2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#include "attributes.h"
#include "svg/svg.h"
#include "blend.h"
#include "xml/repr.h"

#include "display/nr-filter.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-blend.h"
#include "display/nr-filter-types.h"

/* FeBlend base class */

static void sp_feBlend_class_init(SPFeBlendClass *klass);
static void sp_feBlend_init(SPFeBlend *feBlend);

static void sp_feBlend_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feBlend_release(SPObject *object);
static void sp_feBlend_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feBlend_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feBlend_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_feBlend_build_renderer(SPFilterPrimitive *sp_prim, Inkscape::Filters::Filter *filter);

static SPFilterPrimitiveClass *feBlend_parent_class;

GType
sp_feBlend_get_type()
{
    static GType feBlend_type = 0;

    if (!feBlend_type) {
        GTypeInfo feBlend_info = {
            sizeof(SPFeBlendClass),
            NULL, NULL,
            (GClassInitFunc) sp_feBlend_class_init,
            NULL, NULL,
            sizeof(SPFeBlend),
            16,
            (GInstanceInitFunc) sp_feBlend_init,
            NULL,    /* value_table */
        };
        feBlend_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeBlend", &feBlend_info, (GTypeFlags)0);
    }
    return feBlend_type;
}

static void
sp_feBlend_class_init(SPFeBlendClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;
    SPFilterPrimitiveClass *sp_primitive_class = (SPFilterPrimitiveClass *)klass;

    feBlend_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feBlend_build;
    sp_object_class->release = sp_feBlend_release;
    sp_object_class->write = sp_feBlend_write;
    sp_object_class->set = sp_feBlend_set;
    sp_object_class->update = sp_feBlend_update;

    sp_primitive_class->build_renderer = sp_feBlend_build_renderer;
}

static void
sp_feBlend_init(SPFeBlend *feBlend)
{
    feBlend->in2 = Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeBlend variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feBlend_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    SPFeBlend *blend = SP_FEBLEND(object);

    if (((SPObjectClass *) feBlend_parent_class)->build) {
        ((SPObjectClass *) feBlend_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
    sp_object_read_attr(object, "mode");
    sp_object_read_attr(object, "in2");

    /* Unlike normal in, in2 is required attribute. Make sure, we can call
     * it by some name. */
    if (blend->in2 == Inkscape::Filters::NR_FILTER_SLOT_NOT_SET ||
        blend->in2 == Inkscape::Filters::NR_FILTER_UNNAMED_SLOT)
    {
        SPFilter *parent = SP_FILTER(object->parent);
        blend->in2 = sp_filter_primitive_name_previous_out(blend);
        repr->setAttribute("in2", sp_filter_name_for_image(parent, blend->in2));
    }
}

/**
 * Drops any allocated memory.
 */
static void
sp_feBlend_release(SPObject *object)
{
    if (((SPObjectClass *) feBlend_parent_class)->release)
        ((SPObjectClass *) feBlend_parent_class)->release(object);
}

static Inkscape::Filters::FilterBlendMode sp_feBlend_readmode(gchar const *value)
{
    if (!value) return Inkscape::Filters::BLEND_NORMAL;
    switch (value[0]) {
        case 'n':
            if (strncmp(value, "normal", 6) == 0)
                return Inkscape::Filters::BLEND_NORMAL;
            break;
        case 'm':
            if (strncmp(value, "multiply", 8) == 0)
                return Inkscape::Filters::BLEND_MULTIPLY;
            break;
        case 's':
            if (strncmp(value, "screen", 6) == 0)
                return Inkscape::Filters::BLEND_SCREEN;
            break;
        case 'd':
            if (strncmp(value, "darken", 6) == 0)
                return Inkscape::Filters::BLEND_DARKEN;
            break;
        case 'l':
            if (strncmp(value, "lighten", 7) == 0)
                return Inkscape::Filters::BLEND_LIGHTEN;
            break;
        default:
            // do nothing by default
            break;
    }
    return Inkscape::Filters::BLEND_NORMAL;
}

/**
 * Sets a specific value in the SPFeBlend.
 */
static void
sp_feBlend_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeBlend *feBlend = SP_FEBLEND(object);
    (void)feBlend;

    Inkscape::Filters::FilterBlendMode mode;
    int input;
    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_ATTR_MODE:
            mode = sp_feBlend_readmode(value);
            if (mode != feBlend->blend_mode) {
                feBlend->blend_mode = mode;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_IN2:
            input = sp_filter_primitive_read_in(feBlend, value);
            if (input != feBlend->in2) {
                feBlend->in2 = input;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        default:
            if (((SPObjectClass *) feBlend_parent_class)->set)
                ((SPObjectClass *) feBlend_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feBlend_update(SPObject *object, SPCtx *ctx, guint flags)
{
    SPFeBlend *blend = SP_FEBLEND(object);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        sp_object_read_attr(object, "mode");
        sp_object_read_attr(object, "in2");
    }

    /* Unlike normal in, in2 is required attribute. Make sure, we can call
     * it by some name. */
    if (blend->in2 == Inkscape::Filters::NR_FILTER_SLOT_NOT_SET ||
        blend->in2 == Inkscape::Filters::NR_FILTER_UNNAMED_SLOT)
    {
        SPFilter *parent = SP_FILTER(object->parent);
        blend->in2 = sp_filter_primitive_name_previous_out(blend);
        object->repr->setAttribute("in2", sp_filter_name_for_image(parent, blend->in2));
    }

    if (((SPObjectClass *) feBlend_parent_class)->update) {
        ((SPObjectClass *) feBlend_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feBlend_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    SPFeBlend *blend = SP_FEBLEND(object);
    SPFilter *parent = SP_FILTER(object->parent);

    if (!repr) {
        repr = doc->createElement("svg:feBlend");
    }

    gchar const *out_name = sp_filter_name_for_image(parent, blend->in2);
    if (out_name) {
        repr->setAttribute("in2", out_name);
    } else {
        SPObject *i = parent->children;
        while (i && i->next != object) i = i->next;
        SPFilterPrimitive *i_prim = SP_FILTER_PRIMITIVE(i);
        out_name = sp_filter_name_for_image(parent, i_prim->image_out);
        repr->setAttribute("in2", out_name);
        if (!out_name) {
            g_warning("Unable to set in2 for feBlend");
        }
    }

    char const *mode;
    switch(blend->blend_mode) {
        case Inkscape::Filters::BLEND_NORMAL:
            mode = "normal"; break;
        case Inkscape::Filters::BLEND_MULTIPLY:
            mode = "multiply"; break;
        case Inkscape::Filters::BLEND_SCREEN:
            mode = "screen"; break;
        case Inkscape::Filters::BLEND_DARKEN:
            mode = "darken"; break;
        case Inkscape::Filters::BLEND_LIGHTEN:
            mode = "lighten"; break;
        default:
            mode = 0;
    }
    repr->setAttribute("mode", mode);

    if (((SPObjectClass *) feBlend_parent_class)->write) {
        ((SPObjectClass *) feBlend_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

static void sp_feBlend_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeBlend *sp_blend = SP_FEBLEND(primitive);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_BLEND);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterBlend *nr_blend = dynamic_cast<Inkscape::Filters::FilterBlend*>(nr_primitive);
    g_assert(nr_blend != NULL);

    sp_filter_primitive_renderer_common(primitive, nr_primitive);

    nr_blend->set_mode(sp_blend->blend_mode);
    nr_blend->set_input(1, sp_blend->in2);
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
