#ifndef __SP_USE_H__
#define __SP_USE_H__

/*
 * SVG <use> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <sigc++/sigc++.h>
#include "svg/svg-length.h"
#include "sp-item.h"


#define SP_TYPE_USE            (sp_use_get_type ())
#define SP_USE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_USE, SPUse))
#define SP_USE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_USE, SPUseClass))
#define SP_IS_USE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_USE))
#define SP_IS_USE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_USE))

class SPUse;
class SPUseClass;
class SPUseReference;

struct SPUse : public SPItem {
    // item built from the original's repr (the visible clone)
    // relative to the SPUse itself, it is treated as a child, similar to a grouped item relative to its group
    SPObject *child;

    // SVG attrs
    SVGLength x;
    SVGLength y;
    SVGLength width;
    SVGLength height;
    gchar *href;

    // the reference to the original object
    SPUseReference *ref;

    // a sigc connection for delete notifications
    sigc::connection _delete_connection;
    sigc::connection _changed_connection;

    // a sigc connection for transformed signal, used to do move compensation
    sigc::connection _transformed_connection;
};

struct SPUseClass {
    SPItemClass parent_class;
};

GType sp_use_get_type (void);

SPItem *sp_use_unlink (SPUse *use);
SPItem *sp_use_get_original (SPUse *use);
NR::Matrix sp_use_get_parent_transform (SPUse *use);
NR::Matrix sp_use_get_root_transform(SPUse *use);

SPItem *sp_use_root(SPUse *use);
#endif
