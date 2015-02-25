/** \file
 * SVG <feTurbulence> implementation.
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

#include "attributes.h"
#include "svg/svg.h"
#include "turbulence.h"
#include "helper-fns.h"
#include "xml/repr.h"
#include <string.h>

#include "display/nr-filter.h"
#include "display/nr-filter-turbulence.h"

SPFeTurbulence::SPFeTurbulence() : SPFilterPrimitive() {
	this->stitchTiles = 0;
	this->seed = 0;
	this->numOctaves = 0;
	this->type = Inkscape::Filters::TURBULENCE_FRACTALNOISE;

    this->updated=false;
}

SPFeTurbulence::~SPFeTurbulence() {
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeTurbulence variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPFeTurbulence::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPFilterPrimitive::build(document, repr);

	/*LOAD ATTRIBUTES FROM REPR HERE*/
	this->readAttr( "baseFrequency" );
	this->readAttr( "numOctaves" );
	this->readAttr( "seed" );
	this->readAttr( "stitchTiles" );
	this->readAttr( "type" );
}

/**
 * Drops any allocated memory.
 */
void SPFeTurbulence::release() {
	SPFilterPrimitive::release();
}

static bool sp_feTurbulence_read_stitchTiles(gchar const *value){
    if (!value) {
    	return false; // 'noStitch' is default
    }

    switch(value[0]){
        case 's':
            if (strncmp(value, "stitch", 6) == 0) {
            	return true;
            }
            break;
        case 'n':
            if (strncmp(value, "noStitch", 8) == 0) {
            	return false;
            }
            break;
    }

    return false; // 'noStitch' is default
}

static Inkscape::Filters::FilterTurbulenceType sp_feTurbulence_read_type(gchar const *value){
    if (!value) {
    	return Inkscape::Filters::TURBULENCE_TURBULENCE; // 'turbulence' is default
    }

    switch(value[0]){
        case 'f':
            if (strncmp(value, "fractalNoise", 12) == 0) {
            	return Inkscape::Filters::TURBULENCE_FRACTALNOISE;
            }
            break;
        case 't':
            if (strncmp(value, "turbulence", 10) == 0) {
            	return Inkscape::Filters::TURBULENCE_TURBULENCE;
            }
            break;
    }

    return Inkscape::Filters::TURBULENCE_TURBULENCE; // 'turbulence' is default
}

/**
 * Sets a specific value in the SPFeTurbulence.
 */
void SPFeTurbulence::set(unsigned int key, gchar const *value) {
    int read_int;
    double read_num;
    bool read_bool;
    Inkscape::Filters::FilterTurbulenceType read_type;

    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_ATTR_BASEFREQUENCY:
            this->baseFrequency.set(value);

            // From SVG spec: If two <number>s are provided, the first number represents
            // a base frequency in the X direction and the second value represents a base
            // frequency in the Y direction. If one number is provided, then that value is
            // used for both X and Y.
            if (this->baseFrequency.optNumIsSet() == false) {
                this->baseFrequency.setOptNumber(this->baseFrequency.getNumber());
            }

            this->updated = false;
            this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_NUMOCTAVES:
            read_int = value ? (int)floor(helperfns_read_number(value)) : 1;

            if (read_int != this->numOctaves){
                this->numOctaves = read_int;
                this->updated = false;
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_SEED:
            read_num = value ? helperfns_read_number(value) : 0;

            if (read_num != this->seed){
                this->seed = read_num;
                this->updated = false;
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_STITCHTILES:
            read_bool = sp_feTurbulence_read_stitchTiles(value);

            if (read_bool != this->stitchTiles){
                this->stitchTiles = read_bool;
                this->updated = false;
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_TYPE:
            read_type = sp_feTurbulence_read_type(value);

            if (read_type != this->type){
                this->type = read_type;
                this->updated = false;
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
void SPFeTurbulence::update(SPCtx *ctx, guint flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    SPFilterPrimitive::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPFeTurbulence::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = this->getRepr()->duplicate(doc);
    }

    SPFilterPrimitive::write(doc, repr, flags);

    /* turbulence doesn't take input */
    repr->setAttribute("in", 0);

    return repr;
}

void SPFeTurbulence::build_renderer(Inkscape::Filters::Filter* filter) {
    g_assert(this != NULL);
    g_assert(filter != NULL);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_TURBULENCE);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterTurbulence *nr_turbulence = dynamic_cast<Inkscape::Filters::FilterTurbulence*>(nr_primitive);
    g_assert(nr_turbulence != NULL);

    sp_filter_primitive_renderer_common(this, nr_primitive);

    nr_turbulence->set_baseFrequency(0, this->baseFrequency.getNumber());
    nr_turbulence->set_baseFrequency(1, this->baseFrequency.getOptNumber());
    nr_turbulence->set_numOctaves(this->numOctaves);
    nr_turbulence->set_seed(this->seed);
    nr_turbulence->set_stitchTiles(this->stitchTiles);
    nr_turbulence->set_type(this->type);
    nr_turbulence->set_updated(this->updated);
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
