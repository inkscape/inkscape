/**
 * \brief SpinButton widget, that allows entry of both '.' and ',' for the decimal, even when in numeric mode.
 */
/*
 * Author:
 *   Johan B. C. Engelen
 *
 * Copyright (C) 2011 Author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "spinbutton.h"

#include <locale.h>

namespace Inkscape {
namespace UI {
namespace Widget {

void
SpinButton::on_insert_text(const Glib::ustring& text, int* position)
{
    Glib::ustring newtext = text;

    // if in numeric mode: replace '.' or ',' with the locale's decimal point
    if (get_numeric()) {
        size_t found = newtext.find('.');
        if (found != Glib::ustring::npos) {
            newtext.replace(found, 1, localeconv()->decimal_point);
        } else {
            found = newtext.find(',');
            if (found != Glib::ustring::npos) {
                newtext.replace(found, 1, localeconv()->decimal_point);
            }
        }
    }

    // call parent function with replaced text:
    Gtk::SpinButton::on_insert_text(newtext, position);
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

/* 
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
