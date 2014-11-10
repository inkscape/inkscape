#include "config.h"
/*
 * SVGFonts rendering implementation
 *
 * Authors:
 *    Felipe C. da S. Sanches <juca@members.fsf.org>
 *    Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL version 2 or later.
 * Read the file 'COPYING' for more information.
 */

#include <2geom/pathvector.h>
#include <2geom/transforms.h>
#include <cairo.h>
#include <vector>
#include "sp-object.h"
#include "svg/svg.h"
#include "display/cairo-utils.h"
#include "display/nr-svgfonts.h"
#include "display/nr-svgfonts.h"
#include "sp-path.h"
#include "sp-object-group.h"
#include "sp-use.h"
#include "sp-use-reference.h"
#include "display/curve.h"
#include "xml/repr.h"
#include "sp-font-face.h"
#include "sp-glyph.h"
#include "sp-missing-glyph.h"
#include "sp-font.h"
#include "sp-glyph-kerning.h"

// ************************//
// UserFont Implementation //
// ************************//

// I wrote this binding code because Cairomm does not yet support userfonts. I have moved this code to cairomm and sent them a patch.
// Once Cairomm incorporate the UserFonts binding, this code should be removed from inkscape and Cairomm API should be used.

static cairo_user_data_key_t key;

static cairo_status_t font_init_cb (cairo_scaled_font_t  *scaled_font,
                                    cairo_t * /*cairo*/, cairo_font_extents_t *metrics){
    cairo_font_face_t* face = cairo_scaled_font_get_font_face(scaled_font);
    SvgFont* instance = static_cast<SvgFont*>(cairo_font_face_get_user_data(face, &key));
    return instance->scaled_font_init(scaled_font, metrics);
}

static cairo_status_t font_text_to_glyphs_cb ( cairo_scaled_font_t  *scaled_font,
                                               const char           *utf8,
                                               int                  utf8_len,
                                               cairo_glyph_t        **glyphs,
                                               int                  *num_glyphs,
                                               cairo_text_cluster_t **clusters,
                                               int                  *num_clusters,
                                               cairo_text_cluster_flags_t *flags){
    cairo_font_face_t* face = cairo_scaled_font_get_font_face(scaled_font);
    SvgFont* instance = static_cast<SvgFont*>(cairo_font_face_get_user_data(face, &key));
    return instance->scaled_font_text_to_glyphs(scaled_font, utf8, utf8_len, glyphs, num_glyphs, clusters, num_clusters, flags);
}

static cairo_status_t font_render_glyph_cb (cairo_scaled_font_t  *scaled_font,
                                            unsigned long         glyph,
                                            cairo_t              *cr,
                                            cairo_text_extents_t *metrics){
    cairo_font_face_t* face = cairo_scaled_font_get_font_face(scaled_font);
    SvgFont* instance = static_cast<SvgFont*>(cairo_font_face_get_user_data(face, &key));
    return instance->scaled_font_render_glyph(scaled_font, glyph, cr, metrics);
}

UserFont::UserFont(SvgFont* instance){
    this->face = cairo_user_font_face_create ();
    cairo_user_font_face_set_init_func          (this->face, font_init_cb);
    cairo_user_font_face_set_render_glyph_func  (this->face, font_render_glyph_cb);
    cairo_user_font_face_set_text_to_glyphs_func(this->face, font_text_to_glyphs_cb);

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
SvgFont::scaled_font_init (cairo_scaled_font_t  */*scaled_font*/,
                           cairo_font_extents_t */*metrics*/)
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


namespace {

//TODO: in these functions, verify what happens when using unicode strings.

bool MatchVKerningRule(SPVkern const *vkern,
                       SPGlyph *glyph,
                       char const *previous_unicode,
                       gchar const *previous_glyph_name)
{
    bool value = (vkern->u1->contains(previous_unicode[0])
                  || vkern->g1->contains(previous_glyph_name))
        && (vkern->u2->contains(glyph->unicode[0])
            || vkern->g2->contains(glyph->glyph_name.c_str()));

    return value;
}

bool MatchHKerningRule(SPHkern const *hkern,
                       SPGlyph *glyph,
                       char const *previous_unicode,
                       gchar const *previous_glyph_name)
{
    bool value = (hkern->u1->contains(previous_unicode[0])
                  || hkern->g1->contains(previous_glyph_name))
        && (hkern->u2->contains(glyph->unicode[0])
            || hkern->g2->contains(glyph->glyph_name.c_str()));

    return value;
}

} // namespace

cairo_status_t
SvgFont::scaled_font_text_to_glyphs (cairo_scaled_font_t  */*scaled_font*/,
                                     const char           *utf8,
                                     int                  /*utf8_len*/,
                                     cairo_glyph_t        **glyphs,
                                     int                  *num_glyphs,
                                     cairo_text_cluster_t **/*clusters*/,
                                     int                  */*num_clusters*/,
                                     cairo_text_cluster_flags_t */*flags*/)
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
                    SPHkern *hkern = dynamic_cast<SPHkern *>(node);
                    if (hkern && is_horizontal_text && MatchHKerningRule(hkern, this->glyphs[i], previous_unicode, previous_glyph_name) ){
                        x -= (hkern->k / 1000.0);//TODO: use here the height of the font
                    }
                    SPVkern *vkern = dynamic_cast<SPVkern *>(node);
                    if (vkern && !is_horizontal_text && MatchVKerningRule(vkern, this->glyphs[i], previous_unicode, previous_glyph_name) ){
                        y -= (vkern->k / 1000.0);//TODO: use here the "height" of the font
                    }
                }
                previous_unicode = const_cast<char*>(this->glyphs[i]->unicode.c_str());//used for kerning checking
                previous_glyph_name = const_cast<char*>(this->glyphs[i]->glyph_name.c_str());//used for kerning checking
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

