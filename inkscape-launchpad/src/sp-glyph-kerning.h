#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef __SP_GLYPH_KERNING_H__
#define __SP_GLYPH_KERNING_H__

/*
 * SVG <hkern> and <vkern> elements implementation
 *
 * Authors:
 *    Felipe C. da S. Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"
#include "unicoderange.h"

//#define SP_HKERN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_HKERN, SPHkern))
//#define SP_HKERN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_HKERN, SPGlyphKerningClass))
//#define SP_IS_HKERN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_HKERN))
//#define SP_IS_HKERN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_HKERN))

#define SP_HKERN(obj) (dynamic_cast<SPHkern*>((SPObject*)obj))
#define SP_IS_HKERN(obj) (dynamic_cast<const SPHkern*>((SPObject*)obj) != NULL)

//#define SP_VKERN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_VKERN, SPVkern))
//#define SP_VKERN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_VKERN, SPGlyphKerningClass))
//#define SP_IS_VKERN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_VKERN))
//#define SP_IS_VKERN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_VKERN))

#define SP_VKERN(obj) (dynamic_cast<SPVkern*>((SPObject*)obj))
#define SP_IS_VKERN(obj) (dynamic_cast<const SPVkern*>((SPObject*)obj) != NULL)

// CPPIFY: These casting macros are buggy, as Vkern and Hkern aren't "real" classes.

class GlyphNames{
public: 
GlyphNames(const gchar* value);
~GlyphNames();
bool contains(const char* name);
private:
gchar* names;
};

class SPGlyphKerning : public SPObject {
public:
	SPGlyphKerning();
	virtual ~SPGlyphKerning();

    UnicodeRange* u1;
    GlyphNames* g1;
    UnicodeRange* u2;
    GlyphNames* g2;
    double k;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void set(unsigned int key, const gchar* value);

	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags);
};

class SPHkern : public SPGlyphKerning {

};

class SPVkern : public SPGlyphKerning {

};

#endif //#ifndef __SP_GLYPH_KERNING_H__
