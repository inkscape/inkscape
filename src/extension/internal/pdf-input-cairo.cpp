 /*
 * Simple PDF import extension using libpoppler and Cairo's SVG surface.
 *
 * Authors:
 *   miklos erdelyi
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_POPPLER_GLIB

#include "pdf-input-cairo.h"
#include "extension/system.h"
#include "extension/input.h"
#include "document.h"

#include <cairo-svg.h>
#include <poppler/glib/poppler.h>
#include <poppler/glib/poppler-document.h>
#include <poppler/glib/poppler-page.h>

namespace Inkscape {
namespace Extension {
namespace Internal {

static cairo_status_t _write_ustring_cb(void *closure, const unsigned char *data, unsigned int length);

SPDocument *
PdfInputCairo::open(Inkscape::Extension::Input * /*mod*/, const gchar * uri) {

    printf("Attempting to open using PdfInputCairo\n");

    gchar* filename_uri = g_filename_to_uri(uri, NULL, NULL);

    PopplerDocument* document = poppler_document_new_from_file(filename_uri, NULL, NULL);
    if (document == NULL)
        return NULL;

    double width, height;
    PopplerPage* page = poppler_document_get_page(document, 0);
    poppler_page_get_size(page, &width, &height);

    Glib::ustring* output = new Glib::ustring("");
    cairo_surface_t* surface = cairo_svg_surface_create_for_stream(Inkscape::Extension::Internal::_write_ustring_cb,
                                                                   output, width, height);
    cairo_t* cr = cairo_create(surface);

    poppler_page_render(page, cr);
    cairo_show_page(cr);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    SPDocument * doc = sp_document_new_from_mem(output->c_str(), output->length(), TRUE);

    delete output;
    g_object_unref(page);
    g_object_unref(document);

    return doc;
}

static cairo_status_t
        _write_ustring_cb(void *closure, const unsigned char *data, unsigned int length)
{
    Glib::ustring* stream = (Glib::ustring*)closure;
    stream->append((const char*)data, length);

    return CAIRO_STATUS_SUCCESS;
}


#include "clear-n_.h"

void
PdfInputCairo::init(void) {
    Inkscape::Extension::Extension * ext;

    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>PDF Input</name>\n"
            "<id>org.inkscape.input.pdf</id>\n"
            "<input>\n"
                "<extension>.pdf</extension>\n"
                "<mimetype>application/pdf</mimetype>\n"
                "<filetypename>Adobe PDF (*.pdf)</filetypename>\n"
                "<filetypetooltip>PDF Document</filetypetooltip>\n"
            "</input>\n"
        "</inkscape-extension>", new PdfInputCairo());
} // init

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_POPPLER_GLIB */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