void
SvgFont::render_glyph_path(cairo_t* cr, Geom::PathVector* pathv){
    if (!pathv->empty()){
        //This glyph has a path description on its d attribute, so we render it:
        cairo_new_path(cr);

        //adjust scale of the glyph
//        Geom::Scale s(1.0/((SPFont*) node->parent)->horiz_adv_x);
        Geom::Scale s(1.0/1000);//TODO: use here the units-per-em attribute?

        Geom::Rect area( Geom::Point(0,0), Geom::Point(1,1) ); //I need help here!    (reaction: note that the 'area' parameter is an *optional* rect, so you can pass an empty Geom::OptRect() )

        feed_pathvector_to_cairo (cr, *pathv, s, area, false, 0);
        cairo_fill(cr);
    }
}

void
SvgFont::glyph_modified(SPObject* /* blah */, unsigned int /* bleh */){
    this->refresh();
    //TODO: update rendering on svgfonts preview widget (in the svg fonts dialog)
}

Geom::PathVector
SvgFont::flip_coordinate_system(SPFont* spfont, Geom::PathVector pathv){
    double units_per_em = 1000;
    for (SPObject *obj = spfont->children; obj; obj = obj->next){
        if (dynamic_cast<SPFontFace *>(obj)) {
            //XML Tree being directly used here while it shouldn't be.
            sp_repr_get_double(obj->getRepr(), "units_per_em", &units_per_em);
        }
    }

    double baseline_offset = units_per_em - spfont->horiz_origin_y;

    //This matrix flips y-axis and places the origin at baseline
    Geom::Affine m(Geom::Coord(1),Geom::Coord(0),Geom::Coord(0),Geom::Coord(-1),Geom::Coord(0),Geom::Coord(baseline_offset));
    return pathv*m;
}

cairo_status_t
SvgFont::scaled_font_render_glyph (cairo_scaled_font_t  */*scaled_font*/,
                                   unsigned long         glyph,
                                   cairo_t              *cr,
                                   cairo_text_extents_t */*metrics*/)
{
    // This method does the actual rendering of glyphs.

    // We have glyphs.size() glyphs and possibly one missing-glyph declared on this SVG document
    // The id of the missing-glyph is always equal to glyphs.size()
    // All the other glyphs have ids ranging from 0 to glyphs.size()-1

    if (glyph > this->glyphs.size())     return CAIRO_STATUS_SUCCESS;//TODO: this is an error!

    SPObject *node = NULL;
    if (glyph == glyphs.size()){
        if (!missingglyph) {
            return CAIRO_STATUS_SUCCESS;
        }
        node = missingglyph;
    } else {
        node = glyphs[glyph];
    }

    if (!dynamic_cast<SPGlyph *>(node) && !dynamic_cast<SPMissingGlyph *>(node)) {
        return CAIRO_STATUS_SUCCESS;  // FIXME: is this the right code to return?
    }

    SPFont* spfont = dynamic_cast<SPFont *>(node->parent);
    if (!spfont) {
        return CAIRO_STATUS_SUCCESS;  // FIXME: is this the right code to return?
    }

    //glyphs can be described by arbitrary SVG declared in the childnodes of a glyph node
    // or using the d attribute of a glyph node.
    // pathv stores the path description from the d attribute:
    Geom::PathVector pathv;
    
    SPGlyph *glyphNode = dynamic_cast<SPGlyph *>(node);
    if (glyphNode && glyphNode->d) {
        pathv = sp_svg_read_pathv(glyphNode->d);
        pathv = flip_coordinate_system(spfont, pathv);
        render_glyph_path(cr, &pathv);
    } else {
        SPMissingGlyph *missing = dynamic_cast<SPMissingGlyph *>(node);
        if (missing && missing->d) {
            pathv = sp_svg_read_pathv(missing->d);
            pathv = flip_coordinate_system(spfont, pathv);
            render_glyph_path(cr, &pathv);
        }
    }

    if (node->hasChildren()){
        //render the SVG described on this glyph's child nodes.
        for(node = node->children; node; node=node->next){
            {
                SPPath *path = dynamic_cast<SPPath *>(node);
                if (path) {
                    pathv = path->_curve->get_pathvector();
                    pathv = flip_coordinate_system(spfont, pathv);
                    render_glyph_path(cr, &pathv);
                }
            }
            if (dynamic_cast<SPObjectGroup *>(node)) {
                g_warning("TODO: svgfonts: render OBJECTGROUP");
            }
            SPUse *use = dynamic_cast<SPUse *>(node);
            if (use) {
                SPItem* item = use->ref->getObject();
                SPPath *path = dynamic_cast<SPPath *>(item);
                if (path) {
                    SPShape *shape = dynamic_cast<SPShape *>(item);
                    g_assert(shape != NULL);
                    pathv = shape->_curve->get_pathvector();
                    pathv = flip_coordinate_system(spfont, pathv);
                    this->render_glyph_path(cr, &pathv);
                }

                glyph_modified_connection = item->connectModified(sigc::mem_fun(*this, &SvgFont::glyph_modified));
            }
        }
    }

    return CAIRO_STATUS_SUCCESS;
}

cairo_font_face_t*
SvgFont::get_font_face(){
    if (!this->userfont) {
        for(SPObject* node = this->font->children;node;node=node->next){
            SPGlyph *glyph = dynamic_cast<SPGlyph *>(node);
            if (glyph) {
                glyphs.push_back(glyph);
            }
            SPMissingGlyph *missing = dynamic_cast<SPMissingGlyph *>(node);
            if (missing) {
                missingglyph = missing;
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

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
