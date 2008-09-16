/*
 * A quick hack to use the Cairo renderer to write out a file.  This
 * then makes 'save as...' PDF.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Ulf Erikson <ulferikson@users.sf.net>
 *
 * Copyright (C) 2004-2006 Authors
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
#include <print.h>
#include "extension/system.h"
#include "extension/print.h"
#include "extension/db.h"
#include "extension/output.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"

#include "display/curve.h"
#include "display/canvas-bpath.h"
#include "sp-item.h"
#include "sp-root.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

bool
CairoRendererPdfOutput::check (Inkscape::Extension::Extension * module)
{
    if (NULL == Inkscape::Extension::db.get("org.inkscape.output.pdf.cairorenderer"))
        return FALSE;

    return TRUE;
}

static bool
pdf_render_document_to_file(SPDocument *doc, gchar const *filename, unsigned int level,
                            bool texttopath, bool filtertobitmap, int resolution,
                            const gchar * const exportId, bool exportDrawing, bool exportCanvas)
{
    sp_document_ensure_up_to_date(doc);

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
        base = SP_ITEM(sp_document_root(doc));
        pageBoundingBox = !exportDrawing;
    }

    if (!base)
        return false;
    
    /* Create new arena */
    NRArena *arena = NRArena::create();
    unsigned dkey = sp_item_display_key_new(1);
    NRArenaItem *root = sp_item_invoke_show(base, arena, dkey, SP_ITEM_SHOW_DISPLAY);

    /* Create renderer and context */
    CairoRenderer *renderer = new CairoRenderer();
    CairoRenderContext *ctx = renderer->createContext();
    ctx->setPDFLevel(level);
    ctx->setTextToPath(texttopath);
    ctx->setFilterToBitmap(filtertobitmap);
    ctx->setBitmapResolution(resolution);

    bool ret = ctx->setPdfTarget (filename);
    if(ret) {
        /* Render document */
        ret = renderer->setupDocument(ctx, doc, pageBoundingBox, base);
        if (ret) {
            renderer->renderItem(ctx, base);
            ret = ctx->finish();
        }
    }

    /* Release arena */
    sp_item_invoke_hide(base, dkey);
    nr_object_unref((NRObject *) arena);
/* end */
    renderer->destroyContext(ctx);
    delete renderer;

    return ret;
}


/**
    \brief  This function calls the output module with the filename
    \param  mod   unused
    \param  doc   Document to be saved
    \param  uri   Filename to save to (probably will end in .pdf)

    The most interesting thing that this function does is just attach
    an '>' on the front of the filename.  This is the syntax used to
    tell the printing system to save to file.
*/
void
CairoRendererPdfOutput::save (Inkscape::Extension::Output *mod, SPDocument *doc, const gchar *uri)
{
    Inkscape::Extension::Extension * ext;
    unsigned int ret;

    ext = Inkscape::Extension::db.get("org.inkscape.output.pdf.cairorenderer");
    if (ext == NULL)
        return;

    const gchar *new_level = NULL;
    int level = 0;
    try {
        new_level = mod->get_param_enum("PDFversion");
//        if((new_level != NULL) && (g_ascii_strcasecmp("PDF-1.x", new_level) == 0))
//            level = 1;
    }
    catch(...) {
//        g_warning("Parameter <PDFversion> might not exists");
    }

    bool new_textToPath  = FALSE;
    try {
        new_textToPath  = mod->get_param_bool("textToPath");
    }
    catch(...) {
        g_warning("Parameter <textToPath> might not exists");
    }

    bool new_blurToBitmap  = FALSE;
    try {
        new_blurToBitmap  = mod->get_param_bool("blurToBitmap");
    }
    catch(...) {
        g_warning("Parameter <blurToBitmap> might not exists");
    }

    int new_bitmapResolution  = 72;
    try {
        new_bitmapResolution = mod->get_param_int("resolution");
    }
    catch(...) {
        g_warning("Parameter <resolution> might not exists");
    }

    const gchar *new_exportId = NULL;
    try {
        new_exportId = mod->get_param_string("exportId");
    }
    catch(...) {
        g_warning("Parameter <exportId> might not exists");
    }

    bool new_exportDrawing  = FALSE;
    try {
        new_exportDrawing  = mod->get_param_bool("exportDrawing");
    }
    catch(...) {
        g_warning("Parameter <exportDrawing> might not exists");
    }

    bool new_exportCanvas  = FALSE;
    try {
        new_exportCanvas  = mod->get_param_bool("exportCanvas");
    }
    catch(...) {
        g_warning("Parameter <exportCanvas> might not exists");
    }

    gchar * final_name;
    final_name = g_strdup_printf("> %s", uri);
    ret = pdf_render_document_to_file(doc, final_name, level,
                                      new_textToPath, new_blurToBitmap, new_bitmapResolution,
                                      new_exportId, new_exportDrawing, new_exportCanvas);
    g_free(final_name);

    if (!ret)
        throw Inkscape::Extension::Output::save_failed();

    return;
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
			"<param name=\"PDFversion\" gui-text=\"" N_("Restrict to PDF version") "\" type=\"enum\" >\n"
				"<_item value='PDF14'>" N_("PDF 1.4") "</_item>\n"
			"</param>\n"
			"<param name=\"textToPath\" gui-text=\"" N_("Convert texts to paths") "\" type=\"boolean\">false</param>\n"
			"<param name=\"blurToBitmap\" gui-text=\"" N_("Convert filter effects to bitmaps") "\" type=\"boolean\">false</param>\n"
			"<param name=\"resolution\" gui-text=\"" N_("Preferred resolution (DPI) of bitmaps") "\" type=\"int\" min=\"72\" max=\"2400\">90</param>\n"
			"<param name=\"exportDrawing\" gui-text=\"" N_("Export drawing, not page") "\" type=\"boolean\">false</param>\n"
			"<param name=\"exportCanvas\" gui-text=\"" N_("Export canvas") "\" type=\"boolean\">false</param>\n"
			"<param name=\"exportId\" gui-text=\"" N_("Limit export to the object with ID") "\" type=\"string\"></param>\n"
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
