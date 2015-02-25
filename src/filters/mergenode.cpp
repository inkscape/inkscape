/** \file
 * feMergeNode implementation. A feMergeNode contains the name of one
 * input image for feMerge.
 */
/*
 * Authors:
 *   Kees Cook <kees@outflux.net>
 *   Niko Kiirala <niko@kiirala.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004,2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "attributes.h"
#include "xml/repr.h"
#include "filters/mergenode.h"
#include "filters/merge.h"
#include "display/nr-filter-types.h"

SPFeMergeNode::SPFeMergeNode()
    : SPObject(), input(Inkscape::Filters::NR_FILTER_SLOT_NOT_SET) {
}

SPFeMergeNode::~SPFeMergeNode() {
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeMergeNode variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPFeMergeNode::build(SPDocument */*document*/, Inkscape::XML::Node */*repr*/) {
	this->readAttr( "in" );
}

/**
 * Drops any allocated memory.
 */
void SPFeMergeNode::release() {
	SPObject::release();
}

/**
 * Sets a specific value in the SPFeMergeNode.
 */
void SPFeMergeNode::set(unsigned int key, gchar const *value) {
    SPFeMerge *parent = SP_FEMERGE(this->parent);

    if (key == SP_ATTR_IN) {
        int input = sp_filter_primitive_read_in(parent, value);
        if (input != this->input) {
            this->input = input;
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
        }
    }

    /* See if any parents need this value. */
    SPObject::set(key, value);
}

/**
 * Receives update notifications.
 */
void SPFeMergeNode::update(SPCtx *ctx, guint flags) {
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
    }

    SPObject::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPFeMergeNode::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    // Inkscape-only this, not copied during an "plain SVG" dump:
    if (flags & SP_OBJECT_WRITE_EXT) {
        if (repr) {
            // is this sane?
            //repr->mergeFrom(object->getRepr(), "id");
        } else {
            repr = this->getRepr()->duplicate(doc);
        }
    }

    SPObject::write(doc, repr, flags);

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
