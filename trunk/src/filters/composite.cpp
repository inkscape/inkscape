#define __SP_FECOMPOSITE_CPP__

/** \file
 * SVG <feComposite> implementation.
 *
 */
/*
 * Authors:
 *   hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "attributes.h"
#include "svg/svg.h"
#include "composite.h"
#include "helper-fns.h"
#include "xml/repr.h"
#include "display/nr-filter-composite.h"

/* FeComposite base class */

static void sp_feComposite_class_init(SPFeCompositeClass *klass);
static void sp_feComposite_init(SPFeComposite *feComposite);

static void sp_feComposite_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feComposite_release(SPObject *object);
static void sp_feComposite_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feComposite_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feComposite_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_feComposite_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter);

static SPFilterPrimitiveClass *feComposite_parent_class;

GType
sp_feComposite_get_type()
{
    static GType feComposite_type = 0;

    if (!feComposite_type) {
        GTypeInfo feComposite_info = {
            sizeof(SPFeCompositeClass),
            NULL, NULL,
            (GClassInitFunc) sp_feComposite_class_init,
            NULL, NULL,
            sizeof(SPFeComposite),
            16,
            (GInstanceInitFunc) sp_feComposite_init,
            NULL,    /* value_table */
        };
        feComposite_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeComposite", &feComposite_info, (GTypeFlags)0);
    }
    return feComposite_type;
}

static void
sp_feComposite_class_init(SPFeCompositeClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;
    SPFilterPrimitiveClass *sp_primitive_class = (SPFilterPrimitiveClass *)klass;

    feComposite_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feComposite_build;
    sp_object_class->release = sp_feComposite_release;
    sp_object_class->write = sp_feComposite_write;
    sp_object_class->set = sp_feComposite_set;
    sp_object_class->update = sp_feComposite_update;

    sp_primitive_class->build_renderer = sp_feComposite_build_renderer;
}

static void
sp_feComposite_init(SPFeComposite *feComposite)
{
    feComposite->composite_operator = COMPOSITE_DEFAULT;
    feComposite->k1 = 0;
    feComposite->k2 = 0;
    feComposite->k3 = 0;
    feComposite->k4 = 0;
    feComposite->in2 = Inkscape::Filters::NR_FILTER_SLOT_NOT_SET;
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeComposite variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feComposite_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feComposite_parent_class)->build) {
        ((SPObjectClass *) feComposite_parent_class)->build(object, document, repr);
    }

    SPFeComposite *composite = SP_FECOMPOSITE(object);

    sp_object_read_attr(object, "operator");
    if (composite->composite_operator == COMPOSITE_ARITHMETIC) {
        sp_object_read_attr(object, "k1");
        sp_object_read_attr(object, "k2");
        sp_object_read_attr(object, "k3");
        sp_object_read_attr(object, "k4");
    }
    sp_object_read_attr(object, "in2");
}

/**
 * Drops any allocated memory.
 */
static void
sp_feComposite_release(SPObject *object)
{
    if (((SPObjectClass *) feComposite_parent_class)->release)
        ((SPObjectClass *) feComposite_parent_class)->release(object);
}

static FeCompositeOperator
sp_feComposite_read_operator(gchar const *value) {
    if (!value) return COMPOSITE_DEFAULT;

    if (strcmp(value, "over") == 0) return COMPOSITE_OVER;
    else if (strcmp(value, "in") == 0) return COMPOSITE_IN;
    else if (strcmp(value, "out") == 0) return COMPOSITE_OUT;
    else if (strcmp(value, "atop") == 0) return COMPOSITE_ATOP;
    else if (strcmp(value, "xor") == 0) return COMPOSITE_XOR;
    else if (strcmp(value, "arithmetic") == 0) return COMPOSITE_ARITHMETIC;
    return COMPOSITE_DEFAULT;
}

/**
 * Sets a specific value in the SPFeComposite.
 */
static void
sp_feComposite_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeComposite *feComposite = SP_FECOMPOSITE(object);
    (void)feComposite;

    int input;
    FeCompositeOperator op;
    double k_n;
    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_ATTR_OPERATOR:
            op = sp_feComposite_read_operator(value);
            if (op != feComposite->composite_operator) {
                feComposite->composite_operator = op;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SP_ATTR_K1:
            k_n = helperfns_read_number(value);
            if (k_n != feComposite->k1) {
                feComposite->k1 = k_n;
                if (feComposite->composite_operator == COMPOSITE_ARITHMETIC)
                    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SP_ATTR_K2:
            k_n = helperfns_read_number(value);
            if (k_n != feComposite->k2) {
                feComposite->k2 = k_n;
                if (feComposite->composite_operator == COMPOSITE_ARITHMETIC)
                    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SP_ATTR_K3:
            k_n = helperfns_read_number(value);
            if (k_n != feComposite->k3) {
                feComposite->k3 = k_n;
                if (feComposite->composite_operator == COMPOSITE_ARITHMETIC)
                    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SP_ATTR_K4:
            k_n = helperfns_read_number(value);
            if (k_n != feComposite->k4) {
                feComposite->k4 = k_n;
                if (feComposite->composite_operator == COMPOSITE_ARITHMETIC)
                    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SP_ATTR_IN2:
            input = sp_filter_primitive_read_in(feComposite, value);
            if (input != feComposite->in2) {
                feComposite->in2 = input;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        default:
            if (((SPObjectClass *) feComposite_parent_class)->set)
                ((SPObjectClass *) feComposite_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feComposite_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) feComposite_parent_class)->update) {
        ((SPObjectClass *) feComposite_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feComposite_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    // Inkscape-only object, not copied during an "plain SVG" dump:
    if (flags & SP_OBJECT_WRITE_EXT) {
        if (repr) {
            // is this sane?
            //repr->mergeFrom(SP_OBJECT_REPR(object), "id");
        } else {
            repr = SP_OBJECT_REPR(object)->duplicate(doc);
        }
    }

    if (((SPObjectClass *) feComposite_parent_class)->write) {
        ((SPObjectClass *) feComposite_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

static void sp_feComposite_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeComposite *sp_composite = SP_FECOMPOSITE(primitive);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_COMPOSITE);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterComposite *nr_composite = dynamic_cast<Inkscape::Filters::FilterComposite*>(nr_primitive);
    g_assert(nr_composite != NULL);

    sp_filter_primitive_renderer_common(primitive, nr_primitive);

    nr_composite->set_operator(sp_composite->composite_operator);
    nr_composite->set_input(1, sp_composite->in2);
    if (sp_composite->composite_operator == COMPOSITE_ARITHMETIC) {
        nr_composite->set_arithmetic(sp_composite->k1, sp_composite->k2,
                                     sp_composite->k3, sp_composite->k4);
    }
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
