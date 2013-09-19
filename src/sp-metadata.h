#ifndef __SP_METADATA_H__
#define __SP_METADATA_H__

/*
 * SVG <metadata> implementation
 *
 * Authors:
 *   Kees Cook <kees@outflux.net>
 *
 * Copyright (C) 2004 Kees Cook
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"


/* Metadata base class */

#define SP_METADATA(obj) (dynamic_cast<SPMetadata*>((SPObject*)obj))
#define SP_IS_METADATA(obj) (dynamic_cast<const SPMetadata*>((SPObject*)obj) != NULL)

class SPMetadata : public SPObject {
public:
	SPMetadata();
	virtual ~SPMetadata();

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void set(unsigned int key, const gchar* value);
	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags);
};

SPMetadata * sp_document_metadata (SPDocument *document);

#endif
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
