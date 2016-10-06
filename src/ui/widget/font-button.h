/*
 *
 * Copyright (C) 2007 Author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_FONT_BUTTON_H
#define INKSCAPE_UI_WIDGET_FONT_BUTTON_H

#include <gtkmm.h>
#include "labelled.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * A labelled font button for entering font values
 */
class FontButton : public Labelled
{
public:
    /**
     * Construct a FontButton Widget.
     *
     * @param label     Label.
     * @param suffix    Suffix, placed after the widget (defaults to "").
     * @param icon      Icon filename, placed before the label (defaults to "").
     * @param mnemonic  Mnemonic toggle; if true, an underscore (_) in the label
     *                  indicates the next character should be used for the
     *                  mnemonic accelerator key (defaults to false).
     */
    FontButton( Glib::ustring const &label,
           Glib::ustring const &tooltip,
           Glib::ustring const &suffix = "",
           Glib::ustring const &icon = "",
           bool mnemonic = true);

    Glib::ustring getValue() const;
    void setValue (Glib::ustring fontspec);
    /**
    * Signal raised when the font button's value changes.
    */
    Glib::SignalProxy0<void> signal_font_value_changed();
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_RANDOM_H

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
