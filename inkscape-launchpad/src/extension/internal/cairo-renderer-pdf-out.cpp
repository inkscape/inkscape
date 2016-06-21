/*
 * A quick hack to use the Cairo renderer to write out a file.  This
 * then makes 'save as...' PDF.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Ulf Erikson <ulferikson@users.sf.net>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004-2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_CAIRO_PDF

#include "cairo-renderer-pdf-out.h"
#include "cairo-render-context.h"
#include "cairo-renderer.h"
#include "latex-text-renderer.h"
#include <print.h>
#include "extension/system.h"
#include "extension/print.h"
#include "extension/db.h"
#include "extension/output.h"
#include "display/drawing.h"

#include "display/curve.h"
#include "display/canvas-bpath.h"
#include "sp-item.h"
#include "sp-root.h"

#include <2geom/affine.h>
#include "document.h"

#include "util/units.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

bool CairoRendererPdfOutput::check(Inkscape::Extension::Extension * /*module*/)
{
    bool result = true;

    if (NULL == Inkscape::Extension::db.get("org.inkscape.output.pdf.cairorenderer")) {
        result = false;
    }

    return result;
}

static bool
pdf_render_document_to_file(SPDocument *doc, gchar const *filename, unsigned int level,
                            bool texttopath, bool omittext, bool filtertobitmap, int resolution,
                            const gchar * const exportId, bool exportDrawing, bool exportCanvas, float bleedmargin_px)
{
    doc->ensureUpToDate();

/* Start */

    SPItem *base = NULL;

    bool pageBoundingBox = TRUE;
    if (exportId && strcmp(exportId, "")) {
        // we want to export the given item only
        base = SP_ITEM(doc->getObjectById(exportId));
        pageBoundingBox = exportCanvas;
    }
    else {
        // we want to export the entire document from root
        base = doc->getRoot();
        pageBoundingBox = !exportDrawing;
    }

    if (!base) {
        return false;
    }
    
    /* Create new arena */
    Inkscape::Drawing drawing;
    drawing.setExact(true);
    unsigned dkey = SPItem::display_key_new(1);
    base->invoke_show(drawing, dkey, SP_ITEM_SHOW_DISPLAY);

    /* Create renderer and context */
    CairoRenderer *renderer = new CairoRenderer();
    CairoRenderContext *ctx = renderer->createContext();
    ctx->setPDFLevel(level);
    ctx->setTextToPath(texttopath);
    ctx->setOmitText(omittext);
    ctx->setFilterToBitmap(filtertobitmap);
    ctx->setBitmapResolution(resolution);

    bool ret = ctx->setPdfTarget (filename);
    if(ret) {
        /* Render document */
        ret = renderer->setupDocument(ctx, doc, pageBoundingBox, bleedmargin_px, base);
        if (ret) {
            renderer->renderItem(ctx, base);
            ret = ctx->finish();
        }
    }

    base->invoke_hide(dkey);

    renderer->destroyContext(ctx);
    delete renderer;

    return ret;
}

