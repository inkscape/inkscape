 /** \file
 * Native PDF import using libpoppler.
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

#ifdef HAVE_POPPLER

#include "goo/GooString.h"
#include "ErrorCodes.h"
#include "GlobalParams.h"
#include "PDFDoc.h"
#include "Page.h"
#include "Catalog.h"

#include "pdf-input.h"
#include "extension/system.h"
#include "extension/input.h"
#include "svg-builder.h"
#include "pdf-parser.h"

#include "document-private.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

/**
 * Parses the first page of the given PDF document using PdfParser.
 */
SPDocument *
PdfInput::open(::Inkscape::Extension::Input * mod, const gchar * uri) {

    // Initialize the globalParams variable for poppler
    if (!globalParams) {
        globalParams = new GlobalParams();
    }
    GooString *filename_goo = new GooString(uri);
    PDFDoc *pdf_doc = new PDFDoc(filename_goo, NULL, NULL, NULL);   // TODO: Could ask for password
    if (!pdf_doc->isOk()) {
        int error = pdf_doc->getErrorCode();
        delete pdf_doc;
        if (error == errEncrypted) {
            g_message("Document is encrypted.");
        } else {
            g_message("Failed to load document from data (error %d)", error);
        }
 
        return NULL;
    }

    // Get needed page
    int page_num = 1;
    Catalog *catalog = pdf_doc->getCatalog();
    Page *page = catalog->getPage(page_num);

    SPDocument *doc = sp_document_new(NULL, TRUE, TRUE);
    bool saved = sp_document_get_undo_sensitive(doc);
    sp_document_set_undo_sensitive(doc, false); // No need to undo in this temporary document

    // Create builder and parser
    SvgBuilder *builder = new SvgBuilder(doc, pdf_doc->getXRef());
    PdfParser *pdf_parser = new PdfParser(pdf_doc->getXRef(), builder, page_num-1, page->getRotate(),
                                          page->getResourceDict(), page->getCropBox());

    // Parse the document structure
    Object obj;
    page->getContents(&obj);
    if (!obj.isNull()) {
        pdf_parser->parse(&obj);
    }
    
    // Cleanup
    obj.free();
    delete pdf_parser;
    delete builder;
    delete pdf_doc;

    // Restore undo
    sp_document_set_undo_sensitive(doc, saved);

    return doc;
}

#include "../clear-n_.h"

void
PdfInput::init(void) {
    Inkscape::Extension::Extension * ext;

    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension>\n"
            "<name>PDF Input</name>\n"
            "<id>org.inkscape.input.pdf</id>\n"
            "<input>\n"
                "<extension>.pdf</extension>\n"
                "<mimetype>application/pdf</mimetype>\n"
                "<filetypename>Adobe PDF (*.pdf) [native]</filetypename>\n"
                "<filetypetooltip>PDF Document</filetypetooltip>\n"
            "</input>\n"
        "</inkscape-extension>", new PdfInput());
} // init

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_POPPLER */

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
