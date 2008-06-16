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

#include <libnr/n-art-bpath-2geom.h>
#include <2geom/pathvector.h>
#include <2geom/transforms.h>
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
  cr->set_font_size (100);
  cr->move_to (100, 100);
  cr->show_text ("A@!A!@A");

  return TRUE;
}
};//class SvgFontDrawingArea

//*************************//
// UserFont Implementation //
//*************************//

// I wrote this binding code because Cairomm does not yet support userfonts. I have moved this code to cairomm and sent them a patch.
// Once Cairomm incorporate the UserFonts binding, this code should be removed from inkscape and Cairomm API should be used.

static cairo_user_data_key_t key;

static cairo_status_t font_init_cb (cairo_scaled_font_t  *scaled_font,
		       cairo_font_extents_t *metrics){
	cairo_font_face_t*  face;
	face = cairo_scaled_font_get_font_face(scaled_font);
	SvgFont* instance = (SvgFont*) cairo_font_face_get_user_data(face, &key);
	return instance->scaled_font_init(scaled_font, metrics);
}

static cairo_status_t font_text_to_glyphs_cb (cairo_scaled_font_t *scaled_font,
				const char	*utf8,
				cairo_glyph_t	**glyphs,
				int		*num_glyphs){
	cairo_font_face_t*  face;
	face = cairo_scaled_font_get_font_face(scaled_font);
	SvgFont* instance = (SvgFont*) cairo_font_face_get_user_data(face, &key);
	return instance->scaled_font_text_to_glyphs(scaled_font, utf8, glyphs, num_glyphs);
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
	cairo_user_font_face_set_init_func		(this->face, font_init_cb);
	cairo_user_font_face_set_render_glyph_func	(this->face, font_render_glyph_cb);
	cairo_user_font_face_set_text_to_glyphs_func	(this->face, font_text_to_glyphs_cb);

	cairo_font_face_set_user_data (this->face, &key, (void*)instance, (cairo_destroy_func_t) NULL);
}

