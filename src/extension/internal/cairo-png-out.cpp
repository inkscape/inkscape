/*
 * A quick hack to use the Cairo renderer to write out a file.  This
 * then makes 'save as...' PNG.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Ulf Erikson <ulferikson@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_CAIRO_PDF

#include "cairo-png-out.h"
#include "cairo-render-context.h"
#include "cairo-renderer.h"
#include <print.h>
#include "extension/system.h"
#include "extension/print.h"
#include "extension/db.h"
#include "extension/output.h"
#include "display/drawing.h"

#include "display/curve.h"
#include "display/canvas-bpath.h"
#include "sp-item.h"
#include "style.h"
#include "sp-root.h"
#include "sp-shape.h"

#include "io/sys.h"
#include "document.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

bool CairoRendererOutput::check(Inkscape::Extension::Extension * /*module*/)
{
    return true;
}

static bool
png_render_document_to_file(SPDocument *doc, gchar const *filename)
{
    CairoRenderer *renderer;
    CairoRenderContext *ctx;

    doc->ensureUpToDate();

/* Start */

    SPItem *base = doc->getRoot();
    Inkscape::Drawing drawing;
    unsigned dkey = SPItem::display_key_new(1);
    base->invoke_show(drawing, dkey, SP_ITEM_SHOW_DISPLAY);
    
    /* Create renderer and context */
    renderer = new CairoRenderer();
    ctx = renderer->createContext();

    /* Render document */
    bool ret = renderer->setupDocument(ctx, doc, TRUE, 0., NULL);
    if (ret) {
        renderer->renderItem(ctx, base);
        ctx->saveAsPng(filename);
        ret = ctx->finish();
    }
    renderer->destroyContext(ctx);

    base->invoke_hide(dkey);
/* end */
    delete renderer;

    return ret;
}


/**
    \brief  This function calls the output module with the filename
	\param  mod   unused
	\param  doc   Document to be saved
    \param  uri   Filename to save to (probably will end in .png)
*/
void CairoRendererOutput::save(Inkscape::Extension::Output * /*mod*/, SPDocument *doc, gchar const *filename)
{
    if (!png_render_document_to_file(doc, filename)) {
        throw Inkscape::Extension::Output::save_failed();
    }
}

/**
	\brief   A function allocate a copy of this function.

	This is the definition of Cairo PNG out.  This function just
	calls the extension system with the memory allocated XML that
	describes the data.
*/
void
CairoRendererOutput::init (void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>Cairo PNG Output</name>\n"
			"<id>org.inkscape.output.png.cairo</id>\n"
			"<output>\n"
				"<extension>.png</extension>\n"
                "<mimetype>image/png</mimetype>\n"
				"<filetypename>Cairo PNG (*.png)</filetypename>\n"
				"<filetypetooltip>PNG File</filetypetooltip>\n"
			"</output>\n"
		"</inkscape-extension>", new CairoRendererOutput());

	return;
}

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_CAIRO_PDF */
