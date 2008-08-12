#ifndef EXTENSION_INTERNAL_PS_H_SEEN
#define EXTENSION_INTERNAL_PS_H_SEEN

/** \file
 * Declaration of PrintPS, the internal module used to do Postscript Printing.
 */
/*
 * Authors:
 * 	   Lauris Kaplinski <lauris@kaplinski.com>
 * 	   Ted Gould <ted@gould.cx>
 *
 * Lauris' original code is in the public domain.
 * Ted's changes are licensed under the GNU GPL.
 */

#include <config.h>
#include "extension/extension.h"
#include "extension/implementation/implementation.h"
#include <set>
#include <string>

#include "libnr/nr-path.h"

#include <libnrtype/font-instance.h>

#include "svg/stringstream.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

typedef enum {FONT_TYPE1, FONT_TRUETYPE} FontType;

class PrintPS : public Inkscape::Extension::Implementation::Implementation {
    float _width;
    float _height;
    FILE * _begin_stream;//stream to print prolog and document setup of EPS, if font embedding
    FILE * _stream;//(main) stream to print the (E)PS output, or only the script part following prolog/document setup, if font embedding

    unsigned short _dpi;
    bool _bitmap;
    std::set<std::string> _latin1_encoded_fonts;
    bool _newlatin1font_proc_defined;

    GTree * _fonts;//maps fonts used in the document, to (value=)"TRUE" only if the font was effectively embedded, "FALSE" if not.

    //map strings of font types to enumeration of int values
    std::map<std::string, FontType> _fontTypesMap;

    void print_2geomcurve(SVGOStringStream &os, Geom::Curve const & c );
    void print_pathvector(SVGOStringStream &os, Geom::PathVector const &pathv);

    void print_fill_style(SVGOStringStream &os, SPStyle const *style, NRRect const *pbox);
    void print_stroke_style(SVGOStringStream &os, SPStyle const *style);

    char const *PSFontName(SPStyle const *style);
    bool embed_t1(SVGOStringStream &os, font_instance* font);
    bool embed_font(SVGOStringStream &os, font_instance* font);

    void print_glyphlist(SVGOStringStream &os, font_instance* font, Glib::ustring unistring);

    unsigned int print_image(FILE *ofp, guchar *px, unsigned int width, unsigned int height, unsigned int rs,
                             Geom::Matrix const *transform);
    void compress_packbits(int nin, guchar *src, int *nout, guchar *dst);

    /* ASCII 85 variables */
    guint32 ascii85_buf;
    int ascii85_len;
    int ascii85_linewidth;
    /* ASCII 85 Functions */
    void ascii85_init(void);
    void ascii85_flush(SVGOStringStream &os);
    inline void ascii85_out(guchar byte, SVGOStringStream &os);
    void ascii85_nout(int n, guchar *uptr, SVGOStringStream &os);
    void ascii85_done(SVGOStringStream &os);


public:
    PrintPS(void);
    virtual ~PrintPS(void);

    /* Print functions */
    virtual unsigned int setup(Inkscape::Extension::Print *module);
    /*
      virtual unsigned int set_preview(Inkscape::Extension::Print *module);
    */

    virtual unsigned int begin(Inkscape::Extension::Print *module, SPDocument *doc);
    virtual unsigned int finish(Inkscape::Extension::Print *module);

    /* Rendering methods */
    virtual unsigned int bind(Inkscape::Extension::Print *module, Geom::Matrix const *transform, float opacity);
    virtual unsigned int release(Inkscape::Extension::Print *module);
    virtual unsigned int comment(Inkscape::Extension::Print *module, char const *comment);
    virtual unsigned int fill(Inkscape::Extension::Print *module, Geom::PathVector const &pathv, Geom::Matrix const *ctm, SPStyle const *style,
                              NRRect const *pbox, NRRect const *dbox, NRRect const *bbox);
    virtual unsigned int stroke(Inkscape::Extension::Print *module, Geom::PathVector const &pathv, Geom::Matrix const *transform, SPStyle const *style,
                                NRRect const *pbox, NRRect const *dbox, NRRect const *bbox);
    virtual unsigned int image(Inkscape::Extension::Print *module, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
                               Geom::Matrix const *transform, SPStyle const *style);
    virtual unsigned int text(Inkscape::Extension::Print *module, char const *text,
                              Geom::Point p, SPStyle const *style);

    bool textToPath(Inkscape::Extension::Print *ext);
    static void init(void);
    bool fontEmbedded (Inkscape::Extension::Print * ext);
};

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */


#endif /* !EXTENSION_INTERNAL_PS_H_SEEN */

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
