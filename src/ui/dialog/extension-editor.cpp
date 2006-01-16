/**
 * \brief Extension editor
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "extension-editor.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

ExtensionEditor::ExtensionEditor()
    : Dialog ("dialogs.extensioneditor", SP_VERB_NONE /*FIXME*/)
{
    // TODO:  Insert widgets

    show_all_children();
}

ExtensionEditor::~ExtensionEditor()
{
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
