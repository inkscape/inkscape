/*
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "font-button.h"
#include <glibmm/i18n.h>

namespace Inkscape {
namespace UI {
namespace Widget {

FontButton::FontButton(Glib::ustring const &label, Glib::ustring const &tooltip,
              Glib::ustring const &suffix,
              Glib::ustring const &icon,
              bool mnemonic)
           : Labelled(label, tooltip, new Gtk::FontButton("Sans 10"), suffix, icon, mnemonic)
{
}

Glib::ustring FontButton::getValue() const
{
    g_assert(_widget != NULL);
    return static_cast<Gtk::FontButton*>(_widget)->get_font_name();
}


void FontButton::setValue (Glib::ustring fontspec)
{
    g_assert(_widget != NULL);
    static_cast<Gtk::FontButton*>(_widget)->set_font_name(fontspec);
}

Glib::SignalProxy0<void> FontButton::signal_font_value_changed()
{
    g_assert(_widget != NULL);
    return static_cast<Gtk::FontButton*>(_widget)->signal_font_set();
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
