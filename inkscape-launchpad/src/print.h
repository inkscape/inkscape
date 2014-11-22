#ifndef PRINT_H_INKSCAPE
#define PRINT_H_INKSCAPE

/** \file
 * Frontend to printing
 */
/*
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <2geom/forward.h>

namespace Gtk {
class Window;
}

class SPDocument;
class SPStyle;

namespace Inkscape {
namespace Extension {

class Print;

} // namespace Extension
} // namespace Inkscape

struct SPPrintContext {
    Inkscape::Extension::Print *module;
};

unsigned int sp_print_bind(SPPrintContext *ctx, Geom::Affine const &transform, float opacity);
unsigned int sp_print_release(SPPrintContext *ctx);
unsigned int sp_print_comment(SPPrintContext *ctx, char const *comment);
unsigned int sp_print_fill(SPPrintContext *ctx, Geom::PathVector const &pathv, Geom::Affine const &ctm, SPStyle const *style,
                           Geom::OptRect const &pbox, Geom::OptRect const &dbox, Geom::OptRect const &bbox);
unsigned int sp_print_stroke(SPPrintContext *ctx, Geom::PathVector const &pathv, Geom::Affine const &ctm, SPStyle const *style,
                             Geom::OptRect const &pbox, Geom::OptRect const &dbox, Geom::OptRect const &bbox);

unsigned int sp_print_image_R8G8B8A8_N(SPPrintContext *ctx,
                                       unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
                                       Geom::Affine const &transform, SPStyle const *style);

unsigned int sp_print_text(SPPrintContext *ctx, char const *text, Geom::Point p,
                           SPStyle const *style);

void sp_print_get_param(SPPrintContext *ctx, char *name, bool *value);


/* UI */
void sp_print_document(Gtk::Window& parentWindow, SPDocument *doc);
void sp_print_document_to_file(SPDocument *doc, char const *filename);


#endif /* !PRINT_H_INKSCAPE */

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
