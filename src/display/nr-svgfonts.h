#include "config.h"
#ifdef ENABLE_SVG_FONTS
#ifndef __SVGFONTS_H__
#define __SVGFONTS_H__
/*
 * SVGFonts rendering headear
 *
 * Authors:
 *    Felipe C. da S. Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL version 2 or later.
 * Read the file 'COPYING' for more information.
 */

#include "../sp-font.h"
#include "cairo.h"

void nr_svgfonts_append_spfont(SPFont* font);
cairo_font_face_t* nr_svgfonts_get_user_font_face();

#endif //#ifndef __SVGFONTS_H__
#endif //#ifdef ENABLE_SVG_FONTS

