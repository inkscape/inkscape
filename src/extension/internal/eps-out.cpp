/*
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
#include "eps-out.h"
#include <print.h>
#include "extension/system.h"
#include "extension/db.h"
#include "extension/output.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

bool
EpsOutput::check (Inkscape::Extension::Extension * module)
{
    if (NULL == Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_PS))
        return FALSE;

    return TRUE;
}

/**
    \brief  This function calls the print system with the filename
    \param  mod   unused
    \param  doc   Document to be saved
    \param  uri   Filename to save to (probably will end in .eps)

    The most interesting thing that this function does is just attach
    an '>' on the front of the filename.  This is the syntax used to
    tell the printing system to save to file.
*/
void
EpsOutput::save (Inkscape::Extension::Output *mod, SPDocument *doc, const gchar *uri)
{
    gchar * final_name;
    Inkscape::Extension::Extension * ext;

    ext = Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_PS);
    if (ext == NULL)
        return;

    bool old_pageBoundingBox = ext->get_param_bool("pageBoundingBox");
    bool new_val             = mod->get_param_bool("pageBoundingBox");
    ext->set_param_bool("pageBoundingBox", new_val);

    bool old_textToPath      = ext->get_param_bool("textToPath");
    new_val                  = mod->get_param_bool("textToPath");
    ext->set_param_bool("textToPath", new_val);

    bool old_fontEmbedded    = ext->get_param_bool("fontEmbedded");
    new_val                  = mod->get_param_bool("fontEmbedded");
    ext->set_param_bool("fontEmbedded", new_val);

    final_name = g_strdup_printf("> %s", uri);
    sp_print_document_to_file(doc, final_name);
    g_free(final_name);

    ext->set_param_bool("pageBoundingBox", old_pageBoundingBox);
    ext->set_param_bool("textToPath", old_textToPath);
    ext->set_param_bool("fontEmbedded", old_fontEmbedded);

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
EpsOutput::init (void)
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("Encapsulated Postscript Output") "</name>\n"
            "<id>org.inkscape.output.eps</id>\n"
            "<param name=\"pageBoundingBox\" type=\"boolean\" gui-text=\"" N_("Make bounding box around full page") "\">false</param>\n"
            "<param name=\"textToPath\" type=\"boolean\" gui-text=\"" N_("Convert texts to paths") "\">true</param>\n"
            "<param name=\"fontEmbedded\" type=\"boolean\" gui-text=\"" N_("Embed fonts (Type 1 only)") "\">false</param>\n"
            "<output>\n"
                "<extension>.eps</extension>\n"
                "<mimetype>image/x-eps</mimetype>\n"
                "<filetypename>" N_("Encapsulated Postscript (*.eps)") "</filetypename>\n"
                "<filetypetooltip>" N_("Encapsulated Postscript File") "</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>", new EpsOutput());

    return;
}

} } }  /* namespace Inkscape, Extension, Implementation */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
