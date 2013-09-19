/*
 * Abstract base class for non-item groups
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2003 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object-group.h"
#include "xml/repr.h"
#include "document.h"

SPObjectGroup::SPObjectGroup() : SPObject() {
}

SPObjectGroup::~SPObjectGroup() {
}

void SPObjectGroup::child_added(Inkscape::XML::Node *child, Inkscape::XML::Node *ref) {
	SPObject::child_added(child, ref);

	this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}


void SPObjectGroup::remove_child(Inkscape::XML::Node *child) {
	SPObject::remove_child(child);

	this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}


void SPObjectGroup::order_changed(Inkscape::XML::Node *child, Inkscape::XML::Node *old_ref, Inkscape::XML::Node *new_ref) {
	SPObject::order_changed(child, old_ref, new_ref);

	this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}


Inkscape::XML::Node *SPObjectGroup::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if (flags & SP_OBJECT_WRITE_BUILD) {
        if (!repr) {
            repr = xml_doc->createElement("svg:g");
        }

        GSList *l = 0;
        for ( SPObject *child = this->firstChild() ; child ; child = child->getNext() ) {
            Inkscape::XML::Node *crepr = child->updateRepr(xml_doc, NULL, flags);

            if (crepr) {
                l = g_slist_prepend(l, crepr);
            }
        }

        while (l) {
            repr->addChild(static_cast<Inkscape::XML::Node *>(l->data), NULL);
            Inkscape::GC::release(static_cast<Inkscape::XML::Node *>(l->data));
            l = g_slist_remove(l, l->data);
        }
    } else {
        for ( SPObject *child = this->firstChild() ; child ; child = child->getNext() ) {
            child->updateRepr(flags);
        }
    }

    SPObject::write(xml_doc, repr, flags);

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
