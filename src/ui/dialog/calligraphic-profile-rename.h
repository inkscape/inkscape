/** @file
 * @brief Dialog for naming calligraphic profiles
 */
/* Author:
 *   Aubanel MONNIER 
 *
 * Copyright (C) 2007 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_DIALOG_CALLIGRAPHIC_PROFILE_H
#define INKSCAPE_DIALOG_CALLIGRAPHIC_PROFILE_H

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>

namespace Gtk {
#if WITH_GTKMM_3_0
class Grid;
#else
class Table;
#endif
}

class SPDesktop;

namespace Inkscape {
namespace UI {
namespace Dialog {
      
class CalligraphicProfileRename : public Gtk::Dialog {  
public:
    CalligraphicProfileRename();
    virtual ~CalligraphicProfileRename() {}
    Glib::ustring getName() const {
        return "CalligraphicProfileRename";
    }
    
    static void show(SPDesktop *desktop, const Glib::ustring profile_name);
    static bool applied() {
        return instance()._applied;
    }
    static bool deleted() {
        return instance()._deleted;
    }
    static Glib::ustring getProfileName() {
        return instance()._profile_name;
    }

protected:
    void _close();
    void _apply();
    void _delete();

    Gtk::Label        _profile_name_label;
    Gtk::Entry        _profile_name_entry;

#if WITH_GTKMM_3_0
    Gtk::Grid*        _layout_table;
#else
    Gtk::Table*       _layout_table;
#endif

    Gtk::Button       _close_button;
    Gtk::Button       _delete_button;
    Gtk::Button       _apply_button;
    Glib::ustring _profile_name;
    bool _applied;
    bool _deleted;
private:
    static CalligraphicProfileRename &instance() {
        static CalligraphicProfileRename instance_;
        return instance_;
    }
    CalligraphicProfileRename(CalligraphicProfileRename const &); // no copy
    CalligraphicProfileRename &operator=(CalligraphicProfileRename const &); // no assign
};
 
} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_DIALOG_CALLIGRAPHIC_PROFILE_H

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
