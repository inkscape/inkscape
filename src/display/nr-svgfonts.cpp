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

#include <2geom/pathvector.h>
#include <2geom/transforms.h>
#include "../style.h"
#include <cairo.h>
#include <vector>
#include "svg/svg.h"
#include "inkscape-cairo.h"
#include "nr-svgfonts.h"

//*************************//
// UserFont Implementation //
//*************************//

// I wrote this binding code because Cairomm does not yet support userfonts. I have moved this code to cairomm and sent them a patch.
// Once Cairomm incorporate the UserFonts binding, this code should be removed from inkscape and Cairomm API should be used.

static cairo_user_data_key_t key;

static cairo_status_t font_init_cb (cairo_scaled_font_t  *scaled_font,
		       cairo_t */*cairo*/, cairo_font_extents_t *metrics){
	cairo_font_face_t*  face;
	face = cairo_scaled_font_get_font_face(scaled_font);
	SvgFont* instance = (SvgFont*) cairo_font_face_get_user_data(face, &key);
	return instance->scaled_font_init(scaled_font, metrics);
}

static cairo_status_t font_text_to_glyphs_cb (
                cairo_scaled_font_t  *scaled_font,
				const char	         *utf8,
				int                  utf8_len,
				cairo_glyph_t	     **glyphs,
				int		             *num_glyphs,
				cairo_text_cluster_t **clusters,
				int                  *num_clusters,
				cairo_text_cluster_flags_t         *flags){
	cairo_font_face_t*  face;
	face = cairo_scaled_font_get_font_face(scaled_font);
	SvgFont* instance = (SvgFont*) cairo_font_face_get_user_data(face, &key);
	return instance->scaled_font_text_to_glyphs(scaled_font, utf8, utf8_len, glyphs, num_glyphs, clusters, num_clusters, flags);
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

unsigned int size_of_substring(const char* substring, gchar* str){
	const gchar* original_substring = substring;

	while((g_utf8_get_char(substring)==g_utf8_get_char(str)) && g_utf8_get_char(substring) != 0 && g_utf8_get_char(str) != 0){
		substring = g_utf8_next_char(substring);
		str = g_utf8_next_char(str);
	}
	if (g_utf8_get_char(substring)==0)
		return substring - original_substring;
	else
		return 0;
}

//TODO: in these macros, verify what happens when using unicode strings.
#define Match_VKerning_Rule (((SPVkern*)node)->u1->contains(previous_unicode[0])\
		|| ((SPVkern*)node)->g1->contains(previous_glyph_name)) &&\
		(((SPVkern*)node)->u2->contains(this->glyphs[i]->unicode[0])\
		|| ((SPVkern*)node)->g2->contains(this->glyphs[i]->glyph_name.c_str()))

#define Match_HKerning_Rule (((SPHkern*)node)->u1->contains(previous_unicode[0])\
		|| ((SPHkern*)node)->g1->contains(previous_glyph_name)) &&\
		(((SPHkern*)node)->u2->contains(this->glyphs[i]->unicode[0])\
		|| ((SPHkern*)node)->g2->contains(this->glyphs[i]->glyph_name.c_str()))

