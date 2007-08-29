/**
 * \brief Text Properties dialog
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

#include "text-properties.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

TextProperties::TextProperties(Behavior::BehaviorFactory behavior_factory) 
    : Dialog (behavior_factory, "dialogs.textandfont", SP_VERB_DIALOG_TEXT),
      _page_font(1, 1),
      _page_text(1, 1)
{
    // Top level vbox
    Gtk::VBox *vbox = get_vbox();
    vbox->set_spacing(4);

    // Notebook for individual transformations
    vbox->pack_start(_notebook, true, true);

    // TODO:  Insert widgets
    _notebook.append_page(_page_font, _("Font"));
    _notebook.append_page(_page_text, _("Text"));

    set_resizable (true);
    set_size_request(450, 300);
    
    show_all_children();
}

TextProperties::~TextProperties() 
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
