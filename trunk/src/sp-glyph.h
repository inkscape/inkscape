#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef ENABLE_SVG_FONTS
#ifndef __SP_GLYPH_H__
#define __SP_GLYPH_H__

/*
 * SVG <glyph> element implementation
 *
 * Authors:
 *    Felipe C. da S. Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_TYPE_GLYPH (sp_glyph_get_type ())
#define SP_GLYPH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_GLYPH, SPGlyph))
#define SP_GLYPH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_GLYPH, SPGlyphClass))
#define SP_IS_GLYPH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_GLYPH))
#define SP_IS_GLYPH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_GLYPH))

enum glyphArabicForm {
    GLYPH_ARABIC_FORM_INITIAL,
    GLYPH_ARABIC_FORM_MEDIAL,
    GLYPH_ARABIC_FORM_TERMINAL,
    GLYPH_ARABIC_FORM_ISOLATED,
};

enum glyphOrientation {
    GLYPH_ORIENTATION_HORIZONTAL,
    GLYPH_ORIENTATION_VERTICAL,
    GLYPH_ORIENTATION_BOTH
};

struct SPGlyph : public SPObject {
    Glib::ustring unicode;
    Glib::ustring glyph_name;
    char* d;
    glyphOrientation orientation;
    glyphArabicForm arabic_form;
    char* lang;
    double horiz_adv_x;
    double vert_origin_x;
    double vert_origin_y;
    double vert_adv_y;
};

struct SPGlyphClass {
	SPObjectClass parent_class;
};

GType sp_glyph_get_type (void);

#endif //#ifndef __SP_GLYPH_H__
#endif //#ifdef ENABLE_SVG_FONTS
