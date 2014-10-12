/** \file
 * SVG <feBlend> implementation.
 *
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006,2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#include "sp-filter.h"
#include "filters/blend.h"
#include "attributes.h"
#include "svg/svg.h"
#include "xml/repr.h"

#include "display/nr-filter.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-blend.h"
#include "display/nr-filter-types.h"

#include "sp-factory.h"

namespace {
	SPObject* createBlend() {
		return new SPFeBlend();
	}

	bool blendRegistered = SPFactory::instance().registerObject("svg:feBlend", createBlend);
}

SPFeBlend::SPFeBlend()
    : SPFilterPrimitive(), blend_mode(Inkscape::Filters::BLEND_NORMAL),
      in2(Inkscape::Filters::NR_FILTER_SLOT_NOT_SET)
{
}

SPFeBlend::~SPFeBlend() {
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeBlend variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPFeBlend::build(SPDocument *document, Inkscape::XML::Node *repr) {
    SPFilterPrimitive::build(document, repr);

    /*LOAD ATTRIBUTES FROM REPR HERE*/
    this->readAttr( "mode" );
    this->readAttr( "in2" );

    /* Unlike normal in, in2 is required attribute. Make sure, we can call
     * it by some name. */
    if (this->in2 == Inkscape::Filters::NR_FILTER_SLOT_NOT_SET ||
        this->in2 == Inkscape::Filters::NR_FILTER_UNNAMED_SLOT)
    {
        SPFilter *parent = SP_FILTER(this->parent);
        this->in2 = sp_filter_primitive_name_previous_out(this);
        repr->setAttribute("in2", sp_filter_name_for_image(parent, this->in2));
    }
}

/**
 * Drops any allocated memory.
 */
void SPFeBlend::release() {
	SPFilterPrimitive::release();
}

static Inkscape::Filters::FilterBlendMode sp_feBlend_readmode(gchar const *value) {
    if (!value) {
    	return Inkscape::Filters::BLEND_NORMAL;
    }

    switch (value[0]) {
        case 'n':
            if (strncmp(value, "normal", 6) == 0)
                return Inkscape::Filters::BLEND_NORMAL;
            break;
        case 'm':
            if (strncmp(value, "multiply", 8) == 0)
                return Inkscape::Filters::BLEND_MULTIPLY;
            break;
        case 's':
            if (strncmp(value, "screen", 6) == 0)
                return Inkscape::Filters::BLEND_SCREEN;
            if (strncmp(value, "saturation", 6) == 0)
                return Inkscape::Filters::BLEND_SATURATION;
            break;
        case 'd':
            if (strncmp(value, "darken", 6) == 0)
                return Inkscape::Filters::BLEND_DARKEN;
            if (strncmp(value, "difference", 10) == 0)
                return Inkscape::Filters::BLEND_DIFFERENCE;
            break;
        case 'l':
            if (strncmp(value, "lighten", 7) == 0)
                return Inkscape::Filters::BLEND_LIGHTEN;
            if (strncmp(value, "luminosity", 10) == 0)
                return Inkscape::Filters::BLEND_LUMINOSITY;
            break;
        case 'o':
            if (strncmp(value, "overlay", 7) == 0)
                return Inkscape::Filters::BLEND_OVERLAY;
            break;
        case 'c':
            if (strncmp(value, "color-dodge", 11) == 0)
                return Inkscape::Filters::BLEND_COLORDODGE;
            if (strncmp(value, "color-burn", 10) == 0)
                return Inkscape::Filters::BLEND_COLORBURN;
            if (strncmp(value, "color", 5) == 0)
                return Inkscape::Filters::BLEND_COLOR;
            break;
        case 'h':
            if (strncmp(value, "hard-light", 7) == 0)
                return Inkscape::Filters::BLEND_HARDLIGHT;
            if (strncmp(value, "hue", 3) == 0)
                return Inkscape::Filters::BLEND_HUE;
            break;
        case 'e':
            if (strncmp(value, "exclusion", 10) == 0)
                return Inkscape::Filters::BLEND_EXCLUSION;
        default:
            std::cout << "Inkscape::Filters::FilterBlendMode: Unimplemented mode: " << value << std::endl;
            // do nothing by default
            break;
    }

    return Inkscape::Filters::BLEND_NORMAL;
}

/**
 * Sets a specific value in the SPFeBlend.
 */
