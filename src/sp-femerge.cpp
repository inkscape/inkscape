#define __SP_FEMERGE_CPP__

/** \file
 * SVG <feMerge> implementation.
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
#include "sp-femerge.h"
#include "xml/repr.h"

//#define SP_MACROS_SILENT
//#include "macros.h"

#define DEBUG_FEMERGE
#ifdef DEBUG_FEMERGE
# define debug(f, a...) { g_print("%s(%d) %s:", \
                                  __FILE__,__LINE__,__FUNCTION__); \
                          g_print(f, ## a); \
                          g_print("\n"); \
                        }
#else
# define debug(f, a...) /**/
#endif

/* FeMerge base class */

static void sp_feMerge_class_init(SPFeMergeClass *klass);
static void sp_feMerge_init(SPFeMerge *feMerge);

static void sp_feMerge_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feMerge_release(SPObject *object);
static void sp_feMerge_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feMerge_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feMerge_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *feMerge_parent_class;

GType
sp_feMerge_get_type()
{
    static GType feMerge_type = 0;

    if (!feMerge_type) {
        GTypeInfo feMerge_info = {
            sizeof(SPFeMergeClass),
            NULL, NULL,
            (GClassInitFunc) sp_feMerge_class_init,
            NULL, NULL,
            sizeof(SPFeMerge),
            16,
            (GInstanceInitFunc) sp_feMerge_init,
            NULL,    /* value_table */
        };
        feMerge_type = g_type_register_static(SP_TYPE_OBJECT, "SPFeMerge", &feMerge_info, (GTypeFlags)0);
    }
    return feMerge_type;
}

static void
sp_feMerge_class_init(SPFeMergeClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

    feMerge_parent_class = (SPObjectClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feMerge_build;
    sp_object_class->release = sp_feMerge_release;
    sp_object_class->write = sp_feMerge_write;
    sp_object_class->set = sp_feMerge_set;
    sp_object_class->update = sp_feMerge_update;
}

static void
sp_feMerge_init(SPFeMerge *feMerge)
{
    debug("0x%p",feMerge);
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeMerge variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feMerge_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    debug("0x%p",object);
    if (((SPObjectClass *) feMerge_parent_class)->build) {
        ((SPObjectClass *) feMerge_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/
}

/**
 * Drops any allocated memory.
 */
static void
sp_feMerge_release(SPObject *object)
{
    debug("0x%p",object);

    if (((SPObjectClass *) feMerge_parent_class)->release)
        ((SPObjectClass *) feMerge_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPFeMerge.
 */
static void
sp_feMerge_set(SPObject *object, unsigned int key, gchar const *value)
{
    debug("0x%p %s(%u): '%s'",object,
            sp_attribute_name(key),key,value);
    SPFeMerge *feMerge = SP_FEMERGE(object);

    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        default:
            if (((SPObjectClass *) feMerge_parent_class)->set)
                ((SPObjectClass *) feMerge_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feMerge_update(SPObject *object, SPCtx *ctx, guint flags)
{
    debug("0x%p",object);

    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) feMerge_parent_class)->update) {
        ((SPObjectClass *) feMerge_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feMerge_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
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

    if (((SPObjectClass *) feMerge_parent_class)->write) {
        ((SPObjectClass *) feMerge_parent_class)->write(object, repr, flags);
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
