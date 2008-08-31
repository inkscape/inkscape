/*
 * A quick hack to use the print output to write out a file.  This
 * then makes 'save as...' Postscript.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "ps-out.h"
#include <print.h>
#include "extension/system.h"
#include "extension/db.h"
#include "extension/output.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

bool
PsOutput::check( Inkscape::Extension::Extension * /*module*/ )
{
    if (NULL == Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_PS))
        return FALSE;

    return TRUE;
}

/**
    \brief  This function calls the print system with the filename
	\param  mod   unused
	\param  doc   Document to be saved
    \param  uri   Filename to save to (probably will end in .ps)

	The most interesting thing that this function does is just attach
	an '>' on the front of the filename.  This is the syntax used to
	tell the printing system to save to file.
*/
void
PsOutput::save (Inkscape::Extension::Output *mod, SPDocument *doc, const gchar *uri)
{
    Inkscape::Extension::Extension * ext;

    ext = Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_PS);
    if (ext == NULL)
        return;

    bool old_textToPath  = ext->get_param_bool("textToPath");
    bool new_val         = mod->get_param_bool("textToPath");
    ext->set_param_bool("textToPath", new_val);

	gchar * final_name;
	final_name = g_strdup_printf("> %s", uri);
	sp_print_document_to_file(doc, final_name);
	g_free(final_name);

    ext->set_param_bool("textToPath", old_textToPath);

	return;
}

#include "clear-n_.h"

/**
	\brief   A function allocate a copy of this function.

	This is the definition of postscript out.  This function just
	calls the extension system with the memory allocated XML that
	describes the data.
*/
void
PsOutput::init (void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Postscript Output") "</name>\n"
			"<id>org.inkscape.output.ps</id>\n"
			"<param name=\"textToPath\" gui-text=\"" N_("Convert texts to paths") "\" type=\"boolean\">true</param>\n"
			"<param name=\"fontEmbedded\" gui-text=\"" N_("Embed fonts (Type 1 only)") "\" type=\"boolean\">false</param>\n"
			"<output>\n"
				"<extension>.ps</extension>\n"
				"<mimetype>image/x-postscript</mimetype>\n"
				"<filetypename>" N_("PostScript (old exporter via print) (*.ps)") "</filetypename>\n"
				"<filetypetooltip>" N_("PostScript File") "</filetypetooltip>\n"
			"</output>\n"
		"</inkscape-extension>", new PsOutput());

	return;
}

} } }  /* namespace Inkscape, Extension, Implementation */
