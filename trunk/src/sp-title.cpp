#define __SP_TITLE_C__

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

static void sp_title_class_init(SPTitleClass *klass);
static void sp_title_init(SPTitle *rect);
static Inkscape::XML::Node *sp_title_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *title_parent_class;

GType
sp_title_get_type (void)
{
    static GType title_type = 0;

    if (!title_type) {
        GTypeInfo title_info = {
            sizeof (SPTitleClass),
            NULL, NULL,
            (GClassInitFunc) sp_title_class_init,
            NULL, NULL,
            sizeof (SPTitle),
            16,
            (GInstanceInitFunc) sp_title_init,
            NULL,    /* value_table */
        };
        title_type = g_type_register_static (SP_TYPE_OBJECT, "SPTitle", &title_info, (GTypeFlags)0);
    }
    return title_type;
}

static void
sp_title_class_init(SPTitleClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    title_parent_class = (SPObjectClass *) g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->write = sp_title_write;
}

static void
sp_title_init(SPTitle */*desc*/)
{
}

/*
 * \brief Writes it's settings to an incoming repr object, if any
 */
static Inkscape::XML::Node *
sp_title_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    if (!repr) {
        repr = SP_OBJECT_REPR (object)->duplicate(doc);
    }

    if (((SPObjectClass *) title_parent_class)->write)
        ((SPObjectClass *) title_parent_class)->write(object, doc, repr, flags);

    return repr;
}
