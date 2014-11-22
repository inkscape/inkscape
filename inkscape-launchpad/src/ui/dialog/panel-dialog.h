/** @file
 * @brief A panel holding dialog
 */
/* Authors:
 *   C 2007 Gustav Broberg <broberg@kth.se>
 *   C 2012 Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_PANEL_DIALOG_H
#define INKSCAPE_PANEL_DIALOG_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>

#include "verbs.h"
#include "dialog.h"
#include "ui/dialog/swatches.h"
#include "ui/dialog/floating-behavior.h"
#include "ui/dialog/dock-behavior.h"
#include "preferences.h"
#include "inkscape.h"
#include "desktop.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

/**
 * Auxiliary class for the link between UI::Dialog::PanelDialog and UI::Dialog::Dialog.
 *
 * PanelDialog handles signals emitted when a desktop changes, either changing to a
 * different desktop or a new document.
 */
class PanelDialogBase {
public:
    PanelDialogBase(UI::Widget::Panel &panel, char const */*prefs_path*/, int const /*verb_num*/,
                    Glib::ustring const &/*apply_label*/) :
      _panel (panel) { }

    virtual void present() = 0;
    virtual ~PanelDialogBase() {}

    virtual UI::Widget::Panel &getPanel() { return _panel; }

protected:
    static void handle_deactivate_desktop(InkscapeApplication *application, SPDesktop *desktop, void *data) {
        g_return_if_fail(data != NULL);
        static_cast<PanelDialogBase *>(data)->_propagateDesktopDeactivated(application, desktop);
    }

    static void _handle_activate_desktop(InkscapeApplication *application, SPDesktop *desktop, void *data) {
        g_return_if_fail(data != NULL);
        static_cast<PanelDialogBase *>(data)->_propagateDesktopActivated(application, desktop);
    }

    inline virtual void _propagateDocumentReplaced(SPDesktop* desktop, SPDocument *document);
    inline virtual void _propagateDesktopActivated(InkscapeApplication *, SPDesktop *);
    inline virtual void _propagateDesktopDeactivated(InkscapeApplication *, SPDesktop *);

    UI::Widget::Panel &_panel;
    sigc::connection _document_replaced_connection;
};

/**
 * Bridges UI::Widget::Panel and UI::Dialog::Dialog.
 *
 * Where Dialog handles window behaviour, such as closing, position, etc, and where
 * Panel is the actual container for dialog child widgets (and from where the dialog
 * content is made), PanelDialog links these two classes together to create a
 * dockable and floatable dialog. The link with Dialog is made via PanelDialogBase.
 */
template <typename Behavior>
class PanelDialog : public PanelDialogBase, public Inkscape::UI::Dialog::Dialog {

public:
    /**
     * Constructor.
     * 
     * @param contents panel with the actual dialog content.
     * @param prefs_path characteristic path for loading/saving dialog position.
     * @param verb_num the dialog verb.
     */
    PanelDialog(UI::Widget::Panel &contents, char const *prefs_path, int const verb_num,
                Glib::ustring const &apply_label);

    virtual ~PanelDialog() {}

    template <typename T>
    static PanelDialog<Behavior> *create();

    inline virtual void present();

private:
    inline void _presentDialog();

    PanelDialog();
    PanelDialog(PanelDialog<Behavior> const &d);                      // no copy
    PanelDialog<Behavior>& operator=(PanelDialog<Behavior> const &d); // no assign
};


template <>
class PanelDialog<Behavior::FloatingBehavior> :
        public PanelDialogBase, public Inkscape::UI::Dialog::Dialog {

public:
    inline PanelDialog(UI::Widget::Panel &contents, char const *prefs_path, int const verb_num,
                       Glib::ustring const &apply_label);

    virtual ~PanelDialog() {}

    template <typename T>
    static PanelDialog<Behavior::FloatingBehavior> *create();

    inline virtual void present();

private:
    PanelDialog();
    PanelDialog(PanelDialog<Behavior::FloatingBehavior> const &d); // no copy
    PanelDialog<Behavior::FloatingBehavior>&
    operator=(PanelDialog<Behavior::FloatingBehavior> const &d);   // no assign
};



void PanelDialogBase::_propagateDocumentReplaced(SPDesktop *desktop, SPDocument *document)
{
    _panel.signalDocumentReplaced().emit(desktop, document);
}

