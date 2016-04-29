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

#include <sigc++/bind.h>
#include "display/nr-filter-image.h"
#include "uri.h"
#include "uri-references.h"
#include "enums.h"
#include "attributes.h"
#include "svg/svg.h"
#include "image.h"
#include "xml/repr.h"
#include <string.h>

#include "display/nr-filter.h"

SPFeImage::SPFeImage() : SPFilterPrimitive() {
	this->href = NULL;
	this->from_element = 0;
	this->SVGElemRef = NULL;
	this->SVGElem = NULL;

    this->aspect_align = SP_ASPECT_XMID_YMID; // Default
    this->aspect_clip = SP_ASPECT_MEET; // Default
}

SPFeImage::~SPFeImage() {
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeImage variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPFeImage::build(SPDocument *document, Inkscape::XML::Node *repr)
{
    SPFilterPrimitive::build(document, repr);

    /*LOAD ATTRIBUTES FROM REPR HERE*/

    this->readAttr( "preserveAspectRatio" );
    this->readAttr( "xlink:href" );
}

/**
 * Drops any allocated memory.
 */
void SPFeImage::release() {
    this->_image_modified_connection.disconnect();
    this->_href_modified_connection.disconnect();

    if (this->SVGElemRef) {
    	delete this->SVGElemRef;
    }

    SPFilterPrimitive::release();
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
void SPFeImage::set(unsigned int key, gchar const *value) {
    switch(key) {
    /*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_ATTR_XLINK_HREF:
            if (this->href) {
                g_free(this->href);
            }
            this->href = (value) ? g_strdup (value) : NULL;
            if (!this->href) return;
            delete this->SVGElemRef;
            this->SVGElemRef = 0;
            this->SVGElem = 0;
            this->_image_modified_connection.disconnect();
            this->_href_modified_connection.disconnect();
            try{
                Inkscape::URI SVGElem_uri(this->href);
                this->SVGElemRef = new Inkscape::URIReference(this->document);
                this->SVGElemRef->attach(SVGElem_uri);
                this->from_element = true;
                this->_href_modified_connection = this->SVGElemRef->changedSignal().connect(sigc::bind(sigc::ptr_fun(&sp_feImage_href_modified), this));
                if (SPObject *elemref = this->SVGElemRef->getObject()) {
                    this->SVGElem = SP_ITEM(elemref);
                    this->_image_modified_connection = ((SPObject*) this->SVGElem)->connectModified(sigc::bind(sigc::ptr_fun(&sp_feImage_elem_modified), this));
                    this->requestModified(SP_OBJECT_MODIFIED_FLAG);
                    break;
                } else {
                    g_warning("SVG element URI was not found in the document while loading this: %s", value);
                }
            }
            // catches either MalformedURIException or UnsupportedURIException
            catch(const Inkscape::BadURIException & e)
            {
                this->from_element = false;
                /* This occurs when using external image as the source */
                //g_warning("caught Inkscape::BadURIException in sp_feImage_set");
                break;
            }
            break;

        case SP_ATTR_PRESERVEASPECTRATIO:
            /* Copied from sp-image.cpp */
            /* Do setup before, so we can use break to escape */
            this->aspect_align = SP_ASPECT_XMID_YMID; // Default
            this->aspect_clip = SP_ASPECT_MEET; // Default
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
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
                this->aspect_align = align;
                this->aspect_clip = clip;
            }
            break;

        default:
        	SPFilterPrimitive::set(key, value);
            break;
    }
}

/**
 * Receives update notifications.
 */
void SPFeImage::update(SPCtx *ctx, guint flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */
    }

    SPFilterPrimitive::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPFeImage::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = this->getRepr()->duplicate(doc);
    }

    SPFilterPrimitive::write(doc, repr, flags);

    return repr;
}

void SPFeImage::build_renderer(Inkscape::Filters::Filter* filter) {
    g_assert(this != NULL);
    g_assert(filter != NULL);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_IMAGE);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterImage *nr_image = dynamic_cast<Inkscape::Filters::FilterImage*>(nr_primitive);
    g_assert(nr_image != NULL);

    sp_filter_primitive_renderer_common(this, nr_primitive);

    nr_image->from_element = this->from_element;
    nr_image->SVGElem = this->SVGElem;
    nr_image->set_align( this->aspect_align );
    nr_image->set_clip( this->aspect_clip );
    nr_image->set_href(this->href);
    nr_image->set_document(this->document);
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
