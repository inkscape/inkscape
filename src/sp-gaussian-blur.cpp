#define __SP_GAUSSIANBLUR_CPP__

/** \file
 * SVG <gaussianBlur> implementation.
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
#include "sp-gaussian-blur.h"
#include "xml/repr.h"

//#define SP_MACROS_SILENT
//#include "macros.h"

#define DEBUG_GAUSSIANBLUR
#ifdef DEBUG_GAUSSIANBLUR
# define debug(f, a...) { g_print("%s(%d) %s:", \
                                  __FILE__,__LINE__,__FUNCTION__); \
                          g_print(f, ## a); \
                          g_print("\n"); \
                        }
#else
# define debug(f, a...) /**/
#endif

/* GaussianBlur base class */

static void sp_gaussianBlur_class_init(SPGaussianBlurClass *klass);
static void sp_gaussianBlur_init(SPGaussianBlur *gaussianBlur);

static void sp_gaussianBlur_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_gaussianBlur_release(SPObject *object);
static void sp_gaussianBlur_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_gaussianBlur_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_gaussianBlur_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *gaussianBlur_parent_class;

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
        gaussianBlur_type = g_type_register_static(SP_TYPE_OBJECT, "SPGaussianBlur", &gaussianBlur_info, (GTypeFlags)0);
    }
    return gaussianBlur_type;
}

static void
sp_gaussianBlur_class_init(SPGaussianBlurClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    gaussianBlur_parent_class = (SPObjectClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_gaussianBlur_build;
    sp_object_class->release = sp_gaussianBlur_release;
    sp_object_class->write = sp_gaussianBlur_write;
    sp_object_class->set = sp_gaussianBlur_set;
    sp_object_class->update = sp_gaussianBlur_update;
}

static void
sp_gaussianBlur_init(SPGaussianBlur *gaussianBlur)
{
    debug("0x%p",gaussianBlur);

//    gaussianBlur->stdDeviation = 1;
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPGaussianBlur variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_gaussianBlur_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    debug("0x%p",object);
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
    debug("0x%p",object);

    if (((SPObjectClass *) gaussianBlur_parent_class)->release)
        ((SPObjectClass *) gaussianBlur_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPGaussianBlur.
 */
static void
sp_gaussianBlur_set(SPObject *object, unsigned int key, gchar const *value)
{
    debug("0x%p %s(%u): '%s'",object,
            sp_attribute_name(key),key,value);
    SPGaussianBlur *gaussianBlur = SP_GAUSSIANBLUR(object);

    switch(key) {
	case SP_ATTR_STDDEVIATION:
		gaussianBlur->stdDeviation.set(value);
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
    debug("0x%p",object);

    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) gaussianBlur_parent_class)->update) {
        ((SPObjectClass *) gaussianBlur_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_gaussianBlur_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    debug("0x%p",object);

    // Inkscape-only object, not copied during an "plain SVG" dump:
    if (flags & SP_OBJECT_WRITE_EXT) {
        if (repr) {
            // is this sane?
            repr->mergeFrom(SP_OBJECT_REPR(object), "id");
        } else {
            repr = SP_OBJECT_REPR(object)->duplicate();
        }
    }

    if (((SPObjectClass *) gaussianBlur_parent_class)->write) {
        ((SPObjectClass *) gaussianBlur_parent_class)->write(object, repr, flags);
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
