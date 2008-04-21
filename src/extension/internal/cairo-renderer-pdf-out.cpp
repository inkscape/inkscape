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

#include <libnr/n-art-bpath.h>

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
	return TRUE;
}

static bool
pdf_render_document_to_file(SPDocument *doc, gchar const *filename)
{
    sp_document_ensure_up_to_date(doc);

/* Start */
    /* Create new arena */
    SPItem *base = SP_ITEM(sp_document_root(doc));
    NRArena *arena = NRArena::create();
    unsigned dkey = sp_item_display_key_new(1);
    NRArenaItem *root = sp_item_invoke_show(base, arena, dkey, SP_ITEM_SHOW_DISPLAY);

    /* Create renderer and context */
    CairoRenderer *renderer = new CairoRenderer();
    CairoRenderContext *ctx = renderer->createContext();
    ctx->setPdfTarget (filename);
    bool ret = renderer->setupDocument(ctx, doc);
    if (ret) {
        renderer->renderItem(ctx, base);
        ret = ctx->finish();
    }

    /* Release arena */
    sp_item_invoke_hide(base, dkey);
    nr_arena_item_unref(root);
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
    \param  uri   Filename to save to (probably will end in .png)

	The most interesting thing that this function does is just attach
	an '>' on the front of the filename.  This is the syntax used to
	tell the printing system to save to file.
*/
void
CairoRendererPdfOutput::save (Inkscape::Extension::Output *mod, SPDocument *doc, const gchar *uri)
{
    gchar * final_name;
    final_name = g_strdup_printf("> %s", uri);
    bool ret = pdf_render_document_to_file(doc, final_name);
    g_free(final_name);

    if (!ret)
        throw Inkscape::Extension::Output::save_failed();

	return;
}

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
            "<name>Cairo PDF Output (experimental)</name>\n"
			"<id>org.inkscape.output.pdf.cairorenderer</id>\n"
			"<output>\n"
				"<extension>.pdf</extension>\n"
				"<mimetype>application/pdf</mimetype>\n"
				"<filetypename>Cairo PDF experimental (*.pdf)</filetypename>\n"
				"<filetypetooltip>PDF File</filetypetooltip>\n"
			"</output>\n"
		"</inkscape-extension>", new CairoRendererPdfOutput());

	return;
}

} } }  /* namespace Inkscape, Extension, Internal */

#endif /* HAVE_CAIRO_PDF */
