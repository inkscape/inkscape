#include "config.h"
#ifndef NR_SVGFONTS_H_SEEN
#define NR_SVGFONTS_H_SEEN
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

#include "cairo.h"
#include <gtkmm.h>

#include <gtkmm/widget.h>
#include <gtkmm/window.h>
#include <gdkmm/drawable.h>
#include <gdkmm/general.h>  // for cairo helper functions

#include "../sp-glyph.h"
#include "../sp-missing-glyph.h"
#include "../sp-font.h"
#include "../sp-glyph-kerning.h"

class SvgFont;
struct SPFont;

class UserFont{
public:
UserFont(SvgFont* instance);
cairo_font_face_t* face;
};

class SvgFont{
public:
SvgFont(SPFont* spfont);
cairo_font_face_t* get_font_face();
cairo_status_t scaled_font_init (cairo_scaled_font_t *scaled_font, cairo_font_extents_t *metrics);
cairo_status_t scaled_font_text_to_glyphs (cairo_scaled_font_t *scaled_font, const char *utf8, cairo_glyph_t **glyphs, int *num_glyphs);
cairo_status_t scaled_font_render_glyph (cairo_scaled_font_t *scaled_font, unsigned long glyph, cairo_t *cr, cairo_text_extents_t *metrics);

private:
SPFont* font;
UserFont* userfont;
std::vector<SPGlyph*> glyphs;
SPMissingGlyph* missingglyph;

bool drawing_expose_cb (Gtk::Widget *widget, GdkEventExpose *event, gpointer data);
};

#endif //#ifndef NR_SVGFONTS_H_SEEN