/**
    \brief  This function calls the output module with the filename
    \param  mod   unused
    \param  doc   Document to be saved
    \param  filename   Filename to save to (probably will end in .pdf)

    The most interesting thing that this function does is just attach
    an '>' on the front of the filename.  This is the syntax used to
    tell the printing system to save to file.
*/
void
CairoRendererPdfOutput::save(Inkscape::Extension::Output *mod, SPDocument *doc, gchar const *filename)
{
    Inkscape::Extension::Extension * ext;
    unsigned int ret;

    ext = Inkscape::Extension::db.get("org.inkscape.output.pdf.cairorenderer");
    if (ext == NULL)
        return;

    int level = 0;
    try {
        const gchar *new_level = mod->get_param_enum("PDFversion");
        if((new_level != NULL) && (g_ascii_strcasecmp("PDF-1.5", new_level) == 0)) {
            level = 1;
        }
    }
    catch(...) {
        g_warning("Parameter <PDFversion> might not exist");
    }

    bool new_textToPath  = FALSE;
    try {
        new_textToPath = (strcmp(mod->get_param_optiongroup("textToPath"), "paths") == 0);
    }
    catch(...) {
        g_warning("Parameter <textToPath> might not exist");
    }

    bool new_textToLaTeX  = FALSE;
    try {
        new_textToLaTeX = (strcmp(mod->get_param_optiongroup("textToPath"), "LaTeX") == 0);
    }
    catch(...) {
        g_warning("Parameter <textToLaTeX> might not exist");
    }

    bool new_blurToBitmap  = FALSE;
    try {
        new_blurToBitmap  = mod->get_param_bool("blurToBitmap");
    }
    catch(...) {
        g_warning("Parameter <blurToBitmap> might not exist");
    }

    int new_bitmapResolution  = 72;
    try {
        new_bitmapResolution = mod->get_param_int("resolution");
    }
    catch(...) {
        g_warning("Parameter <resolution> might not exist");
    }

    const gchar *new_exportId = NULL;
    try {
        new_exportId = mod->get_param_string("exportId");
    }
    catch(...) {
        g_warning("Parameter <exportId> might not exist");
    }

    bool new_exportCanvas  = true;
    try {
        new_exportCanvas = (strcmp(ext->get_param_optiongroup("area"), "page") == 0);
    } catch(...) {
        g_warning("Parameter <area> might not exist");
    }
    bool new_exportDrawing  = !new_exportCanvas;

    float new_bleedmargin_px = 0.;
    try {
        new_bleedmargin_px = Inkscape::Util::Quantity::convert(mod->get_param_float("bleed"), "mm", "px");
    }
    catch(...) {
        g_warning("Parameter <bleed> might not exist");
    }

    // Create PDF file
    {
        gchar * final_name;
        final_name = g_strdup_printf("> %s", filename);
        ret = pdf_render_document_to_file(doc, final_name, level,
                                          new_textToPath, new_textToLaTeX, new_blurToBitmap, new_bitmapResolution,
                                          new_exportId, new_exportDrawing, new_exportCanvas, new_bleedmargin_px);
        g_free(final_name);

        if (!ret)
            throw Inkscape::Extension::Output::save_failed();
    }

    // Create LaTeX file (if requested)
    if (new_textToLaTeX) {
        ret = latex_render_document_text_to_file(doc, filename, new_exportId, new_exportDrawing, new_exportCanvas, new_bleedmargin_px, true);

        if (!ret)
            throw Inkscape::Extension::Output::save_failed();
    }
}

#include "clear-n_.h"

/**
	\brief   A function allocate a copy of this function.

	This is the definition of Cairo PDF out.  This function just
	calls the extension system with the memory allocated XML that
	describes the data.
*/
void
CairoRendererPdfOutput::init (void)
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>Portable Document Format</name>\n"
            "<id>org.inkscape.output.pdf.cairorenderer</id>\n"
            "<param name=\"PDFversion\" _gui-text=\"" N_("Restrict to PDF version:") "\" type=\"enum\" >\n"
#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 10, 0))
                "<_item value='PDF-1.5'>" N_("PDF 1.5") "</_item>\n"
#endif
                "<_item value='PDF-1.4'>" N_("PDF 1.4") "</_item>\n"
            "</param>\n"
            "<param name=\"textToPath\" _gui-text=\"" N_("Text output options:") "\" type=\"optiongroup\">\n"
                "<_option value=\"embed\">" N_("Embed fonts") "</_option>\n"
                "<_option value=\"paths\">" N_("Convert text to paths") "</_option>\n"
                "<_option value=\"LaTeX\">" N_("Omit text in PDF and create LaTeX file") "</_option>\n"
            "</param>\n"
            "<param name=\"blurToBitmap\" _gui-text=\"" N_("Rasterize filter effects") "\" type=\"boolean\">true</param>\n"
            "<param name=\"resolution\" _gui-text=\"" N_("Resolution for rasterization (dpi):") "\" type=\"int\" min=\"1\" max=\"10000\">96</param>\n"
            "<param name=\"area\" _gui-text=\"" N_("Output page size:") "\" type=\"optiongroup\" >\n"
                "<_option value=\"page\">" N_("Use document's page size") "</_option>"
                "<_option value=\"drawing\">" N_("Use exported object's size") "</_option>"
            "</param>"
            "<param name=\"bleed\" _gui-text=\"" N_("Bleed/margin (mm):") "\" type=\"float\" min=\"-10000\" max=\"10000\">0</param>\n"
            "<param name=\"exportId\" _gui-text=\"" N_("Limit export to the object with ID:") "\" type=\"string\"></param>\n"
            "<output>\n"
                "<extension>.pdf</extension>\n"
                "<mimetype>application/pdf</mimetype>\n"
                "<filetypename>Portable Document Format (*.pdf)</filetypename>\n"
                "<filetypetooltip>PDF File</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>", new CairoRendererPdfOutput());

    return;
}

} } }  /* namespace Inkscape, Extension, Internal */

#endif /* HAVE_CAIRO_PDF */
