#ifndef __SP_CLIPPATH_H__
#define __SP_CLIPPATH_H__

/*
 * SVG <clipPath> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_TYPE_CLIPPATH (sp_clippath_get_type ())
#define SP_CLIPPATH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_CLIPPATH, SPClipPath))
#define SP_CLIPPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_CLIPPATH, SPClipPathClass))
#define SP_IS_CLIPPATH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_CLIPPATH))
#define SP_IS_CLIPPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_CLIPPATH))

class SPClipPathView;

#include "display/nr-arena-forward.h"
#include "libnr/nr-forward.h"
#include "sp-object-group.h"
#include "uri-references.h"
#include "xml/node.h"

struct SPClipPath : public SPObjectGroup {
	class Reference;

	unsigned int clipPathUnits_set : 1;
	unsigned int clipPathUnits : 1;

	SPClipPathView *display;
};

struct SPClipPathClass {
	SPObjectGroupClass parent_class;
};

GType sp_clippath_get_type (void);

class SPClipPathReference : public Inkscape::URIReference {
public:
	SPClipPathReference(SPObject *obj) : URIReference(obj) {}
	SPClipPath *getObject() const {
		return (SPClipPath *)URIReference::getObject();
	}
protected:
    /**
     * If the owner element of this reference (the element with <... clippath="...">)
     * is a child of the clippath it refers to, return false.
     * \return false if obj is not a clippath or if obj is a parent of this
     *         reference's owner element.  True otherwise.
     */
	virtual bool _acceptObject(SPObject *obj) const {
		if (!SP_IS_CLIPPATH(obj)) {
		    return false;
	    }
	    SPObject * const owner = this->getOwner();
        if (obj->isAncestorOf(owner)) {
            Inkscape::XML::Node * const owner_repr = owner->repr;
            Inkscape::XML::Node * const obj_repr = obj->repr;
            gchar const * owner_name = NULL;
            gchar const * owner_clippath = NULL;
            gchar const * obj_name = NULL;
            gchar const * obj_id = NULL;
            if (owner_repr != NULL) {
                owner_name = owner_repr->name();
                owner_clippath = owner_repr->attribute("clippath");
            }
            if (obj_repr != NULL) {
                obj_name = obj_repr->name();
                obj_id = obj_repr->attribute("id");
            }
            g_warning("Ignoring recursive clippath reference "
                      "<%s clippath=\"%s\"> in <%s id=\"%s\">",
                      owner_name, owner_clippath,
                      obj_name, obj_id);
            return false;
        }
        return true;
	}
};

NRArenaItem *sp_clippath_show(SPClipPath *cp, NRArena *arena, unsigned int key);
void sp_clippath_hide(SPClipPath *cp, unsigned int key);

void sp_clippath_set_bbox(SPClipPath *cp, unsigned int key, NRRect *bbox);
void sp_clippath_get_bbox(SPClipPath *cp, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags);

const gchar *sp_clippath_create (GSList *reprs, SPDocument *document, Geom::Matrix const* applyTransform);

#endif
