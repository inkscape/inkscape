/*
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *   Derek P. Moore <derekm@hackunix.org>
 *   Bryce Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Carl Hetherington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "random.h"
#include "widgets/icon.h"

#include <glibmm/i18n.h>

#include <gtkmm/button.h>

namespace Inkscape {
namespace UI {
namespace Widget {

Random::Random(Glib::ustring const &label, Glib::ustring const &tooltip,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Scalar(label, tooltip, suffix, icon, mnemonic)
{
    startseed = 0;
    addReseedButton();
}

Random::Random(Glib::ustring const &label, Glib::ustring const &tooltip,
               unsigned digits,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Scalar(label, tooltip, digits, suffix, icon, mnemonic)
{
    startseed = 0;
    addReseedButton();
}

Random::Random(Glib::ustring const &label, Glib::ustring const &tooltip,
#if WITH_GTKMM_3_0
               Glib::RefPtr<Gtk::Adjustment> &adjust,
#else
               Gtk::Adjustment &adjust,
#endif
               unsigned digits,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Scalar(label, tooltip, adjust, digits, suffix, icon, mnemonic)
{
    startseed = 0;
    addReseedButton();
}

long Random::getStartSeed() const
{
    return startseed;
}

void Random::setStartSeed(long newseed)
{
    startseed = newseed;
}

void Random::addReseedButton()
{
    Gtk::Widget*  pIcon = Gtk::manage( sp_icon_get_icon( "randomize", Inkscape::ICON_SIZE_BUTTON) );
    Gtk::Button * pButton = Gtk::manage(new Gtk::Button());
    pButton->set_relief(Gtk::RELIEF_NONE);
    pIcon->show();
    pButton->add(*pIcon);
    pButton->show();
    pButton->signal_clicked().connect(sigc::mem_fun(*this, &Random::onReseedButtonClick));
    pButton->set_tooltip_text(_("Reseed the random number generator; this creates a different sequence of random numbers."));

    pack_start(*pButton, Gtk::PACK_SHRINK, 0);
}

void
Random::onReseedButtonClick()
{
    startseed = g_random_int();
    signal_reseeded.emit();
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
