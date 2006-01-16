/**
 * \brief Toolbox Widget - A detachable toolbar for buttons and other widgets.
 *
 * Author:
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Derek P. Moore
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_TOOLBOX_H
#define INKSCAPE_UI_WIDGET_TOOLBOX_H

#include <gtkmm/toolbar.h>
#include <gtkmm/menu.h>
#include <gtkmm/action.h>
#include <gtkmm/uimanager.h>
#include "handlebox.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class Toolbox : public HandleBox
{
public:
    Toolbox(Gtk::Toolbar *toolbar,
            Gtk::Orientation orientation = Gtk::ORIENTATION_HORIZONTAL);
    Toolbox(Gtk::Toolbar *toolbar,
            Gtk::ToolbarStyle style,
            Gtk::Orientation orientation = Gtk::ORIENTATION_HORIZONTAL);

    Gtk::Toolbar& get_toolbar();

protected:
    Gtk::Menu *_context_menu;

    Glib::RefPtr<Gtk::ActionGroup>  _action_grp;
    Glib::RefPtr<Gtk::ActionGroup>  _detach_grp;
    Glib::RefPtr<Gtk::UIManager>    _ui_mgr;

    void init_actions();
    void init_orientation(Gtk::Orientation const &orientation);
    void init_style(Gtk::ToolbarStyle const &style);
    bool on_popup_context_menu(int x, int y, int button);
    void on_child_attached(Gtk::Widget *widget);
    void on_child_detached(Gtk::Widget *widget);
    void on_change_style_icons();
    void on_change_style_text();
    void on_change_style_both();
    void on_change_style_both_horiz();
    void on_change_orient_horiz();
    void on_change_orient_vert();
    void on_show_arrow();
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_TOOLBOX_H

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
