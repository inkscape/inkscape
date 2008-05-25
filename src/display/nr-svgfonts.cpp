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
#include "../sp-glyph.h"
#include "../sp-missing-glyph.h"
#include "../sp-font.h"
#include <cairo.h>
#include <vector>
#include "svg/svg.h"
#include "inkscape-cairo.h"
#include <gtk/gtk.h>

static std::vector<SPGlyph*> glyphs;
static SPMissingGlyph* missingglyph = NULL;
static std::vector<SPFont*> svgfonts;

cairo_font_face_t* nr_svgfonts_get_user_font_face();
void nr_svgfonts_append_spfont(SPFont* font);
static cairo_status_t scaled_font_init (cairo_scaled_font_t *scaled_font, cairo_font_extents_t *metrics);
static cairo_status_t scaled_font_unicode_to_glyph (cairo_scaled_font_t *scaled_font, unsigned long unicode, unsigned long *glyph);
static cairo_status_t scaled_font_render_glyph (cairo_scaled_font_t *scaled_font, unsigned long glyph, cairo_t *cr, cairo_text_extents_t *metrics);

void nr_svgfonts_append_spfont(SPFont* font){
    svgfonts.push_back(font);
    nr_svgfonts_get_user_font_face();
}

static cairo_status_t
scaled_font_init (cairo_scaled_font_t  *scaled_font,
		       cairo_font_extents_t *metrics)
{
//TODO
//  metrics->ascent  = .75;
//  metrics->descent = .25;
  return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
scaled_font_unicode_to_glyph (cairo_scaled_font_t *scaled_font,
				   unsigned long        unicode,
				   unsigned long       *glyph)
{
//g_warning("scaled_font_unicode_to_glyph CALL. unicode=%c", (char)unicode);

    unsigned long i;
    for (i=0; i < (unsigned long) glyphs.size(); i++){
        //TODO: compare unicode strings for ligadure glyphs (i.e. scaled_font_text_to_glyph)
        if ( ((unsigned long) glyphs[i]->unicode[0]) == unicode){
            *glyph = i;
            return CAIRO_STATUS_SUCCESS;
        }
    }
    *glyph = i;
    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
scaled_font_render_glyph (cairo_scaled_font_t  *scaled_font,
			       unsigned long         glyph,
			       cairo_t              *cr,
			       cairo_text_extents_t *metrics)
{
//g_warning("scaled_font_render_glyph CALL. glyph=%d", (int)glyph);

    if (glyph > glyphs.size())     return CAIRO_STATUS_SUCCESS;

    SPObject* node;
    if (glyph == glyphs.size()){
        if (!missingglyph) return CAIRO_STATUS_SUCCESS;
        node = (SPObject*) missingglyph;
        g_warning("RENDER MISSING-GLYPH");
    } else {
        node = (SPObject*) glyphs[glyph];
        g_warning("RENDER %c", glyphs[glyph]->unicode[0]);
    }

    NArtBpath *bpath = NULL;
    if (SP_IS_GLYPH(node)) bpath = sp_svg_read_path(((SPGlyph*)node)->d);
    if (SP_IS_MISSING_GLYPH(node)) bpath = sp_svg_read_path(((SPMissingGlyph*)node)->d);

    cairo_new_path(cr);

    NR::scale s(1.0/((SPFont*) node->parent)->horiz_adv_x);
    NR::Matrix t(s);
    NRRect area(0,0,1,1); //I need help here!
    feed_curve_to_cairo (cr, bpath, t, area.upgrade(), false, 0);
    cairo_fill(cr);

    return CAIRO_STATUS_SUCCESS;
}


bool drawing_expose_cb (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  cairo_t *cr = gdk_cairo_create (widget->window);

  cairo_set_font_face (cr, nr_svgfonts_get_user_font_face());
  cairo_set_font_size (cr, 200);
  cairo_move_to (cr, 200, 200);
  cairo_show_text (cr, "A@!A");

  cairo_move_to (cr, 200, 400);
  cairo_show_text (cr, "A!@A");

  cairo_destroy (cr);

  return TRUE;
}


cairo_font_face_t * nr_svgfonts_get_user_font_face(){
    static cairo_font_face_t *user_font_face = NULL;

    if (!user_font_face) {
        for(SPObject* node = svgfonts[0]->children;node;node=node->next){
            if (SP_IS_GLYPH(node)){
	        g_warning("glyphs.push_back((SPGlyph*)node); (node->unicode[0]='%c')", ((SPGlyph*)node)->unicode[0]);
                glyphs.push_back((SPGlyph*)node);
            }
            if (SP_IS_MISSING_GLYPH(node)){
	        g_warning("missingglyph=(SPMissingGlyph*)node;");
                missingglyph=(SPMissingGlyph*)node;
            }
        }
	user_font_face = cairo_user_font_face_create ();
	cairo_user_font_face_set_init_func             (user_font_face, scaled_font_init);
	cairo_user_font_face_set_render_glyph_func     (user_font_face, scaled_font_render_glyph);
	cairo_user_font_face_set_unicode_to_glyph_func (user_font_face, scaled_font_unicode_to_glyph);

        GtkWidget *window;
        GtkWidget *drawing;

        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_window_set_default_size (GTK_WINDOW(window), 1200, 850);
        drawing = gtk_drawing_area_new ();
        gtk_container_add (GTK_CONTAINER (window), drawing);
        gtk_widget_show_all (window);
        g_signal_connect (drawing, "expose-event", G_CALLBACK (drawing_expose_cb), NULL);
    }
    return user_font_face;
}
#endif //#ifdef ENABLE_SVG_FONTS

