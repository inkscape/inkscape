/*
 * Inkscape::Widgets::shrink_wrap_button - shrink a button to minimum size
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_WIDGETS_SHRINK_WRAP_BUTTON_H
#define SEEN_INKSCAPE_WIDGETS_SHRINK_WRAP_BUTTON_H

namespace Gtk { class Button; }

namespace Inkscape {
namespace Widgets {

void shrink_wrap_button(Gtk::Button &button);

}
}

#endif
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
