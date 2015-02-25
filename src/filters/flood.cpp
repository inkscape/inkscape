/** \file
 * SVG <feFlood> implementation.
 *
 */
/*
 * Authors:
 *   hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "strneq.h"

#include "attributes.h"
#include "svg/svg.h"
#include "svg/svg-color.h"
#include "filters/flood.h"
#include "xml/repr.h"
#include "helper-fns.h"
#include "display/nr-filter.h"
#include "display/nr-filter-flood.h"

SPFeFlood::SPFeFlood() : SPFilterPrimitive() {
	this->color = 0;

    this->opacity = 1;
    this->icc = NULL;
}

SPFeFlood::~SPFeFlood() {
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeFlood variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPFeFlood::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPFilterPrimitive::build(document, repr);

	/*LOAD ATTRIBUTES FROM REPR HERE*/
	this->readAttr( "flood-opacity" );
	this->readAttr( "flood-color" );
}

/**
 * Drops any allocated memory.
 */
void SPFeFlood::release() {
	SPFilterPrimitive::release();
}

/**
 * Sets a specific value in the SPFeFlood.
 */
void SPFeFlood::set(unsigned int key, gchar const *value) {
    gchar const *cend_ptr = NULL;
    gchar *end_ptr = NULL;
    guint32 read_color;
    double read_num;
    bool dirty = false;
    
    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_PROP_FLOOD_COLOR:
            cend_ptr = NULL;
            read_color = sp_svg_read_color(value, &cend_ptr, 0xffffffff);

            if (cend_ptr && read_color != this->color){
                this->color = read_color;
                dirty=true;
            }

            if (cend_ptr){
                while (g_ascii_isspace(*cend_ptr)) {
                    ++cend_ptr;
                }

                if (strneq(cend_ptr, "icc-color(", 10)) {
                    if (!this->icc) {
                    	this->icc = new SVGICCColor();
                    }

                    if ( ! sp_svg_read_icc_color( cend_ptr, this->icc ) ) {
                        delete this->icc;
                        this->icc = NULL;
                    }

                    dirty = true;
                }
            }

            if (dirty) {
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_PROP_FLOOD_OPACITY:
            if (value) {
                read_num = g_ascii_strtod(value, &end_ptr);

                if (end_ptr != NULL) {
                    if (*end_ptr) {
                        g_warning("Unable to convert \"%s\" to number", value);
                        read_num = 1;
                    }
                }
            } else {
                read_num = 1;
            }

            if (read_num != this->opacity) {
                this->opacity = read_num;
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
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
void SPFeFlood::update(SPCtx *ctx, guint flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    SPFilterPrimitive::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPFeFlood::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = this->getRepr()->duplicate(doc);
    }

    SPFilterPrimitive::write(doc, repr, flags);

    return repr;
}

void SPFeFlood::build_renderer(Inkscape::Filters::Filter* filter) {
    g_assert(this != NULL);
    g_assert(filter != NULL);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_FLOOD);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterFlood *nr_flood = dynamic_cast<Inkscape::Filters::FilterFlood*>(nr_primitive);
    g_assert(nr_flood != NULL);

    sp_filter_primitive_renderer_common(this, nr_primitive);
    
    nr_flood->set_opacity(this->opacity);
    nr_flood->set_color(this->color);
    nr_flood->set_icc(this->icc);
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
