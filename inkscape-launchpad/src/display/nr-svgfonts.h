#ifndef NR_SVGFONTS_H_SEEN
#define NR_SVGFONTS_H_SEEN
/*
 * SVGFonts rendering headear
 *
 * Authors:
 *    Felipe C. da S. Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL version 2 or later.
 * Read the file 'COPYING' for more information.
 */

#include <cairo.h>
#include <sigc++/connection.h>

class SvgFont;
class SPFont;
class SPGlyph;
class SPMissingGlyph;

extern "C" { typedef struct _GdkEventExpose GdkEventExpose; }

namespace Gtk {
class Widget;
}

class UserFont {
public:
    UserFont(SvgFont* instance);
    cairo_font_face_t* face;
};

class SvgFont {
public:
    SvgFont(SPFont* spfont);
    void refresh();
    cairo_font_face_t* get_font_face();
    cairo_status_t scaled_font_init (cairo_scaled_font_t *scaled_font, cairo_font_extents_t *metrics);
    cairo_status_t scaled_font_text_to_glyphs (cairo_scaled_font_t *scaled_font, const char *utf8, int utf8_len, cairo_glyph_t **glyphs, int *num_glyphs, cairo_text_cluster_t **clusters, int *num_clusters, cairo_text_cluster_flags_t *flags);
    cairo_status_t scaled_font_render_glyph (cairo_scaled_font_t *scaled_font, unsigned long glyph, cairo_t *cr, cairo_text_extents_t *metrics);

    Geom::PathVector flip_coordinate_system(SPFont* spfont, Geom::PathVector pathv);
    void render_glyph_path(cairo_t* cr, Geom::PathVector* pathv);
    void glyph_modified(SPObject *, unsigned int);

private:
    SPFont* font;
    UserFont* userfont;
    std::vector<SPGlyph*> glyphs;
    SPMissingGlyph* missingglyph;
    sigc::connection glyph_modified_connection;

    bool drawing_expose_cb (Gtk::Widget *widget, GdkEventExpose *event, void* data);
};

#endif //#ifndef NR_SVGFONTS_H_SEEN

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
