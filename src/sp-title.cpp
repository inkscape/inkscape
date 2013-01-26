/*
 * SVG <title> implementation
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

#include "sp-title.h"
#include "xml/repr.h"

static Inkscape::XML::Node *sp_title_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

G_DEFINE_TYPE(SPTitle, sp_title, SP_TYPE_OBJECT);

static void
sp_title_class_init(SPTitleClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    sp_object_class->write = sp_title_write;
}

static void
sp_title_init(SPTitle */*desc*/)
{
}

/**
 * Writes it's settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *sp_title_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    if (!repr) {
        repr = object->getRepr()->duplicate(doc);
    }

    if (((SPObjectClass *) sp_title_parent_class)->write) {
        ((SPObjectClass *) sp_title_parent_class)->write(object, doc, repr, flags);
    }

    return repr;
}
