/** \file
 * SVG <feImage> implementation.
 *
 */
/*
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Abhishek Sharma
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
#include "enums.h"
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

GType sp_feImage_get_type()
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

static void sp_feImage_class_init(SPFeImageClass *klass)
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

static void sp_feImage_init(SPFeImage *feImage)
{
    feImage->aspect_align = SP_ASPECT_XMID_YMID; // Default
    feImage->aspect_clip = SP_ASPECT_MEET; // Default
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeImage variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void sp_feImage_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    // Save document reference so we can load images with relative paths.
    SPFeImage *feImage = SP_FEIMAGE(object);
    feImage->document = document;

    if (((SPObjectClass *) feImage_parent_class)->build) {
        ((SPObjectClass *) feImage_parent_class)->build(object, document, repr);
    }

    /*LOAD ATTRIBUTES FROM REPR HERE*/

    object->readAttr( "preserveAspectRatio" );
    object->readAttr( "xlink:href" );

}

/**
 * Drops any allocated memory.
 */
static void sp_feImage_release(SPObject *object)
{
    SPFeImage *feImage = SP_FEIMAGE(object);
    feImage->_image_modified_connection.disconnect();
    feImage->_href_modified_connection.disconnect();
    if (feImage->SVGElemRef) delete feImage->SVGElemRef;

    if (((SPObjectClass *) feImage_parent_class)->release)
        ((SPObjectClass *) feImage_parent_class)->release(object);
}

static void sp_feImage_elem_modified(SPObject* /*href*/, guint /*flags*/, SPObject* obj)
{
    obj->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void sp_feImage_href_modified(SPObject* /*old_elem*/, SPObject* new_elem, SPObject* obj)
{
    SPFeImage *feImage = SP_FEIMAGE(obj);
    feImage->_image_modified_connection.disconnect();
    if (new_elem) {
        feImage->SVGElem = SP_ITEM(new_elem);
        feImage->_image_modified_connection = ((SPObject*) feImage->SVGElem)->connectModified(sigc::bind(sigc::ptr_fun(&sp_feImage_elem_modified), obj));
    } else {
        feImage->SVGElem = 0;
    }

    obj->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Sets a specific value in the SPFeImage.
 */
static void sp_feImage_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPFeImage *feImage = SP_FEIMAGE(object);
    (void)feImage;
    switch(key) {
    /*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_ATTR_XLINK_HREF:
            if (feImage->href) {
                g_free(feImage->href);
            }
            feImage->href = (value) ? g_strdup (value) : NULL;
            if (!feImage->href) return;
            delete feImage->SVGElemRef;
            feImage->SVGElemRef = 0;
            feImage->SVGElem = 0;
            feImage->_image_modified_connection.disconnect();
            feImage->_href_modified_connection.disconnect();
            try{
                Inkscape::URI SVGElem_uri(feImage->href);
                feImage->SVGElemRef = new Inkscape::URIReference(feImage->document);
                feImage->SVGElemRef->attach(SVGElem_uri);
                feImage->from_element = true;
                feImage->_href_modified_connection = feImage->SVGElemRef->changedSignal().connect(sigc::bind(sigc::ptr_fun(&sp_feImage_href_modified), object));
                if (SPObject *elemref = feImage->SVGElemRef->getObject()) {
                    feImage->SVGElem = SP_ITEM(elemref);
                    feImage->_image_modified_connection = ((SPObject*) feImage->SVGElem)->connectModified(sigc::bind(sigc::ptr_fun(&sp_feImage_elem_modified), object));
                    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
                    break;
                } else {
                    g_warning("SVG element URI was not found in the document while loading feImage");
                }
            }
            // catches either MalformedURIException or UnsupportedURIException
            catch(const Inkscape::BadURIException & e)
            {
                feImage->from_element = false;
                /* This occurs when using external image as the source */
                //g_warning("caught Inkscape::BadURIException in sp_feImage_set");
                break;
            }
            break;

        case SP_ATTR_PRESERVEASPECTRATIO:
            /* Copied from sp-image.cpp */
            /* Do setup before, so we can use break to escape */
            feImage->aspect_align = SP_ASPECT_XMID_YMID; // Default
            feImage->aspect_clip = SP_ASPECT_MEET; // Default
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
            if (value) {
                int len;
                gchar c[256];
                const gchar *p, *e;
                unsigned int align, clip;
                p = value;
                while (*p && *p == 32) p += 1;
                if (!*p) break;
                e = p;
                while (*e && *e != 32) e += 1;
                len = e - p;
                if (len > 8) break;
                memcpy (c, value, len);
                c[len] = 0;
                /* Now the actual part */
                if (!strcmp (c, "none")) {
                    align = SP_ASPECT_NONE;
                } else if (!strcmp (c, "xMinYMin")) {
                    align = SP_ASPECT_XMIN_YMIN;
                } else if (!strcmp (c, "xMidYMin")) {
                    align = SP_ASPECT_XMID_YMIN;
                } else if (!strcmp (c, "xMaxYMin")) {
                    align = SP_ASPECT_XMAX_YMIN;
                } else if (!strcmp (c, "xMinYMid")) {
                    align = SP_ASPECT_XMIN_YMID;
                } else if (!strcmp (c, "xMidYMid")) {
                    align = SP_ASPECT_XMID_YMID;
                } else if (!strcmp (c, "xMaxYMid")) {
                    align = SP_ASPECT_XMAX_YMID;
                } else if (!strcmp (c, "xMinYMax")) {
                    align = SP_ASPECT_XMIN_YMAX;
                } else if (!strcmp (c, "xMidYMax")) {
                    align = SP_ASPECT_XMID_YMAX;
                } else if (!strcmp (c, "xMaxYMax")) {
                    align = SP_ASPECT_XMAX_YMAX;
                } else {
                    g_warning("Illegal preserveAspectRatio: %s", c);
                    break;
                }
                clip = SP_ASPECT_MEET;
                while (*e && *e == 32) e += 1;
                if (*e) {
                    if (!strcmp (e, "meet")) {
                        clip = SP_ASPECT_MEET;
                    } else if (!strcmp (e, "slice")) {
                        clip = SP_ASPECT_SLICE;
                    } else {
                        break;
                    }
                }
                feImage->aspect_align = align;
                feImage->aspect_clip = clip;
            }
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
static void sp_feImage_update(SPObject *object, SPCtx *ctx, guint flags)
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
static Inkscape::XML::Node * sp_feImage_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = object->getRepr()->duplicate(doc);
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
    nr_image->set_align( sp_image->aspect_align );
    nr_image->set_clip( sp_image->aspect_clip );
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
