#define __SP_FEIMAGE_CPP__

/** \file
 * SVG <feImage> implementation.
 *
 */
/*
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *   hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2007 Felipe Sanches
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "uri.h"
#include "uri-references.h"
#include "attributes.h"
#include "svg/svg.h"
#include "image.h"
#include "xml/repr.h"
#include <string.h>

#include "display/nr-filter.h"
#include "display/nr-filter-image.h"

/* FeImage base class */

static void sp_feImage_class_init(SPFeImageClass *klass);
static void sp_feImage_init(SPFeImage *feImage);

static void sp_feImage_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_feImage_release(SPObject *object);
static void sp_feImage_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_feImage_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_feImage_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_feImage_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter);

static SPFilterPrimitiveClass *feImage_parent_class;

GType
sp_feImage_get_type()
{
    static GType feImage_type = 0;

    if (!feImage_type) {
        GTypeInfo feImage_info = {
            sizeof(SPFeImageClass),
            NULL, NULL,
            (GClassInitFunc) sp_feImage_class_init,
            NULL, NULL,
            sizeof(SPFeImage),
            16,
            (GInstanceInitFunc) sp_feImage_init,
            NULL,    /* value_table */
        };
        feImage_type = g_type_register_static(SP_TYPE_FILTER_PRIMITIVE, "SPFeImage", &feImage_info, (GTypeFlags)0);
    }
    return feImage_type;
}

static void
sp_feImage_class_init(SPFeImageClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)klass;
    SPFilterPrimitiveClass * sp_primitive_class = (SPFilterPrimitiveClass *)klass;

    feImage_parent_class = (SPFilterPrimitiveClass*)g_type_class_peek_parent(klass);

    sp_object_class->build = sp_feImage_build;
    sp_object_class->release = sp_feImage_release;
    sp_object_class->write = sp_feImage_write;
    sp_object_class->set = sp_feImage_set;
    sp_object_class->update = sp_feImage_update;

    sp_primitive_class->build_renderer = sp_feImage_build_renderer;
}

static void
sp_feImage_init(SPFeImage */*feImage*/)
{
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeImage variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_feImage_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    // Save document reference so we can load images with relative paths.
    SPFeImage *feImage = SP_FEIMAGE(object);
    feImage->document = document;

    if (((SPObjectClass *) feImage_parent_class)->build) {
        ((SPObjectClass *) feImage_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/

    sp_object_read_attr(object, "x");
    sp_object_read_attr(object, "y");
    sp_object_read_attr(object, "width");
    sp_object_read_attr(object, "height");
    sp_object_read_attr(object, "xlink:href");

}

/**
 * Drops any allocated memory.
 */
static void
sp_feImage_release(SPObject *object)
{
    SPFeImage *feImage = SP_FEIMAGE(object);
    feImage->_modified_connection.disconnect();
    if (feImage->SVGElemRef) delete feImage->SVGElemRef;

    if (((SPObjectClass *) feImage_parent_class)->release)
        ((SPObjectClass *) feImage_parent_class)->release(object);
}

static void
sp_feImage_elem_modified(SPObject* /*href*/, guint /*flags*/, SPObject* obj)
{
    obj->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Sets a specific value in the SPFeImage.
 */
static void
sp_feImage_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeImage *feImage = SP_FEIMAGE(object);
    (void)feImage;
    Inkscape::URI *SVGElem_uri = NULL;
    switch(key) {
    /*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_ATTR_XLINK_HREF:
            if (feImage->href) {
                g_free(feImage->href);
            }
            feImage->href = (value) ? g_strdup (value) : NULL;
            if (!feImage->href) return;
            try{
                SVGElem_uri = new Inkscape::URI(feImage->href);
                feImage->SVGElemRef = new Inkscape::URIReference(feImage->document);
                feImage->from_element = true;
                feImage->SVGElemRef->attach(*SVGElem_uri);
                feImage->SVGElem = SP_ITEM(feImage->SVGElemRef->getObject());

                delete SVGElem_uri;
                feImage->_modified_connection = ((SPObject*) feImage->SVGElem)->connectModified(sigc::bind(sigc::ptr_fun(&sp_feImage_elem_modified), object));
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
                break;
            }
            catch(const Inkscape::UnsupportedURIException & e)
            {
                feImage->from_element = false;
                g_warning("caught Inkscape::UnsupportedURIException in sp_feImage_set");
                break;
            }


        case SP_ATTR_X:
            feImage->x.readOrUnset(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y:
            feImage->y.readOrUnset(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_WIDTH:
            feImage->width.readOrUnset(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_HEIGHT:
            feImage->height.readOrUnset(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) feImage_parent_class)->set)
                ((SPObjectClass *) feImage_parent_class)->set(object, key, value);
            break;
    }

}

/**
 * Receives update notifications.
 */
static void
sp_feImage_update(SPObject *object, SPCtx *ctx, guint flags)
{

    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */
    }

    if (((SPObjectClass *) feImage_parent_class)->update) {
        ((SPObjectClass *) feImage_parent_class)->update(object, ctx, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_feImage_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
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

    if (((SPObjectClass *) feImage_parent_class)->write) {
        ((SPObjectClass *) feImage_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}

static void sp_feImage_build_renderer(SPFilterPrimitive *primitive, Inkscape::Filters::Filter *filter) {
    g_assert(primitive != NULL);
    g_assert(filter != NULL);

    SPFeImage *sp_image = SP_FEIMAGE(primitive);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_IMAGE);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterImage *nr_image = dynamic_cast<Inkscape::Filters::FilterImage*>(nr_primitive);
    g_assert(nr_image != NULL);

    sp_filter_primitive_renderer_common(primitive, nr_primitive);

    nr_image->from_element = sp_image->from_element;
    nr_image->SVGElem = sp_image->SVGElem;
    nr_image->set_region(sp_image->x, sp_image->y, sp_image->width, sp_image->height);
    nr_image->set_href(sp_image->href);
    nr_image->set_document(sp_image->document);
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
