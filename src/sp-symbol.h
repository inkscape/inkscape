#ifndef __SP_SYMBOL_H__
#define __SP_SYMBOL_H__

/*
 * SVG <symbol> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * This is quite similar in logic to <svg>
 * Maybe we should merge them somehow (Lauris)
 */

#define SP_TYPE_SYMBOL (sp_symbol_get_type ())
#define SP_SYMBOL(obj) (dynamic_cast<SPSymbol*>((SPObject*)obj))
#define SP_IS_SYMBOL(obj) (dynamic_cast<const SPSymbol*>((SPObject*)obj) != NULL)

#include <2geom/affine.h>
#include "svg/svg-length.h"
#include "enums.h"
#include "sp-item-group.h"

class SPSymbol : public SPGroup {
public:
	SPSymbol();
	virtual ~SPSymbol();

	/* viewBox; */
	unsigned int viewBox_set : 1;
	Geom::Rect viewBox;

	/* preserveAspectRatio */
	unsigned int aspect_set : 1;
	unsigned int aspect_align : 4;
	unsigned int aspect_clip : 1;

	/* Child to parent additional transform */
	Geom::Affine c2p;

	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void release();
	virtual void set(unsigned int key, gchar const* value);
	virtual void update(SPCtx *ctx, guint flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags);

	virtual void modified(unsigned int flags);
	virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);

	virtual Inkscape::DrawingItem* show(Inkscape::Drawing &drawing, unsigned int key, unsigned int flags);
	virtual void print(SPPrintContext *ctx);
	virtual Geom::OptRect bbox(Geom::Affine const &transform, SPItem::BBoxType type);
	virtual void hide (unsigned int key);
};

#endif