cairo_status_t
SvgFont::scaled_font_text_to_glyphs (cairo_scaled_font_t *scaled_font,
				const char	*utf8,
				int             utf8_len,
				cairo_glyph_t	**glyphs,
				int		*num_glyphs,
				cairo_text_cluster_t **clusters,
				int                  *num_clusters,
				cairo_text_cluster_flags_t         *flags)
{
	//This function receives a text string to be rendered. It then defines what is the sequence of glyphs that
	// is used to properly render this string. It also defines the respective coordinates of each glyph. Thus, it
	// has to read the attributes of the SVGFont hkern and vkern nodes in order to adjust the glyph kerning.
	//It also determines the usage of the missing-glyph in portions of the string that does not match any of the declared glyphs.

    unsigned long i;
    int count = 0;
    gchar* _utf8 = (gchar*) utf8;
    unsigned int len;

    bool missing;
    //First we findout whats the number of glyphs needed.
    while(g_utf8_get_char(_utf8)){
		missing = true;
		for (i=0; i < (unsigned long) this->glyphs.size(); i++){
			if ( (len = size_of_substring(this->glyphs[i]->unicode.c_str(), _utf8)) ){
				//TODO: store this cluster
				_utf8+=len;
				count++;
				missing=false;
				break;
			}
		}
		if (missing){
			//TODO: store this cluster
			_utf8++;
			count++;
		}
    }


    //We use that info to allocate memory for the glyphs
    *glyphs = (cairo_glyph_t*) malloc(count*sizeof(cairo_glyph_t));

    char* previous_unicode = NULL; //This is used for kerning
    gchar* previous_glyph_name = NULL; //This is used for kerning

    count=0;
    double x=0, y=0;//These vars store the position of the glyph within the rendered string
    bool is_horizontal_text = true; //TODO
    _utf8 = (char*) utf8;

    while(g_utf8_get_char(_utf8)){
		len = 0;
        for (i=0; i < (unsigned long) this->glyphs.size(); i++){
			//check whether is there a glyph declared on the SVG document
			// that matches with the text string in its current position
			if ( (len = size_of_substring(this->glyphs[i]->unicode.c_str(), _utf8)) ){
				for(SPObject* node = this->font->children;previous_unicode && node;node=node->next){
					//apply glyph kerning if appropriate
					if (SP_IS_HKERN(node) && is_horizontal_text && Match_HKerning_Rule ){
						x -= (((SPHkern*)node)->k / 1000.0);//TODO: use here the height of the font
					}
					if (SP_IS_VKERN(node) && !is_horizontal_text && Match_VKerning_Rule ){
						y -= (((SPVkern*)node)->k / 1000.0);//TODO: use here the "height" of the font
					}
				}
				previous_unicode = (char*) this->glyphs[i]->unicode.c_str();//used for kerning checking
				previous_glyph_name = (char*) this->glyphs[i]->glyph_name.c_str();//used for kerning checking
				(*glyphs)[count].index = i;
				(*glyphs)[count].x = x;
				(*glyphs)[count++].y = y;

				//advance glyph coordinates:
				if (is_horizontal_text) x+=(this->font->horiz_adv_x/1000.0);//TODO: use here the height of the font
				else y+=(this->font->vert_adv_y/1000.0);//TODO: use here the "height" of the font
				_utf8+=len; //advance 'len' bytes in our string pointer
				//continue;
				goto raptorz;
            }
        }
raptorz:
		if (len==0){
			(*glyphs)[count].index = i;
			(*glyphs)[count].x = x;
			(*glyphs)[count++].y = y;

			//advance glyph coordinates:
			if (is_horizontal_text) x+=(this->font->horiz_adv_x/1000.0);//TODO: use here the height of the font
			else y+=(this->font->vert_adv_y/1000.0);//TODO: use here the "height" of the font

			_utf8 = g_utf8_next_char(_utf8); //advance 1 char in our string pointer
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
    } else {
        node = (SPObject*) this->glyphs[glyph];
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
		//adjust scale of the glyph
//        Geom::Scale s(1.0/((SPFont*) node->parent)->horiz_adv_x);
        Geom::Scale s(1.0/1000);//TODO: use here the units-per-em attribute?
		//This matrix flips the glyph vertically
        Geom::Matrix m(Geom::Coord(1),Geom::Coord(0),Geom::Coord(0),Geom::Coord(-1),Geom::Coord(0),Geom::Coord(0));
		//then we offset it
//		pathv += Geom::Point(Geom::Coord(0),Geom::Coord(-((SPFont*) node->parent)->horiz_adv_x));
		pathv += Geom::Point(Geom::Coord(0),Geom::Coord(-1000));//TODO: use here the units-per-em attribute?

        Geom::Rect area( Geom::Point(0,0), Geom::Point(1,1) ); //I need help here!    (reaction: note that the 'area' parameter is an *optional* rect, so you can pass an empty Geom::OptRect() )

        feed_pathvector_to_cairo (cr, pathv, s*m, area, false, 0);
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
                this->glyphs.push_back((SPGlyph*)node);
            }
            if (SP_IS_MISSING_GLYPH(node)){
                this->missingglyph=(SPMissingGlyph*)node;
            }
        }
	this->userfont = new UserFont(this);
    }
    return this->userfont->face;
}

void SvgFont::refresh(){
	this->glyphs.clear();
	delete this->userfont;
	this->userfont = NULL;
}

#endif //#ifdef ENABLE_SVG_FONTS
