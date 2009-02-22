#ifndef __SP_DASH_SELECTOR_NEW_H__
#define __SP_DASH_SELECTOR_NEW_H__

/** @file
 * @brief Option menu for selecting dash patterns
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Maximilian Albert <maximilian.albert> (gtkmm-ification)
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/ustring.h>
#include <gtkmm/box.h>
#include <sigc++/signal.h>

namespace Gtk {
class Container;
class OptionMenu;
class MenuItem;
class Adjustment;
}

// TODO: should we rather derive this from OptionMenu and add the spinbutton somehow else?
class SPDashSelector : public Gtk::HBox {
public:
    SPDashSelector();
    ~SPDashSelector();

    void set_dash(int ndash, double *dash, double offset);
    void get_dash(int *ndash, double **dash, double *offset);

    sigc::signal<void> changed_signal;

private:
    static void init_dashes();
    void dash_activate(Gtk::MenuItem *mi);
    void offset_value_changed();
    Gtk::MenuItem *menu_item_new(double *pattern);

    Gtk::OptionMenu *dash;
    Gtk::Adjustment *offset;
    
    static gchar const *const _prefs_path;
};

#endif

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
