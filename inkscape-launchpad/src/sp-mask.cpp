/*
 * SVG <mask> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <string>
#include <2geom/transforms.h>

#include "display/drawing.h"
#include "display/drawing-group.h"
#include "xml/repr.h"

#include "enums.h"
#include "attributes.h"
#include "document.h"
#include "document-private.h"
#include "sp-item.h"

#include "sp-mask.h"

struct SPMaskView {
	SPMaskView *next;
	unsigned int key;
	Inkscape::DrawingItem *arenaitem;
	Geom::OptRect bbox;
};

SPMaskView *sp_mask_view_new_prepend (SPMaskView *list, unsigned int key, Inkscape::DrawingItem *arenaitem);
SPMaskView *sp_mask_view_list_remove (SPMaskView *list, SPMaskView *view);

SPMask::SPMask() : SPObjectGroup() {
	this->maskUnits_set = FALSE;
	this->maskUnits = SP_CONTENT_UNITS_OBJECTBOUNDINGBOX;

	this->maskContentUnits_set = FALSE;
	this->maskContentUnits = SP_CONTENT_UNITS_USERSPACEONUSE;

	this->display = NULL;
}

SPMask::~SPMask() {
}

void SPMask::build(SPDocument* doc, Inkscape::XML::Node* repr) {
	SPObjectGroup::build(doc, repr);

	this->readAttr( "maskUnits" );
	this->readAttr( "maskContentUnits" );

	/* Register ourselves */
	doc->addResource("mask", this);
}

void SPMask::release() {
    if (this->document) {
        // Unregister ourselves
        this->document->removeResource("mask", this);
    }

    while (this->display) {
        // We simply unref and let item manage this in handler
        this->display = sp_mask_view_list_remove(this->display, this->display);
    }

    SPObjectGroup::release();
}

void SPMask::set(unsigned int key, const gchar* value) {
	switch (key) {
	case SP_ATTR_MASKUNITS:
		this->maskUnits = SP_CONTENT_UNITS_OBJECTBOUNDINGBOX;
		this->maskUnits_set = FALSE;
		
		if (value) {
			if (!strcmp (value, "userSpaceOnUse")) {
				this->maskUnits = SP_CONTENT_UNITS_USERSPACEONUSE;
				this->maskUnits_set = TRUE;
			} else if (!strcmp (value, "objectBoundingBox")) {
				this->maskUnits_set = TRUE;
			}
		}
		
		this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_MASKCONTENTUNITS:
		this->maskContentUnits = SP_CONTENT_UNITS_USERSPACEONUSE;
		this->maskContentUnits_set = FALSE;
		
		if (value) {
			if (!strcmp (value, "userSpaceOnUse")) {
				this->maskContentUnits_set = TRUE;
			} else if (!strcmp (value, "objectBoundingBox")) {
				this->maskContentUnits = SP_CONTENT_UNITS_OBJECTBOUNDINGBOX;
				this->maskContentUnits_set = TRUE;
			}
		}
		
		this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	default:
		SPObjectGroup::set(key, value);
		break;
	}
}

void SPMask::child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref) {
	/* Invoke SPObjectGroup implementation */
	SPObjectGroup::child_added(child, ref);

	/* Show new object */
	SPObject *ochild = this->document->getObjectByRepr(child);
	
	if (SP_IS_ITEM (ochild)) {
		for (SPMaskView *v = this->display; v != NULL; v = v->next) {
			Inkscape::DrawingItem *ac = SP_ITEM (ochild)->invoke_show(v->arenaitem->drawing(), v->key, SP_ITEM_REFERENCE_FLAGS);
			
			if (ac) {
			    v->arenaitem->prependChild(ac);
			}
		}
	}
}


