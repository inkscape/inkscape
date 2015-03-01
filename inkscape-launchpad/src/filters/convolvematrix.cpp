/** \file
 * SVG <feConvolveMatrix> implementation.
 *
 */
/*
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include <math.h>
#include <vector>
#include "attributes.h"
#include "svg/svg.h"
#include "filters/convolvematrix.h"
#include "helper-fns.h"
#include "xml/repr.h"
#include "display/nr-filter.h"
#include "display/nr-filter-convolve-matrix.h"

SPFeConvolveMatrix::SPFeConvolveMatrix() : SPFilterPrimitive() {
	this->bias = 0;
	this->divisorIsSet = 0;
	this->divisor = 0;

    //Setting default values:
    this->order.set("3 3");
    this->targetX = 1;
    this->targetY = 1;
    this->edgeMode = Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_DUPLICATE;
    this->preserveAlpha = false;

    //some helper variables:
    this->targetXIsSet = false;
    this->targetYIsSet = false;
    this->kernelMatrixIsSet = false;
}

SPFeConvolveMatrix::~SPFeConvolveMatrix() {
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeConvolveMatrix variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPFeConvolveMatrix::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPFilterPrimitive::build(document, repr);

	/*LOAD ATTRIBUTES FROM REPR HERE*/
	this->readAttr( "order" );
	this->readAttr( "kernelMatrix" );
	this->readAttr( "divisor" );
	this->readAttr( "bias" );
	this->readAttr( "targetX" );
	this->readAttr( "targetY" );
	this->readAttr( "edgeMode" );
	this->readAttr( "kernelUnitLength" );
	this->readAttr( "preserveAlpha" );
}

/**
 * Drops any allocated memory.
 */
void SPFeConvolveMatrix::release() {
	SPFilterPrimitive::release();
}

static Inkscape::Filters::FilterConvolveMatrixEdgeMode sp_feConvolveMatrix_read_edgeMode(gchar const *value){
    if (!value) {
    	return Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_DUPLICATE; //duplicate is default
    }
    
    switch (value[0]) {
        case 'd':
            if (strncmp(value, "duplicate", 9) == 0) {
            	return Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_DUPLICATE;
            }
            break;
        case 'w':
            if (strncmp(value, "wrap", 4) == 0) {
            	return Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_WRAP;
            }
            break;
        case 'n':
            if (strncmp(value, "none", 4) == 0) {
            	return Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_NONE;
            }
            break;
    }
    
    return Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_DUPLICATE; //duplicate is default
}

/**
 * Sets a specific value in the SPFeConvolveMatrix.
 */
