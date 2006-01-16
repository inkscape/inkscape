/**
 * \brief Fill and Stroke dialog
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

#include "fill-and-stroke.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

FillAndStroke::FillAndStroke() 
    : Dialog ("dialogs.fillstroke", SP_VERB_DIALOG_FILL_STROKE),
      _page_fill(1, 1),
      _page_stroke_paint(1, 1),
      _page_stroke_style(1, 1)
{
    // Top level vbox
    Gtk::VBox *vbox = get_vbox();
    vbox->set_spacing(4);

    // Notebook for individual transformations
    vbox->pack_start(_notebook, true, true);

    _notebook.append_page(_page_fill,         _("Fill"));
    _notebook.append_page(_page_stroke_paint, _("Stroke Paint"));
    _notebook.append_page(_page_stroke_style, _("Stroke Style"));

    // TODO:  Insert widgets

    show_all_children();
}

FillAndStroke::~FillAndStroke() 
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
