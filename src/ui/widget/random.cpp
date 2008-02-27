/**
 * \brief Scalar Widget - A labelled text box, with spin buttons and optional
 *        icon or suffix, for entering arbitrary number values. It adds an extra
 *       number called "startseed", that is not UI edittable, but should be put in SVG.
 *      This does NOT generate a random number, but provides merely the saving of 
 *      the startseed value.
 *
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

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Construct a Random scalar Widget.
 *
 * \param label     Label.
 * \param suffix    Suffix, placed after the widget (defaults to "").
 * \param icon      Icon filename, placed before the label (defaults to "").
 * \param mnemonic  Mnemonic toggle; if true, an underscore (_) in the label
 *                  indicates the next character should be used for the
 *                  mnemonic accelerator key (defaults to false).
 */
Random::Random(Glib::ustring const &label, Glib::ustring const &tooltip,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Scalar(label, tooltip, suffix, icon, mnemonic)
{
    startseed = 0;
    addReseedButton();
}

/**
 * Construct a  Random Scalar Widget.
 *
 * \param label     Label.
 * \param digits    Number of decimal digits to display.
 * \param suffix    Suffix, placed after the widget (defaults to "").
 * \param icon      Icon filename, placed before the label (defaults to "").
 * \param mnemonic  Mnemonic toggle; if true, an underscore (_) in the label
 *                  indicates the next character should be used for the
 *                  mnemonic accelerator key (defaults to false).
 */
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

/**
 * Construct a Random Scalar Widget.
 *
 * \param label     Label.
 * \param adjust    Adjustment to use for the SpinButton.
 * \param digits    Number of decimal digits to display (defaults to 0).
 * \param suffix    Suffix, placed after the widget (defaults to "").
 * \param icon      Icon filename, placed before the label (defaults to "").
 * \param mnemonic  Mnemonic toggle; if true, an underscore (_) in the label
 *                  indicates the next character should be used for the
 *                  mnemonic accelerator key (defaults to true).
 */
Random::Random(Glib::ustring const &label, Glib::ustring const &tooltip,
               Gtk::Adjustment &adjust,
               unsigned digits,
               Glib::ustring const &suffix,
               Glib::ustring const &icon,
               bool mnemonic)
    : Scalar(label, tooltip, adjust, digits, suffix, icon, mnemonic)
{
    startseed = 0;
    addReseedButton();
}

/** Gets the startseed  */
long
Random::getStartSeed() const
{
    return startseed;
}

/** Sets the startseed number */
void
Random::setStartSeed(long newseed)
{
    startseed = newseed;
}

/** Add reseed button to the widget */
void
Random::addReseedButton()
{
    Gtk::Widget*  pIcon = Gtk::manage( sp_icon_get_icon( "randomize", Inkscape::ICON_SIZE_BUTTON) );
    Gtk::Button * pButton = Gtk::manage(new Gtk::Button());
    pButton->set_relief(Gtk::RELIEF_NONE);
    pIcon->show();
    pButton->add(*pIcon);
    pButton->show();
    pButton->signal_clicked().connect(sigc::mem_fun(*this, &Random::onReseedButtonClick));
    _tooltips.set_tip(*pButton, _("Reseed the random number generator; this creates a different sequence of random numbers."));

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
