/*
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Carl Hetherington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "labelled.h"

/* For getting the Gtkmmified Icon manager */
#include "widgets/icon.h"
#include <gtkmm/label.h>

namespace Inkscape {
namespace UI {
namespace Widget {

Labelled::Labelled(Glib::ustring const &label, Glib::ustring const &tooltip,
                   Gtk::Widget *widget,
                   Glib::ustring const &suffix,
                   Glib::ustring const &icon,
                   bool mnemonic)
    : _widget(widget),
      _label(new Gtk::Label(label, 1.0, 0.5, mnemonic)),
      _suffix(new Gtk::Label(suffix, 0.0, 0.5))
{
    g_assert(g_utf8_validate(icon.c_str(), -1, NULL));
    if (icon != "") {
        _icon = sp_icon_get_icon(icon.c_str(), Inkscape::ICON_SIZE_LARGE_TOOLBAR);
        pack_start(*Gtk::manage(_icon), Gtk::PACK_SHRINK);
    }
    pack_start(*Gtk::manage(_label), Gtk::PACK_EXPAND_WIDGET, 6);
    pack_start(*Gtk::manage(_widget), Gtk::PACK_SHRINK, 6);
    if (mnemonic) {
        _label->set_mnemonic_widget(*_widget);
    }
    widget->set_tooltip_text(tooltip);
}


void Labelled::setWidgetSizeRequest(int width, int height)
{
    if (_widget)
        _widget->set_size_request(width, height);


}

Gtk::Widget const *
Labelled::getWidget() const
{
    return _widget;
}

Gtk::Label const *
Labelled::getLabel() const
{
    return _label;
}

void
Labelled::setLabelText(const Glib::ustring &str)
{
    _label->set_text(str);
}

void
Labelled::setTooltipText(const Glib::ustring &tooltip)
{
    _label->set_tooltip_text(tooltip);
    _widget->set_tooltip_text(tooltip);
}

bool Labelled::on_mnemonic_activate ( bool group_cycling )
{
    return _widget->mnemonic_activate ( group_cycling );
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
