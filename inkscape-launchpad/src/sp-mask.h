#ifndef SEEN_SP_MASK_H
#define SEEN_SP_MASK_H

/*
 * SVG <mask> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/rect.h>
#include "sp-object-group.h"
#include "uri-references.h"
#include "xml/node.h"

#define SP_MASK(obj) (dynamic_cast<SPMask*>((SPObject*)obj))
#define SP_IS_MASK(obj) (dynamic_cast<const SPMask*>((SPObject*)obj) != NULL)

struct SPMaskView;

namespace Inkscape {

class Drawing;
class DrawingItem;

} // namespace Inkscape


class SPMask : public SPObjectGroup {
public:
	SPMask();
	virtual ~SPMask();

	unsigned int maskUnits_set : 1;
	unsigned int maskUnits : 1;

	unsigned int maskContentUnits_set : 1;
	unsigned int maskContentUnits : 1;

	SPMaskView *display;

	Inkscape::DrawingItem *sp_mask_show(Inkscape::Drawing &drawing, unsigned int key);
	void sp_mask_hide(unsigned int key);

	void sp_mask_set_bbox(unsigned int key, Geom::OptRect const &bbox);

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);

	virtual void set(unsigned int key, const char* value);

	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual void modified(unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);
};

class SPMaskReference : public Inkscape::URIReference {
public:
	SPMaskReference(SPObject *obj) : URIReference(obj) {}
	SPMask *getObject() const {
		return static_cast<SPMask *>(URIReference::getObject());
	}
protected:
    /**
     * If the owner element of this reference (the element with <... mask="...">)
     * is a child of the mask it refers to, return false.
     * \return false if obj is not a mask or if obj is a parent of this
     *         reference's owner element.  True otherwise.
     */
	virtual bool _acceptObject(SPObject *obj) const {
		if (!SP_IS_MASK(obj)) {
		    return false;
	    }
	    SPObject * const owner = this->getOwner();
        if (!URIReference::_acceptObject(obj)) {
	  //XML Tree being used directly here while it shouldn't be...
	  Inkscape::XML::Node * const owner_repr = owner->getRepr();
	  //XML Tree being used directly here while it shouldn't be...
	  Inkscape::XML::Node * const obj_repr = obj->getRepr();
            char const * owner_name = "";
            char const * owner_mask = "";
            char const * obj_name = "";
            char const * obj_id = "";
            if (owner_repr != NULL) {
                owner_name = owner_repr->name();
                owner_mask = owner_repr->attribute("mask");
            }
            if (obj_repr != NULL) {
                obj_name = obj_repr->name();
                obj_id = obj_repr->attribute("id");
            }
            printf("WARNING: Ignoring recursive mask reference "
                      "<%s mask=\"%s\"> in <%s id=\"%s\">",
                      owner_name, owner_mask,
                      obj_name, obj_id);
            return false;
        }
        return true;
	}
};

const char *sp_mask_create (std::vector<Inkscape::XML::Node*> &reprs, SPDocument *document, Geom::Affine const* applyTransform);

#endif // SEEN_SP_MASK_H
