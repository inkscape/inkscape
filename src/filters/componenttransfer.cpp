/** \file
 * SVG <feComponentTransfer> implementation.
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

#include <string.h>

#include "document.h"
#include "attributes.h"
#include "svg/svg.h"
#include "filters/componenttransfer.h"
#include "filters/componenttransfer-funcnode.h"
#include "xml/repr.h"
#include "display/nr-filter.h"
#include "display/nr-filter-component-transfer.h"

SPFeComponentTransfer::SPFeComponentTransfer()
    : SPFilterPrimitive(), renderer(NULL)
{
}

SPFeComponentTransfer::~SPFeComponentTransfer() {
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeComponentTransfer variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPFeComponentTransfer::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPFilterPrimitive::build(document, repr);

	/*LOAD ATTRIBUTES FROM REPR HERE*/

	//do we need this?
	//document->addResource("feComponentTransfer", object);
}

static void sp_feComponentTransfer_children_modified(SPFeComponentTransfer *sp_componenttransfer)
{
    if (sp_componenttransfer->renderer) {
        bool set[4] = {false, false, false, false};
        SPObject* node = sp_componenttransfer->children;
        for(;node;node=node->next){
            int i = 4;

            SPFeFuncNode *funcNode = SP_FEFUNCNODE(node);

            switch (funcNode->channel) {
            case SPFeFuncNode::R:
                i = 0;
                break;
            case SPFeFuncNode::G:
                i = 1;
                break;
            case SPFeFuncNode::B:
                i = 2;
                break;
            case SPFeFuncNode::A:
                i = 3;
                break;
            }

            if (i == 4) {
                g_warning("Unrecognized channel for component transfer.");
                break;
            }
            sp_componenttransfer->renderer->type[i] = ((SPFeFuncNode *) node)->type;
            sp_componenttransfer->renderer->tableValues[i] = ((SPFeFuncNode *) node)->tableValues;
            sp_componenttransfer->renderer->slope[i] = ((SPFeFuncNode *) node)->slope;
            sp_componenttransfer->renderer->intercept[i] = ((SPFeFuncNode *) node)->intercept;
            sp_componenttransfer->renderer->amplitude[i] = ((SPFeFuncNode *) node)->amplitude;
            sp_componenttransfer->renderer->exponent[i] = ((SPFeFuncNode *) node)->exponent;
            sp_componenttransfer->renderer->offset[i] = ((SPFeFuncNode *) node)->offset;
            set[i] = true;
        }
        // Set any types not explicitly set to the identity transform
        for(int i=0;i<4;i++) {
            if (!set[i]) {
                sp_componenttransfer->renderer->type[i] = Inkscape::Filters::COMPONENTTRANSFER_TYPE_IDENTITY;
            }
        }
    }
}

/**
 * Callback for child_added event.
 */
void SPFeComponentTransfer::child_added(Inkscape::XML::Node *child, Inkscape::XML::Node *ref) {
    SPFilterPrimitive::child_added(child, ref);

    sp_feComponentTransfer_children_modified(this);
    this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Callback for remove_child event.
 */
void SPFeComponentTransfer::remove_child(Inkscape::XML::Node *child) {
    SPFilterPrimitive::remove_child(child);

    sp_feComponentTransfer_children_modified(this);
    this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Drops any allocated memory.
 */
void SPFeComponentTransfer::release() {
	SPFilterPrimitive::release();
}

/**
 * Sets a specific value in the SPFeComponentTransfer.
 */
void SPFeComponentTransfer::set(unsigned int key, gchar const *value) {
    switch(key) {
        /*DEAL WITH SETTING ATTRIBUTES HERE*/
        default:
        	SPFilterPrimitive::set(key, value);
            break;
    }
}

/**
 * Receives update notifications.
 */
void SPFeComponentTransfer::update(SPCtx *ctx, guint flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    SPFilterPrimitive::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPFeComponentTransfer::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = this->getRepr()->duplicate(doc);
    }

    SPFilterPrimitive::write(doc, repr, flags);

    return repr;
}

void SPFeComponentTransfer::build_renderer(Inkscape::Filters::Filter* filter) {
    g_assert(this != NULL);
    g_assert(filter != NULL);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_COMPONENTTRANSFER);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterComponentTransfer *nr_componenttransfer = dynamic_cast<Inkscape::Filters::FilterComponentTransfer*>(nr_primitive);
    g_assert(nr_componenttransfer != NULL);

    this->renderer = nr_componenttransfer;
    sp_filter_primitive_renderer_common(this, nr_primitive);


    sp_feComponentTransfer_children_modified(this);    //do we need it?!
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
