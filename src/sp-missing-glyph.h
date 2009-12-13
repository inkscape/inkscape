#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef ENABLE_SVG_FONTS
#ifndef __SP_MISSING_GLYPH_H__
#define __SP_MISSING_GLYPH_H__

/*
 * SVG <missing-glyph> element implementation
 *
 * Authors:
 *    Felipe C. da S. Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_TYPE_MISSING_GLYPH (sp_missing_glyph_get_type ())
#define SP_MISSING_GLYPH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_MISSING_GLYPH, SPMissingGlyph))
#define SP_MISSING_GLYPH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_MISSING_GLYPH, SPMissingGlyphClass))
#define SP_IS_MISSING_GLYPH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_MISSING_GLYPH))
#define SP_IS_MISSING_GLYPH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_MISSING_GLYPH))

struct SPMissingGlyph : public SPObject {
    char* d;
    double horiz_adv_x;
    double vert_origin_x;
    double vert_origin_y;
    double vert_adv_y;
};

struct SPMissingGlyphClass {
	SPObjectClass parent_class;
};

GType sp_missing_glyph_get_type (void);

#endif //#ifndef __SP_MISSING_GLYPH_H__
#endif //#ifdef ENABLE_SVG_FONTS
