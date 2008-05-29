#include "config.h"
#ifdef ENABLE_SVG_FONTS
/*
 * SVGFonts rendering implementation
 *
 * Authors:
 *    Felipe C. da S. Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL version 2 or later.
 * Read the file 'COPYING' for more information.
 */

#include <libnr/n-art-bpath.h>
#include "../style.h"
#include <cairo.h>
#include <vector>
#include "svg/svg.h"
#include "inkscape-cairo.h"
#include "nr-svgfonts.h"

//***********************************//
// SvgFontDrawingArea Implementation //
//***********************************//
class SvgFontDrawingArea : Gtk::DrawingArea{
public:
SvgFontDrawingArea(SvgFont* svgfont){
	this->svgfont = svgfont;
}
private:
SvgFont* svgfont;

bool on_expose_event (GdkEventExpose *event){
  Glib::RefPtr<Gdk::Window> window = get_window();
  Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
  cr->set_font_face( Cairo::RefPtr<Cairo::FontFace>(new Cairo::FontFace(this->svgfont->get_font_face(), false /* does not have reference */)) );
  cr->set_font_size (200);
  cr->move_to (200, 200);
  cr->show_text ("A@!A");

  cr->move_to (200, 400);
  cr->show_text ("A!@A");

  return TRUE;
}
};//class SvgFontDrawingArea

//*************************//
// UserFont Implementation //
//*************************//
static cairo_user_data_key_t key;

static cairo_status_t font_init_cb (cairo_scaled_font_t  *scaled_font,
		       cairo_font_extents_t *metrics){
	cairo_font_face_t*  face;
	face = cairo_scaled_font_get_font_face(scaled_font);
	SvgFont* instance = (SvgFont*) cairo_font_face_get_user_data(face, &key);
	return instance->scaled_font_init(scaled_font, metrics);
}

static cairo_status_t font_unicode_to_glyph_cb (cairo_scaled_font_t *scaled_font,
				   unsigned long        unicode,
				   unsigned long       *glyph){
	cairo_font_face_t*  face;
	face = cairo_scaled_font_get_font_face(scaled_font);
	SvgFont* instance = (SvgFont*) cairo_font_face_get_user_data(face, &key);
	return instance->scaled_font_unicode_to_glyph(scaled_font, unicode, glyph);
}

static cairo_status_t font_render_glyph_cb (cairo_scaled_font_t  *scaled_font,
			       unsigned long         glyph,
			       cairo_t              *cr,
			       cairo_text_extents_t *metrics){
	cairo_font_face_t*  face;
	face = cairo_scaled_font_get_font_face(scaled_font);
	SvgFont* instance = (SvgFont*) cairo_font_face_get_user_data(face, &key);
	return instance->scaled_font_render_glyph(scaled_font, glyph, cr, metrics);
}

UserFont::UserFont(SvgFont* instance){
	this->face = cairo_user_font_face_create ();
	cairo_user_font_face_set_init_func             (this->face, font_init_cb);
	cairo_user_font_face_set_render_glyph_func     (this->face, font_render_glyph_cb);
	cairo_user_font_face_set_unicode_to_glyph_func (this->face, font_unicode_to_glyph_cb);

	cairo_font_face_set_user_data (this->face, &key, (void*)instance, (cairo_destroy_func_t) NULL);
}

//******************************//
// SvgFont class Implementation //
//******************************//
SvgFont::SvgFont(SPFont* spfont){
	this->font = spfont;
	this->missingglyph = NULL;
	this->userfont = NULL;

        Gtk::Window* window;
        SvgFontDrawingArea* font_da;

        window = new Gtk::Window();
        window->set_default_size (1200, 850);
        font_da = new SvgFontDrawingArea(this);
        window->add((Gtk::Widget&) *font_da);
        window->show_all();
}

cairo_status_t
SvgFont::scaled_font_init (cairo_scaled_font_t  *scaled_font,
		       cairo_font_extents_t *metrics)
{
//TODO
//  metrics->ascent  = .75;
//  metrics->descent = .25;
  return CAIRO_STATUS_SUCCESS;
}

cairo_status_t
SvgFont::scaled_font_unicode_to_glyph (cairo_scaled_font_t *scaled_font,
				   unsigned long        unicode,
				   unsigned long       *glyph)
{
//g_warning("scaled_font_unicode_to_glyph CALL. unicode=%c", (char)unicode);

    unsigned long i;
    for (i=0; i < (unsigned long) this->glyphs.size(); i++){
        //TODO: compare unicode strings for ligadure glyphs (i.e. scaled_font_text_to_glyph)
        if ( ((unsigned long) this->glyphs[i]->unicode[0]) == unicode){
            *glyph = i;
            return CAIRO_STATUS_SUCCESS;
        }
    }
    *glyph = i;
    return CAIRO_STATUS_SUCCESS;
}

cairo_status_t
SvgFont::scaled_font_render_glyph (cairo_scaled_font_t  *scaled_font,
			       unsigned long         glyph,
			       cairo_t              *cr,
			       cairo_text_extents_t *metrics)
{
//g_warning("scaled_font_render_glyph CALL. glyph=%d", (int)glyph);

    if (glyph > this->glyphs.size())     return CAIRO_STATUS_SUCCESS;

    SPObject* node;
    if (glyph == this->glyphs.size()){
        if (!this->missingglyph) return CAIRO_STATUS_SUCCESS;
        node = (SPObject*) this->missingglyph;
        g_warning("RENDER MISSING-GLYPH");
    } else {
        node = (SPObject*) this->glyphs[glyph];
        g_warning("RENDER %c", this->glyphs[glyph]->unicode[0]);
    }

    NArtBpath *bpath = NULL;
    if (SP_IS_GLYPH(node) && ((SPGlyph*)node)->d) bpath = sp_svg_read_path(((SPGlyph*)node)->d);
    if (SP_IS_MISSING_GLYPH(node) && ((SPMissingGlyph*)node)->d) bpath = sp_svg_read_path(((SPMissingGlyph*)node)->d);
    
    if (!bpath) return CAIRO_STATUS_SUCCESS;

    cairo_new_path(cr);

    NR::scale s(1.0/((SPFont*) node->parent)->horiz_adv_x);
    NR::Matrix t(s);
    NRRect area(0,0,1,1); //I need help here!
    feed_curve_to_cairo (cr, bpath, t, area.upgrade(), false, 0);
    cairo_fill(cr);

    return CAIRO_STATUS_SUCCESS;
}

cairo_font_face_t*
SvgFont::get_font_face(){
    if (!this->userfont) {
        for(SPObject* node = this->font->children;node;node=node->next){
            if (SP_IS_GLYPH(node)){
	        g_warning("glyphs.push_back((SPGlyph*)node); (node->unicode[0]='%c')", ((SPGlyph*)node)->unicode[0]);
                this->glyphs.push_back((SPGlyph*)node);
            }
            if (SP_IS_MISSING_GLYPH(node)){
	        g_warning("missingglyph=(SPMissingGlyph*)node;");
                this->missingglyph=(SPMissingGlyph*)node;
            }
        }
	this->userfont = new UserFont(this);
    }
    return this->userfont->face;
}
#endif //#ifdef ENABLE_SVG_FONTS

