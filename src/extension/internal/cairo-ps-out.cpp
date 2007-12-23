/*
 * A quick hack to use the Cairo renderer to write out a file.  This
 * then makes 'save as...' PS.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Ulf Erikson <ulferikson@users.sf.net>
 *   Adib Taraben <theAdib@yahoo.com>
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_CAIRO_PDF

#include "cairo-ps-out.h"
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
#include "style.h"
#include "sp-root.h"
#include "sp-shape.h"

#include "io/sys.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

bool
CairoPsOutput::check (Inkscape::Extension::Extension * module)
{
	return TRUE;
}

static bool
ps_print_document_to_file(SPDocument *doc, gchar const *filename)
{
    CairoRenderer *renderer;
    CairoRenderContext *ctx;

    sp_document_ensure_up_to_date(doc);

/* Start */
    /* Create new arena */
    SPItem *base = SP_ITEM(sp_document_root(doc));
    NRArena *arena = NRArena::create();
    unsigned dkey = sp_item_display_key_new(1);
    NRArenaItem *root = sp_item_invoke_show(base, arena, dkey, SP_ITEM_SHOW_DISPLAY);

    /* Create renderer and context */
    renderer = new CairoRenderer();
    ctx = renderer->createContext();
    bool ret = ctx->setPsTarget(filename);
    if(ret) {
        /* Render document */
	ret = renderer->setupDocument(ctx, doc);
	if (ret) {
	    renderer->renderItem(ctx, base);
	    ret = ctx->finish();
	}
    }
    renderer->destroyContext(ctx);

    /* Release arena */
    sp_item_invoke_hide(base, dkey);
    nr_arena_item_unref(root);
    nr_object_unref((NRObject *) arena);
/* end */
    delete renderer;

    return ret;
}


/**
    \brief  This function calls the output module with the filename
	\param  mod   unused
	\param  doc   Document to be saved
    \param  uri   Filename to save to (probably will end in .ps)
*/
void
CairoPsOutput::save (Inkscape::Extension::Output *mod, SPDocument *doc, const gchar *uri)
{
    Inkscape::Extension::Extension * ext;
    unsigned int ret;

    ext = Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_CAIRO_PS);
    if (ext == NULL)
        return;

    bool old_textToPath  = ext->get_param_bool("textToPath");
    bool new_val         = mod->get_param_bool("textToPath");
    ext->set_param_bool("textToPath", new_val);

    bool old_blurToBitmap  = ext->get_param_bool("blurToBitmap");
    new_val         = mod->get_param_bool("blurToBitmap");
    ext->set_param_bool("blurToBitmap", new_val);


	gchar * final_name;
	final_name = g_strdup_printf("> %s", uri);
	ret = ps_print_document_to_file(doc, final_name);
	g_free(final_name);

    ext->set_param_bool("blurToBitmap", old_blurToBitmap);
    ext->set_param_bool("textToPath", old_textToPath);

	if (!ret)
	    throw Inkscape::Extension::Output::save_failed();

	return;

}

bool
CairoPsOutput::textToPath(Inkscape::Extension::Print * ext)
{
    return ext->get_param_bool("textToPath");
}

#include "clear-n_.h"

/**
	\brief   A function allocate a copy of this function.

	This is the definition of Cairo PS out.  This function just
	calls the extension system with the memory allocated XML that
	describes the data.
*/
void
CairoPsOutput::init (void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension>\n"
			"<name>Cairo PS Output</name>\n"
			"<id>org.inkscape.print.ps.cairo</id>\n"
			"<param name=\"PSlevel\" gui-text=\"" N_("Restrict to PS level") "\" type=\"enum\" >\n"
#if (CAIRO_VERSION >= 010502)
                "<item value='PS2'>" N_("PostScript level 2") "</item>\n"
#endif
				"<item value='PS3'>" N_("PostScript 3") "</item>\n"
            "</param>\n"
			"<param name=\"textToPath\" gui-text=\"" N_("Convert texts to paths") "\" type=\"boolean\">true</param>\n"
			"<param name=\"blurToBitmap\" gui-text=\"" N_("Convert blur effects to bitmaps") "\" type=\"boolean\">false</param>\n"
			"<output>\n"
				"<extension>.ps</extension>\n"
                "<mimetype>application/ps</mimetype>\n"
				"<filetypename>Cairo PS (*.ps)</filetypename>\n"
				"<filetypetooltip>PS File</filetypetooltip>\n"
			"</output>\n"
		"</inkscape-extension>", new CairoPsOutput());

	return;
}

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_CAIRO_PDF */
