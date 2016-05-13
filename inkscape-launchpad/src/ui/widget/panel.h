/*
 * Authors:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 * Copyright (C) 2005 Jon A. Cruz
 * Copyright (C) 2012 Kris De Gussem
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef SEEN_INKSCAPE_UI_WIDGET_PANEL_H
#define SEEN_INKSCAPE_UI_WIDGET_PANEL_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/box.h>
#include <gtkmm/arrow.h>
#include <gtkmm/button.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>
#include "enums.h"
#include <vector>
#include <map>

class SPDesktop;
class SPDocument;

namespace Gtk {
	class CheckMenuItem;

#if WITH_GTKMM_3_0
	class ButtonBox;
#else
	class HButtonBox;
#endif

	class MenuItem;
}

struct InkscapeApplication;

namespace Inkscape {

class Selection;

namespace UI {

class PreviewFillable;

namespace Widget {

/**
 * A generic dockable container.
 *
 * Inkscape::UI::Widget::Panel is a base class from which dockable dialogs
 * are created. A new dockable dialog is created by deriving a class from panel.
 * Child widgets are private data members of Panel (no need to use pointers and
 * new).
 *
 * @see UI::Dialog::DesktopTracker to handle desktop change, selection change and selected object modifications.
 * @see UI::Dialog::DialogManager manages the dialogs within inkscape.
 */
#if WITH_GTKMM_3_0
class Panel : public Gtk::Box {
#else
class Panel : public Gtk::VBox {
#endif

public:
    static void prep();

    /**
     * Construct a Panel.
     *
     * @param label label for the panel of a dialog, shown at the top.
     * @param prefs_path characteristic path to load/save dialog position.
     * @param verb_num the dialog verb.
     */
    Panel(Glib::ustring const &label = "", gchar const *prefs_path = 0,
          int verb_num = 0, Glib::ustring const &apply_label = "",
          bool menu_desired = false);

    virtual ~Panel();

    gchar const *getPrefsPath() const;
    
    /**
     * Sets a label for the panel and displays it in the panel at the top (is not the title bar of a floating dialog).
     */
    void setLabel(Glib::ustring const &label);
    Glib::ustring const &getLabel() const;
    int const &getVerb() const;
    Glib::ustring const &getApplyLabel() const;

    virtual void setOrientation(SPAnchorType how);

    virtual void present();  //< request to be present

    void restorePanelPrefs();

    virtual void setDesktop(SPDesktop *desktop);
    SPDesktop *getDesktop() { return _desktop; }

    /* Signal accessors */
    virtual sigc::signal<void, int> &signalResponse();
    virtual sigc::signal<void> &signalPresent();

    /* Methods providing a Gtk::Dialog like interface for adding buttons that emit Gtk::RESPONSE
     * signals on click. */
    Gtk::Button* addResponseButton (const Glib::ustring &button_text, int response_id, bool pack_start=false);
    Gtk::Button* addResponseButton (const Gtk::StockID &stock_id, int response_id, bool pack_start=false);
    void setDefaultResponse(int response_id);
    void setResponseSensitive(int response_id, bool setting);

    virtual sigc::signal<void, SPDesktop *, SPDocument *> &signalDocumentReplaced();
    virtual sigc::signal<void, SPDesktop *> &signalActivateDesktop();
    virtual sigc::signal<void, SPDesktop *> &signalDeactiveDesktop();

protected:
    /**
     * Returns a pointer to a Gtk::Box containing the child widgets.
     */
    Gtk::Box *_getContents() { return &_contents; }
    void _setTargetFillable(PreviewFillable *target);
    void _regItem(Gtk::MenuItem* item, int group, int id);

    virtual void _handleAction(int set_id, int item_id);
    virtual void _apply();

    virtual void _handleResponse(int response_id);

    /* Helper methods */
    void _addResponseButton(Gtk::Button *button, int response_id, bool pack_start=false);
    Inkscape::Selection *_getSelection();

    /**
     * Stores characteristic path for loading/saving the dialog position.
     */
    Glib::ustring const _prefs_path;
    bool _menu_desired;
    SPAnchorType _anchor;

    /* Signals */
    sigc::signal<void, int> _signal_response;
    sigc::signal<void>      _signal_present;
    sigc::signal<void, SPDesktop *, SPDocument *> _signal_document_replaced;
    sigc::signal<void, SPDesktop *> _signal_activate_desktop;
    sigc::signal<void, SPDesktop *> _signal_deactive_desktop;

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

#if WITH_GTKMM_3_0
    Gtk::ButtonBox *_action_area;  //< stores response buttons
#else
    Gtk::HButtonBox *_action_area;  //< stores response buttons
#endif

    std::vector<Gtk::Widget *> _non_horizontal;
    std::vector<Gtk::Widget *> _non_vertical;
    PreviewFillable *_fillable;

    /* A map to store which widget that emits a certain response signal */
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
