/*
 * Authors:
 *   Murray C
 *
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "frame.h"


// Inkscape::UI::Widget::Frame

namespace Inkscape {
namespace UI {
namespace Widget {

Frame::Frame(Glib::ustring const &label_text /*= ""*/, gboolean label_bold /*= TRUE*/ )
    : _label(label_text, 1.0, 0.5, TRUE),
      _alignment()
{
    set_shadow_type(Gtk::SHADOW_NONE);

    //Put an indented GtkAlignment inside the frame.
    //Further children should be children of this GtkAlignment:
    Gtk::Frame::add(_alignment);
    set_padding(4, 0, 8, 0);

    set_label_widget(_label);
    set_label(label_text, label_bold);

    show_all_children();
}

void
Frame::add(Widget& widget)
{
    _alignment.add(widget);
}

void
Frame::set_label(const Glib::ustring &label_text, gboolean label_bold /*= TRUE*/)
{
    if (label_bold) {
        _label.set_markup(Glib::ustring("<b>") + label_text + "</b>");
    } else {
        _label.set_text(label_text);
    }
}

void
Frame::set_padding (guint padding_top, guint padding_bottom, guint padding_left, guint padding_right)
{
    _alignment.set_padding(padding_top, padding_bottom, padding_left, padding_right);
}

Gtk::Label const *
Frame::get_label_widget() const
{
    return &_label;
}

} // namespace Widget
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
