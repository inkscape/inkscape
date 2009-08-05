#define __SP_GAUSSIANBLUR_CPP__

/** \file
 * SVG <gaussianBlur> implementation.
 *
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006,2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "attributes.h"
#include "svg/svg.h"
#include "sp-gaussian-blur.h"
#include "xml/repr.h"

#include "display/nr-filter.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-gaussian.h"
#include "display/nr-filter-types.h"

//#define SP_MACROS_SILENT
//#include "macros.h"

/* GaussianBlur base class */

static void sp_gaussianBlur_class_init(SPGaussianBlurClass *klass);
static void sp_gaussianBlur_init(SPGaussianBlur *gaussianBlur);

static void sp_gaussianBlur_build(SPObject *object, Document *document, Inkscape::XML::Node *repr);
static void sp_gaussianBlur_release(SPObject *object);
static void sp_gaussianBlur_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_gaussianBlur_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_gaussianBlur_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_gaussianBlur_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter);

static SPFilterPrimitiveClass *gaussianBlur_parent_class;

GType
sp_gaussianBlur_get_type()
{
    static GType gaussianBlur_type = 0;

    if (!gaussianBlur_type) {
        GTypeInfo gaussianBlur_info = {
            sizeof(SPGaussianBlurClass),
            NULL, NULL,
            (GClassInitFunc) sp_gaussianBlur_class_init,
            NULL, NULL,
            sizeof(SPGaussianBlur),
            16,
            (GInstanceInitFunc) sp_gaussianBlur_init,
            NULL,    /* value_table */
        };
        gaussianBlur_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPGaussianBlur", &gaussianBlur_info, (GTypeFlags)0);
    }
    return gaussianBlur_type;
}

static void
sp_gaussianBlur_class_init(SPGaussianBlurClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;
    SPFilterPrimitiveClass *sp_primitive_class = (SPFilterPrimitiveClass *)klass;

    gaussianBlur_parent_class = (SPFilterPrimitiveClass *)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_gaussianBlur_build;
    sp_object_class->release = sp_gaussianBlur_release;
    sp_object_class->write = sp_gaussianBlur_write;
    sp_object_class->set = sp_gaussianBlur_set;
    sp_object_class->update = sp_gaussianBlur_update;

    sp_primitive_class->build_renderer = sp_gaussianBlur_build_renderer;
}

static void
sp_gaussianBlur_init(SPGaussianBlur */*gaussianBlur*/)
{
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPGaussianBlur variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_gaussianBlur_build(SPObject *object, Document *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) gaussianBlur_parent_class)->build) {
        ((SPObjectClass *) gaussianBlur_parent_class)->build(object, document, repr);
    }

    sp_object_read_attr(object, "stdDeviation");

}

/**
 * Drops any allocated memory.
 */
static void
sp_gaussianBlur_release(SPObject *object)
{

    if (((SPObjectClass *) gaussianBlur_parent_class)->release)
        ((SPObjectClass *) gaussianBlur_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPGaussianBlur.
 */
static void
sp_gaussianBlur_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPGaussianBlur *gaussianBlur = SP_GAUSSIANBLUR(object);

    switch(key) {
        case SP_ATTR_STDDEVIATION:
            gaussianBlur->stdDeviation.set(value);
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) gaussianBlur_parent_class)->set)
                ((SPObjectClass *) gaussianBlur_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_gaussianBlur_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        sp_object_read_attr(object, "stdDeviation");
    }

    if (((SPObjectClass *) gaussianBlur_parent_class)->update) {
        ((SPObjectClass *) gaussianBlur_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_gaussianBlur_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = SP_OBJECT_REPR(object)->duplicate(doc);
    }

    if (((SPObjectClass *) gaussianBlur_parent_class)->write) {
        ((SPObjectClass *) gaussianBlur_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}


void  sp_gaussianBlur_setDeviation(SPGaussianBlur *blur, float num)
{
    blur->stdDeviation.setNumber(num);
}
void  sp_gaussianBlur_setDeviation(SPGaussianBlur *blur, float num, float optnum)
{
    blur->stdDeviation.setNumber(num);
    blur->stdDeviation.setOptNumber(optnum);
}

static void sp_gaussianBlur_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter) {
    SPGaussianBlur *sp_blur = SP_GAUSSIANBLUR(primitive);

    int handle = filter->add_primitive(Inkscape::Filters::NR_FILTER_GAUSSIANBLUR);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(handle);
    Inkscape::Filters::FilterGaussian *nr_blur = dynamic_cast<Inkscape::Filters::FilterGaussian*>(nr_primitive);

    sp_filter_primitive_renderer_common(primitive, nr_primitive);

    gfloat num = sp_blur->stdDeviation.getNumber();
    if (num >= 0.0) {
        gfloat optnum = sp_blur->stdDeviation.getOptNumber();
        if(optnum >= 0.0)
            nr_blur->set_deviation((double) num, (double) optnum);
        else
            nr_blur->set_deviation((double) num);
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
