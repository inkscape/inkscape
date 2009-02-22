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
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/menu.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/table.h>
#include <gtkmm/tooltips.h>

#include "inkscape.h"
#include "ui/previewfillable.h"
#include "selection.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class Panel : public Gtk::VBox {

public:
    static void prep();

    virtual ~Panel();
    Panel(Glib::ustring const &label = "", gchar const *prefs_path = 0,
          int verb_num = 0, Glib::ustring const &apply_label = "",
          bool menu_desired = false);

    gchar const *getPrefsPath() const;
    void setLabel(Glib::ustring const &label);
    Glib::ustring const &getLabel() const;
    int const &getVerb() const;
    Glib::ustring const &getApplyLabel() const;

    virtual void setOrientation(Gtk::AnchorType how);

    virtual void present();  //< request to be present

    void restorePanelPrefs();

    virtual void setDesktop(SPDesktop *desktop);
    SPDesktop *getDesktop() { return _desktop; }

    /** Signal accessors */
    virtual sigc::signal<void, int> &signalResponse();
    virtual sigc::signal<void> &signalPresent();

    /** Methods providing a Gtk::Dialog like interface for adding buttons that emit Gtk::RESPONSE
     *  signals on click. */
    Gtk::Button* addResponseButton (const Glib::ustring &button_text, int response_id);
    Gtk::Button* addResponseButton (const Gtk::StockID &stock_id, int response_id);
    void setDefaultResponse(int response_id);
    void setResponseSensitive(int response_id, bool setting);

    virtual sigc::signal<void, SPDesktop *, SPDocument *> &signalDocumentReplaced();
    virtual sigc::signal<void, Inkscape::Application *, SPDesktop *> &signalActivateDesktop();
    virtual sigc::signal<void, Inkscape::Application *, SPDesktop *> &signalDeactiveDesktop();

protected:
    Gtk::Box *_getContents() { return &_contents; }
    void _setTargetFillable(PreviewFillable *target);
    void _regItem(Gtk::MenuItem* item, int group, int id);

    virtual void _handleAction(int set_id, int item_id);
    virtual void _apply();

    virtual void _handleResponse(int response_id);

    /** Helper methods */
    void _addResponseButton(Gtk::Button *button, int response_id);
    Inkscape::Selection *_getSelection();

    /** Tooltips object for all descendants to use */
    Gtk::Tooltips _tooltips;

    Glib::ustring const _prefs_path;

    bool _menu_desired;
    Gtk::AnchorType _anchor;

    /** Signals */
    sigc::signal<void, int> _signal_response;
    sigc::signal<void>      _signal_present;
    sigc::signal<void, SPDesktop *, SPDocument *> _signal_document_replaced;
    sigc::signal<void, Inkscape::Application *, SPDesktop *> _signal_activate_desktop;
    sigc::signal<void, Inkscape::Application *, SPDesktop *> _signal_deactive_desktop;

private:
    void _init();
    void _bounceCall(int i, int j);

    void _popper(GdkEventButton *btn);
    void _wrapToggled(Gtk::CheckMenuItem *toggler);

    SPDesktop       *_desktop;

    Glib::ustring    _label;
    Glib::ustring    _apply_label;
    int              _verb_num;

    Gtk::HBox        _top_bar;
    Gtk::VBox        _right_bar;
    Gtk::VBox        _contents;
    Gtk::Label       _tab_title;
    Gtk::Arrow       _temp_arrow;
    Gtk::EventBox    _menu_popper;
    Gtk::Button      _close_button;
    Gtk::Menu       *_menu;
    Gtk::HButtonBox *_action_area;  //< stores response buttons
    std::vector<Gtk::Widget *> _non_horizontal;
    std::vector<Gtk::Widget *> _non_vertical;
    PreviewFillable *_fillable;

    /** A map to store which widget that emits a certain response signal */
    typedef std::map<int, Gtk::Widget *> ResponseMap;
    ResponseMap _response_map;
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
