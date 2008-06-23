#include "config.h"
#ifdef ENABLE_SVG_FONTS
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
#include "unicoderange.h"

#define SP_TYPE_HKERN (sp_glyph_kerning_h_get_type ())
#define SP_HKERN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_HKERN, SPHkern))
#define SP_HKERN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_HKERN, SPGlyphKerningClass))
#define SP_IS_HKERN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_HKERN))
#define SP_IS_HKERN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_HKERN))

#define SP_TYPE_VKERN (sp_glyph_kerning_v_get_type ())
#define SP_VKERN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_VKERN, SPVkern))
#define SP_VKERN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_VKERN, SPGlyphKerningClass))
#define SP_IS_VKERN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_VKERN))
#define SP_IS_VKERN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_VKERN))

class GlyphNames{
public: 
GlyphNames(const gchar* value);
~GlyphNames();
bool contains(gchar* name);
private:
gchar* names;
};

struct SPGlyphKerning : public SPObject {
    UnicodeRange* u1;
    GlyphNames* g1;
    UnicodeRange* u2;
    GlyphNames* g2;
    double k;
};

struct SPHkern : public SPGlyphKerning {};
struct SPVkern : public SPGlyphKerning {};

struct SPGlyphKerningClass {
	SPObjectClass parent_class;
};

GType sp_glyph_kerning_h_get_type (void);
GType sp_glyph_kerning_v_get_type (void);

#endif //#ifndef __SP_GLYPH_KERNING_H__
#endif //#ifdef ENABLE_SVG_FONTS
