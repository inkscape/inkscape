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
#include "sp-object-group.h"
#include "uri-references.h"
#include <libnr/nr-forward.h>

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
	virtual bool _acceptObject(SPObject *obj) const {
		return SP_IS_CLIPPATH(obj);
	}
};

NRArenaItem *sp_clippath_show(SPClipPath *cp, NRArena *arena, unsigned int key);
void sp_clippath_hide(SPClipPath *cp, unsigned int key);

void sp_clippath_set_bbox(SPClipPath *cp, unsigned int key, NRRect *bbox);
void sp_clippath_get_bbox(SPClipPath *cp, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags);

const gchar *sp_clippath_create (GSList *reprs, Document *document, Geom::Matrix const* applyTransform);

#endif
