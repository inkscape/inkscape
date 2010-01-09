#ifndef __SP_MASK_H__
#define __SP_MASK_H__

/*
 * SVG <mask> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_TYPE_MASK (sp_mask_get_type ())
#define SP_MASK(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_MASK, SPMask))
#define SP_MASK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_MASK, SPMaskClass))
#define SP_IS_MASK(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_MASK))
#define SP_IS_MASK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_MASK))

class SPMask;
class SPMaskClass;
class SPMaskView;

#include "display/nr-arena-forward.h"
#include "libnr/nr-forward.h"
#include "sp-object-group.h"
#include "uri-references.h"
#include "xml/node.h"

struct SPMask : public SPObjectGroup {
	unsigned int maskUnits_set : 1;
	unsigned int maskUnits : 1;

	unsigned int maskContentUnits_set : 1;
	unsigned int maskContentUnits : 1;

	SPMaskView *display;
};

struct SPMaskClass {
	SPObjectGroupClass parent_class;
};

GType sp_mask_get_type (void);

class SPMaskReference : public Inkscape::URIReference {
public:
	SPMaskReference(SPObject *obj) : URIReference(obj) {}
	SPMask *getObject() const {
		return (SPMask *)URIReference::getObject();
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
        if (obj->isAncestorOf(owner)) {
            Inkscape::XML::Node * const owner_repr = owner->repr;
            Inkscape::XML::Node * const obj_repr = obj->repr;
            gchar const * owner_name = NULL;
            gchar const * owner_mask = NULL;
            gchar const * obj_name = NULL;
            gchar const * obj_id = NULL;
            if (owner_repr != NULL) {
                owner_name = owner_repr->name();
                owner_mask = owner_repr->attribute("mask");
            }
            if (obj_repr != NULL) {
                obj_name = obj_repr->name();
                obj_id = obj_repr->attribute("id");
            }
            g_warning("Ignoring recursive mask reference "
                      "<%s mask=\"%s\"> in <%s id=\"%s\">",
                      owner_name, owner_mask,
                      obj_name, obj_id);
            return false;
        }
        return true;
	}
};

NRArenaItem *sp_mask_show (SPMask *mask, NRArena *arena, unsigned int key);
void sp_mask_hide (SPMask *mask, unsigned int key);

void sp_mask_set_bbox (SPMask *mask, unsigned int key, NRRect *bbox);

const gchar *sp_mask_create (GSList *reprs, SPDocument *document, Geom::Matrix const* applyTransform);

#endif
