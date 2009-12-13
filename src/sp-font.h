#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef SP_FONT_H_SEEN
#define SP_FONT_H_SEEN

/*
 * SVG <font> element implementation
 *
 * Authors:
 *    Felipe C. da S. Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_TYPE_FONT (sp_font_get_type ())
#define SP_FONT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FONT, SPFont))
#define SP_FONT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FONT, SPFontClass))
#define SP_IS_FONT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FONT))
#define SP_IS_FONT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FONT))

struct SPFont : public SPObject {
    double horiz_origin_x;
    double horiz_origin_y;
    double horiz_adv_x;
    double vert_origin_x;
    double vert_origin_y;
    double vert_adv_y;
};

struct SPFontClass {
    SPObjectClass parent_class;
};

GType sp_font_get_type (void);

#endif //#ifndef SP_FONT_H_SEEN
