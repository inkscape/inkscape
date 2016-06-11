/*
 * A quick hack to use the Cairo renderer to write out a file.  This
 * then makes 'save as...' PS.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Ulf Erikson <ulferikson@users.sf.net>
 *   Adib Taraben <theAdib@gmail.com>
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

#include "cairo-ps.h"
#include "cairo-ps-out.h"
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
#include "style.h"
#include "sp-root.h"
#include "sp-shape.h"

#include "io/sys.h"
#include "document.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

bool CairoPsOutput::check (Inkscape::Extension::Extension * /*module*/)
{
    if (NULL == Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_CAIRO_PS)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

bool CairoEpsOutput::check (Inkscape::Extension::Extension * /*module*/)
{
    if (NULL == Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_CAIRO_EPS)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static bool
ps_print_document_to_file(SPDocument *doc, gchar const *filename, unsigned int level, bool texttopath, bool omittext,
                          bool filtertobitmap, int resolution, const gchar * const exportId, bool exportDrawing, bool exportCanvas, float bleedmargin_px, bool eps = false)
{
    doc->ensureUpToDate();

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

    if (!base)
        return false;

    Inkscape::Drawing drawing;
    unsigned dkey = SPItem::display_key_new(1);
    base->invoke_show(drawing, dkey, SP_ITEM_SHOW_DISPLAY);

    /* Create renderer and context */
    CairoRenderer *renderer = new CairoRenderer();
    CairoRenderContext *ctx = renderer->createContext();
    ctx->setPSLevel(level);
    ctx->setEPS(eps);
    ctx->setTextToPath(texttopath);
    ctx->setOmitText(omittext);
    ctx->setFilterToBitmap(filtertobitmap);
    ctx->setBitmapResolution(resolution);

    bool ret = ctx->setPsTarget(filename);
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
    \param  filename   Filename to save to (probably will end in .ps)
*/
void
CairoPsOutput::save(Inkscape::Extension::Output *mod, SPDocument *doc, gchar const *filename)
{
    Inkscape::Extension::Extension * ext;
    unsigned int ret;

    ext = Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_CAIRO_PS);
    if (ext == NULL)
        return;

    int level = CAIRO_PS_LEVEL_2;
    try {
        const gchar *new_level = mod->get_param_enum("PSlevel");
        if((new_level != NULL) && (g_ascii_strcasecmp("PS3", new_level) == 0)) {
            level = CAIRO_PS_LEVEL_3;
        }
    } catch(...) {}

    bool new_textToPath  = FALSE;
    try {
        new_textToPath = (strcmp(mod->get_param_optiongroup("textToPath"), "paths") == 0);
    } catch(...) {}

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
    } catch(...) {}

    int new_bitmapResolution  = 72;
    try {
        new_bitmapResolution = mod->get_param_int("resolution");
    } catch(...) {}

    bool new_areaPage  = true;
    try {
        new_areaPage = (strcmp(mod->get_param_optiongroup("area"), "page") == 0);
    } catch(...) {}

    bool new_areaDrawing  = !new_areaPage;

    float bleedmargin_px = 0.;
    try {
        bleedmargin_px = mod->get_param_float("bleed");
    } catch(...) {}

    const gchar *new_exportId = NULL;
    try {
        new_exportId = mod->get_param_string("exportId");
    } catch(...) {}

    // Create PS
    {
        gchar * final_name;
        final_name = g_strdup_printf("> %s", filename);
        ret = ps_print_document_to_file(doc, final_name, level, new_textToPath,
                                        new_textToLaTeX, new_blurToBitmap,
                                        new_bitmapResolution, new_exportId,
                                        new_areaDrawing, new_areaPage,
                                        bleedmargin_px);
        g_free(final_name);

        if (!ret)
            throw Inkscape::Extension::Output::save_failed();
    }

    // Create LaTeX file (if requested)
    if (new_textToLaTeX) {
        ret = latex_render_document_text_to_file(doc, filename, new_exportId, new_areaDrawing, new_areaPage, 0., false);

        if (!ret)
            throw Inkscape::Extension::Output::save_failed();
    }
}


/**
    \brief  This function calls the output module with the filename
	\param  mod   unused
	\param  doc   Document to be saved
    \param  filename   Filename to save to (probably will end in .ps)
*/
void
CairoEpsOutput::save(Inkscape::Extension::Output *mod, SPDocument *doc, gchar const *filename)
{
    Inkscape::Extension::Extension * ext;
    unsigned int ret;

    ext = Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_CAIRO_EPS);
    if (ext == NULL)
        return;

    int level = CAIRO_PS_LEVEL_2;
    try {
        const gchar *new_level = mod->get_param_enum("PSlevel");
        if((new_level != NULL) && (g_ascii_strcasecmp("PS3", new_level) == 0)) {
            level = CAIRO_PS_LEVEL_3;
        }
    } catch(...) {}

    bool new_textToPath  = FALSE;
    try {
        new_textToPath = (strcmp(mod->get_param_optiongroup("textToPath"), "paths") == 0);
    } catch(...) {}

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
    } catch(...) {}

    int new_bitmapResolution  = 72;
    try {
        new_bitmapResolution = mod->get_param_int("resolution");
    } catch(...) {}

    bool new_areaPage  = true;
    try {
        new_areaPage = (strcmp(mod->get_param_optiongroup("area"), "page") == 0);
    } catch(...) {}

    bool new_areaDrawing  = !new_areaPage;

    float bleedmargin_px = 0.;
    try {
        bleedmargin_px = mod->get_param_float("bleed");
    } catch(...) {}

    const gchar *new_exportId = NULL;
    try {
        new_exportId = mod->get_param_string("exportId");
    } catch(...) {}

    // Create EPS
    {
        gchar * final_name;
        final_name = g_strdup_printf("> %s", filename);
        ret = ps_print_document_to_file(doc, final_name, level, new_textToPath,
                                        new_textToLaTeX, new_blurToBitmap,
                                        new_bitmapResolution, new_exportId,
                                        new_areaDrawing, new_areaPage,
                                        bleedmargin_px, true);
        g_free(final_name);

        if (!ret)
            throw Inkscape::Extension::Output::save_failed();
    }

    // Create LaTeX file (if requested)
    if (new_textToLaTeX) {
        ret = latex_render_document_text_to_file(doc, filename, new_exportId, new_areaDrawing, new_areaPage, 0., false);

        if (!ret)
            throw Inkscape::Extension::Output::save_failed();
    }
}


