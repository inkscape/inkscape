/*
 * SVG <desc> implementation
 *
 * Authors:
 *   Jeff Schiller <codedread@gmail.com>
 *
 * Copyright (C) 2008 Jeff Schiller
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "sp-desc.h"
#include "xml/repr.h"

static Inkscape::XML::Node *sp_desc_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

G_DEFINE_TYPE(SPDesc, sp_desc, SP_TYPE_OBJECT);

static void sp_desc_class_init(SPDescClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *)(klass);

    sp_object_class->write = sp_desc_write;
}

static void sp_desc_init(SPDesc */*desc*/)
{
}

/**
 * Writes it's settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *sp_desc_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    if (!repr) {
        repr = object->getRepr()->duplicate(doc);
    }

    if ((static_cast<SPObjectClass *>(sp_desc_parent_class))->write) {
        (static_cast<SPObjectClass *>(sp_desc_parent_class))->write(object, doc, repr, flags);
    }

    return repr;
}