//******************************//
// SvgFont class Implementation //
//******************************//
SvgFont::SvgFont(SPFont* spfont){
	this->font = spfont;
	this->missingglyph = NULL;
	this->userfont = NULL;

	//This is an auxiliary gtkWindow used only while we do not have proper Pango integration with cairo-user-fonts.
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

unsigned int compare_them(char* s1, char* s2){
	unsigned int p=0;
	while((s1[p] == s2[p]) && s1[p] != '\0' && s2[p] != '\0') p++;
	if (s1[p]=='\0') return p;
	else return 0;
}

cairo_status_t
SvgFont::scaled_font_text_to_glyphs (cairo_scaled_font_t *scaled_font,
				const char	*utf8,
				cairo_glyph_t	**glyphs,
				int		*num_glyphs)
{
	//This function receives a text string to be rendered. It then defines what is the sequence of glyphs that
	// is used to properly render this string. It alse defines the respective coordinates of each glyph. Thus, it
	// has to read the attributes od the SVGFont hkern and vkern nodes in order to adjust the glyph kerning.
	//It also determines the usage of the missing-glyph in portions of the string that does not match any of the declared glyphs.

    unsigned long i;
    int count = 0;
    char* _utf8 = (char*) utf8;
    unsigned int len;

    //First we findout whats the worst case number of glyphs.
    while(_utf8[0] != '\0'){
	_utf8++;
	count++;
    }

    //We use that info to allocate memory for the glyphs
    *glyphs = (cairo_glyph_t*) malloc(count*sizeof(cairo_glyph_t));

    char* previous_unicode = NULL; //This is used for kerning 
    count=0;
    double x=0, y=0;//These vars store the position of the glyph within the rendered string
    bool is_horizontal_text = true; //TODO
    _utf8 = (char*) utf8;
    while(_utf8[0] != '\0'){
	len = 0;
        for (i=0; i < (unsigned long) this->glyphs.size(); i++){
            if ( (len = compare_them(this->glyphs[i]->unicode, _utf8)) ){
		//check whether is there a glyph declared on the SVG document
		// that matches with the text string in its current position
	        for(SPObject* node = this->font->children;previous_unicode && node;node=node->next){
		    //apply glyph kerning if appropriate
	            if (SP_IS_HKERN(node) && is_horizontal_text){
	                if ( (((SPHkern*)node)->u1->contains(previous_unicode[0])) && (((SPHkern*)node)->u2->contains(this->glyphs[i]->unicode[0]) ))//TODO: verify what happens when using unicode strings.
				x -= (((SPHkern*)node)->k / this->font->horiz_adv_x);
	            }
		    if (SP_IS_VKERN(node) && !is_horizontal_text){
	                if ( (((SPVkern*)node)->u1->contains(previous_unicode[0])) && (((SPVkern*)node)->u2->contains(this->glyphs[i]->unicode[0]) ))//TODO: idem
				y -= (((SPVkern*)node)->k / this->font->vert_adv_y);
	            }
		}
		previous_unicode = this->glyphs[i]->unicode;//used for kerning checking
                (*glyphs)[count].index = i;
                (*glyphs)[count].x = x;
                (*glyphs)[count++].y = y;
		//advance glyph coordinates:
		if (is_horizontal_text) x++;
		else y++;
                _utf8+=len; //advance 'len' chars in our string pointer
		continue;
            }
        }
	if (!len){
        	(*glyphs)[count].index = i;
        	(*glyphs)[count].x = x;
        	(*glyphs)[count++].y = y;
		//advance glyph coordinates:
		if (is_horizontal_text) x++;
		else y++;
		_utf8++; //advance 1 char in our string pointer
	}
    }
    *num_glyphs = count;
    return CAIRO_STATUS_SUCCESS;
}

cairo_status_t
SvgFont::scaled_font_render_glyph (cairo_scaled_font_t  *scaled_font,
			       unsigned long         glyph,
			       cairo_t              *cr,
			       cairo_text_extents_t *metrics)
{
    // This method does the actual rendering of glyphs.

    // We have glyphs.size() glyphs and possibly one missing-glyph declared on this SVG document 
    // The id of the missing-glyph is always equal to glyphs.size()
    // All the other glyphs have ids ranging from 0 to glyphs.size()-1

    if (glyph > this->glyphs.size())     return CAIRO_STATUS_SUCCESS;//TODO: this is an error!

    SPObject* node;
    if (glyph == this->glyphs.size()){
        if (!this->missingglyph) return CAIRO_STATUS_SUCCESS;
        node = (SPObject*) this->missingglyph;
        g_warning("RENDER MISSING-GLYPH");
    } else {
        node = (SPObject*) this->glyphs[glyph];
        g_warning("RENDER %s", this->glyphs[glyph]->unicode);
    }

    //glyphs can be described by arbitrary SVG declared in the childnodes of a glyph node
    // or using the d attribute of a glyph node.
    // pathv stores the path description from the d attribute:
    Geom::PathVector pathv;
    if (SP_IS_GLYPH(node) && ((SPGlyph*)node)->d) {
        pathv = sp_svg_read_pathv(((SPGlyph*)node)->d);
    } else if (SP_IS_MISSING_GLYPH(node) && ((SPMissingGlyph*)node)->d) {
        pathv = sp_svg_read_pathv(((SPMissingGlyph*)node)->d);
    } else {
        return CAIRO_STATUS_SUCCESS;  // FIXME: is this the right code to return?
    }

    if (!pathv.empty()){
        //This glyph has a path description on its d attribute, so we render it:
        cairo_new_path(cr);
        Geom::Scale s(1.0/((SPFont*) node->parent)->horiz_adv_x);
        NRRect area(0,0,1,1); //I need help here!
        feed_pathvector_to_cairo (cr, pathv, s, area.upgrade(), false, 0);
        cairo_fill(cr);
    }

    //TODO: render the SVG described on this glyph's child nodes.
    return CAIRO_STATUS_SUCCESS;
}

cairo_font_face_t*
SvgFont::get_font_face(){
    if (!this->userfont) {
        for(SPObject* node = this->font->children;node;node=node->next){
            if (SP_IS_GLYPH(node)){
	        g_warning("glyphs.push_back((SPGlyph*)node); (node->unicode='%s')", ((SPGlyph*)node)->unicode);
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