void SPFeBlend::set(unsigned int key, gchar const *value) {
    Inkscape::Filters::FilterBlendMode mode;
    int input;

    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_ATTR_MODE:
            mode = sp_feBlend_readmode(value);

            if (mode != this->blend_mode) {
                this->blend_mode = mode;
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_IN2:
            input = sp_filter_primitive_read_in(this, value);

            if (input != this->in2) {
                this->in2 = input;
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
void SPFeBlend::update(SPCtx *ctx, guint flags) {
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        this->readAttr( "mode" );
        this->readAttr( "in2" );
    }

    /* Unlike normal in, in2 is required attribute. Make sure, we can call
     * it by some name. */
    /* This may not be true.... see issue at 
     * http://www.w3.org/TR/filter-effects/#feBlendElement (but it doesn't hurt). */
    if (this->in2 == Inkscape::Filters::NR_FILTER_SLOT_NOT_SET ||
        this->in2 == Inkscape::Filters::NR_FILTER_UNNAMED_SLOT)
    {
        SPFilter *parent = SP_FILTER(this->parent);
        this->in2 = sp_filter_primitive_name_previous_out(this);

        // TODO: XML Tree being used directly here while it shouldn't be.
        this->getRepr()->setAttribute("in2", sp_filter_name_for_image(parent, this->in2));
    }

    SPFilterPrimitive::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPFeBlend::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    SPFilter *parent = SP_FILTER(this->parent);

    if (!repr) {
        repr = doc->createElement("svg:feBlend");
    }

    gchar const *in2_name = sp_filter_name_for_image(parent, this->in2);

    if( !in2_name ) {

        // This code is very similar to sp_filter_primtive_name_previous_out()
        SPObject *i = parent->children;

        // Find previous filter primitive
        while (i && i->next != this) {
        	i = i->next;
        }

        if( i ) {
            SPFilterPrimitive *i_prim = SP_FILTER_PRIMITIVE(i);
            in2_name = sp_filter_name_for_image(parent, i_prim->image_out);
        }
    }

    if (in2_name) {
        repr->setAttribute("in2", in2_name);
    } else {
        g_warning("Unable to set in2 for feBlend");
    }

    char const *mode;
    switch(this->blend_mode) {
        case Inkscape::Filters::BLEND_NORMAL:
            mode = "normal";      break;
        case Inkscape::Filters::BLEND_MULTIPLY:
            mode = "multiply";    break;
        case Inkscape::Filters::BLEND_SCREEN:
            mode = "screen";      break;
        case Inkscape::Filters::BLEND_DARKEN:
            mode = "darken";      break;
        case Inkscape::Filters::BLEND_LIGHTEN:
            mode = "lighten";     break;
        // New
        case Inkscape::Filters::BLEND_OVERLAY:
            mode = "overlay";     break;
        case Inkscape::Filters::BLEND_COLORDODGE:
            mode = "color-dodge"; break;
        case Inkscape::Filters::BLEND_COLORBURN:
            mode = "color-burn";  break;
        case Inkscape::Filters::BLEND_HARDLIGHT:
            mode = "hard-light";  break;
        case Inkscape::Filters::BLEND_SOFTLIGHT:
            mode = "soft-light";  break;
        case Inkscape::Filters::BLEND_DIFFERENCE:
            mode = "difference";  break;
        case Inkscape::Filters::BLEND_EXCLUSION:
            mode = "exclusion";   break;
        case Inkscape::Filters::BLEND_HUE:
            mode = "hue";         break;
        case Inkscape::Filters::BLEND_SATURATION:
            mode = "saturation";  break;
        case Inkscape::Filters::BLEND_COLOR:
            mode = "color";       break;
        case Inkscape::Filters::BLEND_LUMINOSITY:
            mode = "luminosity";  break;
        default:
            mode = 0;
    }

    repr->setAttribute("mode", mode);

    SPFilterPrimitive::write(doc, repr, flags);

    return repr;
}

void SPFeBlend::build_renderer(Inkscape::Filters::Filter* filter) {
    g_assert(this != NULL);
    g_assert(filter != NULL);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_BLEND);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterBlend *nr_blend = dynamic_cast<Inkscape::Filters::FilterBlend*>(nr_primitive);
    g_assert(nr_blend != NULL);

    sp_filter_primitive_renderer_common(this, nr_primitive);

    nr_blend->set_mode(this->blend_mode);
    nr_blend->set_input(1, this->in2);
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
