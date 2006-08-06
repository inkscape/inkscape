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
pdf_print_document_to_file(SPDocument *doc, gchar const *filename)
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
    mod->base = SP_ITEM(sp_document_root(doc));
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

	gchar * final_name;
	final_name = g_strdup_printf("> %s", uri);
	ret = pdf_print_document_to_file(doc, final_name);
	g_free(final_name);
        
	if (!ret)
	    throw Inkscape::Extension::Output::save_failed();

	return;
}

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
			"<name>Cairo PDF Output</name>\n"
			"<id>org.inkscape.output.pdf.cairo</id>\n"
			"<output>\n"
				"<extension>.pdf</extension>\n"
				"<mimetype>application/pdf</mimetype>\n"
				"<filetypename>Cairo PDF (*.pdf)</filetypename>\n"
				"<filetypetooltip>PDF File</filetypetooltip>\n"
			"</output>\n"
		"</inkscape-extension>", new CairoPdfOutput());

	return;
}

} } }  /* namespace Inkscape, Extension, Implementation */