void SPMask::update(SPCtx* ctx, unsigned int flags) {
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
	
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    std::vector<SPObject *> children = this->childList(false);
    for (std::vector<SPObject *>::const_iterator child = children.begin();child != children.end();++child) {
        sp_object_ref(*child);
    }
    
    
    for (std::vector<SPObject *>::const_iterator child = children.begin();child != children.end();++child) {
        if (flags || ((*child)->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            (*child)->updateDisplay(ctx, flags);
        }
        
        sp_object_unref(*child);
    }

    for (SPMaskView *v = this->display; v != NULL; v = v->next) {
        Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
        
        if (this->maskContentUnits == SP_CONTENT_UNITS_OBJECTBOUNDINGBOX && v->bbox) {
            Geom::Affine t = Geom::Scale(v->bbox->dimensions());
            t.setTranslation(v->bbox->min());
            g->setChildTransform(t);
        } else {
            g->setChildTransform(Geom::identity());
        }
    }
}

void SPMask::modified(unsigned int flags) {
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
	
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    std::vector<SPObject *> children = this->childList(false);
    for (std::vector<SPObject *>::const_iterator child = children.begin();child != children.end();++child) {
        sp_object_ref(*child);
    }
    
    for (std::vector<SPObject *>::const_iterator child = children.begin();child != children.end();++child) {
        if (flags || ((*child)->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            (*child)->emitModified(flags);
        }
        
        sp_object_unref(*child);
    }
}

Inkscape::XML::Node* SPMask::write(Inkscape::XML::Document* xml_doc, Inkscape::XML::Node* repr, guint flags) {
	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = xml_doc->createElement("svg:mask");
	}

	SPObjectGroup::write(xml_doc, repr, flags);

	return repr;
}

// Create a mask element (using passed elements), add it to <defs>
const gchar *
sp_mask_create (std::vector<Inkscape::XML::Node*> &reprs, SPDocument *document, Geom::Affine const* applyTransform)
{
    Inkscape::XML::Node *defsrepr = document->getDefs()->getRepr();

    Inkscape::XML::Document *xml_doc = document->getReprDoc();
    Inkscape::XML::Node *repr = xml_doc->createElement("svg:mask");
    repr->setAttribute("maskUnits", "userSpaceOnUse");
    
    defsrepr->appendChild(repr);
    const gchar *mask_id = repr->attribute("id");
    SPObject *mask_object = document->getObjectById(mask_id);
    
    for (std::vector<Inkscape::XML::Node*>::const_iterator it = reprs.begin(); it != reprs.end(); ++it) {
        Inkscape::XML::Node *node = (*it);
        SPItem *item = SP_ITEM(mask_object->appendChildRepr(node));
        
        if (NULL != applyTransform) {
            Geom::Affine transform (item->transform * (*applyTransform));
            item->doWriteTransform(item->getRepr(), transform);
        }
    }

    if (repr != defsrepr->lastChild())
        defsrepr->changeOrder(repr, defsrepr->lastChild()); // workaround for bug 989084
    
    Inkscape::GC::release(repr);
    return mask_id;
}

Inkscape::DrawingItem *SPMask::sp_mask_show(Inkscape::Drawing &drawing, unsigned int key) {
	g_return_val_if_fail (this != NULL, NULL);
	g_return_val_if_fail (SP_IS_MASK (this), NULL);

	Inkscape::DrawingGroup *ai = new Inkscape::DrawingGroup(drawing);
	this->display = sp_mask_view_new_prepend (this->display, key, ai);

	for ( SPObject *child = this->firstChild() ; child; child = child->getNext() ) {
		if (SP_IS_ITEM (child)) {
			Inkscape::DrawingItem *ac = SP_ITEM (child)->invoke_show (drawing, key, SP_ITEM_REFERENCE_FLAGS);

			if (ac) {
				ai->prependChild(ac);
			}
		}
	}

	if (this->maskContentUnits == SP_CONTENT_UNITS_OBJECTBOUNDINGBOX && this->display->bbox) {
	    Geom::Affine t = Geom::Scale(this->display->bbox->dimensions());
	    t.setTranslation(this->display->bbox->min());
	    ai->setChildTransform(t);
	}

	return ai;
}

void SPMask::sp_mask_hide(unsigned int key) {
	g_return_if_fail (this != NULL);
	g_return_if_fail (SP_IS_MASK (this));

	for ( SPObject *child = this->firstChild(); child; child = child->getNext()) {
		if (SP_IS_ITEM (child)) {
			SP_ITEM(child)->invoke_hide (key);
		}
	}

	for (SPMaskView *v = this->display; v != NULL; v = v->next) {
		if (v->key == key) {
			/* We simply unref and let item to manage this in handler */
			this->display = sp_mask_view_list_remove (this->display, v);
			return;
		}
	}

	g_assert_not_reached ();
}

void SPMask::sp_mask_set_bbox(unsigned int key, Geom::OptRect const &bbox) {
	for (SPMaskView *v = this->display; v != NULL; v = v->next) {
		if (v->key == key) {
		    v->bbox = bbox;
		    break;
		}
	}
}

/* Mask views */

SPMaskView *
sp_mask_view_new_prepend (SPMaskView *list, unsigned int key, Inkscape::DrawingItem *arenaitem)
{
	SPMaskView *new_mask_view = g_new (SPMaskView, 1);

	new_mask_view->next = list;
	new_mask_view->key = key;
	new_mask_view->arenaitem = arenaitem;
	new_mask_view->bbox = Geom::OptRect();

	return new_mask_view;
}

SPMaskView *
sp_mask_view_list_remove (SPMaskView *list, SPMaskView *view)
{
	if (view == list) {
		list = list->next;
	} else {
		SPMaskView *prev;
		prev = list;
		while (prev->next != view) prev = prev->next;
		prev->next = view->next;
	}

	delete view->arenaitem;
	g_free (view);

	return list;
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
