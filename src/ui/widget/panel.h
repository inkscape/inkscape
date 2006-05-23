/**
 * \brief Generic Panel widget - A generic dockable container.
 *
 * Authors:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 * Copyright (C) 2005 Jon A. Cruz
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef SEEN_INKSCAPE_UI_WIDGET_PANEL_H
#define SEEN_INKSCAPE_UI_WIDGET_PANEL_H

#include <vector>
#include <gtkmm/arrow.h>
#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/menu.h>
#include <gtkmm/optionmenu.h>
#include "../previewfillable.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class Panel : public Gtk::VBox
{
public:
    Panel();
    virtual ~Panel();
    Panel(Glib::ustring const &label, gchar const *prefs_path = 0, bool menuDesired = false );

    void setLabel(Glib::ustring const &label);
    Glib::ustring const &getLabel() const;

    virtual void setOrientation( Gtk::AnchorType how );

    const gchar *_prefs_path;
    void restorePanelPrefs();

protected:
    Gtk::Box* _getContents() { return &contents; }
    void _setTargetFillable( PreviewFillable *target );
    void _regItem( Gtk::MenuItem* item, int group, int id );

    virtual void _handleAction( int setId, int itemId );
    bool _menuDesired;

    Gtk::AnchorType _anchor;

private:
    void init();
    void bounceCall(int i, int j);

    void _popper(GdkEventButton* btn);
    void _wrapToggled(Gtk::CheckMenuItem* toggler);

    Glib::ustring   label;

    Gtk::HBox       topBar;
    Gtk::VBox       rightBar;
    Gtk::VBox       contents;
    Gtk::Label      tabTitle;
    Gtk::Arrow      _tempArrow;
    Gtk::EventBox   menuPopper;
    Gtk::Button     closeButton;
    Gtk::Menu*       menu;
    std::vector<Gtk::Widget*> nonHorizontal;
    std::vector<Gtk::Widget*> nonVertical;
    PreviewFillable *_fillable;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // SEEN_INKSCAPE_UI_WIDGET_PANEL_H

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
