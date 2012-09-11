/*
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Carl Hetherington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_LABELLED_H
#define INKSCAPE_UI_WIDGET_LABELLED_H

#include <gtkmm/box.h>

namespace Gtk {
class Label;
}

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Adds a label with optional icon or suffix to another widget.
 */
class Labelled : public Gtk::HBox
{
public:

    /**
     * Construct a Labelled Widget.
     *
     * @param label     Label.
     * @param widget    Widget to label; should be allocated with new, as it will
     *                  be passed to Gtk::manage().
     * @param suffix    Suffix, placed after the widget (defaults to "").
     * @param icon      Icon filename, placed before the label (defaults to "").
     * @param mnemonic  Mnemonic toggle; if true, an underscore (_) in the text
     *                  indicates the next character should be used for the
     *                  mnemonic accelerator key (defaults to true).
     */
    Labelled(Glib::ustring const &label, Glib::ustring const &tooltip,
             Gtk::Widget *widget,
             Glib::ustring const &suffix = "",
             Glib::ustring const &icon = "",
             bool mnemonic = true);

    /**
     * Allow the setting of the width of the labelled widget
     */
    void setWidgetSizeRequest(int width, int height);
    Gtk::Widget const *getWidget() const;
    Gtk::Label const *getLabel() const;

    void setLabelText(const Glib::ustring &str);
    void setTooltipText(const Glib::ustring &tooltip);

private:
    virtual bool on_mnemonic_activate( bool group_cycling );

protected:

    Gtk::Widget  *_widget;
    Gtk::Label   *_label;
    Gtk::Label   *_suffix;
    Gtk::Widget  *_icon;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_LABELLED_H

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
