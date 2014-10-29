#ifndef SEEN_SP_SYMBOL_H
#define SEEN_SP_SYMBOL_H

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

#include <2geom/affine.h>
#include "sp-item-group.h"
#include "viewbox.h"

#define SP_TYPE_SYMBOL (sp_symbol_get_type ())
#define SP_SYMBOL(obj) (dynamic_cast<SPSymbol*>((SPObject*)obj))
#define SP_IS_SYMBOL(obj) (dynamic_cast<const SPSymbol*>((SPObject*)obj) != NULL)

class SPSymbol : public SPGroup, public SPViewBox {
public:
	SPSymbol();
	virtual ~SPSymbol();

	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void release();
	virtual void set(unsigned int key, char const* value);
	virtual void update(SPCtx *ctx, unsigned int flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);

	virtual void modified(unsigned int flags);
	virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);

	virtual Inkscape::DrawingItem* show(Inkscape::Drawing &drawing, unsigned int key, unsigned int flags);
	virtual void print(SPPrintContext *ctx);
	virtual Geom::OptRect bbox(Geom::Affine const &transform, SPItem::BBoxType type) const;
	virtual void hide (unsigned int key);
};

#endif
