#ifndef __SP_GLYPH_KERNING_H__
#define __SP_GLYPH_KERNING_H__

/*
 * SVG <hkern> and <vkern> elements implementation
 *
 * Authors:
 *    Felipe C. da S. Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_TYPE_GLYPH_KERNING (sp_glyph_kerning_get_type ())
#define SP_GLYPH_KERNING(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_GLYPH_KERNING, SPGlyphKerning))
#define SP_GLYPH_KERNING_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_GLYPH_KERNING, SPGlyphKerningClass))
#define SP_IS_GLYPH_KERNING(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_GLYPH_KERNING))
#define SP_IS_GLYPH_KERNING_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_GLYPH_KERNING))

struct SPGlyphKerning : public SPObject {
    char* u1;
    char* g1;
    char* u2;
    char* g2;
    double k;
};

struct SPGlyphKerningClass {
	SPObjectClass parent_class;
};

GType sp_glyph_kerning_get_type (void);

#endif //#ifndef __SP_GLYPH_KERNING_H__
