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

#include <gtkmm.h>
#include <libnr/nr-path.h>
#include "forward.h"
#include "extension/extension-forward.h"

struct SPPrintContext {
    Inkscape::Extension::Print *module;
};

unsigned int sp_print_bind(SPPrintContext *ctx, NR::Matrix const &transform, float opacity);
unsigned int sp_print_bind(SPPrintContext *ctx, NR::Matrix const *transform, float opacity);
unsigned int sp_print_release(SPPrintContext *ctx);
unsigned int sp_print_comment(SPPrintContext *ctx, char const *comment);
unsigned int sp_print_fill(SPPrintContext *ctx, NRBPath const *bpath, NR::Matrix const *ctm, SPStyle const *style,
                           NRRect const *pbox, NRRect const *dbox, NRRect const *bbox);
unsigned int sp_print_stroke(SPPrintContext *ctx, NRBPath const *bpath, NR::Matrix const *transform, SPStyle const *style,
                             NRRect const *pbox, NRRect const *dbox, NRRect const *bbox);

unsigned int sp_print_image_R8G8B8A8_N(SPPrintContext *ctx,
                                       guchar *px, unsigned int w, unsigned int h, unsigned int rs,
                                       NR::Matrix const *transform, SPStyle const *style);

unsigned int sp_print_text(SPPrintContext *ctx, char const *text, NR::Point p,
                           SPStyle const *style);

void sp_print_get_param(SPPrintContext *ctx, gchar *name, bool *value);


/* UI */
void sp_print_preview_document(SPDocument *doc);
void sp_print_document(Gtk::Window& parentWindow, SPDocument *doc);
void sp_print_document_to_file(SPDocument *doc, gchar const *filename);


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
