/** \file
 * SVG <skeleton> implementation, used as an example for a base starting class
 * when implementing new sp-objects.
 *
 * In vi, three global search-and-replaces will let you rename everything
 * in this and the .h file:
 *
 *   :%s/SKELETON/YOURNAME/g
 *   :%s/Skeleton/Yourname/g
 *   :%s/skeleton/yourname/g
 */
/*
 * Authors:
 *   Kees Cook <kees@outflux.net>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004 Kees Cook
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "attributes.h"
#include "sp-skeleton.h"
#include "xml/repr.h"

#define DEBUG_SKELETON
#ifdef DEBUG_SKELETON
# define debug(f, a...) { g_print("%s(%d) %s:", \
                                  __FILE__,__LINE__,__FUNCTION__); \
                          g_print(f, ## a); \
                          g_print("\n"); \
                        }
#else
# define debug(f, a...) /**/
#endif

/* Skeleton base class */
static void sp_skeleton_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_skeleton_release(SPObject *object);
static void sp_skeleton_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_skeleton_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_skeleton_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

G_DEFINE_TYPE(SPSkeleton, sp_skeleton, SP_TYPE_OBJECT);

static void
sp_skeleton_class_init(SPSkeletonClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;

<<<<<<< TREE
    sp_object_class->build = sp_skeleton_build;
=======
    skeleton_parent_class = (SPObjectClass*)g_type_class_peek_parent(klass);

    //sp_object_class->build = sp_skeleton_build;
>>>>>>> MERGE-SOURCE
    sp_object_class->release = sp_skeleton_release;
    sp_object_class->write = sp_skeleton_write;
    sp_object_class->set = sp_skeleton_set;
    sp_object_class->update = sp_skeleton_update;
}

static void
sp_skeleton_init(SPSkeleton *skeleton)
{
    debug("0x%p",skeleton);
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPSkeleton variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_skeleton_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    debug("0x%p",object);
<<<<<<< TREE
    if (((SPObjectClass *) sp_skeleton_parent_class)->build) {
        ((SPObjectClass *) sp_skeleton_parent_class)->build(object, document, repr);
    }
=======
//    if (((SPObjectClass *) skeleton_parent_class)->build) {
//        ((SPObjectClass *) skeleton_parent_class)->build(object, document, repr);
//    }
>>>>>>> MERGE-SOURCE

    /*
       Pay attention to certain settings here

    object->readAttr( "xlink:href" );
    object->readAttr( "attributeName" );
    object->readAttr( "attributeType" );
    object->readAttr( "begin" );
    object->readAttr( "dur" );
    object->readAttr( "end" );
    object->readAttr( "min" );
    object->readAttr( "max" );
    object->readAttr( "restart" );
    object->readAttr( "repeatCount" );
    object->readAttr( "repeatDur" );
    object->readAttr( "fill" );
    */
}

/**
 * Drops any allocated memory.
 */
static void
sp_skeleton_release(SPObject *object)
{
    debug("0x%p",object);

    /* deal with our children and our selves here */

    if (((SPObjectClass *) sp_skeleton_parent_class)->release)
        ((SPObjectClass *) sp_skeleton_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPSkeleton.
 */
static void
sp_skeleton_set(SPObject *object, unsigned int key, gchar const *value)
{
    debug("0x%p %s(%u): '%s'",object,
            sp_attribute_name(key),key,value ? value : "<no value>");
    //SPSkeleton *skeleton = SP_SKELETON(object);

    /* See if any parents need this value. */
    if (((SPObjectClass *) sp_skeleton_parent_class)->set) {
        ((SPObjectClass *) sp_skeleton_parent_class)->set(object, key, value);
    }
}

/**
 * Receives update notifications.
 */
static void
sp_skeleton_update(SPObject *object, SPCtx *ctx, guint flags)
{
    debug("0x%p",object);
    //SPSkeleton *skeleton = SP_SKELETON(object);

    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    if (((SPObjectClass *) sp_skeleton_parent_class)->update) {
        ((SPObjectClass *) sp_skeleton_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_skeleton_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    debug("0x%p",object);
    //SPSkeleton *skeleton = SP_SKELETON(object);

    // Inkscape-only object, not copied during an "plain SVG" dump:
    if (flags & SP_OBJECT_WRITE_EXT) {
        if (repr) {
            // is this sane?
            repr->mergeFrom(object->getRepr(), "id");
        } else {
            repr = object->getRepr()->duplicate(doc);
        }
    }

    if (((SPObjectClass *) sp_skeleton_parent_class)->write) {
        ((SPObjectClass *) sp_skeleton_parent_class)->write(object, doc, repr, flags);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
