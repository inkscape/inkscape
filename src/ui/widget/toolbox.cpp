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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/radioaction.h>
#include <gtk/gtkmain.h>
#include "ui/widget/toolbox.h"
#include "path-prefix.h"

namespace Inkscape {
namespace UI {
namespace Widget {

Toolbox::Toolbox(Gtk::Toolbar *toolbar,
                 Gtk::Orientation orientation)
    : HandleBox(toolbar)
{
    init_actions();
    init_orientation(orientation);
    init_style(toolbar->get_toolbar_style());
}

Toolbox::Toolbox(Gtk::Toolbar *toolbar,
                 Gtk::ToolbarStyle style,
                 Gtk::Orientation orientation)
    : HandleBox(toolbar)
{
    init_actions();
    init_orientation(orientation);
    init_style(style);
}

Gtk::Toolbar&
Toolbox::get_toolbar()
{
    return static_cast<Gtk::Toolbar&>(*_widget);
}

static Glib::ustring
get_uidir_filename(char const *basename_utf8)
{
    char *const ret_str = g_build_filename(INKSCAPE_UIDIR, basename_utf8, NULL);
    Glib::ustring const ret(ret_str);
    g_free(ret_str);
    return ret;
}

void
Toolbox::init_actions()
{
    _action_grp = Gtk::ActionGroup::create();

    Gtk::RadioAction::Group icons;
    _action_grp->add(Gtk::RadioAction::create(icons, "Icons", "Icons Only"),
                     sigc::mem_fun(*this, &Toolbox::on_change_style_icons));
    _action_grp->add(Gtk::RadioAction::create(icons, "Text", "Text Only"),
                     sigc::mem_fun(*this, &Toolbox::on_change_style_text));
    _action_grp->add(Gtk::RadioAction::create(icons, "Both", "Text Below Icons"),
                     sigc::mem_fun(*this, &Toolbox::on_change_style_both));
    _action_grp->add(Gtk::RadioAction::create(icons, "BothHoriz", "Text Beside Icons"),
                     sigc::mem_fun(*this, &Toolbox::on_change_style_both_horiz));

    _detach_grp = Gtk::ActionGroup::create();

    Gtk::RadioAction::Group orient;
    _detach_grp->add(Gtk::RadioAction::create(orient, "OrientHoriz", "Horizontal"),
                     sigc::mem_fun(*this, &Toolbox::on_change_orient_horiz));
    _detach_grp->add(Gtk::RadioAction::create(orient, "OrientVert", "Vertical"),
                     sigc::mem_fun(*this, &Toolbox::on_change_orient_vert));

    _detach_grp->add(Gtk::ToggleAction::create("ShowArrow", "Show Arrow",
                     Glib::ustring(), true),
                     sigc::mem_fun(*this, &Toolbox::on_show_arrow));

    _ui_mgr = Gtk::UIManager::create();
    _ui_mgr->insert_action_group(_action_grp);
    _ui_mgr->insert_action_group(_detach_grp);
    _ui_mgr->add_ui_from_file(get_uidir_filename("toolbox.xml"));

    _context_menu = static_cast<Gtk::Menu*>(_ui_mgr->get_widget("/ToolboxMenu"));

    static_cast<Gtk::Toolbar*>(_widget)->signal_popup_context_menu()
        .connect(sigc::mem_fun(*this, &Toolbox::on_popup_context_menu));

    _detach_grp->set_sensitive(false);
}

void
Toolbox::init_orientation(Gtk::Orientation const &orientation)
{
    static_cast<Gtk::Toolbar*>(_widget)->set_orientation(orientation);
    if (orientation == Gtk::ORIENTATION_VERTICAL) {
        set_handle_position(Gtk::POS_TOP);
    }
    switch (orientation) {
        case Gtk::ORIENTATION_HORIZONTAL: {
            Glib::RefPtr<Gtk::RadioAction>::cast_static(_detach_grp->get_action("OrientHoriz"))
                ->set_active();
            break;
        }
        case Gtk::ORIENTATION_VERTICAL: {
            Glib::RefPtr<Gtk::RadioAction>::cast_static(_detach_grp->get_action("OrientVert"))
                ->set_active();
            break;
        }
    }
}

void
Toolbox::init_style(Gtk::ToolbarStyle const &style)
{
    switch (style) {
        case Gtk::TOOLBAR_ICONS: {
            Glib::RefPtr<Gtk::RadioAction>::cast_static(_action_grp->get_action("Icons"))
                ->activate();
            break;
        }
        case Gtk::TOOLBAR_TEXT: {
            Glib::RefPtr<Gtk::RadioAction>::cast_static(_action_grp->get_action("Text"))
                ->activate();
            break;
        }
        case Gtk::TOOLBAR_BOTH: {
            Glib::RefPtr<Gtk::RadioAction>::cast_static(_action_grp->get_action("Both"))
                ->activate();
            break;
        }
        case Gtk::TOOLBAR_BOTH_HORIZ: {
            Glib::RefPtr<Gtk::RadioAction>::cast_static(_action_grp->get_action("BothHoriz"))
                ->activate();
            break;
        }
    }
}

bool
Toolbox::on_popup_context_menu(int x, int y, int button)
{
    (void)x;
    (void)y;
    _context_menu->popup(button, gtk_get_current_event_time());
    return true;
}

void
Toolbox::on_child_attached(Gtk::Widget *widget)
{
    (void)widget;
    Gtk::Toolbar *toolbar = static_cast<Gtk::Toolbar*>(_widget);

    if (!(toolbar->get_show_arrow())) {
        Glib::RefPtr<Gtk::RadioAction>::cast_static(_detach_grp->get_action("ShowArrow"))
            ->activate();
    }

    if (get_handle_position() == Gtk::POS_LEFT
        && toolbar->get_orientation() != Gtk::ORIENTATION_HORIZONTAL) {
        Glib::RefPtr<Gtk::RadioAction>::cast_static(_detach_grp->get_action("OrientHoriz"))
            ->activate();
    } else if (get_handle_position() == Gtk::POS_TOP
               && toolbar->get_orientation() != Gtk::ORIENTATION_VERTICAL) {
        Glib::RefPtr<Gtk::RadioAction>::cast_static(_detach_grp->get_action("OrientVert"))
            ->activate();
    }

    _detach_grp->set_sensitive(false);
}

void
Toolbox::on_child_detached(Gtk::Widget *widget)
{
    (void)widget;
    _detach_grp->set_sensitive(true);

    Glib::RefPtr<Gtk::RadioAction>::cast_static(_detach_grp->get_action("ShowArrow"))
        ->set_active(false);
}

void
Toolbox::on_change_style_icons()
{
    Glib::RefPtr<Gtk::RadioAction> action = Glib::RefPtr<Gtk::RadioAction>::cast_static(_action_grp->get_action("Icons"));
    if (action->get_active()) {
        static_cast<Gtk::Toolbar*>(_widget)->set_toolbar_style(Gtk::TOOLBAR_ICONS);
    }
}

void
Toolbox::on_change_style_text()
{
    Glib::RefPtr<Gtk::RadioAction> action = Glib::RefPtr<Gtk::RadioAction>::cast_static(_action_grp->get_action("Text"));
    if (action->get_active()) {
        static_cast<Gtk::Toolbar*>(_widget)->set_toolbar_style(Gtk::TOOLBAR_TEXT);
    }
}

void
Toolbox::on_change_style_both()
{
    Glib::RefPtr<Gtk::RadioAction> action = Glib::RefPtr<Gtk::RadioAction>::cast_static(_action_grp->get_action("Both"));
    if (action->get_active()) {
        static_cast<Gtk::Toolbar*>(_widget)->set_toolbar_style(Gtk::TOOLBAR_BOTH);
    }
}

void
Toolbox::on_change_style_both_horiz()
{
    Glib::RefPtr<Gtk::RadioAction> action = Glib::RefPtr<Gtk::RadioAction>::cast_static(_action_grp->get_action("BothHoriz"));
    if (action->get_active()) {
        static_cast<Gtk::Toolbar*>(_widget)->set_toolbar_style(Gtk::TOOLBAR_BOTH_HORIZ);
    }
}

void
Toolbox::on_change_orient_horiz()
{
    Glib::RefPtr<Gtk::RadioAction> action = Glib::RefPtr<Gtk::RadioAction>::cast_static(_detach_grp->get_action("OrientHoriz"));
    if (action->get_active()) {
        static_cast<Gtk::Toolbar*>(_widget)->set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    }
}

void
Toolbox::on_change_orient_vert()
{
    Glib::RefPtr<Gtk::RadioAction> action = Glib::RefPtr<Gtk::RadioAction>::cast_static(_detach_grp->get_action("OrientVert"));
    if (action->get_active()) {
        static_cast<Gtk::Toolbar*>(_widget)->set_orientation(Gtk::ORIENTATION_VERTICAL);
    }
}

void
Toolbox::on_show_arrow()
{
    Glib::RefPtr<Gtk::RadioAction> action = Glib::RefPtr<Gtk::RadioAction>::cast_static(_detach_grp->get_action("ShowArrow"));
    Gtk::Toolbar *toolbar = static_cast<Gtk::Toolbar*>(_widget);
    if (action->get_active()) {
        toolbar->set_show_arrow(true);
    } else {
        toolbar->set_show_arrow(false);
    }
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