void PanelDialogBase::_propagateDesktopActivated(InkscapeApplication *application, SPDesktop *desktop)
{
    _document_replaced_connection =
        desktop->connectDocumentReplaced(sigc::mem_fun(*this, &PanelDialogBase::_propagateDocumentReplaced));
    _panel.signalActivateDesktop().emit(application, desktop);
}

void PanelDialogBase::_propagateDesktopDeactivated(InkscapeApplication *application, SPDesktop *desktop)
{
    _document_replaced_connection.disconnect();
    _panel.signalDeactiveDesktop().emit(application, desktop);
}


template <typename B>
PanelDialog<B>::PanelDialog(Widget::Panel &panel, char const *prefs_path, int const verb_num,
                            Glib::ustring const &apply_label) :
    PanelDialogBase(panel, prefs_path, verb_num, apply_label),
    Dialog(&B::create, prefs_path, verb_num, apply_label)
{
    Gtk::Box *vbox = get_vbox();
    _panel.signalResponse().connect(sigc::mem_fun(*this, &PanelDialog::_handleResponse));
    _panel.signalPresent().connect(sigc::mem_fun(*this, &PanelDialog::_presentDialog));

    vbox->pack_start(_panel, true, true, 0);

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    _propagateDesktopActivated(INKSCAPE, desktop);

    _document_replaced_connection =
        desktop->connectDocumentReplaced(sigc::mem_fun(*this, &PanelDialog::_propagateDocumentReplaced));

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/dialogs/showclose") || !apply_label.empty()) {
        // TODO: make the order of buttons obey the global preference
        if (!apply_label.empty()) {
            panel.addResponseButton(apply_label, Gtk::RESPONSE_APPLY);
            panel.setDefaultResponse(Gtk::RESPONSE_APPLY);
        }
        panel.addResponseButton(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
    }

    show_all_children();
}

template <typename B> template <typename P>
PanelDialog<B> *PanelDialog<B>::create()
{
    UI::Widget::Panel &panel = P::getInstance();
    return new PanelDialog<B>(panel, panel.getPrefsPath(), panel.getVerb(), panel.getApplyLabel());
}

template <typename B>
void PanelDialog<B>::present()
{
    _panel.present();
}

template <typename B>
void PanelDialog<B>::_presentDialog()
{
    Dialog::present();
}

PanelDialog<Behavior::FloatingBehavior>::PanelDialog(UI::Widget::Panel &panel, char const *prefs_path,
                                                     int const verb_num, Glib::ustring const &apply_label) :
    PanelDialogBase(panel, prefs_path, verb_num, apply_label),
    Dialog(&Behavior::FloatingBehavior::create, prefs_path, verb_num, apply_label)
{
    Gtk::Box *vbox = get_vbox();
    _panel.signalResponse().connect(sigc::mem_fun(*this, &PanelDialog::_handleResponse));

    vbox->pack_start(_panel, true, true, 0);

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    _propagateDesktopActivated(INKSCAPE, desktop);

    _document_replaced_connection =
        desktop->connectDocumentReplaced(sigc::mem_fun(*this, &PanelDialog::_propagateDocumentReplaced));

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/dialogs/showclose") || !apply_label.empty()) {
        // TODO: make the order of buttons obey the global preference
        if (!apply_label.empty()) {
            panel.addResponseButton(apply_label, Gtk::RESPONSE_APPLY);
            panel.setDefaultResponse(Gtk::RESPONSE_APPLY);
        }
        panel.addResponseButton(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
    }

    show_all_children();
}

void PanelDialog<Behavior::FloatingBehavior>::present()
{
    Dialog::present();
    _panel.present();
}

/**
 * Specialized factory method for panel dialogs with floating behavior in order to make them work as
 * singletons, i.e. allow them track the current active desktop.
 */
template <typename P>
PanelDialog<Behavior::FloatingBehavior> *PanelDialog<Behavior::FloatingBehavior>::create()
{
    UI::Widget::Panel &panel = P::getInstance();
    PanelDialog<Behavior::FloatingBehavior> *instance =
        new PanelDialog<Behavior::FloatingBehavior>(panel, panel.getPrefsPath(),
                                                    panel.getVerb(), panel.getApplyLabel());

    g_signal_connect(G_OBJECT(INKSCAPE), "activate_desktop", G_CALLBACK(_handle_activate_desktop), instance);
    g_signal_connect(G_OBJECT(INKSCAPE), "deactivate_desktop", G_CALLBACK(handle_deactivate_desktop), instance);

    return instance;
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //INKSCAPE_PANEL_DIALOG_H

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
