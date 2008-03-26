/*
 * A quick hack to use the print output to write out a file.  This
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

#include "cairo-pdf-out.h"
#include <print.h>
#include "extension/system.h"
#include "extension/print.h"
#include "extension/db.h"
#include "extension/output.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"
#include "sp-path.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

bool
CairoPdfOutput::check (Inkscape::Extension::Extension * module)
{
	if (NULL == Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_CAIRO_PDF))
		return FALSE;

	return TRUE;
}


static unsigned int
pdf_print_document_to_file(SPDocument *doc, gchar const *filename, unsigned int pdf_level, bool texttopath, bool filtertobitmap)
{
    Inkscape::Extension::Print *mod;
    SPPrintContext context;
    gchar const *oldconst;
    gchar *oldoutput;
    unsigned int ret;

    sp_document_ensure_up_to_date(doc);

    mod = Inkscape::Extension::get_print(SP_MODULE_KEY_PRINT_CAIRO_PDF);
    oldconst = mod->get_param_string("destination");
    oldoutput = g_strdup(oldconst);
    mod->set_param_string("destination", (gchar *)filename);

/* Start */
    context.module = mod;
    /* fixme: This has to go into module constructor somehow */
    /* Create new arena */
    const gchar* exportId = mod->get_param_string("exportId");
    bool exportDrawing = mod->get_param_bool("exportDrawing");
    if (exportId && strcmp(exportId, "")) {
        // we want to export the given item only, not page
        mod->base = SP_ITEM(doc->getObjectById(exportId));
        mod->set_param_bool("pageBoundingBox", FALSE);
    } else {
        // we want to export the entire document from root
        mod->base = SP_ITEM(sp_document_root(doc));
        if (exportDrawing)
            mod->set_param_bool("pageBoundingBox", FALSE);
        else
            mod->set_param_bool("pageBoundingBox", TRUE);
    }
    mod->arena = NRArena::create();
    mod->dkey = sp_item_display_key_new(1);
    mod->root = sp_item_invoke_show(mod->base, mod->arena, mod->dkey, SP_ITEM_SHOW_DISPLAY);

    /* Print document */
    ret = mod->begin(doc);
    if (ret) {
        sp_item_invoke_print(mod->base, &context);
        ret = mod->finish();
    }

    /* Release arena */
    sp_item_invoke_hide(mod->base, mod->dkey);
    mod->base = NULL;
    nr_arena_item_unref(mod->root);
    mod->root = NULL;
    nr_object_unref((NRObject *) mod->arena);
    mod->arena = NULL;
/* end */

    mod->set_param_string("destination", oldoutput);
    g_free(oldoutput);

    return ret;
}


/**
    \brief  This function calls the print system with the filename
	\param  mod   unused
	\param  doc   Document to be saved
    \param  uri   Filename to save to (probably will end in .pdf)

	The most interesting thing that this function does is just attach
	an '>' on the front of the filename.  This is the syntax used to
	tell the printing system to save to file.
*/
void
CairoPdfOutput::save (Inkscape::Extension::Output *mod, SPDocument *doc, const gchar *uri)
{
    Inkscape::Extension::Extension * ext;
    unsigned int ret;

    ext = Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_CAIRO_PDF);
    if (ext == NULL)
        return;

    bool old_textToPath  = FALSE;
    bool new_textToPath  = FALSE;
    try {
        old_textToPath  = ext->get_param_bool("textToPath");
        new_textToPath  = mod->get_param_bool("textToPath");
        ext->set_param_bool("textToPath", new_textToPath);
    }
    catch(...) {
        g_warning("Parameter <textToPath> might not exist");
    }

    bool old_blurToBitmap  = FALSE;
    bool new_blurToBitmap  = FALSE;
    try {
        old_blurToBitmap  = ext->get_param_bool("blurToBitmap");
        new_blurToBitmap  = mod->get_param_bool("blurToBitmap");
        ext->set_param_bool("blurToBitmap", new_blurToBitmap);
    }
    catch(...) {
        g_warning("Parameter <blurToBitmap> might not exist");
    }

    const gchar* old_exportId = NULL;
    const gchar* new_exportId = NULL;
    try {
        old_exportId  = ext->get_param_string("exportId");
        new_exportId  = mod->get_param_string("exportId");
        ext->set_param_string("exportId", new_exportId);
    }
    catch(...) {
        g_warning("Parameter <exportId> might not exist");
    }

    bool old_exportDrawing = false;
    bool new_exportDrawing = false;
    try {
        old_exportDrawing  = ext->get_param_bool("exportDrawing");
        new_exportDrawing  = mod->get_param_bool("exportDrawing");
        ext->set_param_bool("exportDrawing", new_exportDrawing);
    }
    catch(...) {
        g_warning("Parameter <exportDrawing> might not exist");
    }

    gchar * final_name;
    final_name = g_strdup_printf("> %s", uri);
    ret = pdf_print_document_to_file(doc, final_name, 0, new_textToPath, new_blurToBitmap);
    g_free(final_name);

    try {
        ext->set_param_bool("blurToBitmap", old_blurToBitmap);
    }
    catch(...) {
        g_warning("Parameter <blurToBitmap> might not exist");
    }
    try {
        ext->set_param_bool("textToPath", old_textToPath);
    }
    catch(...) {
        g_warning("Parameter <textToPath> might not exist");
    }
    try {
        ext->set_param_string("exportId", old_exportId);
    }
    catch(...) {
        g_warning("Parameter <exportId> might not exist");
    }
    try {
        ext->set_param_bool("exportDrawing", old_exportDrawing);
    }
    catch(...) {
        g_warning("Parameter <exportDrawing> might not exist");
    }

    if (!ret)
        throw Inkscape::Extension::Output::save_failed();

    return;
}

#include "clear-n_.h"
/**
	\brief   A function allocate a copy of this function.

	This is the definition of PDF out.  This function just
	calls the extension system with the memory allocated XML that
	describes the data.
*/
void
CairoPdfOutput::init (void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension>\n"
			"<name>" N_("Cairo PDF Output") "</name>\n"
			"<id>org.inkscape.output.pdf.cairo</id>\n"
			"<param name=\"PDFversion\" gui-text=\"" N_("Restrict to PDF version") "\" type=\"enum\" >\n"
				"<_item value='PDF14'>" N_("PDF 1.4") "</_item>\n"
      "</param>\n"
			"<param name=\"textToPath\" gui-text=\"" N_("Convert texts to paths") "\" type=\"boolean\">false</param>\n"
			"<param name=\"blurToBitmap\" gui-text=\"" N_("Convert blur effects to bitmaps") "\" type=\"boolean\">false</param>\n"
      "<param name=\"resolution\" gui-text=\"" N_("Preferred resolution (DPI) of bitmaps") "\" type=\"int\" min=\"72\" max=\"2400\">90</param>\n"
      "<param name=\"exportDrawing\" gui-text=\"" N_("Export drawing, not page") "\" type=\"boolean\">false</param>\n"
      "<param name=\"exportId\" gui-text=\"" N_("Limit export to the object with ID") "\" type=\"string\"></param>\n"
      "<output>\n"
				"<extension>.pdf</extension>\n"
				"<mimetype>application/pdf</mimetype>\n"
				"<filetypename>" N_("PDF via Cairo (*.pdf)") "</filetypename>\n"
				"<filetypetooltip>" N_("PDF File") "</filetypetooltip>\n"
			"</output>\n"
		"</inkscape-extension>", new CairoPdfOutput());

	return;
}

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_CAIRO_PDF */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