void SPFeConvolveMatrix::set(unsigned int key, gchar const *value) {
    double read_num;
    int read_int;
    bool read_bool;
    Inkscape::Filters::FilterConvolveMatrixEdgeMode read_mode;
   
    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SP_ATTR_ORDER:
            this->order.set(value);
            
            //From SVG spec: If <orderY> is not provided, it defaults to <orderX>.
            if (this->order.optNumIsSet() == false) {
                this->order.setOptNumber(this->order.getNumber());
            }
            
            if (this->targetXIsSet == false) {
            	this->targetX = (int) floor(this->order.getNumber()/2);
            }
            
            if (this->targetYIsSet == false) {
            	this->targetY = (int) floor(this->order.getOptNumber()/2);
            }
            
            this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_KERNELMATRIX:
            if (value){
                this->kernelMatrixIsSet = true;
                this->kernelMatrix = helperfns_read_vector(value);
                
                if (! this->divisorIsSet) {
                    this->divisor = 0;
                    
                    for (unsigned int i = 0; i< this->kernelMatrix.size(); i++) {
                        this->divisor += this->kernelMatrix[i];
                    }
                    
                    if (this->divisor == 0) {
                    	this->divisor = 1;
                    }
                }
                
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            } else {
                g_warning("For feConvolveMatrix you MUST pass a kernelMatrix parameter!");
            }
            break;
        case SP_ATTR_DIVISOR:
            if (value) { 
                read_num = helperfns_read_number(value);
                
                if (read_num == 0) {
                    // This should actually be an error, but given our UI it is more useful to simply set divisor to the default.
                    if (this->kernelMatrixIsSet) {
                        for (unsigned int i = 0; i< this->kernelMatrix.size(); i++) {
                            read_num += this->kernelMatrix[i];
                        }
                    }
                    
                    if (read_num == 0) {
                    	read_num = 1;
                    }
                    
                    if (this->divisorIsSet || this->divisor!=read_num) {
                        this->divisorIsSet = false;
                        this->divisor = read_num;
                        this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
                    }
                } else if (!this->divisorIsSet || this->divisor!=read_num) {
                    this->divisorIsSet = true;
                    this->divisor = read_num;
                    this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
                }
            }
            break;
        case SP_ATTR_BIAS:
            read_num = 0;
            if (value) {
            	read_num = helperfns_read_number(value);
            }
            
            if (read_num != this->bias){
                this->bias = read_num;
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_TARGETX:
            if (value) {
                read_int = (int) helperfns_read_number(value);
                
                if (read_int < 0 || read_int > this->order.getNumber()){
                    g_warning("targetX must be a value between 0 and orderX! Assuming floor(orderX/2) as default value.");
                    read_int = (int) floor(this->order.getNumber()/2.0);
                }
                
                this->targetXIsSet = true;
                
                if (read_int != this->targetX){
                    this->targetX = read_int;
                    this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
                }
            }
            break;
        case SP_ATTR_TARGETY:
            if (value) {
                read_int = (int) helperfns_read_number(value);
                
                if (read_int < 0 || read_int > this->order.getOptNumber()){
                    g_warning("targetY must be a value between 0 and orderY! Assuming floor(orderY/2) as default value.");
                    read_int = (int) floor(this->order.getOptNumber()/2.0);
                }
                
                this->targetYIsSet = true;
                
                if (read_int != this->targetY){
                    this->targetY = read_int;
                    this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
                }
            }
            break;
        case SP_ATTR_EDGEMODE:
            read_mode = sp_feConvolveMatrix_read_edgeMode(value);
            
            if (read_mode != this->edgeMode){
                this->edgeMode = read_mode;
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        case SP_ATTR_KERNELUNITLENGTH:
            this->kernelUnitLength.set(value);
            
            //From SVG spec: If the <dy> value is not specified, it defaults to the same value as <dx>.
            if (this->kernelUnitLength.optNumIsSet() == false) {
                this->kernelUnitLength.setOptNumber(this->kernelUnitLength.getNumber());
            }
            
            this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_PRESERVEALPHA:
            read_bool = helperfns_read_bool(value, false);
            
            if (read_bool != this->preserveAlpha){
                this->preserveAlpha = read_bool;
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
void SPFeConvolveMatrix::update(SPCtx *ctx, guint flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    SPFilterPrimitive::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPFeConvolveMatrix::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = this->getRepr()->duplicate(doc);
    }


    SPFilterPrimitive::write(doc, repr, flags);

    return repr;
}

void SPFeConvolveMatrix::build_renderer(Inkscape::Filters::Filter* filter) {
    g_assert(this != NULL);
    g_assert(filter != NULL);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_CONVOLVEMATRIX);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterConvolveMatrix *nr_convolve = dynamic_cast<Inkscape::Filters::FilterConvolveMatrix*>(nr_primitive);
    g_assert(nr_convolve != NULL);

    sp_filter_primitive_renderer_common(this, nr_primitive);

    nr_convolve->set_targetX(this->targetX);
    nr_convolve->set_targetY(this->targetY);
    nr_convolve->set_orderX( (int)this->order.getNumber() );
    nr_convolve->set_orderY( (int)this->order.getOptNumber() );
    nr_convolve->set_kernelMatrix(this->kernelMatrix);
    nr_convolve->set_divisor(this->divisor);
    nr_convolve->set_bias(this->bias);
    nr_convolve->set_preserveAlpha(this->preserveAlpha);
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
