/** @file
 * Entry widget for typing color value in css form
 *//*
 * Authors:
 *   Tomasz Boczkowski <penginsbacon@gmail.com>
 *
 * Copyright (C) 2014 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include <glibmm.h>
#include <glibmm/i18n.h>
#include <iomanip>

#include "color-entry.h"

namespace Inkscape {
namespace UI {
namespace Widget {

ColorEntry::ColorEntry(SelectedColor &color)
    : _color(color)
    , _updating(false)
    , _updatingrgba(false)
{
    _color_changed_connection = color.signal_changed.connect(sigc::mem_fun(this, &ColorEntry::_onColorChanged));
    _color_dragged_connection = color.signal_dragged.connect(sigc::mem_fun(this, &ColorEntry::_onColorChanged));
    _onColorChanged();

    set_max_length(8);
    set_width_chars(8);
    set_tooltip_text(_("Hexadecimal RGBA value of the color"));
}

ColorEntry::~ColorEntry()
{
    _color_changed_connection.disconnect();
    _color_dragged_connection.disconnect();
}

void ColorEntry::on_changed()
{
    if (_updating) {
        return;
    }
    if (_updatingrgba) {
        return;  // Typing text into entry box
    }

    Glib::ustring text = get_text();
    bool changed = false;

    // Coerce the value format to eight hex digits
    if (!text.empty() && text[0] == '#') {
        changed = true;
        text.erase(0, 1);
        if (text.size() == 6) {
            // it was a standard RGB hex
            unsigned int alpha = SP_COLOR_F_TO_U(_color.alpha());
            text += Glib::ustring::format(std::hex, std::setw(2), std::setfill(L'0'), alpha);
        }
    }

    gchar *str = g_strdup(text.c_str());
    gchar *end = 0;
    guint64 rgba = g_ascii_strtoull(str, &end, 16);
    if (end != str) {
        ptrdiff_t len = end - str;
        if (len < 8) {
            rgba = rgba << (4 * (8 - len));
        }
        _updatingrgba = true;
        if (changed) {
            set_text(str);
        }
        SPColor color(rgba);
        _color.setColorAlpha(color, SP_RGBA32_A_F(rgba));
        _updatingrgba = false;
    }
    g_free(str);
}


void ColorEntry::_onColorChanged()
{
    if (_updatingrgba) {
        return;
    }

    SPColor color = _color.color();
    gdouble alpha = _color.alpha();

    guint32 rgba = color.toRGBA32(alpha);
    Glib::ustring text = Glib::ustring::format(std::hex, std::setw(8), std::setfill(L'0'), rgba);

    Glib::ustring old_text = get_text();
    if (old_text != text) {
        _updating = true;
        set_text(text);
        _updating = false;
    }
}
}
}
}
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
