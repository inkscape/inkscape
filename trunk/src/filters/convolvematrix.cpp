#define __SP_FECONVOLVEMATRIX_CPP__

/** \file
 * SVG <feConvolveMatrix> implementation.
 *
 */
/*
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *   hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#include <vector>
#include "attributes.h"
#include "svg/svg.h"
#include "convolvematrix.h"
#include "helper-fns.h"
#include "xml/repr.h"
#include "display/nr-filter-convolve-matrix.h"
#include <math.h>

/* FeConvolveMatrix base class */

static void sp_feConvolveMatrix_class_init(SPFeConvolveMatrixClass *klass);
static void sp_feConvolveMatrix_init(SPFeConvolveMatrix *feConvolveMatrix);

static void sp_feConvolveMatrix_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feConvolveMatrix_release(SPObject *object);
static void sp_feConvolveMatrix_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feConvolveMatrix_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feConvolveMatrix_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_feConvolveMatrix_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter);

static SPFilterPrimitiveClass *feConvolveMatrix_parent_class;

GType
sp_feConvolveMatrix_get_type()
{
    static GType feConvolveMatrix_type = 0;

    if (!feConvolveMatrix_type) {
        GTypeInfo feConvolveMatrix_info = {
            sizeof(SPFeConvolveMatrixClass),
            NULL, NULL,
            (GClassInitFunc) sp_feConvolveMatrix_class_init,
            NULL, NULL,
            sizeof(SPFeConvolveMatrix),
            16,
            (GInstanceInitFunc) sp_feConvolveMatrix_init,
            NULL,    /* value_table */
        };
        feConvolveMatrix_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeConvolveMatrix", &feConvolveMatrix_info, (GTypeFlags)0);
    }
    return feConvolveMatrix_type;
}

static void
sp_feConvolveMatrix_class_init(SPFeConvolveMatrixClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;
    SPFilterPrimitiveClass *sp_primitive_class = (SPFilterPrimitiveClass *)klass;

    feConvolveMatrix_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feConvolveMatrix_build;
    sp_object_class->release = sp_feConvolveMatrix_release;
    sp_object_class->write = sp_feConvolveMatrix_write;
    sp_object_class->set = sp_feConvolveMatrix_set;
    sp_object_class->update = sp_feConvolveMatrix_update;

    sp_primitive_class->build_renderer = sp_feConvolveMatrix_build_renderer;
}

