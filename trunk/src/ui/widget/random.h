/**
 * \brief Random Scalar Widget - A labelled text box, with spin buttons and optional
 *        icon or suffix, for entering arbitrary number values and generating a random number from it.
 *
 * Authors:
 *  Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 2007 Author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_RANDOM_H
#define INKSCAPE_UI_WIDGET_RANDOM_H

#include "scalar.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class Random : public Scalar
{
public:
    Random(Glib::ustring const &label,
           Glib::ustring const &tooltip,
           Glib::ustring const &suffix = "",
           Glib::ustring const &icon = "",
           bool mnemonic = true);
    Random(Glib::ustring const &label,
           Glib::ustring const &tooltip,
           unsigned digits,
           Glib::ustring const &suffix = "",
           Glib::ustring const &icon = "",
           bool mnemonic = true);
    Random(Glib::ustring const &label,
           Glib::ustring const &tooltip,
           Gtk::Adjustment &adjust,
           unsigned digits = 0,
           Glib::ustring const &suffix = "",
           Glib::ustring const &icon = "",
           bool mnemonic = true);

    long getStartSeed() const;
    void setStartSeed(long newseed);

    sigc::signal <void> signal_reseeded;

protected:
    long startseed;

private:
    void addReseedButton();
    void onReseedButtonClick();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
