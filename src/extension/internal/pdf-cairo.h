#ifndef EXTENSION_INTERNAL_PDF_CAIRO_H_SEEN
#define EXTENSION_INTERNAL_PDF_CAIRO_H_SEEN

/** \file
 * Declaration of PrintCairoPDF, the internal module used to do PDF printing with Cairo.
 */
/*
 * Authors:
 * 	   Lauris Kaplinski <lauris@kaplinski.com>
 * 	   Ted Gould <ted@gould.cx>
 *
 * Lauris' original code is in the public domain.
 * Ted's changes are licensed under the GNU GPL.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_CAIRO_PDF

#include "extension/extension.h"
#include "extension/implementation/implementation.h"
#include <set>
#include <string>

#include "libnr/nr-path.h"
#include "libnrtype/font-instance.h"

#include "svg/stringstream.h"
#include "sp-gradient.h"

#include <cairo.h>

namespace Inkscape {
namespace Extension {
namespace Internal {

class PrintCairoPDF : public Inkscape::Extension::Implementation::Implementation {
    float _width;
    float _height;
    FILE *_stream;
    cairo_t *cr;
    cairo_surface_t *pdf_surface;
    PangoLayout *_layout;
//    PangoContext *_context;
    std::vector<float> _alpha_stack;
    double _last_tx, _last_ty;
    
    unsigned short _dpi;
    bool _bitmap;

    cairo_pattern_t *create_pattern_for_paint(SPPaintServer const *const paintserver, NRRect const *pbox, float alpha);
    
    void print_fill_style(cairo_t *cr, SPStyle const *const style, NRRect const *pbox);
    void print_stroke_style(cairo_t *cr, SPStyle const *style, NRRect const *pbox);

#ifndef RENDER_WITH_PANGO_CAIRO
    NR::Point draw_glyphs(cairo_t *cr, NR::Point p, PangoFont *font, PangoGlyphString *glyph_string,
                          bool vertical, bool stroke);
#endif

public:
    PrintCairoPDF(void);
    virtual ~PrintCairoPDF(void);

    /* Print functions */
    virtual unsigned int setup(Inkscape::Extension::Print *module);
    /*
      virtual unsigned int set_preview(Inkscape::Extension::Print *module);
    */

    virtual unsigned int begin(Inkscape::Extension::Print *module, SPDocument *doc);
    virtual unsigned int finish(Inkscape::Extension::Print *module);

    /* Rendering methods */
    virtual unsigned int bind(Inkscape::Extension::Print *module, NR::Matrix const *transform, float opacity);
    virtual unsigned int release(Inkscape::Extension::Print *module);
    virtual unsigned int comment(Inkscape::Extension::Print *module, char const *comment);
    virtual unsigned int fill(Inkscape::Extension::Print *module, Geom::PathVector const &pathv, NR::Matrix const *ctm, SPStyle const *style,
                              NRRect const *pbox, NRRect const *dbox, NRRect const *bbox);
    virtual unsigned int stroke(Inkscape::Extension::Print *module, Geom::PathVector const &pathv, NR::Matrix const *transform, SPStyle const *style,
                                NRRect const *pbox, NRRect const *dbox, NRRect const *bbox);
    virtual unsigned int image(Inkscape::Extension::Print *module, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
                               NR::Matrix const *transform, SPStyle const *style);
    virtual unsigned int text(Inkscape::Extension::Print *module, char const *text,
                              NR::Point p, SPStyle const *style);

    bool textToPath(Inkscape::Extension::Print *ext);
    static void init(void);
};

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* HAVE_CAIRO_PDF */

#endif /* !EXTENSION_INTERNAL_PDF_CAIRO_H_SEEN */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
