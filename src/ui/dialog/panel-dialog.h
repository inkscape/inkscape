/** @file
 * @brief A panel holding dialog
 */
/* Authors:
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2007 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_PANEL_DIALOG_H
#define INKSCAPE_PANEL_DIALOG_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/stock.h>

#include "verbs.h"
#include "dialog.h"
#include "ui/dialog/swatches.h"
#include "ui/dialog/floating-behavior.h"
#include "ui/dialog/dock-behavior.h"
#include "preferences.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

class PanelDialogBase {
public:
    PanelDialogBase(Panel &panel, char const */*prefs_path*/, int const /*verb_num*/,
                    Glib::ustring const &/*apply_label*/) :
      _panel (panel) { }

    virtual void present() = 0;
    virtual ~PanelDialogBase() {}

    virtual Panel &getPanel() { return _panel; }

protected:
    static void handle_deactivate_desktop(Inkscape::Application *application, SPDesktop *desktop, void *data) {
        g_return_if_fail(data != NULL);
        static_cast<PanelDialogBase *>(data)->_propagateDesktopDeactivated(application, desktop);
    }

    static void _handle_activate_desktop(Inkscape::Application *application, SPDesktop *desktop, void *data) {
        g_return_if_fail(data != NULL);
        static_cast<PanelDialogBase *>(data)->_propagateDesktopActivated(application, desktop);
    }

    inline virtual void _propagateDocumentReplaced(SPDesktop* desktop, SPDocument *document);
    inline virtual void _propagateDesktopActivated(Inkscape::Application *, SPDesktop *);
    inline virtual void _propagateDesktopDeactivated(Inkscape::Application *, SPDesktop *);

    Panel &_panel;
    sigc::connection _document_replaced_connection;
};

template <typename Behavior>
class PanelDialog : public PanelDialogBase, public Inkscape::UI::Dialog::Dialog {

public:
    PanelDialog(Panel &contents, char const *prefs_path, int const verb_num,
                Glib::ustring const &apply_label);

    virtual ~PanelDialog() {}

    template <typename T>
    static PanelDialog<Behavior> *create();

    inline virtual void present();

private:
    inline void _presentDialog();

    PanelDialog();  // no constructor without params
    PanelDialog(PanelDialog<Behavior> const &d);                      // no copy
    PanelDialog<Behavior>& operator=(PanelDialog<Behavior> const &d); // no assign
};


template <>
class PanelDialog<Behavior::FloatingBehavior> :
        public PanelDialogBase, public Inkscape::UI::Dialog::Dialog {

public:
    inline PanelDialog(Panel &contents, char const *prefs_path, int const verb_num,
                       Glib::ustring const &apply_label);

    virtual ~PanelDialog() {}

    template <typename T>
    static PanelDialog<Behavior::FloatingBehavior> *create();

    inline virtual void present();

private:
    PanelDialog();  // no constructor without params
    PanelDialog(PanelDialog<Behavior::FloatingBehavior> const &d); // no copy
    PanelDialog<Behavior::FloatingBehavior>&
    operator=(PanelDialog<Behavior::FloatingBehavior> const &d);   // no assign
};



void
PanelDialogBase::_propagateDocumentReplaced(SPDesktop *desktop, SPDocument *document)
{
    _panel.signalDocumentReplaced().emit(desktop, document);
}

void
PanelDialogBase::_propagateDesktopActivated(Inkscape::Application *application, SPDesktop *desktop)
{
    _document_replaced_connection =
        desktop->connectDocumentReplaced(sigc::mem_fun(*this, &PanelDialogBase::_propagateDocumentReplaced));
    _panel.signalActivateDesktop().emit(application, desktop);
}

void
PanelDialogBase::_propagateDesktopDeactivated(Inkscape::Application *application, SPDesktop *desktop)
{
    _document_replaced_connection.disconnect();
    _panel.signalDeactiveDesktop().emit(application, desktop);
}


template <typename B>
PanelDialog<B>::PanelDialog(Panel &panel, char const *prefs_path, int const verb_num,
                            Glib::ustring const &apply_label) :
    PanelDialogBase(panel, prefs_path, verb_num, apply_label),
    Dialog(&B::create, prefs_path, verb_num, apply_label)
{
    Gtk::VBox *vbox = get_vbox();
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
PanelDialog<B> *
PanelDialog<B>::create()
{
    Panel &panel = P::getInstance();
    return new PanelDialog<B>(panel, panel.getPrefsPath(), panel.getVerb(), panel.getApplyLabel());
}

template <typename B>
void
PanelDialog<B>::present()
{
    _panel.present(); 
}

template <typename B>
void
PanelDialog<B>::_presentDialog()
{
    Dialog::present(); 
}

PanelDialog<Behavior::FloatingBehavior>::PanelDialog(Panel &panel, char const *prefs_path,
                                                     int const verb_num, Glib::ustring const &apply_label) :
    PanelDialogBase(panel, prefs_path, verb_num, apply_label),
    Dialog(&Behavior::FloatingBehavior::create, prefs_path, verb_num, apply_label)
{
    Gtk::VBox *vbox = get_vbox();
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

void
PanelDialog<Behavior::FloatingBehavior>::present()
{ 
    Dialog::present();
    _panel.present();
}

/**
 * Specialize factory method for panel dialogs with floating behavior in order to make them work as
 * singletons, i.e. allow them track the current active desktop.
 */
template <typename P>
PanelDialog<Behavior::FloatingBehavior> *
PanelDialog<Behavior::FloatingBehavior>::create()
{
    Panel &panel = P::getInstance();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
