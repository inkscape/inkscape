#ifndef __SP_FONTFACE_H__
#define __SP_FONTFACE_H__

/*
 * SVG <font-face> element implementation
 *
 * Section 20.8.3 of the W3C SVG 1.1 spec
 * available at: 
 * http://www.w3.org/TR/SVG/fonts.html#FontFaceElement
 *
 * Authors:
 *    Felipe C. da S. Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"

#define SP_TYPE_FONTFACE (sp_fontface_get_type ())
#define SP_FONTFACE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FONTFACE, SPFontFace))
#define SP_FONTFACE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FONTFACE, SPFontFaceClass))
#define SP_IS_FONTFACE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FONTFACE))
#define SP_IS_FONTFACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FONTFACE))

enum FontFaceStyleType{
	SP_FONTFACE_STYLE_ALL,
	SP_FONTFACE_STYLE_NORMAL,
	SP_FONTFACE_STYLE_ITALIC,
	SP_FONTFACE_STYLE_OBLIQUE
};

enum FontFaceVariantType{
	SP_FONTFACE_VARIANT_NORMAL,
	SP_FONTFACE_VARIANT_SMALL_CAPS
};

enum FontFaceWeightType{
	SP_FONTFACE_WEIGHT_ALL,
	SP_FONTFACE_WEIGHT_NORMAL,
	SP_FONTFACE_WEIGHT_BOLD,
	SP_FONTFACE_WEIGHT_100,
	SP_FONTFACE_WEIGHT_200,
	SP_FONTFACE_WEIGHT_300,
	SP_FONTFACE_WEIGHT_400,
	SP_FONTFACE_WEIGHT_500,
	SP_FONTFACE_WEIGHT_600,
	SP_FONTFACE_WEIGHT_700,
	SP_FONTFACE_WEIGHT_800,
	SP_FONTFACE_WEIGHT_900
};

enum FontFaceStretchType{
	SP_FONTFACE_SRTETCH_ALL,
	SP_FONTFACE_SRTETCH_NORMAL,
	SP_FONTFACE_SRTETCH_ULTRA_CONDENSED,
	SP_FONTFACE_SRTETCH_EXTRA_CONDENSED,
	SP_FONTFACE_SRTETCH_CONDENSED,
	SP_FONTFACE_SRTETCH_SEMI_CONDENSED,
	SP_FONTFACE_SRTETCH_SEMI_EXPANDED,
	SP_FONTFACE_SRTETCH_EXPANDED,
	SP_FONTFACE_SRTETCH_EXTRA_EXPANDED,
	SP_FONTFACE_SRTETCH_ULTRA_EXPANDED
};

enum FontFaceUnicodeRangeType{
	FONTFACE_UNICODERANGE_FIXME_HERE,
};

struct SPFontFace : public SPObject {
    char* font_family;
    std::vector<FontFaceStyleType> font_style;
    std::vector<FontFaceVariantType> font_variant;
    std::vector<FontFaceWeightType> font_weight;
    std::vector<FontFaceStretchType> font_stretch;
    char* font_size;
    std::vector<FontFaceUnicodeRangeType> unicode_range;
    double units_per_em;
    std::vector<int> panose_1;
    double stemv;
    double stemh;
    double slope;
    double cap_height;
    double x_height;
    double accent_height;
    double ascent;
    double descent;
    char* widths;
    char* bbox;
    double ideographic;
    double alphabetic;
    double mathematical;
    double hanging;
    double v_ideographic;
    double v_alphabetic;
    double v_mathematical;
    double v_hanging;
    double underline_position;
    double underline_thickness;
    double strikethrough_position;
    double strikethrough_thickness;
    double overline_position;
    double overline_thickness;
};

struct SPFontFaceClass {
	SPObjectClass parent_class;
};

GType sp_fontface_get_type (void);

#endif //#ifndef __SP_FONTFACE_H__
