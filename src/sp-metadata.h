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

#define SP_TYPE_METADATA (sp_metadata_get_type ())
#define SP_METADATA(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_METADATA, SPMetadata))
#define SP_IS_METADATA(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_METADATA))

class SPMetadata;
class SPMetadataClass;

struct SPMetadata : public SPObject {
};

struct SPMetadataClass {
	SPObjectClass parent_class;
};

GType sp_metadata_get_type (void);

SPMetadata * sp_document_metadata (Document *document);

#endif
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