static void
sp_feConvolveMatrix_init(SPFeConvolveMatrix *feConvolveMatrix)
{
    //Setting default values:
    feConvolveMatrix->order.set("3 3");
    feConvolveMatrix->targetX = 1;
    feConvolveMatrix->targetY = 1;
    feConvolveMatrix->edgeMode = Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_DUPLICATE;
    feConvolveMatrix->preserveAlpha = false;

    //some helper variables:
    feConvolveMatrix->targetXIsSet = false;
    feConvolveMatrix->targetYIsSet = false;
    feConvolveMatrix->kernelMatrixIsSet = false;
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeConvolveMatrix variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feConvolveMatrix_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) feConvolveMatrix_parent_class)->build) {
        ((SPObjectClass *) feConvolveMatrix_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
    sp_object_read_attr(object, "order");
    sp_object_read_attr(object, "kernelMatrix");
    sp_object_read_attr(object, "divisor");
    sp_object_read_attr(object, "bias");
    sp_object_read_attr(object, "targetX");
    sp_object_read_attr(object, "targetY");
    sp_object_read_attr(object, "edgeMode");
    sp_object_read_attr(object, "kernelUnitLength");
    sp_object_read_attr(object, "preserveAlpha");
}

/**
 * Drops any allocated memory.
 */
static void
sp_feConvolveMatrix_release(SPObject *object)
{
    if (((SPObjectClass *) feConvolveMatrix_parent_class)->release)
        ((SPObjectClass *) feConvolveMatrix_parent_class)->release(object);
}

static Inkscape::Filters::FilterConvolveMatrixEdgeMode sp_feConvolveMatrix_read_edgeMode(gchar const *value){
    if (!value) return Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_DUPLICATE; //duplicate is default
    switch(value[0]){
        case 'd':
            if (strncmp(value, "duplicate", 9) == 0) return Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_DUPLICATE;
            break;
        case 'w':
            if (strncmp(value, "wrap", 4) == 0) return Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_WRAP;
            break;
        case 'n':
            if (strncmp(value, "none", 4) == 0) return Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_NONE;
            break;
    }
    return Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_DUPLICATE; //duplicate is default
}

/**
 * Sets a specific value in the SPFeConvolveMatrix.
 */
static void
sp_feConvolveMatrix_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeConvolveMatrix *feConvolveMatrix = SP_FECONVOLVEMATRIX(object);
    (void)feConvolveMatrix;
    double read_num;
    int read_int;
    bool read_bool;
    Inkscape::Filters::FilterConvolveMatrixEdgeMode read_mode;
   
    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_ATTR_ORDER:
            feConvolveMatrix->order.set(value);
            //From SVG spec: If <orderY> is not provided, it defaults to <orderX>.
            if (feConvolveMatrix->order.optNumIsSet() == false)
                feConvolveMatrix->order.setOptNumber(feConvolveMatrix->order.getNumber());
            if (feConvolveMatrix->targetXIsSet == false) feConvolveMatrix->targetX = (int) floor(feConvolveMatrix->order.getNumber()/2);
            if (feConvolveMatrix->targetYIsSet == false) feConvolveMatrix->targetY = (int) floor(feConvolveMatrix->order.getOptNumber()/2);
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_KERNELMATRIX:
            if (value){
                feConvolveMatrix->kernelMatrixIsSet = true;
                feConvolveMatrix->kernelMatrix = helperfns_read_vector(value, (int) (feConvolveMatrix->order.getNumber() * feConvolveMatrix->order.getOptNumber()));
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            } else {
                g_warning("For feConvolveMatrix you MUST pass a kernelMatrix parameter!");
            }
            break;
        case SP_ATTR_DIVISOR:
            if (!value){
                read_num = 1; 
            } else {
                read_num = helperfns_read_number(value);
                if (read_num == 0) {
                    if (feConvolveMatrix->kernelMatrixIsSet){
                        g_warning("You shouldn't pass a divisor value equal to 0! Assuming the sum of all values in kernelMatrix as the default value.");
                        for (unsigned int i = 0; i< feConvolveMatrix->kernelMatrix.size(); i++)
                            read_num += feConvolveMatrix->kernelMatrix[i];
                    } else {
                        g_warning("You shouldn't pass a divisor value equal to 0! Assuming 1 as the default value.");
                        read_num = 1;
                    }
                }
            }
            if (read_num != feConvolveMatrix->divisor){
                feConvolveMatrix->divisor = read_num;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_BIAS:
            read_num = 0;
            if (value) read_num = helperfns_read_number(value);
            if (read_num != feConvolveMatrix->bias){
                feConvolveMatrix->bias = read_num;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_TARGETX:
            read_int = (int) helperfns_read_number(value);
            if (read_int < 0 || read_int > feConvolveMatrix->order.getNumber()){
                g_warning("targetX must be a value between 0 and orderX! Assuming orderX as default value.");
                read_int = (int) feConvolveMatrix->order.getNumber();
            }
            feConvolveMatrix->targetXIsSet = true;
            if (read_int != feConvolveMatrix->targetX){
                feConvolveMatrix->targetX = read_int;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_TARGETY:
            read_int = (int) helperfns_read_number(value);
            if (read_int < 0 || read_int > feConvolveMatrix->order.getOptNumber()){
                g_warning("targetY must be a value between 0 and orderY! Assuming orderY as default value.");
                read_int = (int) feConvolveMatrix->order.getOptNumber();
            }
            feConvolveMatrix->targetYIsSet = true;
            if (read_int != feConvolveMatrix->targetY){
                feConvolveMatrix->targetY = read_int;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_EDGEMODE:
            read_mode = sp_feConvolveMatrix_read_edgeMode(value);
            if (read_mode != feConvolveMatrix->edgeMode){
                feConvolveMatrix->edgeMode = read_mode;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_KERNELUNITLENGTH:
            feConvolveMatrix->kernelUnitLength.set(value);
            //From SVG spec: If the <dy> value is not specified, it defaults to the same value as <dx>.
            if (feConvolveMatrix->kernelUnitLength.optNumIsSet() == false)
                feConvolveMatrix->kernelUnitLength.setOptNumber(feConvolveMatrix->kernelUnitLength.getNumber());
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_PRESERVEALPHA:
            read_bool = helperfns_read_bool(value, false);
            if (read_bool != feConvolveMatrix->preserveAlpha){
                feConvolveMatrix->preserveAlpha = read_bool;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        default:
            if (((SPObjectClass *) feConvolveMatrix_parent_class)->set)
                ((SPObjectClass *) feConvolveMatrix_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feConvolveMatrix_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) feConvolveMatrix_parent_class)->update) {
        ((SPObjectClass *) feConvolveMatrix_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feConvolveMatrix_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
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

    if (((SPObjectClass *) feConvolveMatrix_parent_class)->write) {
        ((SPObjectClass *) feConvolveMatrix_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

static void sp_feConvolveMatrix_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeConvolveMatrix *sp_convolve = SP_FECONVOLVEMATRIX(primitive);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_CONVOLVEMATRIX);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterConvolveMatrix *nr_convolve = dynamic_cast<Inkscape::Filters::FilterConvolveMatrix*>(nr_primitive);
    g_assert(nr_convolve != NULL);

    sp_filter_primitive_renderer_common(primitive, nr_primitive);

    nr_convolve->set_targetX(sp_convolve->targetX);
    nr_convolve->set_targetY(sp_convolve->targetY);
    nr_convolve->set_orderX( (int)sp_convolve->order.getNumber() );
    nr_convolve->set_orderY( (int)sp_convolve->order.getOptNumber() );
    nr_convolve->set_kernelMatrix(sp_convolve->kernelMatrix);
    nr_convolve->set_divisor(sp_convolve->divisor);
    nr_convolve->set_bias(sp_convolve->bias);
    nr_convolve->set_preserveAlpha(sp_convolve->preserveAlpha);

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
