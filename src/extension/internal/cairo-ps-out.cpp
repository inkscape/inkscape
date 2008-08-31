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
	if (NULL == Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_CAIRO_PS))
		return FALSE;

	return TRUE;}

static bool
ps_print_document_to_file(SPDocument *doc, gchar const *filename, unsigned int level, bool texttopath, bool filtertobitmap, int resolution)
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
    ctx->setPSLevel(level);
    ctx->setTextToPath(texttopath);
    ctx->setFilterToBitmap(filtertobitmap);
    ctx->setBitmapResolution(resolution);

    bool ret = ctx->setPsTarget(filename);
    if(ret) {
        /* Render document */
        ret = renderer->setupDocument(ctx, doc, TRUE, NULL);
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

    const gchar *old_level = NULL;
    const gchar *new_level = NULL;
    int level = 1;
    try {
        old_level = ext->get_param_enum("PSlevel");
        new_level = mod->get_param_enum("PSlevel");
        if((new_level != NULL) && (g_ascii_strcasecmp("PS2", new_level) == 0))
            level = 0;
//        ext->set_param_enum("PSlevel", new_level);
    }
    catch(...) {
        g_warning("Parameter <PSlevel> might not exists");
    }

    bool old_textToPath  = FALSE;
    bool new_textToPath  = FALSE;
    try {
        old_textToPath  = ext->get_param_bool("textToPath");
        new_textToPath  = mod->get_param_bool("textToPath");
        ext->set_param_bool("textToPath", new_textToPath);
    }
    catch(...) {
        g_warning("Parameter <textToPath> might not exists");
    }

    bool old_blurToBitmap  = FALSE;
    bool new_blurToBitmap  = FALSE;
    try {
        old_blurToBitmap  = ext->get_param_bool("blurToBitmap");
        new_blurToBitmap  = mod->get_param_bool("blurToBitmap");
        ext->set_param_bool("blurToBitmap", new_blurToBitmap);
    }
    catch(...) {
        g_warning("Parameter <blurToBitmap> might not exists");
    }

    int old_bitmapResolution  = 72;
    int new_bitmapResolution  = 72;
    try {
        old_bitmapResolution  = ext->get_param_int("resolution");
        new_bitmapResolution = mod->get_param_int("resolution");
        ext->set_param_int("resolution", new_bitmapResolution);
    }
    catch(...) {
        g_warning("Parameter <resolution> might not exists");
    }

	gchar * final_name;
	final_name = g_strdup_printf("> %s", uri);
	ret = ps_print_document_to_file(doc, final_name, level, new_textToPath, new_blurToBitmap, new_bitmapResolution);
	g_free(final_name);

    try {
        ext->set_param_int("resolution", old_bitmapResolution);
    }
    catch(...) {
        g_warning("Parameter <resolution> might not exists");
    }
    try {
        ext->set_param_bool("blurToBitmap", old_blurToBitmap);
    }
    catch(...) {
        g_warning("Parameter <blurToBitmap> might not exists");
    }
    try {
        ext->set_param_bool("textToPath", old_textToPath);
    }
    catch(...) {
        g_warning("Parameter <textToPath> might not exists");
    }
    try {
//        ext->set_param_enum("PSlevel", old_level);
    }
    catch(...) {
        g_warning("Parameter <PSlevel> might not exists");
    }


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
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("PostScript") "</name>\n"
			"<id>" SP_MODULE_KEY_PRINT_CAIRO_PS "</id>\n"
			"<param name=\"PSlevel\" gui-text=\"" N_("Restrict to PS level") "\" type=\"enum\" >\n"
				"<_item value='PS3'>" N_("PostScript level 3") "</_item>\n"
#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 5, 2))
                "<_item value='PS2'>" N_("PostScript level 2") "</_item>\n"
#endif
            "</param>\n"
			"<param name=\"textToPath\" gui-text=\"" N_("Convert texts to paths") "\" type=\"boolean\">false</param>\n"
			"<param name=\"blurToBitmap\" gui-text=\"" N_("Convert blur effects to bitmaps") "\" type=\"boolean\">false</param>\n"
			"<param name=\"resolution\" gui-text=\"" N_("Preferred resolution (DPI) of bitmaps") "\" type=\"int\" min=\"72\" max=\"2400\">90</param>\n"
			"<output>\n"
				"<extension>.ps</extension>\n"
                                "<mimetype>image/x-postscript</mimetype>\n"
				"<filetypename>" N_("PostScript (*.ps)") "</filetypename>\n"
				"<filetypetooltip>" N_("PostScript File") "</filetypetooltip>\n"
			"</output>\n"
		"</inkscape-extension>", new CairoPsOutput());

	return;
}

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_CAIRO_PDF */
