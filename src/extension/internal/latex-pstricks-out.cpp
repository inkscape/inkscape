/*
 * Authors:
 *   Michael Forbes <miforbes@mbhs.edu>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "latex-pstricks-out.h"
#include "sp-path.h"
#include <print.h>
#include "extension/system.h"
#include "extension/print.h"
#include "extension/db.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"





namespace Inkscape {
namespace Extension {
namespace Internal {

LatexOutput::LatexOutput (void) // The null constructor
{
    return;
}

LatexOutput::~LatexOutput (void) //The destructor
{
    return;
}

bool
LatexOutput::check (Inkscape::Extension::Extension * module)
{
	if (NULL == Inkscape::Extension::db.get("org.inkscape.print.latex"))
		return FALSE;
    return TRUE;
}


void
LatexOutput::save (Inkscape::Extension::Output *mod2, SPDocument *doc, const gchar *uri)
{
    Inkscape::Extension::Print *mod;
    SPPrintContext context;
    const gchar * oldconst;
    gchar * oldoutput;
    unsigned int ret;

    sp_document_ensure_up_to_date (doc);

    mod = Inkscape::Extension::get_print(SP_MODULE_KEY_PRINT_LATEX);
    oldconst = mod->get_param_string("destination");
    oldoutput = g_strdup(oldconst);
    mod->set_param_string("destination", (gchar *)uri);

    /* Start */
    context.module = mod;
    /* fixme: This has to go into module constructor somehow */
    /* Create new arena */
    mod->base = SP_ITEM (sp_document_root (doc));
    mod->arena = NRArena::create();
    mod->dkey = sp_item_display_key_new (1);
    mod->root = sp_item_invoke_show (mod->base, mod->arena, mod->dkey, SP_ITEM_SHOW_DISPLAY);
    /* Print document */
    ret = mod->begin (doc);
    sp_item_invoke_print (mod->base, &context);
    ret = mod->finish ();
    /* Release arena */
    sp_item_invoke_hide (mod->base, mod->dkey);
    mod->base = NULL;
    nr_arena_item_unref (mod->root);
    mod->root = NULL;
    nr_object_unref ((NRObject *) mod->arena);
    mod->arena = NULL;
    /* end */

    mod->set_param_string("destination", oldoutput);
    g_free(oldoutput);

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
LatexOutput::init (void)
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension>\n"
            "<name>" N_("LaTeX Output") "</name>\n"
            "<id>org.inkscape.output.latex</id>\n"
            "<output>\n"
                "<extension>.tex</extension>\n"
                "<mimetype>text/x-tex</mimetype>\n"
                "<filetypename>" N_("LaTeX With PSTricks macros (*.tex)") "</filetypename>\n"
                "<filetypetooltip>" N_("LaTeX PSTricks File") "</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>", new LatexOutput());

    return;
}

} } }  /* namespace Inkscape, Extension, Implementation */

/*
  Local Variables:
  mode:cpp
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
