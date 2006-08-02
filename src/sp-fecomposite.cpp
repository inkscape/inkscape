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
#include "sp-fecomposite.h"
#include "xml/repr.h"

//#define SP_MACROS_SILENT
//#include "macros.h"

#define DEBUG_FECOMPOSITE
#ifdef DEBUG_FECOMPOSITE
# define debug(f, a...) { g_print("%s(%d) %s:", \
                                  __FILE__,__LINE__,__FUNCTION__); \
                          g_print(f, ## a); \
                          g_print("\n"); \
                        }
#else
# define debug(f, a...) /**/
#endif

/* FeComposite base class */

static void sp_feComposite_class_init(SPFeCompositeClass *klass);
static void sp_feComposite_init(SPFeComposite *feComposite);

static void sp_feComposite_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feComposite_release(SPObject *object);
static void sp_feComposite_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feComposite_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feComposite_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *feComposite_parent_class;

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
        feComposite_type = g_type_register_static(SP_TYPE_OBJECT, "SPFeComposite", &feComposite_info, (GTypeFlags)0);
    }
    return feComposite_type;
}

static void
sp_feComposite_class_init(SPFeCompositeClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    feComposite_parent_class = (SPObjectClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feComposite_build;
    sp_object_class->release = sp_feComposite_release;
    sp_object_class->write = sp_feComposite_write;
    sp_object_class->set = sp_feComposite_set;
    sp_object_class->update = sp_feComposite_update;
}

static void
sp_feComposite_init(SPFeComposite *feComposite)
{
    debug("0x%p",feComposite);
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeComposite variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feComposite_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    debug("0x%p",object);
    if (((SPObjectClass *) feComposite_parent_class)->build) {
        ((SPObjectClass *) feComposite_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
}

/**
 * Drops any allocated memory.
 */
static void
sp_feComposite_release(SPObject *object)
{
    debug("0x%p",object);

    if (((SPObjectClass *) feComposite_parent_class)->release)
        ((SPObjectClass *) feComposite_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPFeComposite.
 */
static void
sp_feComposite_set(SPObject *object, unsigned int key, gchar const *value)
{
    debug("0x%p %s(%u): '%s'",object,
            sp_attribute_name(key),key,value);
    SPFeComposite *feComposite = SP_FECOMPOSITE(object);

    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
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
    debug("0x%p",object);

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
sp_feComposite_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
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

    if (((SPObjectClass *) feComposite_parent_class)->write) {
        ((SPObjectClass *) feComposite_parent_class)->write(object, repr, flags);
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
