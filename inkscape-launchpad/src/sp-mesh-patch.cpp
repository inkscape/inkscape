/** @file
 * @gradient meshpatch class.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Tavmjong Bah <tavjong@free.fr>
 *
 * Copyright (C) 1999,2005 authors
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2012 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include "sp-mesh-patch.h"
#include "style.h"

#include "attributes.h"
#include "xml/repr.h"

SPMeshpatch* SPMeshpatch::getNextMeshpatch()
{
    SPMeshpatch *result = 0;

    for (SPObject* obj = getNext(); obj && !result; obj = obj->getNext()) {
        if (SP_IS_MESHPATCH(obj)) {
            result = SP_MESHPATCH(obj);
        }
    }

    return result;
}

SPMeshpatch* SPMeshpatch::getPrevMeshpatch()
{
    SPMeshpatch *result = 0;

    for (SPObject* obj = getPrev(); obj; obj = obj->getPrev()) {
        // The closest previous SPObject that is an SPMeshpatch *should* be ourself.
        if (SP_IS_MESHPATCH(obj)) {
            SPMeshpatch* meshpatch = SP_MESHPATCH(obj);
            // Sanity check to ensure we have a proper sibling structure.
            if (meshpatch->getNextMeshpatch() == this) {
                result = meshpatch;
            } else {
                g_warning("SPMeshpatch previous/next relationship broken");
            }
            break;
        }
    }

    return result;
}


/*
 * Mesh Patch
 */

SPMeshpatch::SPMeshpatch() : SPObject() {
    this->tensor_string = NULL;
}

SPMeshpatch::~SPMeshpatch() {
}

void SPMeshpatch::build(SPDocument* doc, Inkscape::XML::Node* repr) {
	SPObject::build(doc, repr);

	this->readAttr( "tensor" );
}

/**
 * Virtual build: set meshpatch attributes from its associated XML node.
 */

void SPMeshpatch::set(unsigned int key, const gchar* value) {
    switch (key) {
        case SP_ATTR_TENSOR: {
            if (value) {
                this->tensor_string = new Glib::ustring( value );
                // std::cout << "sp_meshpatch_set: Tensor string: " << patch->tensor_string->c_str() << std::endl;
            }
            break;
        }
        default: {
            // Do nothing
        }
    }
}

/**
 * Virtual set: set attribute to value.
 */

Inkscape::XML::Node* SPMeshpatch::write(Inkscape::XML::Document* xml_doc, Inkscape::XML::Node* repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:meshpatch");
    }

    SPObject::write(xml_doc, repr, flags);

    return repr;
}

/**
 * Virtual write: write object attributes to repr.
 */

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