bool
CairoPsOutput::textToPath(Inkscape::Extension::Print * ext)
{
    return ext->get_param_bool("textToPath");
}

bool
CairoEpsOutput::textToPath(Inkscape::Extension::Print * ext)
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
            "<param name=\"PSlevel\" _gui-text=\"" N_("Restrict to PS level:") "\" type=\"enum\" >\n"
                "<_item value='PS3'>" N_("PostScript level 3") "</_item>\n"
                "<_item value='PS2'>" N_("PostScript level 2") "</_item>\n"
            "</param>\n"
            "<param name=\"textToPath\" _gui-text=\"" N_("Text output options:") "\" type=\"optiongroup\">\n"
                "<_option value=\"embed\">" N_("Embed fonts") "</_option>\n"
                "<_option value=\"paths\">" N_("Convert text to paths") "</_option>\n"
                "<_option value=\"LaTeX\">" N_("Omit text in PDF and create LaTeX file") "</_option>\n"
            "</param>\n"
            "<param name=\"blurToBitmap\" _gui-text=\"" N_("Rasterize filter effects") "\" type=\"boolean\">true</param>\n"
            "<param name=\"resolution\" _gui-text=\"" N_("Resolution for rasterization (dpi):") "\" type=\"int\" min=\"1\" max=\"10000\">96</param>\n"
            "<param name=\"area\" _gui-text=\"" N_("Output page size") "\" type=\"optiongroup\" >\n"
                "<_option value=\"page\">" N_("Use document's page size") "</_option>"
                "<_option value=\"drawing\">" N_("Use exported object's size") "</_option>"
            "</param>"
            "<param name=\"bleed\" _gui-text=\"" N_("Bleed/margin (mm):") "\" type=\"float\" min=\"-10000\" max=\"10000\">0</param>\n"
            "<param name=\"exportId\" _gui-text=\"" N_("Limit export to the object with ID:") "\" type=\"string\"></param>\n"
            "<output>\n"
            "<extension>.ps</extension>\n"
                "<mimetype>image/x-postscript</mimetype>\n"
                "<filetypename>" N_("PostScript (*.ps)") "</filetypename>\n"
                "<filetypetooltip>" N_("PostScript File") "</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>", new CairoPsOutput());

    return;
}

/**
	\brief   A function allocate a copy of this function.

	This is the definition of Cairo EPS out.  This function just
	calls the extension system with the memory allocated XML that
	describes the data.
*/
void
CairoEpsOutput::init (void)
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("Encapsulated PostScript") "</name>\n"
            "<id>" SP_MODULE_KEY_PRINT_CAIRO_EPS "</id>\n"
            "<param name=\"PSlevel\" _gui-text=\"" N_("Restrict to PS level:") "\" type=\"enum\" >\n"
                "<_item value='PS3'>" N_("PostScript level 3") "</_item>\n"
                "<_item value='PS2'>" N_("PostScript level 2") "</_item>\n"
            "</param>\n"
            "<param name=\"textToPath\" _gui-text=\"" N_("Text output options:") "\" type=\"optiongroup\">\n"
                "<_option value=\"embed\">" N_("Embed fonts") "</_option>\n"
                "<_option value=\"paths\">" N_("Convert text to paths") "</_option>\n"
                "<_option value=\"LaTeX\">" N_("Omit text in PDF and create LaTeX file") "</_option>\n"
            "</param>\n"
            "<param name=\"blurToBitmap\" _gui-text=\"" N_("Rasterize filter effects") "\" type=\"boolean\">true</param>\n"
            "<param name=\"resolution\" _gui-text=\"" N_("Resolution for rasterization (dpi):") "\" type=\"int\" min=\"1\" max=\"10000\">96</param>\n"
            "<param name=\"area\" _gui-text=\"" N_("Output page size") "\" type=\"optiongroup\" >\n"
                "<_option value=\"page\">" N_("Use document's page size") "</_option>"
                "<_option value=\"drawing\">" N_("Use exported object's size") "</_option>"
            "</param>"
            "<param name=\"bleed\" _gui-text=\"" N_("Bleed/margin (mm)") "\" type=\"float\" min=\"-10000\" max=\"10000\">0</param>\n"
            "<param name=\"exportId\" _gui-text=\"" N_("Limit export to the object with ID:") "\" type=\"string\"></param>\n"
            "<output>\n"
                "<extension>.eps</extension>\n"
                "<mimetype>image/x-e-postscript</mimetype>\n"
                "<filetypename>" N_("Encapsulated PostScript (*.eps)") "</filetypename>\n"
                "<filetypetooltip>" N_("Encapsulated PostScript File") "</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>", new CairoEpsOutput());

    return;
}

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_CAIRO_PDF */
