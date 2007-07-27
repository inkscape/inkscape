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

#include <vector>
#include "attributes.h"
#include "svg/svg.h"
#include "sp-feconvolvematrix.h"
#include "xml/repr.h"
#include "display/nr-filter-convolve-matrix.h"

/* FeConvolveMatrix base class */

static void sp_feConvolveMatrix_class_init(SPFeConvolveMatrixClass *klass);
static void sp_feConvolveMatrix_init(SPFeConvolveMatrix *feConvolveMatrix);

static void sp_feConvolveMatrix_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feConvolveMatrix_release(SPObject *object);
static void sp_feConvolveMatrix_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feConvolveMatrix_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feConvolveMatrix_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);
static void sp_feConvolveMatrix_build_renderer(SPFilterPrimitive *primitive, NR::Filter *filter);

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
{}

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


static std::vector<gdouble> read_kernel_matrix(const gchar* value, int size){
        std::vector<gdouble> v(size, (gdouble) 0);
        int i;
        gchar** values = g_strsplit(value , " ", size);
        for (i=0;i<size;i++)
                v[i] = g_ascii_strtod(values[i], NULL);
        return v;
}

static double
sp_feConvolveMatrix_read_number(gchar const *value) {
    if (!value) return 0;
    char *end;
    double ret = g_ascii_strtod(value, &end);
    if (*end) {
        g_warning("Unable to convert \"%s\" to number", value);
        // We could leave this out, too. If strtod can't convert
        // anything, it will return zero.
        ret = 0;
    }
    return ret;
}

static int sp_feConvolveMatrix_read_edgeMode(gchar const *value){
    if (!value) return 0; //duplicate is default
    switch(value[0]){
        case 'd':
            if (strncmp(value, "duplicate", 9) == 0) return 0;
            break;
        case 'w':
            if (strncmp(value, "wrap", 4) == 0) return 1;
            break;
        case 'n':
            if (strncmp(value, "none", 4) == 0) return 2;
            break;
    }
    return 0; //duplicate is default
}


static bool sp_feConvolveMatrix_read_bool(gchar const *value){
    if (!value) return false; //false is default
    switch(value[0]){
        case 't':
            if (strncmp(value, "true", 4) == 0) return true;
            break;
        case 'f':
            if (strncmp(value, "false", 5) == 0) return false;
            break;
    }
    return false; //false is default
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
   
    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_ATTR_ORDER:
            feConvolveMatrix->order.set(value);
            //From SVG spec: If <orderY> is not provided, it defaults to <orderX>.
            if (feConvolveMatrix->order.optNumIsSet() == false)
                feConvolveMatrix->order.setOptNumber(feConvolveMatrix->order.getNumber());
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_KERNELMATRIX:
            feConvolveMatrix->kernelMatrix = read_kernel_matrix(value, (int) (feConvolveMatrix->order.getNumber() * feConvolveMatrix->order.getOptNumber()));
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_DIVISOR:
            read_num = sp_feConvolveMatrix_read_number(value);
            if (read_num != feConvolveMatrix->divisor){
                feConvolveMatrix->divisor = read_num;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_BIAS:
            read_num = sp_feConvolveMatrix_read_number(value);
            if (read_num != feConvolveMatrix->bias){
                feConvolveMatrix->bias = read_num;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_TARGETX:
            read_int = (int) sp_feConvolveMatrix_read_number(value);
            if (read_int != feConvolveMatrix->targetX){
                feConvolveMatrix->targetX = read_int;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_TARGETY:
            read_int = (int) sp_feConvolveMatrix_read_number(value);
            if (read_int != feConvolveMatrix->targetY){
                feConvolveMatrix->targetY = read_int;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_EDGEMODE:
            read_int = (int) sp_feConvolveMatrix_read_edgeMode(value);
            if (read_int != feConvolveMatrix->edgeMode){
                feConvolveMatrix->edgeMode = read_int;
                object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_KERNELUNITLENGTH:
            feConvolveMatrix->kernelUnitLength.set(value);
            object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_PRESERVEALPHA:
            read_bool = sp_feConvolveMatrix_read_bool(value);
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
sp_feConvolveMatrix_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    // Inkscape-only object, not copied during an "plain SVG" dump:
    if (flags & SP_OBJECT_WRITE_EXT) {
        if (repr) {
            // is this sane?
            repr->mergeFrom(SP_OBJECT_REPR(object), "id");
        } else {
            repr = SP_OBJECT_REPR(object)->duplicate(NULL); // FIXME
        }
    }

    if (((SPObjectClass *) feConvolveMatrix_parent_class)->write) {
        ((SPObjectClass *) feConvolveMatrix_parent_class)->write(object, repr, flags);
    }

    return repr;
}

static void sp_feConvolveMatrix_build_renderer(SPFilterPrimitive *primitive, NR::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeConvolveMatrix *sp_convolve = SP_FECONVOLVEMATRIX(primitive);

    int primitive_n = filter->add_primitive(NR::NR_FILTER_CONVOLVEMATRIX);
    NR::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    NR::FilterConvolveMatrix *nr_convolve = dynamic_cast<NR::FilterConvolveMatrix*>(nr_primitive);
    g_assert(nr_convolve != NULL);

    sp_filter_primitive_renderer_common(primitive, nr_primitive);

    nr_convolve->set_targetX(sp_convolve->targetX);
    nr_convolve->set_targetY(sp_convolve->targetY);
    nr_convolve->set_orderX( (int)sp_convolve->order.getNumber() );
    nr_convolve->set_orderY( (int)sp_convolve->order.getOptNumber() );
    nr_convolve->set_kernelMatrix(sp_convolve->kernelMatrix);
    nr_convolve->set_divisor(sp_convolve->divisor);
    nr_convolve->set_bias(sp_convolve->bias);

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
