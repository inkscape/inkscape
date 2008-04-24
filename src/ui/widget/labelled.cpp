/**
 * \brief Labelled Widget - Adds a label with optional icon or suffix to
 *        another widget.
 *
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

/* For getting the Gtkmmified Icon manager */
#include "widgets/icon.h"

#include "labelled.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Construct a Labelled Widget.
 *
 * \param label     Label.
 * \param widget    Widget to label; should be allocated with new, as it will
 *                  be passed to Gtk::manage().
 * \param suffix    Suffix, placed after the widget (defaults to "").
 * \param icon      Icon filename, placed before the label (defaults to "").
 * \param mnemonic  Mnemonic toggle; if true, an underscore (_) in the text
 *                  indicates the next character should be used for the
 *                  mnemonic accelerator key (defaults to true).
 */
Labelled::Labelled(Glib::ustring const &label, Glib::ustring const &tooltip,
                   Gtk::Widget *widget,
                   Glib::ustring const &suffix,
                   Glib::ustring const &icon,
                   bool mnemonic)
    : _widget(widget),
      _label(new Gtk::Label(label, 1.0, 0.5, mnemonic)),
      _suffix(new Gtk::Label(suffix, 0.0, 0.5)),
      _tooltips()
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
    _tooltips.set_tip(*_widget, tooltip);
}


/**
 * Allow the setting of the width of the labelled widget
 */
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
